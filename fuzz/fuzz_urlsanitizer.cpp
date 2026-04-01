// AFL++ fuzz harness for AddressBar URL sanitization
// Reads input from stdin, tests that arbitrary user input is safely handled

#include <QCoreApplication>
#include <QUrl>
#include <QFile>
#include <QRegularExpression>

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

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QFile input;
    input.open(stdin, QIODevice::ReadOnly);
    QByteArray data = input.readAll();

    if (data.isEmpty() || data.size() > 4096) return 0;

    QString text = QString::fromUtf8(data);

    // Fuzz URL sanitization with arbitrary input
    QUrl result = sanitizeInput(text);

    if (result.isValid()) {
        result.toString();
        result.host();
        result.scheme();
        result.path();
    }

    // Also fuzz QUrl::fromUserInput directly
    QUrl directUrl = QUrl::fromUserInput(text);
    if (directUrl.isValid()) {
        directUrl.toDisplayString();
    }

    return 0;
}
