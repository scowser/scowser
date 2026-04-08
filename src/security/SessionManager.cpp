#include "security/SessionManager.h"

#include <QWebEngineCookieStore>
#include <QWebEngineSettings>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QFile>
#include <QDebug>

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_ephemeralProfile(nullptr)
    , m_persistentProfile(nullptr)
{
    createEphemeralProfile();
    createPersistentProfile();
    loadSavedSessions();
}

SessionManager::~SessionManager()
{
    clearAllData();
}

void SessionManager::createEphemeralProfile()
{
    // Off-the-record profile — no data written to disk
    m_ephemeralProfile = new QWebEngineProfile(this);

    // Disable persistent cookies and cache. Do NOT clear
    // setPersistentStoragePath — Chromium needs a valid data directory.
    m_ephemeralProfile->setHttpCacheType(QWebEngineProfile::NoCache);
    m_ephemeralProfile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    m_ephemeralProfile->setHttpCacheMaximumSize(0);

    // Disable spell checking (can phone home)
    m_ephemeralProfile->setSpellCheckEnabled(false);

    // Configure ephemeral settings
    auto *settings = m_ephemeralProfile->settings();
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);

    // Set a generic user agent to reduce fingerprinting
    QString genericUA = QString(
        "Mozilla/5.0 (X11; Linux x86_64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/120.0.0.0 Safari/537.36");
    m_ephemeralProfile->setHttpUserAgent(genericUA);

    qDebug() << "SessionManager: Ephemeral profile created";
}

void SessionManager::createPersistentProfile()
{
    // Named profile with disk persistence for saved-session tabs
    m_persistentProfile = new QWebEngineProfile(QStringLiteral("scowser-persistent"), this);

    m_persistentProfile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);
    m_persistentProfile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

    // Disable spell checking (can phone home)
    m_persistentProfile->setSpellCheckEnabled(false);

    // Enable local storage for persistent tabs
    auto *settings = m_persistentProfile->settings();
    settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);

    // Same generic user agent
    QString genericUA = QString(
        "Mozilla/5.0 (X11; Linux x86_64) "
        "AppleWebKit/537.36 (KHTML, like Gecko) "
        "Chrome/120.0.0.0 Safari/537.36");
    m_persistentProfile->setHttpUserAgent(genericUA);

    qDebug() << "SessionManager: Persistent profile created";
}

void SessionManager::clearAllData()
{
    clearCookies();
    clearCache();
    clearLocalStorage();
    clearHistory();

    emit sessionCleared();
    qDebug() << "SessionManager: All session data cleared";
}

void SessionManager::clearCookies()
{
    if (m_ephemeralProfile) {
        m_ephemeralProfile->cookieStore()->deleteAllCookies();
    }
}

void SessionManager::clearCache()
{
    if (m_ephemeralProfile) {
        m_ephemeralProfile->clearHttpCache();
    }
}

void SessionManager::clearLocalStorage()
{
    if (m_ephemeralProfile) {
        m_ephemeralProfile->clearAllVisitedLinks();
    }
}

void SessionManager::clearHistory()
{
    // With ephemeral profile, history is not persisted
    // This is a no-op but kept for API completeness
}

void SessionManager::setEphemeral(bool ephemeral)
{
    if (m_isEphemeral == ephemeral) return;

    m_isEphemeral = ephemeral;

    if (ephemeral) {
        clearAllData();
    }

    emit ephemeralModeChanged(ephemeral);
}

// --- Saved session management ---

void SessionManager::saveTab(const QString &url, const QString &title)
{
    if (isTabSaved(url)) return;

    m_savedTabs.append({url, title});
    writeSavedSessions();
    emit savedTabsChanged();
    qDebug() << "SessionManager: Saved tab" << url;
}

void SessionManager::unsaveTab(const QString &url)
{
    for (int i = 0; i < m_savedTabs.size(); ++i) {
        if (m_savedTabs[i].url == url) {
            m_savedTabs.removeAt(i);
            writeSavedSessions();
            emit savedTabsChanged();
            qDebug() << "SessionManager: Unsaved tab" << url;
            return;
        }
    }
}

bool SessionManager::isTabSaved(const QString &url) const
{
    for (const auto &tab : m_savedTabs) {
        if (tab.url == url)
            return true;
    }
    return false;
}

QString SessionManager::sessionsFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return configDir + "/sessions.json";
}

void SessionManager::loadSavedSessions()
{
    QString path = sessionsFilePath();
    QFile file(path);
    if (!file.exists()) return;

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "SessionManager: Could not open sessions file:" << path;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) return;

    m_savedTabs.clear();
    QJsonArray arr = doc.array();
    for (const auto &val : arr) {
        QJsonObject obj = val.toObject();
        QString url = obj.value("url").toString();
        QString title = obj.value("title").toString();
        if (!url.isEmpty()) {
            m_savedTabs.append({url, title});
        }
    }

    qDebug() << "SessionManager: Loaded" << m_savedTabs.size() << "saved tabs";
}

void SessionManager::writeSavedSessions() const
{
    QString path = sessionsFilePath();

    // Ensure config directory exists
    QDir dir = QFileInfo(path).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QJsonArray arr;
    for (const auto &tab : m_savedTabs) {
        QJsonObject obj;
        obj["url"] = tab.url;
        obj["title"] = tab.title;
        arr.append(obj);
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "SessionManager: Could not write sessions file:" << path;
        return;
    }

    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    file.close();
}
