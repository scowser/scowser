#include "security/AdBlocker.h"

#include <QFile>
#include <QTextStream>
#include <QUrl>
#include <QDebug>

// Built-in tracker domains as a fallback when no filter lists are loaded
const QSet<QString> AdBlocker::s_builtinTrackers = {
    // Analytics
    "google-analytics.com",
    "googletagmanager.com",
    "doubleclick.net",
    "googlesyndication.com",
    "googleadservices.com",
    "analytics.google.com",
    "facebook.com/tr",
    "connect.facebook.net",
    "pixel.facebook.com",

    // Advertising
    "ads.twitter.com",
    "ad.doubleclick.net",
    "pagead2.googlesyndication.com",
    "adservice.google.com",

    // Tracking
    "sb.scorecardresearch.com",
    "b.scorecardresearch.com",
    "beacon.krxd.net",
    "cdn.krxd.net",
    "t.co",
    "bat.bing.com",
    "pixel.quantserve.com",
    "tracking.synthasite.net",

    // Telemetry
    "telemetry.mozilla.org",
    "incoming.telemetry.mozilla.org",
    "data.microsoft.com",
    "vortex.data.microsoft.com",
    "settings-win.data.microsoft.com",

    // Chromium telemetry endpoints
    "clients1.google.com",
    "clients2.google.com",
    "clients3.google.com",
    "clients4.google.com",
    "clients5.google.com",
    "update.googleapis.com",
    "safebrowsing.googleapis.com",
    "sb-ssl.google.com",
};

bool FilterRule::matches(const QString &url, const QString &domain) const
{
    switch (type) {
    case Domain:
        return domain == pattern || domain.endsWith("." + pattern);
    case Regex:
        return regex.match(url).hasMatch();
    case Wildcard:
        return url.contains(pattern);
    }
    return false;
}

AdBlocker::AdBlocker(QObject *parent)
    : QObject(parent)
{
    loadDefaultLists();
}

void AdBlocker::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool AdBlocker::shouldBlock(const QUrl &url, const QUrl &firstPartyUrl) const
{
    if (!m_enabled)
        return false;

    QString host = url.host().toLower();
    QString urlString = url.toString();

    // Don't block first-party requests
    if (host == firstPartyUrl.host()) {
        return false;
    }

    // Check domain allowlist (exceptions)
    if (m_allowedDomains.contains(host)) {
        return false;
    }
    for (const auto &allowed : m_allowedDomains) {
        if (host.endsWith("." + allowed)) {
            return false;
        }
    }

    // Check built-in tracker list
    if (s_builtinTrackers.contains(host)) {
        ++m_blockedCount;
        return true;
    }
    for (const auto &tracker : s_builtinTrackers) {
        if (host.endsWith("." + tracker)) {
            ++m_blockedCount;
            return true;
        }
    }

    // Check domain blocklist
    if (m_blockedDomains.contains(host)) {
        ++m_blockedCount;
        return true;
    }
    for (const auto &blocked : m_blockedDomains) {
        if (host.endsWith("." + blocked)) {
            ++m_blockedCount;
            return true;
        }
    }

    // Check filter rules
    for (const auto &rule : m_rules) {
        if (rule.isException && rule.matches(urlString, host)) {
            return false;
        }
    }
    for (const auto &rule : m_rules) {
        if (!rule.isException && rule.matches(urlString, host)) {
            ++m_blockedCount;
            return true;
        }
    }

    return false;
}

void AdBlocker::loadDefaultLists()
{
    // Try to load EasyList and EasyPrivacy from resources
    loadFilterList(":/blocklists/easylist.txt");
    loadFilterList(":/blocklists/easyprivacy.txt");
}

void AdBlocker::loadFilterList(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "AdBlocker: Could not load filter list:" << filePath;
        return;
    }

    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('!') || line.startsWith('[')) {
            continue; // Skip comments and metadata
        }
        parseRule(line);
    }

    qDebug() << "AdBlocker: Loaded" << m_rules.size() << "rules from" << filePath;
}

void AdBlocker::parseRule(const QString &line)
{
    bool isException = line.startsWith("@@");
    QString rule = isException ? line.mid(2) : line;

    // Domain-based rules: ||domain.com^
    if (rule.startsWith("||") && rule.endsWith("^")) {
        QString domain = rule.mid(2, rule.length() - 3);
        addDomainRule(domain, isException);
        return;
    }

    // Domain-based rules without trailing ^: ||domain.com
    if (rule.startsWith("||")) {
        QString domain = rule.mid(2);
        if (!domain.contains('/') && !domain.contains('*')) {
            addDomainRule(domain, isException);
            return;
        }
    }

    // Wildcard/pattern rules
    if (rule.contains('*') || rule.contains('^')) {
        FilterRule filterRule;
        filterRule.isException = isException;
        filterRule.type = FilterRule::Regex;

        // Convert AdBlock pattern to regex
        QString regexPattern = QRegularExpression::escape(rule);
        regexPattern.replace("\\*", ".*");
        regexPattern.replace("\\^", "[^\\w\\d\\-.%]");

        filterRule.regex = QRegularExpression(regexPattern);
        filterRule.pattern = rule;

        if (filterRule.regex.isValid()) {
            m_rules.append(filterRule);
        }
        return;
    }

    // Simple substring match
    FilterRule filterRule;
    filterRule.isException = isException;
    filterRule.type = FilterRule::Wildcard;
    filterRule.pattern = rule;
    m_rules.append(filterRule);
}

void AdBlocker::addDomainRule(const QString &domain, bool isException)
{
    if (isException) {
        m_allowedDomains.insert(domain.toLower());
    } else {
        m_blockedDomains.insert(domain.toLower());
    }
}

void AdBlocker::addCustomRule(const QString &rule)
{
    parseRule(rule);
}
