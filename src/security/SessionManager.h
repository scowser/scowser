#pragma once

#include <QObject>
#include <QWebEngineProfile>
#include <memory>

class SessionManager : public QObject {
    Q_OBJECT

public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager() override;

    // Get the ephemeral (off-the-record) profile
    QWebEngineProfile *ephemeralProfile() const { return m_ephemeralProfile; }

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

signals:
    void sessionCleared();
    void ephemeralModeChanged(bool ephemeral);

private:
    void createEphemeralProfile();

    QWebEngineProfile *m_ephemeralProfile;
    bool m_isEphemeral = true;
};
