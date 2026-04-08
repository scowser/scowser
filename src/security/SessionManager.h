#pragma once

#include <QObject>
#include <QWebEngineProfile>
#include <QJsonArray>
#include <QVector>
#include <memory>

struct SavedTab {
    QString url;
    QString title;
};

class SessionManager : public QObject {
    Q_OBJECT

public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager() override;

    // Get the ephemeral (off-the-record) profile
    QWebEngineProfile *ephemeralProfile() const { return m_ephemeralProfile; }

    // Get the persistent profile for saved-session tabs
    QWebEngineProfile *persistentProfile() const { return m_persistentProfile; }

    // Wipe all session data immediately
    void clearAllData();

    // Clear specific data types
    void clearCookies();
    void clearCache();
    void clearLocalStorage();
    void clearHistory();

    // Session state
    bool isEphemeral() const { return m_isEphemeral; }
    void setEphemeral(bool ephemeral);

    // Saved session management
    void saveTab(const QString &url, const QString &title);
    void unsaveTab(const QString &url);
    bool isTabSaved(const QString &url) const;
    QVector<SavedTab> savedTabs() const { return m_savedTabs; }

    // Persistence
    void loadSavedSessions();
    void writeSavedSessions() const;
    QString sessionsFilePath() const;

signals:
    void sessionCleared();
    void ephemeralModeChanged(bool ephemeral);
    void savedTabsChanged();

private:
    void createEphemeralProfile();
    void createPersistentProfile();

    QWebEngineProfile *m_ephemeralProfile;
    QWebEngineProfile *m_persistentProfile;
    bool m_isEphemeral = true;
    QVector<SavedTab> m_savedTabs;
};
