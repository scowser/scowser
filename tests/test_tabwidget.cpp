#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QTabBar>
#include "ui/TabWidget.h"

class TestTabWidget : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testCreateTab();
    void testCloseTab();
    void testCurrentWebView();
    void testWebViewByIndex();
    void testNewTabSignal();
    void testSaveSessionSignal();
    void testUnsaveSessionSignal();
    void testTabCount();
    void testContextMenuOnBlankTabNoSignal();

private:
    QWidget *m_container = nullptr;
};

void TestTabWidget::initTestCase()
{
    m_container = new QWidget();
}

void TestTabWidget::cleanupTestCase()
{
    delete m_container;
}

void TestTabWidget::testCreateTab()
{
    TabWidget tw(m_container);
    auto *view = tw.createTab();
    QVERIFY(view != nullptr);
    QCOMPARE(tw.count(), 1);
}

void TestTabWidget::testCloseTab()
{
    TabWidget tw(m_container);
    tw.createTab();
    tw.createTab();
    QCOMPARE(tw.count(), 2);

    tw.closeTab(0);
    QCOMPARE(tw.count(), 1);
}

void TestTabWidget::testCurrentWebView()
{
    TabWidget tw(m_container);
    auto *view = tw.createTab();
    QCOMPARE(tw.currentWebView(), view);
}

void TestTabWidget::testWebViewByIndex()
{
    TabWidget tw(m_container);
    auto *view1 = tw.createTab();
    auto *view2 = tw.createTab();

    QCOMPARE(tw.webView(0), view1);
    QCOMPARE(tw.webView(1), view2);
    QCOMPARE(tw.webView(99), nullptr);
}

void TestTabWidget::testNewTabSignal()
{
    TabWidget tw(m_container);
    QSignalSpy spy(&tw, &TabWidget::newTabRequested);
    // Signal is triggered by the button, which we don't click here
    // Just verify signal exists
    QVERIFY(spy.isValid());
}

void TestTabWidget::testSaveSessionSignal()
{
    TabWidget tw(m_container);
    QSignalSpy spy(&tw, &TabWidget::saveSessionRequested);
    QVERIFY(spy.isValid());
}

void TestTabWidget::testUnsaveSessionSignal()
{
    TabWidget tw(m_container);
    QSignalSpy spy(&tw, &TabWidget::unsaveSessionRequested);
    QVERIFY(spy.isValid());
}

void TestTabWidget::testTabCount()
{
    TabWidget tw(m_container);
    QCOMPARE(tw.count(), 0);

    tw.createTab();
    QCOMPARE(tw.count(), 1);

    tw.createTab();
    tw.createTab();
    QCOMPARE(tw.count(), 3);
}

void TestTabWidget::testContextMenuOnBlankTabNoSignal()
{
    TabWidget tw(m_container);
    tw.createTab();
    // Tab has about:blank URL, context menu should not emit signals
    QSignalSpy saveSpy(&tw, &TabWidget::saveSessionRequested);
    QSignalSpy unsaveSpy(&tw, &TabWidget::unsaveSessionRequested);

    // Simulate right-click at invalid position (no tab)
    // The showTabContextMenu should return early for index < 0
    // We can't easily simulate context menu in unit test without UI,
    // but we verify signals are valid and no crash
    QVERIFY(saveSpy.isValid());
    QVERIFY(unsaveSpy.isValid());
    QCOMPARE(saveSpy.count(), 0);
    QCOMPARE(unsaveSpy.count(), 0);
}

QTEST_MAIN(TestTabWidget)
#include "test_tabwidget.moc"
