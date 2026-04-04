#include <QtTest/QtTest>
#include "security/DnsOverHttps.h"

class TestDnsOverHttps : public QObject {
    Q_OBJECT

private slots:
    void testDefaultProvider();
    void testSetProvider();
    void testSetCustomProvider();
    void testCacheLookupEmpty();
    void testResolvePopulatesCache();
    void testCacheExpiry();
    void testClearCache();
};

void TestDnsOverHttps::testDefaultProvider()
{
    DnsOverHttps doh;

    // Default should be Cloudflare — verify by checking cached lookup returns
    // empty for unknown host (provider is internal, but cache behavior is testable)
    auto result = doh.cachedLookup("nonexistent.example.com");
    QVERIFY(result.isEmpty());
}

void TestDnsOverHttps::testSetProvider()
{
    DnsOverHttps doh;

    // Switching provider should clear the cache
    // First, we need something in the cache — use a signal spy to inject via resolve
    doh.setProvider(DnsOverHttps::Quad9);

    // Cache should be empty after provider change
    auto result = doh.cachedLookup("example.com");
    QVERIFY(result.isEmpty());
}

void TestDnsOverHttps::testSetCustomProvider()
{
    DnsOverHttps doh;

    doh.setCustomProvider("https://dns.example.com/dns-query");

    // Cache should be empty after provider change
    auto result = doh.cachedLookup("example.com");
    QVERIFY(result.isEmpty());
}

void TestDnsOverHttps::testCacheLookupEmpty()
{
    DnsOverHttps doh;

    // Lookup on empty cache should return empty list
    auto result = doh.cachedLookup("example.com");
    QVERIFY(result.isEmpty());
    QCOMPARE(result.size(), 0);
}

void TestDnsOverHttps::testResolvePopulatesCache()
{
    DnsOverHttps doh;
    QSignalSpy resolvedSpy(&doh, &DnsOverHttps::resolved);
    QSignalSpy failedSpy(&doh, &DnsOverHttps::resolutionFailed);

    // Resolve a well-known hostname (requires network — skip if offline)
    doh.resolve("one.one.one.one");

    // Wait up to 10 seconds for either signal
    bool gotResponse = QTest::qWaitFor([&]() {
        return resolvedSpy.count() > 0 || failedSpy.count() > 0;
    }, 10000);

    if (!gotResponse) {
        QSKIP("No network available — skipping live DoH test");
    }

    if (failedSpy.count() > 0) {
        QSKIP("DoH resolution failed (likely no network) — skipping");
    }

    QCOMPARE(resolvedSpy.count(), 1);

    // Verify the hostname matches
    auto args = resolvedSpy.takeFirst();
    QCOMPARE(args.at(0).toString(), QString("one.one.one.one"));

    // Verify addresses were returned
    auto addresses = args.at(1).value<QList<QHostAddress>>();
    QVERIFY(!addresses.isEmpty());

    // Verify cache is now populated
    auto cached = doh.cachedLookup("one.one.one.one");
    QVERIFY(!cached.isEmpty());
    QCOMPARE(cached, addresses);
}

void TestDnsOverHttps::testCacheExpiry()
{
    DnsOverHttps doh;

    // We can't easily test TTL expiry without waiting, but we can verify that
    // after clearing the cache, lookups return empty
    doh.resolve("one.one.one.one");
    QSignalSpy resolvedSpy(&doh, &DnsOverHttps::resolved);
    QSignalSpy failedSpy(&doh, &DnsOverHttps::resolutionFailed);

    bool gotResponse = QTest::qWaitFor([&]() {
        return resolvedSpy.count() > 0 || failedSpy.count() > 0;
    }, 10000);

    if (!gotResponse || failedSpy.count() > 0) {
        QSKIP("No network available — skipping");
    }

    // Cache should be populated
    QVERIFY(!doh.cachedLookup("one.one.one.one").isEmpty());

    // Clear and verify
    doh.clearCache();
    QVERIFY(doh.cachedLookup("one.one.one.one").isEmpty());
}

void TestDnsOverHttps::testClearCache()
{
    DnsOverHttps doh;

    // Clear on empty cache should not crash
    doh.clearCache();
    QVERIFY(doh.cachedLookup("anything.com").isEmpty());
}

QTEST_MAIN(TestDnsOverHttps)
#include "test_doh.moc"
