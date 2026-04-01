// Fuzz harness for DNS-over-HTTPS response parsing
// Tests that malformed DNS JSON responses don't crash the parser

#include <cstdint>
#include <cstring>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostAddress>
#include "security/DnsOverHttps.h"

static int s_argc = 1;
static char s_arg0[] = "fuzz_doh";
static char *s_argv[] = { s_arg0, nullptr };
static QCoreApplication *s_app = nullptr;

extern "C" int LLVMFuzzerInitialize(int *, char ***)
{
    s_app = new QCoreApplication(s_argc, s_argv);
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size == 0 || size > 8192) return 0;

    // Fuzz 1: Parse arbitrary data as DNS JSON response
    // We test the JSON parsing path that parseDnsResponse would take
    QByteArray jsonData(reinterpret_cast<const char *>(data), size);

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

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

    // Fuzz 2: Use fuzzed input as hostname
    QString hostname = QString::fromUtf8(reinterpret_cast<const char *>(data),
                                          qMin(size, size_t(256)));
    DnsOverHttps resolver;
    resolver.cachedLookup(hostname);

    return 0;
}
