#include <QtTest/QtTest>
#include <QSignalSpy>
#include "ui/AddressBar.h"

class TestAddressBar : public QObject {
    Q_OBJECT

private slots:
    void testInitialState();
    void testSetUrlHttps();
    void testSetUrlHttp();
    void testSetUrlAboutBlank();
    void testSetUrlEmpty();
    void testSetSecurityIndicatorSecure();
    void testSetSecurityIndicatorInsecure();
    void testUrlEnteredSignal();
    void testSearchQuery();
    void testBaredomainDefaultsToHttps();
    void testSecurityToggle();
};

void TestAddressBar::testInitialState()
{
    AddressBar bar;
    QVERIFY(!bar.isSecure());
    QCOMPARE(bar.text(), QString());
}

void TestAddressBar::testSetUrlHttps()
{
    AddressBar bar;
    bar.setUrl(QUrl("https://example.com"));

    QVERIFY(bar.isSecure());
    QVERIFY(bar.text().contains("example.com"));
}

void TestAddressBar::testSetUrlHttp()
{
    AddressBar bar;
    bar.setUrl(QUrl("http://example.com"));

    QVERIFY(!bar.isSecure());
    QVERIFY(bar.text().contains("example.com"));
}

void TestAddressBar::testSetUrlAboutBlank()
{
    AddressBar bar;
    // First set a real URL
    bar.setUrl(QUrl("https://example.com"));
    QVERIFY(bar.isSecure());

    // Then navigate to about:blank
    bar.setUrl(QUrl("about:blank"));
    QVERIFY(!bar.isSecure());
    QCOMPARE(bar.text(), QString());
}

void TestAddressBar::testSetUrlEmpty()
{
    AddressBar bar;
    bar.setUrl(QUrl("https://example.com"));
    bar.setUrl(QUrl());

    QVERIFY(!bar.isSecure());
    QCOMPARE(bar.text(), QString());
}

void TestAddressBar::testSetSecurityIndicatorSecure()
{
    AddressBar bar;
    bar.setSecurityIndicator(true);

    QVERIFY(bar.isSecure());
    QCOMPARE(bar.property("secure").toBool(), true);
}

void TestAddressBar::testSetSecurityIndicatorInsecure()
{
    AddressBar bar;
    bar.setSecurityIndicator(false);

    QVERIFY(!bar.isSecure());
    QCOMPARE(bar.property("secure").toBool(), false);
}

void TestAddressBar::testUrlEnteredSignal()
{
    AddressBar bar;
    QSignalSpy spy(&bar, &AddressBar::urlEntered);

    bar.setText("https://example.com");
    QTest::keyClick(&bar, Qt::Key_Return);

    QCOMPARE(spy.count(), 1);
    QUrl emitted = spy.at(0).at(0).toUrl();
    QCOMPARE(emitted.scheme(), QString("https"));
    QCOMPARE(emitted.host(), QString("example.com"));
}

void TestAddressBar::testSearchQuery()
{
    AddressBar bar;
    QSignalSpy spy(&bar, &AddressBar::urlEntered);

    bar.setText("hello world");
    QTest::keyClick(&bar, Qt::Key_Return);

    QCOMPARE(spy.count(), 1);
    QUrl emitted = spy.at(0).at(0).toUrl();
    QVERIFY(emitted.host().contains("duckduckgo.com"));
}

void TestAddressBar::testBaredomainDefaultsToHttps()
{
    AddressBar bar;
    QSignalSpy spy(&bar, &AddressBar::urlEntered);

    bar.setText("example.com");
    QTest::keyClick(&bar, Qt::Key_Return);

    QCOMPARE(spy.count(), 1);
    QUrl emitted = spy.at(0).at(0).toUrl();
    QCOMPARE(emitted.scheme(), QString("https"));
}

void TestAddressBar::testSecurityToggle()
{
    AddressBar bar;

    bar.setSecurityIndicator(true);
    QVERIFY(bar.isSecure());

    bar.setSecurityIndicator(false);
    QVERIFY(!bar.isSecure());

    bar.setSecurityIndicator(true);
    QVERIFY(bar.isSecure());
}

QTEST_MAIN(TestAddressBar)
#include "test_addressbar.moc"
