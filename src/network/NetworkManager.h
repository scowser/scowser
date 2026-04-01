#pragma once

#include <QNetworkAccessManager>
#include <QSslConfiguration>

class CertificatePinner;

class NetworkManager : public QNetworkAccessManager {
    Q_OBJECT

public:
    explicit NetworkManager(CertificatePinner *pinner, QObject *parent = nullptr);

protected:
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request,
                                  QIODevice *outgoingData) override;

private slots:
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    void configureStrictTLS();

    CertificatePinner *m_certPinner;
    QSslConfiguration m_sslConfig;
};
