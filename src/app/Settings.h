#pragma once

#include <QObject>
#include <QSettings>
#include "security/DnsOverHttps.h"

class Settings : public QObject {
    Q_OBJECT

public:
    explicit Settings(QObject *parent = nullptr);
    explicit Settings(const QString &filePath, QObject *parent = nullptr);

    // DNS
    DnsOverHttps::Provider dnsProvider() const;
    void setDnsProvider(DnsOverHttps::Provider provider);
    QString customDnsUrl() const;
    void setCustomDnsUrl(const QString &url);

    // Search
    QString searchEngineUrl() const;
    void setSearchEngineUrl(const QString &url);

    // Privacy
    bool ephemeralSessions() const;
    void setEphemeralSessions(bool enabled);
    bool doNotTrack() const;
    void setDoNotTrack(bool enabled);

    // Security
    bool adBlockingEnabled() const;
    void setAdBlockingEnabled(bool enabled);
    bool javaScriptEnabled() const;
    void setJavaScriptEnabled(bool enabled);

signals:
    void dnsProviderChanged(DnsOverHttps::Provider provider);
    void customDnsUrlChanged(const QString &url);
    void searchEngineUrlChanged(const QString &url);
    void ephemeralSessionsChanged(bool enabled);
    void doNotTrackChanged(bool enabled);
    void adBlockingEnabledChanged(bool enabled);
    void javaScriptEnabledChanged(bool enabled);

private:
    QSettings m_settings;
};
