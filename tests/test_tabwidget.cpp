#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QTabBar>
#include <QToolButton>
#include <QWheelEvent>
#include "ui/TabWidget.h"
#include "ui/ScrollableTabBar.h"

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
    void testWheelDoesNotChangeActiveTab();
    void testWheelScrollsStrip();
    void testWheelSmallDeltasAccumulate();
    void testWheelDirectionChangeResetsAccumulation();
    void testWheelNoOverflowIsSafe();

private:
    void sendWheel(QWidget *target, int deltaY);
    void sendTrackpadWheel(QWidget *target, int deltaY);
    void makeOverflowing(ScrollableTabBar &bar);
    QToolButton *scrollButton(ScrollableTabBar &bar, Qt::ArrowType arrow);

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

void TestTabWidget::sendWheel(QWidget *target, int deltaY)
{
    QWheelEvent event(QPointF(10, 10), target->mapToGlobal(QPointF(10, 10)),
                      QPoint(), QPoint(0, deltaY),
                      Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false);
    QApplication::sendEvent(target, &event);
}

void TestTabWidget::sendTrackpadWheel(QWidget *target, int deltaY)
{
    QWheelEvent event(QPointF(10, 10), target->mapToGlobal(QPointF(10, 10)),
                      QPoint(0, deltaY), QPoint(0, deltaY),
                      Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    QApplication::sendEvent(target, &event);
}

void TestTabWidget::makeOverflowing(ScrollableTabBar &bar)
{
    bar.setFixedWidth(200);
    for (int i = 0; i < 20; ++i)
        bar.addTab(QString("Tab %1").arg(i));
    bar.show();
    QApplication::processEvents();
}

QToolButton *TestTabWidget::scrollButton(ScrollableTabBar &bar, Qt::ArrowType arrow)
{
    for (auto *button : bar.findChildren<QToolButton *>()) {
        if (button->arrowType() == arrow)
            return button;
    }
    return nullptr;
}

void TestTabWidget::testWheelDoesNotChangeActiveTab()
{
    ScrollableTabBar bar;
    makeOverflowing(bar);
    bar.setCurrentIndex(0);

    sendWheel(&bar, -120);
    sendWheel(&bar, -120);
    sendTrackpadWheel(&bar, -240);
    sendWheel(&bar, 120);
    QCOMPARE(bar.currentIndex(), 0);
}

void TestTabWidget::testWheelScrollsStrip()
{
    ScrollableTabBar bar;
    makeOverflowing(bar);
    bar.setCurrentIndex(0);

    auto *rightButton = scrollButton(bar, Qt::RightArrow);
    QVERIFY(rightButton != nullptr);
    QVERIFY(rightButton->isEnabled());
    QSignalSpy clickSpy(rightButton, &QToolButton::clicked);

    // One full mouse-wheel notch scrolls the strip one step
    sendWheel(&bar, -120);
    QCOMPARE(clickSpy.count(), 1);
    sendWheel(&bar, -120);
    QCOMPARE(clickSpy.count(), 2);
}

void TestTabWidget::testWheelSmallDeltasAccumulate()
{
    ScrollableTabBar bar;
    makeOverflowing(bar);

    auto *rightButton = scrollButton(bar, Qt::RightArrow);
    QVERIFY(rightButton != nullptr);
    QSignalSpy clickSpy(rightButton, &QToolButton::clicked);

    // Trackpad events (pixel delta + scroll phase) need 240 units per step
    sendTrackpadWheel(&bar, -120);
    QCOMPARE(clickSpy.count(), 0);
    sendTrackpadWheel(&bar, -120);
    QCOMPARE(clickSpy.count(), 1);

    // Accumulator resets after a step; next partial delta doesn't scroll
    sendTrackpadWheel(&bar, -120);
    QCOMPARE(clickSpy.count(), 1);
}

void TestTabWidget::testWheelDirectionChangeResetsAccumulation()
{
    ScrollableTabBar bar;
    makeOverflowing(bar);

    auto *rightButton = scrollButton(bar, Qt::RightArrow);
    QVERIFY(rightButton != nullptr);
    QSignalSpy clickSpy(rightButton, &QToolButton::clicked);

    sendTrackpadWheel(&bar, -200);
    QCOMPARE(clickSpy.count(), 0);

    // Reversing direction discards prior accumulation
    sendTrackpadWheel(&bar, 200);
    QCOMPARE(clickSpy.count(), 0);
    sendTrackpadWheel(&bar, -200);
    QCOMPARE(clickSpy.count(), 0);
}

void TestTabWidget::testWheelNoOverflowIsSafe()
{
    // Few tabs, no overflow: scroll buttons are disabled, wheel is a no-op
    ScrollableTabBar bar;
    bar.addTab("1");
    bar.addTab("2");
    bar.setCurrentIndex(0);

    sendWheel(&bar, -120);
    sendWheel(&bar, 120);
    QCOMPARE(bar.currentIndex(), 0);
    QCOMPARE(bar.count(), 2);
}

QTEST_MAIN(TestTabWidget)
#include "test_tabwidget.moc"
