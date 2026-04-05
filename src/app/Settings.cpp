#include "app/Settings.h"

static const char *KeyDownloadDirectory = "downloads/directory";
static const char *KeyDnsProvider = "dns/provider";
static const char *KeyDnsCustomUrl = "dns/customUrl";
static const char *KeySearchEngineUrl = "search/engineUrl";
static const char *KeyEphemeralSessions = "privacy/ephemeralSessions";
static const char *KeyDoNotTrack = "privacy/doNotTrack";
static const char *KeyAdBlocking = "security/adBlocking";
static const char *KeyJavaScript = "security/javaScript";

static const char *DefaultSearchEngineUrl = "https://duckduckgo.com/?q=%1";

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings(QSettings::IniFormat, QSettings::UserScope, "scowser", "scowser")
{
}

Settings::Settings(const QString &filePath, QObject *parent)
    : QObject(parent)
    , m_settings(filePath, QSettings::IniFormat)
{
}

// --- Downloads ---

QString Settings::downloadDirectory() const
{
    return m_settings.value(KeyDownloadDirectory,
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)).toString();
}

void Settings::setDownloadDirectory(const QString &path)
{
    if (downloadDirectory() == path)
        return;
    m_settings.setValue(KeyDownloadDirectory, path);
    emit downloadDirectoryChanged(path);
}

// --- DNS ---

DnsOverHttps::Provider Settings::dnsProvider() const
{
    int val = m_settings.value(KeyDnsProvider, DnsOverHttps::Cloudflare).toInt();
    if (val < DnsOverHttps::Cloudflare || val > DnsOverHttps::Custom)
        return DnsOverHttps::Cloudflare;
    return static_cast<DnsOverHttps::Provider>(val);
}

void Settings::setDnsProvider(DnsOverHttps::Provider provider)
{
    if (dnsProvider() == provider)
        return;
    m_settings.setValue(KeyDnsProvider, static_cast<int>(provider));
    emit dnsProviderChanged(provider);
}

QString Settings::customDnsUrl() const
{
    return m_settings.value(KeyDnsCustomUrl, QString()).toString();
}

void Settings::setCustomDnsUrl(const QString &url)
{
    if (customDnsUrl() == url)
        return;
    m_settings.setValue(KeyDnsCustomUrl, url);
    emit customDnsUrlChanged(url);
}

// --- Search ---

QString Settings::searchEngineUrl() const
{
    return m_settings.value(KeySearchEngineUrl, DefaultSearchEngineUrl).toString();
}

void Settings::setSearchEngineUrl(const QString &url)
{
    if (searchEngineUrl() == url)
        return;
    m_settings.setValue(KeySearchEngineUrl, url);
    emit searchEngineUrlChanged(url);
}

// --- Privacy ---

bool Settings::ephemeralSessions() const
{
    return m_settings.value(KeyEphemeralSessions, true).toBool();
}

void Settings::setEphemeralSessions(bool enabled)
{
    if (ephemeralSessions() == enabled)
        return;
    m_settings.setValue(KeyEphemeralSessions, enabled);
    emit ephemeralSessionsChanged(enabled);
}

bool Settings::doNotTrack() const
{
    return m_settings.value(KeyDoNotTrack, true).toBool();
}

void Settings::setDoNotTrack(bool enabled)
{
    if (doNotTrack() == enabled)
        return;
    m_settings.setValue(KeyDoNotTrack, enabled);
    emit doNotTrackChanged(enabled);
}

// --- Security ---

bool Settings::adBlockingEnabled() const
{
    return m_settings.value(KeyAdBlocking, true).toBool();
}

void Settings::setAdBlockingEnabled(bool enabled)
{
    if (adBlockingEnabled() == enabled)
        return;
    m_settings.setValue(KeyAdBlocking, enabled);
    emit adBlockingEnabledChanged(enabled);
}

bool Settings::javaScriptEnabled() const
{
    return m_settings.value(KeyJavaScript, true).toBool();
}

void Settings::setJavaScriptEnabled(bool enabled)
{
    if (javaScriptEnabled() == enabled)
        return;
    m_settings.setValue(KeyJavaScript, enabled);
    emit javaScriptEnabledChanged(enabled);
}
