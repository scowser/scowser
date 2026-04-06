#include <QtTest/QtTest>
#include <QPlainTextEdit>
#include <QToolButton>
#include <QMainWindow>
#include "ui/LogPanel.h"

static bool runningUnderValgrind()
{
#ifdef __linux__
    return qEnvironmentVariableIsSet("RUNNING_UNDER_VALGRIND");
#else
    return false;
#endif
}

static bool shouldSkipHighlighterTests()
{
#ifdef SANITIZER_BUILD
    return true;
#endif
    return runningUnderValgrind();
}

class TestLogPanel : public QObject {
    Q_OBJECT

private slots:
    void testPanelCreation();
    void testAppendMessage();
    void testLogLevels();
    void testClearLog();
    void testDefaultDockPosition();
    void testToggleDockOrientation();
};

void TestLogPanel::testPanelCreation()
{
    QMainWindow mainWin;
    LogPanel panel(&mainWin);

    QCOMPARE(panel.objectName(), QString("logPanel"));
    QCOMPARE(panel.windowTitle(), QString("Logs"));

    auto *logView = panel.findChild<QPlainTextEdit *>("logView");
    QVERIFY(logView != nullptr);
    QVERIFY(logView->isReadOnly());
}

void TestLogPanel::testAppendMessage()
{
    if (shouldSkipHighlighterTests()) {
        QSKIP("Skipping under Valgrind/sanitizer (Qt QSyntaxHighlighter SSE false positives)");
    }

    QMainWindow mainWin;
    LogPanel panel(&mainWin);

    panel.appendMessage(QtDebugMsg, "test message");

    auto *logView = panel.findChild<QPlainTextEdit *>("logView");
    QVERIFY(logView != nullptr);
    QVERIFY(logView->toPlainText().contains("DEBUG"));
    QVERIFY(logView->toPlainText().contains("test message"));
}

void TestLogPanel::testLogLevels()
{
    if (shouldSkipHighlighterTests()) {
        QSKIP("Skipping under Valgrind/sanitizer (Qt QSyntaxHighlighter SSE false positives)");
    }

    QMainWindow mainWin;
    LogPanel panel(&mainWin);

    panel.appendMessage(QtDebugMsg, "debug msg");
    panel.appendMessage(QtInfoMsg, "info msg");
    panel.appendMessage(QtWarningMsg, "warn msg");
    panel.appendMessage(QtCriticalMsg, "critical msg");

    auto *logView = panel.findChild<QPlainTextEdit *>("logView");
    QString text = logView->toPlainText();

    QVERIFY(text.contains("DEBUG"));
    QVERIFY(text.contains("INFO"));
    QVERIFY(text.contains("WARNING"));
    QVERIFY(text.contains("CRITICAL"));
    QCOMPARE(logView->document()->blockCount(), 4);
}

void TestLogPanel::testClearLog()
{
    if (shouldSkipHighlighterTests()) {
        QSKIP("Skipping under Valgrind/sanitizer (Qt QSyntaxHighlighter SSE false positives)");
    }

    QMainWindow mainWin;
    LogPanel panel(&mainWin);

    panel.appendMessage(QtDebugMsg, "something");
    panel.clearLog();

    auto *logView = panel.findChild<QPlainTextEdit *>("logView");
    QVERIFY(logView->toPlainText().isEmpty());
}

void TestLogPanel::testDefaultDockPosition()
{
    QMainWindow mainWin;
    LogPanel panel(&mainWin);

    // Default should be right (not bottom)
    QVERIFY(!panel.isDockedBottom());
}

void TestLogPanel::testToggleDockOrientation()
{
    QMainWindow mainWin;
    LogPanel panel(&mainWin);

    QVERIFY(!panel.isDockedBottom());

    panel.toggleDockOrientation();
    QVERIFY(panel.isDockedBottom());

    panel.toggleDockOrientation();
    QVERIFY(!panel.isDockedBottom());
}

QTEST_MAIN(TestLogPanel)
#include "test_logpanel.moc"
