#pragma once

#include <QObject>
#include <QSslCertificate>
#include <QMap>
#include <QSet>
#include <QByteArray>

class CertificatePinner : public QObject {
    Q_OBJECT

public:
    explicit CertificatePinner(QObject *parent = nullptr);

    bool validateChain(const QString &hostname,
                       const QList<QSslCertificate> &chain) const;

    void pinCertificate(const QString &hostname,
                        const QByteArray &publicKeyHash);

    void pinCertificateFromPem(const QString &hostname,
                               const QByteArray &pem);

    bool hasPins(const QString &hostname) const;
    void clearPins();

    // Certificate transparency check
    bool checkCertificateTransparency(const QSslCertificate &cert) const;

    // Check if certificate uses acceptable key strength
    static bool hasStrongKey(const QSslCertificate &cert);

signals:
    void pinningFailed(const QString &hostname, const QString &reason);
    void certificateAccepted(const QString &hostname);

private:
    QByteArray computeSPKIHash(const QSslCertificate &cert) const;
    bool matchesDomain(const QString &pattern, const QString &hostname) const;

    // hostname -> set of acceptable SPKI SHA-256 hashes (base64)
    QMap<QString, QSet<QByteArray>> m_pins;
};
