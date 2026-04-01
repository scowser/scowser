#include <QSslCertificateExtension>
#include "security/CertificatePinner.h"

#include <QCryptographicHash>
#include <QSslKey>
#include <QDebug>

CertificatePinner::CertificatePinner(QObject *parent)
    : QObject(parent)
{
}

bool CertificatePinner::validateChain(const QString &hostname,
                                       const QList<QSslCertificate> &chain) const
{
    if (chain.isEmpty()) {
        emit const_cast<CertificatePinner *>(this)->pinningFailed(
            hostname, "Empty certificate chain");
        return false;
    }

    // Check key strength for all certs in chain
    for (const auto &cert : chain) {
        if (!hasStrongKey(cert)) {
            emit const_cast<CertificatePinner *>(this)->pinningFailed(
                hostname, "Weak key in certificate chain");
            return false;
        }

        // Reject expired certificates
        if (cert.expiryDate() < QDateTime::currentDateTime()) {
            emit const_cast<CertificatePinner *>(this)->pinningFailed(
                hostname, "Expired certificate in chain");
            return false;
        }
    }

    // If we have pins for this host, validate against them
    QString lookupHost = hostname.toLower();
    QSet<QByteArray> pins;

    // Check exact match
    if (m_pins.contains(lookupHost)) {
        pins = m_pins[lookupHost];
    }

    // Check wildcard pins (e.g., pins for "example.com" match "sub.example.com")
    if (pins.isEmpty()) {
        for (auto it = m_pins.constBegin(); it != m_pins.constEnd(); ++it) {
            if (matchesDomain(it.key(), lookupHost)) {
                pins = it.value();
                break;
            }
        }
    }

    // If no pins configured for this host, accept (but log)
    if (pins.isEmpty()) {
        emit const_cast<CertificatePinner *>(this)->certificateAccepted(hostname);
        return true;
    }

    // Check if any certificate in the chain matches a pin
    for (const auto &cert : chain) {
        QByteArray hash = computeSPKIHash(cert);
        if (pins.contains(hash)) {
            emit const_cast<CertificatePinner *>(this)->certificateAccepted(hostname);
            return true;
        }
    }

    emit const_cast<CertificatePinner *>(this)->pinningFailed(
        hostname, "No certificate in chain matched any pinned key");
    return false;
}

void CertificatePinner::pinCertificate(const QString &hostname,
                                        const QByteArray &publicKeyHash)
{
    m_pins[hostname.toLower()].insert(publicKeyHash);
}

void CertificatePinner::pinCertificateFromPem(const QString &hostname,
                                               const QByteArray &pem)
{
    QSslCertificate cert(pem, QSsl::Pem);
    if (cert.isNull()) {
        qWarning() << "CertificatePinner: Invalid PEM certificate for" << hostname;
        return;
    }

    QByteArray hash = computeSPKIHash(cert);
    pinCertificate(hostname, hash);
}

bool CertificatePinner::hasPins(const QString &hostname) const
{
    return m_pins.contains(hostname.toLower());
}

void CertificatePinner::clearPins()
{
    m_pins.clear();
}

bool CertificatePinner::checkCertificateTransparency(const QSslCertificate &cert) const
{
    // Check for Certificate Transparency SCT extensions
    // OID 1.3.6.1.4.1.11129.2.4.2 is the SCT list extension
    auto extensions = cert.extensions();
    for (const auto &ext : extensions) {
        if (ext.oid() == "1.3.6.1.4.1.11129.2.4.2") {
            return true;
        }
    }
    return false;
}

bool CertificatePinner::hasStrongKey(const QSslCertificate &cert)
{
    QSslKey key = cert.publicKey();
    if (key.isNull()) return false;

    int keySize = key.length();

    switch (key.algorithm()) {
    case QSsl::Rsa:
        return keySize >= 2048;
    case QSsl::Ec:
        return keySize >= 256;
    case QSsl::Dsa:
        return false; // DSA is deprecated
    default:
        return keySize >= 2048;
    }
}

QByteArray CertificatePinner::computeSPKIHash(const QSslCertificate &cert) const
{
    // Compute SHA-256 hash of the Subject Public Key Info (SPKI)
    QByteArray der = cert.publicKey().toDer();
    QByteArray hash = QCryptographicHash::hash(der, QCryptographicHash::Sha256);
    return hash.toBase64();
}

bool CertificatePinner::matchesDomain(const QString &pattern,
                                       const QString &hostname) const
{
    if (pattern == hostname) return true;

    // Check if hostname is a subdomain of pattern
    if (hostname.endsWith("." + pattern)) return true;

    return false;
}
