// AFL++ fuzz harness for DNS-over-HTTPS response parsing
// Reads input from stdin, tests that malformed DNS JSON doesn't crash the parser

#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostAddress>
#include "security/DnsOverHttps.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QFile input;
    input.open(stdin, QIODevice::ReadOnly);
    QByteArray data = input.readAll();

    if (data.isEmpty() || data.size() > 8192) return 0;

    // Fuzz 1: Parse arbitrary data as DNS JSON response
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error == QJsonParseError::NoError) {
        QJsonObject root = doc.object();
        root["Status"].toInt();

        QJsonArray answers = root["Answer"].toArray();
        for (const auto &answer : answers) {
            QJsonObject record = answer.toObject();
            int type = record["type"].toInt();
            if (type == 1 || type == 28) {
                QHostAddress addr(record["data"].toString());
                addr.isNull();
                record["TTL"].toInt();
            }
        }
    }

    // Fuzz 2: Use fuzzed input as hostname for cache lookup
    QString hostname = QString::fromUtf8(data.left(256));
    DnsOverHttps resolver;
    resolver.cachedLookup(hostname);

    return 0;
}
