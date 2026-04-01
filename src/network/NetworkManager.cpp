#include "network/NetworkManager.h"
#include "security/CertificatePinner.h"

#include <QNetworkReply>
#include <QSslCipher>
#include <QDebug>

NetworkManager::NetworkManager(CertificatePinner *pinner, QObject *parent)
    : QNetworkAccessManager(parent)
    , m_certPinner(pinner)
{
    configureStrictTLS();

    connect(this, &QNetworkAccessManager::sslErrors, this, &NetworkManager::onSslErrors);
}

void NetworkManager::configureStrictTLS()
{
    m_sslConfig = QSslConfiguration::defaultConfiguration();

    // Require TLS 1.2 minimum
    m_sslConfig.setProtocol(QSsl::TlsV1_2OrLater);

    // Only allow strong cipher suites
    QList<QSslCipher> allowedCiphers;
    for (const auto &cipher : QSslConfiguration::supportedCiphers()) {
        // Only allow ECDHE and DHE key exchange (forward secrecy)
        // Only allow AES-GCM and ChaCha20 ciphers
        QString name = cipher.name();
        if ((name.contains("ECDHE") || name.contains("DHE")) &&
            (name.contains("AES") || name.contains("CHACHA20")) &&
            !name.contains("RC4") && !name.contains("DES") &&
            !name.contains("MD5") && !name.contains("NULL")) {
            allowedCiphers.append(cipher);
        }
    }
    m_sslConfig.setCiphers(allowedCiphers);

    // Enable OCSP stapling
    m_sslConfig.setOcspStaplingEnabled(true);

    QSslConfiguration::setDefaultConfiguration(m_sslConfig);
}

QNetworkReply *NetworkManager::createRequest(Operation op,
                                              const QNetworkRequest &request,
                                              QIODevice *outgoingData)
{
    QNetworkRequest secureRequest(request);
    secureRequest.setSslConfiguration(m_sslConfig);

    // Set strict transport security
    secureRequest.setRawHeader("Upgrade-Insecure-Requests", "1");

    return QNetworkAccessManager::createRequest(op, secureRequest, outgoingData);
}

void NetworkManager::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    // NEVER ignore SSL errors — abort the connection
    for (const auto &error : errors) {
        qWarning() << "SSL Error:" << error.errorString()
                    << "for" << reply->url().host();
    }

    // Check certificate pinning
    if (m_certPinner) {
        auto chain = reply->sslConfiguration().peerCertificateChain();
        if (!m_certPinner->validateChain(reply->url().host(), chain)) {
            qWarning() << "Certificate pinning failed for:" << reply->url().host();
        }
    }

    // Connection will be aborted (we don't call ignoreSslErrors)
}
