// Fuzz harness for AdBlocker rule parsing and matching
// Tests that malformed filter rules and URLs don't crash the ad blocker

#include <cstdint>
#include <cstring>
#include <QCoreApplication>
#include <QUrl>
#include "security/AdBlocker.h"

static int s_argc = 1;
static char s_arg0[] = "fuzz_adblocker";
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

    // Fuzz 1: Parse arbitrary filter rules
    AdBlocker blocker;
    for (const auto &line : input.split('\n')) {
        blocker.addCustomRule(line);
    }

    // Fuzz 2: Match arbitrary URLs against the rules
    QUrl testUrl = QUrl::fromUserInput(input);
    QUrl pageUrl("https://example.com");
    blocker.shouldBlock(testUrl, pageUrl);

    // Fuzz 3: Use the fuzzed input as the first-party URL too
    QUrl trackerUrl("https://google-analytics.com/collect");
    blocker.shouldBlock(trackerUrl, testUrl);

    return 0;
}
