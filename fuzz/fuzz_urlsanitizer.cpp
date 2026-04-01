// Fuzz harness for AddressBar URL sanitization
// Tests that arbitrary user input is safely handled without crashes

#include <cstdint>
#include <cstring>
#include <QCoreApplication>
#include <QUrl>
#include <QRegularExpression>

static int s_argc = 1;
static char s_arg0[] = "fuzz_urlsanitizer";
static char *s_argv[] = { s_arg0, nullptr };
static QCoreApplication *s_app = nullptr;

extern "C" int LLVMFuzzerInitialize(int *, char ***)
{
    s_app = new QCoreApplication(s_argc, s_argv);
    return 0;
}

// Replicate AddressBar::sanitizeInput logic for fuzzing without a GUI
static QUrl sanitizeInput(const QString &text)
{
    if (text.isEmpty()) return QUrl();

    static QRegularExpression urlPattern(R"(^(https?://|[a-zA-Z0-9][-a-zA-Z0-9]*\.[a-zA-Z]{2,}))");

    if (text.startsWith("http://") || text.startsWith("https://")) {
        return QUrl::fromUserInput(text);
    }

    if (urlPattern.match(text).hasMatch()) {
        return QUrl::fromUserInput("https://" + text);
    }

    return QUrl("https://duckduckgo.com/?q=" + QUrl::toPercentEncoding(text));
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size == 0 || size > 4096) return 0;

    QString input = QString::fromUtf8(reinterpret_cast<const char *>(data), size);

    // Fuzz URL sanitization with arbitrary input
    QUrl result = sanitizeInput(input);

    // Verify the result is either empty or valid — should never crash
    if (result.isValid()) {
        result.toString();
        result.host();
        result.scheme();
        result.path();
    }

    // Also fuzz QUrl::fromUserInput directly
    QUrl directUrl = QUrl::fromUserInput(input);
    if (directUrl.isValid()) {
        directUrl.toDisplayString();
    }

    return 0;
}
