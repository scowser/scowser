// AFL++ fuzz harness for CSP enforcement
// Reads input from stdin, tests that arbitrary CSP headers are handled safely

#include <QCoreApplication>
#include <QUrl>
#include <QFile>
#include "security/CSPEnforcer.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QFile input;
    input.open(stdin, QIODevice::ReadOnly);
    QByteArray data = input.readAll();

    if (data.isEmpty() || data.size() > 8192) return 0;

    QString text = QString::fromUtf8(data);

    CSPEnforcer enforcer;
    QUrl pageUrl("https://example.com");

    // Fuzz 1: Parse and strengthen arbitrary CSP headers
    enforcer.enforcePolicy(pageUrl, text);

    // Fuzz 2: Use fuzzed input as a custom default policy
    enforcer.setDefaultPolicy(text);
    enforcer.enforcePolicy(pageUrl, "");

    // Fuzz 3: Check resource allowance with fuzzed URLs and directives
    QUrl resourceUrl = QUrl::fromUserInput(text);
    if (resourceUrl.isValid()) {
        enforcer.allowsResource("script-src", resourceUrl, pageUrl);
        enforcer.allowsResource("default-src", resourceUrl, pageUrl);
        enforcer.allowsResource("style-src", resourceUrl, pageUrl);
        enforcer.allowsResource("img-src", resourceUrl, pageUrl);
    }

    // Fuzz 4: Use fuzzed input as directive name
    QUrl knownUrl("https://cdn.example.com/lib.js");
    enforcer.allowsResource(text, knownUrl, pageUrl);

    // Fuzz 5: Log violation with fuzzed data
    enforcer.logViolation(pageUrl, text, resourceUrl);

    return 0;
}
