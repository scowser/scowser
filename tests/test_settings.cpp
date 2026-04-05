#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTemporaryFile>
#include "app/Settings.h"

class TestSettings : public QObject {
    Q_OBJECT

private slots:
    void testDefaults();
    void testDnsProvider();
    void testCustomDnsUrl();
    void testSearchEngineUrl();
    void testEphemeralSessions();
    void testDoNotTrack();
    void testAdBlocking();
    void testJavaScript();
    void testPersistence();
    void testSignalsOnlyOnChange();
    void testDnsProviderBoundsCheck();
};

static QString tempSettingsPath()
{
    // QTemporaryFile gives us a unique path; we close it so QSettings can own it
    QTemporaryFile tmp;
    tmp.setAutoRemove(false);
    tmp.open();
    QString path = tmp.fileName();
    tmp.close();
    return path;
}

void TestSettings::testDefaults()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    QCOMPARE(settings.dnsProvider(), DnsOverHttps::Cloudflare);
    QCOMPARE(settings.customDnsUrl(), QString());
    QCOMPARE(settings.searchEngineUrl(), QString("https://duckduckgo.com/?q=%1"));
    QCOMPARE(settings.ephemeralSessions(), true);
    QCOMPARE(settings.doNotTrack(), true);
    QCOMPARE(settings.adBlockingEnabled(), true);
    QCOMPARE(settings.javaScriptEnabled(), true);

    QFile::remove(path);
}

void TestSettings::testDnsProvider()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    QSignalSpy spy(&settings, &Settings::dnsProviderChanged);

    settings.setDnsProvider(DnsOverHttps::Quad9);
    QCOMPARE(settings.dnsProvider(), DnsOverHttps::Quad9);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).value<DnsOverHttps::Provider>(), DnsOverHttps::Quad9);

    settings.setDnsProvider(DnsOverHttps::Custom);
    QCOMPARE(settings.dnsProvider(), DnsOverHttps::Custom);
    QCOMPARE(spy.count(), 1);

    QFile::remove(path);
}

void TestSettings::testCustomDnsUrl()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    QSignalSpy spy(&settings, &Settings::customDnsUrlChanged);

    settings.setCustomDnsUrl("https://dns.example.com/dns-query");
    QCOMPARE(settings.customDnsUrl(), QString("https://dns.example.com/dns-query"));
    QCOMPARE(spy.count(), 1);

    QFile::remove(path);
}

void TestSettings::testSearchEngineUrl()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    QSignalSpy spy(&settings, &Settings::searchEngineUrlChanged);

    QString newUrl = "https://search.example.com/?q=%1";
    settings.setSearchEngineUrl(newUrl);
    QCOMPARE(settings.searchEngineUrl(), newUrl);
    QCOMPARE(spy.count(), 1);

    QFile::remove(path);
}

void TestSettings::testEphemeralSessions()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    QSignalSpy spy(&settings, &Settings::ephemeralSessionsChanged);

    settings.setEphemeralSessions(false);
    QCOMPARE(settings.ephemeralSessions(), false);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toBool(), false);

    QFile::remove(path);
}

void TestSettings::testDoNotTrack()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    QSignalSpy spy(&settings, &Settings::doNotTrackChanged);

    settings.setDoNotTrack(false);
    QCOMPARE(settings.doNotTrack(), false);
    QCOMPARE(spy.count(), 1);

    QFile::remove(path);
}

void TestSettings::testAdBlocking()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    QSignalSpy spy(&settings, &Settings::adBlockingEnabledChanged);

    settings.setAdBlockingEnabled(false);
    QCOMPARE(settings.adBlockingEnabled(), false);
    QCOMPARE(spy.count(), 1);

    QFile::remove(path);
}

void TestSettings::testJavaScript()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    QSignalSpy spy(&settings, &Settings::javaScriptEnabledChanged);

    settings.setJavaScriptEnabled(false);
    QCOMPARE(settings.javaScriptEnabled(), false);
    QCOMPARE(spy.count(), 1);

    QFile::remove(path);
}

void TestSettings::testPersistence()
{
    QString path = tempSettingsPath();

    // Write settings
    {
        Settings settings(path);
        settings.setDnsProvider(DnsOverHttps::Quad9);
        settings.setCustomDnsUrl("https://custom.dns/query");
        settings.setSearchEngineUrl("https://search.example.com/?q=%1");
        settings.setEphemeralSessions(false);
        settings.setDoNotTrack(false);
        settings.setAdBlockingEnabled(false);
        settings.setJavaScriptEnabled(false);
    }

    // Read them back from a new instance
    {
        Settings settings(path);
        QCOMPARE(settings.dnsProvider(), DnsOverHttps::Quad9);
        QCOMPARE(settings.customDnsUrl(), QString("https://custom.dns/query"));
        QCOMPARE(settings.searchEngineUrl(), QString("https://search.example.com/?q=%1"));
        QCOMPARE(settings.ephemeralSessions(), false);
        QCOMPARE(settings.doNotTrack(), false);
        QCOMPARE(settings.adBlockingEnabled(), false);
        QCOMPARE(settings.javaScriptEnabled(), false);
    }

    QFile::remove(path);
}

void TestSettings::testSignalsOnlyOnChange()
{
    QString path = tempSettingsPath();
    Settings settings(path);

    QSignalSpy spy(&settings, &Settings::dnsProviderChanged);

    // Default is Cloudflare — setting to Cloudflare should NOT emit
    settings.setDnsProvider(DnsOverHttps::Cloudflare);
    QCOMPARE(spy.count(), 0);

    // Now change — should emit
    settings.setDnsProvider(DnsOverHttps::Quad9);
    QCOMPARE(spy.count(), 1);

    // Set same value again — should NOT emit
    settings.setDnsProvider(DnsOverHttps::Quad9);
    QCOMPARE(spy.count(), 1);

    QFile::remove(path);
}

void TestSettings::testDnsProviderBoundsCheck()
{
    QString path = tempSettingsPath();

    // Manually write an out-of-range value
    {
        QSettings raw(path, QSettings::IniFormat);
        raw.setValue("dns/provider", 999);
    }

    Settings settings(path);
    // Should fall back to Cloudflare for invalid values
    QCOMPARE(settings.dnsProvider(), DnsOverHttps::Cloudflare);

    QFile::remove(path);
}

QTEST_MAIN(TestSettings)
#include "test_settings.moc"
