#include "network/RequestInterceptor.h"
#include "security/AdBlocker.h"

#include <QUrl>
#include <QDebug>

// Chromium internal endpoints that must always be blocked
static const QStringList s_telemetryHosts = {
    "accounts.google.com",
    "clients1.google.com",
    "clients2.google.com",
    "clients3.google.com",
    "clients4.google.com",
    "clients5.google.com",
    "safebrowsing.googleapis.com",
    "update.googleapis.com",
    "optimizationguide-pa.googleapis.com",
    "content-autofill.googleapis.com",
    "sb-ssl.google.com",
    "ssl.gstatic.com/safebrowsing",
};

RequestInterceptor::RequestInterceptor(AdBlocker *adBlocker, QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
    , m_adBlocker(adBlocker)
{
}

void RequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    QUrl requestUrl = info.requestUrl();
    QUrl firstPartyUrl = info.firstPartyUrl();

    // Always block Chromium telemetry
    if (isChromiumTelemetry(requestUrl)) {
        info.block(true);
        qDebug() << "Blocked telemetry:" << requestUrl.host();
        return;
    }

    // Block non-HTTPS requests (except localhost and about:)
    QString scheme = requestUrl.scheme();
    if (scheme == "http") {
        QString host = requestUrl.host();
        if (host != "localhost" && host != "127.0.0.1" && host != "::1") {
            // Upgrade to HTTPS
            QUrl httpsUrl = requestUrl;
            httpsUrl.setScheme("https");
            info.redirect(httpsUrl);
            qDebug() << "Upgraded to HTTPS:" << requestUrl.host();
            return;
        }
    }

    // Ad/tracker blocking
    if (m_adBlocker && m_adBlocker->shouldBlock(requestUrl, firstPartyUrl)) {
        info.block(true);
        emit requestBlocked(requestUrl);
        qDebug() << "Blocked:" << requestUrl.host();
        return;
    }

    // Apply security headers
    enforceSecurityHeaders(info);

    emit requestAllowed(requestUrl);
}

bool RequestInterceptor::isChromiumTelemetry(const QUrl &url) const
{
    QString host = url.host().toLower();
    for (const auto &telemetryHost : s_telemetryHosts) {
        if (host == telemetryHost || host.endsWith("." + telemetryHost)) {
            return true;
        }
    }
    return false;
}

void RequestInterceptor::enforceSecurityHeaders(QWebEngineUrlRequestInfo &info)
{
    // Set Do-Not-Track header
    info.setHttpHeader("DNT", "1");

    // Set Sec-GPC (Global Privacy Control)
    info.setHttpHeader("Sec-GPC", "1");

    // Remove referrer for cross-origin requests
    if (info.requestUrl().host() != info.firstPartyUrl().host()) {
        info.setHttpHeader("Referer", "");
    }
}
