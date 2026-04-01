#include "security/DnsOverHttps.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDebug>

DnsOverHttps::DnsOverHttps(QObject *parent)
    : QObject(parent)
{
}

void DnsOverHttps::setProvider(Provider provider)
{
    m_provider = provider;
    clearCache();
}

void DnsOverHttps::setCustomProvider(const QString &url)
{
    m_provider = Custom;
    m_customProviderUrl = url;
    clearCache();
}

QString DnsOverHttps::providerUrl() const
{
    switch (m_provider) {
    case Cloudflare:
        return "https://cloudflare-dns.com/dns-query";
    case Quad9:
        return "https://dns.quad9.net:5053/dns-query";
    case Custom:
        return m_customProviderUrl;
    }
    return "https://cloudflare-dns.com/dns-query";
}

void DnsOverHttps::resolve(const QString &hostname)
{
    // Check cache first
    if (m_cache.contains(hostname)) {
        const auto &entry = m_cache[hostname];
        if (entry.expiry > QDateTime::currentDateTime()) {
            emit resolved(hostname, entry.addresses);
            return;
        }
        m_cache.remove(hostname);
    }

    // Build DoH request (JSON API)
    QUrl url(providerUrl());
    QUrlQuery query;
    query.addQueryItem("name", hostname);
    query.addQueryItem("type", "A");
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/dns-json");
    // Don't send cookies or auth with DNS requests
    request.setAttribute(QNetworkRequest::CookieLoadControlAttribute,
                          QNetworkRequest::Manual);
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute,
                          QNetworkRequest::Manual);

    auto *reply = m_networkManager.get(request);
    reply->setProperty("hostname", hostname);
    connect(reply, &QNetworkReply::finished, this, &DnsOverHttps::onReplyFinished);
}

QList<QHostAddress> DnsOverHttps::cachedLookup(const QString &hostname) const
{
    if (m_cache.contains(hostname)) {
        const auto &entry = m_cache[hostname];
        if (entry.expiry > QDateTime::currentDateTime()) {
            return entry.addresses;
        }
    }
    return {};
}

void DnsOverHttps::clearCache()
{
    m_cache.clear();
}

void DnsOverHttps::onReplyFinished()
{
    auto *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    reply->deleteLater();
    QString hostname = reply->property("hostname").toString();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "DoH resolution failed for" << hostname << ":" << reply->errorString();
        emit resolutionFailed(hostname, reply->errorString());
        return;
    }

    parseDnsResponse(hostname, reply->readAll());
}

void DnsOverHttps::parseDnsResponse(const QString &hostname, const QByteArray &data)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit resolutionFailed(hostname, "Invalid DNS response JSON");
        return;
    }

    QJsonObject root = doc.object();

    if (root["Status"].toInt() != 0) {
        emit resolutionFailed(hostname, "DNS query returned error status");
        return;
    }

    QJsonArray answers = root["Answer"].toArray();
    QList<QHostAddress> addresses;
    int minTtl = 300; // Default 5 minute TTL

    for (const auto &answer : answers) {
        QJsonObject record = answer.toObject();
        int type = record["type"].toInt();

        // Type 1 = A record, Type 28 = AAAA record
        if (type == 1 || type == 28) {
            QHostAddress addr(record["data"].toString());
            if (!addr.isNull()) {
                addresses.append(addr);
            }
            int ttl = record["TTL"].toInt();
            if (ttl > 0 && ttl < minTtl) {
                minTtl = ttl;
            }
        }
    }

    if (!addresses.isEmpty()) {
        // Cache the result
        DnsEntry entry;
        entry.addresses = addresses;
        entry.expiry = QDateTime::currentDateTime().addSecs(minTtl);
        m_cache[hostname] = entry;

        emit resolved(hostname, addresses);
    } else {
        emit resolutionFailed(hostname, "No addresses in DNS response");
    }
}
