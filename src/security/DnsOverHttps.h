#pragma once

#include <QObject>
#include <QHostAddress>
#include <QNetworkAccessManager>
#include <QMap>
#include <QDateTime>

struct DnsEntry {
    QList<QHostAddress> addresses;
    QDateTime expiry;
};

class DnsOverHttps : public QObject {
    Q_OBJECT

public:
    enum Provider {
        Cloudflare,  // 1.1.1.1
        Quad9,       // 9.9.9.9
        Custom
    };

    explicit DnsOverHttps(QObject *parent = nullptr);

    void setProvider(Provider provider);
    void setCustomProvider(const QString &url);

    void resolve(const QString &hostname);
    QList<QHostAddress> cachedLookup(const QString &hostname) const;
    void clearCache();

signals:
    void resolved(const QString &hostname, const QList<QHostAddress> &addresses);
    void resolutionFailed(const QString &hostname, const QString &error);

private slots:
    void onReplyFinished();

private:
    QString providerUrl() const;
    void parseDnsResponse(const QString &hostname, const QByteArray &data);

    QNetworkAccessManager m_networkManager;
    Provider m_provider = Cloudflare;
    QString m_customProviderUrl;
    QMap<QString, DnsEntry> m_cache;
};
