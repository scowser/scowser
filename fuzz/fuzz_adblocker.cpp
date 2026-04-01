// AFL++ fuzz harness for AdBlocker rule parsing and matching
// Reads input from stdin, tests that arbitrary data doesn't crash the ad blocker

#include <QCoreApplication>
#include <QUrl>
#include <QFile>
#include "security/AdBlocker.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QFile input;
    input.open(stdin, QIODevice::ReadOnly);
    QByteArray data = input.readAll();

    if (data.isEmpty() || data.size() > 8192) return 0;

    QString text = QString::fromUtf8(data);

    // Fuzz 1: Parse arbitrary filter rules
    AdBlocker blocker;
    for (const auto &line : text.split('\n')) {
        blocker.addCustomRule(line);
    }

    // Fuzz 2: Match arbitrary URLs against the rules
    QUrl testUrl = QUrl::fromUserInput(text);
    QUrl pageUrl("https://example.com");
    blocker.shouldBlock(testUrl, pageUrl);

    // Fuzz 3: Use the fuzzed input as the first-party URL too
    QUrl trackerUrl("https://google-analytics.com/collect");
    blocker.shouldBlock(trackerUrl, testUrl);

    return 0;
}
