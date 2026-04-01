#include "security/SessionManager.h"

#include <QWebEngineCookieStore>
#include <QWebEngineSettings>
#include <QDebug>

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_ephemeralProfile(nullptr)
{
    createEphemeralProfile();
}

SessionManager::~SessionManager()
{
    clearAllData();
}

void SessionManager::createEphemeralProfile()
{
    // Off-the-record profile — no data written to disk
    m_ephemeralProfile = new QWebEngineProfile(this);

    // Disable all persistent storage
    m_ephemeralProfile->setHttpCacheType(QWebEngineProfile::NoCache);
    m_ephemeralProfile->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);
    m_ephemeralProfile->setPersistentStoragePath(QString());
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
