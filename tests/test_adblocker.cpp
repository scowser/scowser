#include <QtTest/QtTest>
#include "security/AdBlocker.h"

class TestAdBlocker : public QObject {
    Q_OBJECT

private slots:
    void testBlocksKnownTrackers();
    void testAllowsFirstParty();
    void testAllowsNonTrackers();
    void testDomainRules();
    void testCustomRules();
};

void TestAdBlocker::testBlocksKnownTrackers()
{
    AdBlocker blocker;

    QUrl tracker("https://google-analytics.com/collect");
    QUrl page("https://example.com");

    QVERIFY(blocker.shouldBlock(tracker, page));
}

void TestAdBlocker::testAllowsFirstParty()
{
    AdBlocker blocker;

    // First-party requests should never be blocked
    QUrl url("https://example.com/script.js");
    QUrl page("https://example.com");

    QVERIFY(!blocker.shouldBlock(url, page));
}

void TestAdBlocker::testAllowsNonTrackers()
{
    AdBlocker blocker;

    QUrl url("https://cdn.jsdelivr.net/npm/library.js");
    QUrl page("https://example.com");

    QVERIFY(!blocker.shouldBlock(url, page));
}

void TestAdBlocker::testDomainRules()
{
    AdBlocker blocker;

    // Subdomain should also be blocked
    QUrl tracker("https://sub.google-analytics.com/track");
    QUrl page("https://example.com");

    QVERIFY(blocker.shouldBlock(tracker, page));
}

void TestAdBlocker::testCustomRules()
{
    AdBlocker blocker;

    blocker.addCustomRule("||evil-tracker.com^");

    QUrl tracker("https://evil-tracker.com/pixel.gif");
    QUrl page("https://example.com");

    QVERIFY(blocker.shouldBlock(tracker, page));
}

QTEST_MAIN(TestAdBlocker)
#include "test_adblocker.moc"
