#pragma once

#include <QWebEngineUrlRequestInterceptor>

class AdBlocker;
class CSPEnforcer;

class RequestInterceptor : public QWebEngineUrlRequestInterceptor {
    Q_OBJECT

public:
    explicit RequestInterceptor(AdBlocker *adBlocker, QObject *parent = nullptr);

    void interceptRequest(QWebEngineUrlRequestInfo &info) override;

signals:
    void requestBlocked(const QUrl &url);
    void requestAllowed(const QUrl &url);

private:
    bool isChromiumTelemetry(const QUrl &url) const;
    void enforceSecurityHeaders(QWebEngineUrlRequestInfo &info);

    AdBlocker *m_adBlocker;
};
