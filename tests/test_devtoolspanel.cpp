#include <QtTest/QtTest>
#include <QMainWindow>
#include <QWebEnginePage>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include "ui/DevToolsPanel.h"

class TestDevToolsPanel : public QObject {
    Q_OBJECT

private slots:
    void testPanelCreation();
    void testDefaultDockPosition();
    void testToggleDockOrientation();
    void testLazyDevToolsView();
    void testAttachToPage();
    void testDetach();
    void testReattachToAnotherPage();
    void testAttachSamePageIsNoOp();
    void testInspectedPageDestroyed();
    void testNoToolbarChrome();
    void testFirstShowGetsUsableSize();
    void testRightDockGetsUsableWidth();
};

void TestDevToolsPanel::testPanelCreation()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    QCOMPARE(panel.objectName(), QString("devToolsPanel"));
    QCOMPARE(panel.windowTitle(), QString("DevTools"));
    QVERIFY(panel.inspectedPage() == nullptr);
}

void TestDevToolsPanel::testDefaultDockPosition()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    // Default should be bottom (like browser devtools)
    QVERIFY(panel.isDockedBottom());
}

void TestDevToolsPanel::testToggleDockOrientation()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    QVERIFY(panel.isDockedBottom());

    panel.toggleDockOrientation();
    QVERIFY(!panel.isDockedBottom());

    panel.toggleDockOrientation();
    QVERIFY(panel.isDockedBottom());
}

void TestDevToolsPanel::testLazyDevToolsView()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    // No WebEngine view (and no renderer process) until first attach
    QVERIFY(panel.findChild<QWebEngineView *>("devToolsView") == nullptr);

    QWebEnginePage page(QWebEngineProfile::defaultProfile());
    panel.attachToPage(&page);

    QVERIFY(panel.findChild<QWebEngineView *>("devToolsView") != nullptr);
    panel.detach();
}

void TestDevToolsPanel::testAttachToPage()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    QWebEnginePage page(QWebEngineProfile::defaultProfile());
    panel.attachToPage(&page);

    QCOMPARE(panel.inspectedPage(), &page);

    auto *devView = panel.findChild<QWebEngineView *>("devToolsView");
    QVERIFY(devView != nullptr);
    QCOMPARE(devView->page()->inspectedPage(), &page);

    panel.detach();
}

void TestDevToolsPanel::testDetach()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    QWebEnginePage page(QWebEngineProfile::defaultProfile());
    panel.attachToPage(&page);
    panel.detach();

    QVERIFY(panel.inspectedPage() == nullptr);

    auto *devView = panel.findChild<QWebEngineView *>("devToolsView");
    QVERIFY(devView != nullptr);
    QVERIFY(devView->page()->inspectedPage() == nullptr);
}

void TestDevToolsPanel::testReattachToAnotherPage()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    QWebEnginePage pageA(QWebEngineProfile::defaultProfile());
    QWebEnginePage pageB(QWebEngineProfile::defaultProfile());

    panel.attachToPage(&pageA);
    QCOMPARE(panel.inspectedPage(), &pageA);

    panel.attachToPage(&pageB);
    QCOMPARE(panel.inspectedPage(), &pageB);

    auto *devView = panel.findChild<QWebEngineView *>("devToolsView");
    QCOMPARE(devView->page()->inspectedPage(), &pageB);

    panel.detach();
}

void TestDevToolsPanel::testAttachSamePageIsNoOp()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    QWebEnginePage page(QWebEngineProfile::defaultProfile());
    panel.attachToPage(&page);
    panel.attachToPage(&page);

    QCOMPARE(panel.inspectedPage(), &page);
    panel.detach();
}

void TestDevToolsPanel::testInspectedPageDestroyed()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    auto *page = new QWebEnginePage(QWebEngineProfile::defaultProfile());
    panel.attachToPage(page);
    QCOMPARE(panel.inspectedPage(), page);

    delete page;

    QVERIFY(panel.inspectedPage() == nullptr);
}

void TestDevToolsPanel::testNoToolbarChrome()
{
    QMainWindow mainWin;
    DevToolsPanel panel(&mainWin);

    // The panel must be chrome-less: no title label or toolbar buttons,
    // and the native dock title bar replaced with an empty widget.
    QVERIFY(panel.findChild<QWidget *>("devToolsPanelToolbar") == nullptr);
    QVERIFY(panel.titleBarWidget() != nullptr);
    QVERIFY(panel.titleBarWidget()->children().isEmpty());
}

void TestDevToolsPanel::testFirstShowGetsUsableSize()
{
    QMainWindow mainWin;
    mainWin.setCentralWidget(new QWidget(&mainWin));
    mainWin.resize(1280, 800);

    DevToolsPanel panel(&mainWin);
    panel.hide();
    mainWin.show();

    panel.show();

    // The deferred resize should expand the collapsed dock to a usable height
    QTRY_VERIFY(panel.height() >= 150);
}

void TestDevToolsPanel::testRightDockGetsUsableWidth()
{
    QMainWindow mainWin;
    mainWin.setCentralWidget(new QWidget(&mainWin));
    mainWin.resize(1280, 800);

    DevToolsPanel panel(&mainWin);
    mainWin.show();
    panel.show();

    panel.toggleDockOrientation();
    QVERIFY(!panel.isDockedBottom());

    // The deferred resize should expand the pane to 30% of the window width
    QTRY_VERIFY(panel.width() >= mainWin.width() * 3 / 10);
}

QTEST_MAIN(TestDevToolsPanel)
#include "test_devtoolspanel.moc"
