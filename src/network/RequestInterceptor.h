#pragma once

#include <QWebEngineUrlRequestInterceptor>
#include <QSet>

class AdBlocker;
class DnsOverHttps;

class RequestInterceptor : public QWebEngineUrlRequestInterceptor {
    Q_OBJECT

public:
    explicit RequestInterceptor(AdBlocker *adBlocker, DnsOverHttps *dnsResolver,
                                QObject *parent = nullptr);

    void interceptRequest(QWebEngineUrlRequestInfo &info) override;

    bool doNotTrack() const { return m_doNotTrack; }
    void setDoNotTrack(bool enabled);

signals:
    void requestBlocked(const QUrl &url);
    void requestAllowed(const QUrl &url);

private:
    bool isChromiumTelemetry(const QUrl &url) const;
    void enforceSecurityHeaders(QWebEngineUrlRequestInfo &info);
    void prefetchDns(const QString &hostname);

    AdBlocker *m_adBlocker;
    DnsOverHttps *m_dnsResolver;
    QSet<QString> m_prefetchedHosts;
    bool m_doNotTrack = true;
};
