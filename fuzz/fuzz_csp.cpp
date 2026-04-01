// Fuzz harness for CSP enforcement
// Tests that arbitrary CSP headers and URLs are handled safely

#include <cstdint>
#include <cstring>
#include <QCoreApplication>
#include <QUrl>
#include "security/CSPEnforcer.h"

static int s_argc = 1;
static char s_arg0[] = "fuzz_csp";
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

    QString input = QString::fromUtf8(reinterpret_cast<const char *>(data), size);

    CSPEnforcer enforcer;
    QUrl pageUrl("https://example.com");

    // Fuzz 1: Parse and strengthen arbitrary CSP headers
    QString result = enforcer.enforcePolicy(pageUrl, input);

    // Fuzz 2: Use fuzzed input as a custom default policy
    enforcer.setDefaultPolicy(input);
    enforcer.enforcePolicy(pageUrl, "");

    // Fuzz 3: Check resource allowance with fuzzed URLs and directives
    QUrl resourceUrl = QUrl::fromUserInput(input);
    if (resourceUrl.isValid()) {
        enforcer.allowsResource("script-src", resourceUrl, pageUrl);
        enforcer.allowsResource("default-src", resourceUrl, pageUrl);
        enforcer.allowsResource("style-src", resourceUrl, pageUrl);
        enforcer.allowsResource("img-src", resourceUrl, pageUrl);
    }

    // Fuzz 4: Use fuzzed input as directive name
    QUrl knownUrl("https://cdn.example.com/lib.js");
    enforcer.allowsResource(input, knownUrl, pageUrl);

    // Fuzz 5: Log violation with fuzzed data
    enforcer.logViolation(pageUrl, input, resourceUrl);

    return 0;
}
