#include <QtTest/QtTest>
#include <QApplication>
#include <QFile>

class TestTheme : public QObject {
    Q_OBJECT

private slots:
    void testStyleSheetResourceExists();
    void testStyleSheetNotEmpty();
    void testStyleSheetContainsCoreSelectors();
    void testIconResourcesExist();
};

void TestTheme::testStyleSheetResourceExists()
{
    QFile qss(":/style/scowser.qss");
    QVERIFY2(qss.exists(), "Stylesheet resource :/style/scowser.qss must exist");
}

void TestTheme::testStyleSheetNotEmpty()
{
    QFile qss(":/style/scowser.qss");
    QVERIFY(qss.open(QIODevice::ReadOnly | QIODevice::Text));
    QByteArray content = qss.readAll();
    QVERIFY2(!content.isEmpty(), "Stylesheet must not be empty");
}

void TestTheme::testStyleSheetContainsCoreSelectors()
{
    QFile qss(":/style/scowser.qss");
    QVERIFY(qss.open(QIODevice::ReadOnly | QIODevice::Text));
    QString content = qss.readAll();

    QVERIFY2(content.contains("QMainWindow"), "Stylesheet must style QMainWindow");
    QVERIFY2(content.contains("QToolBar"), "Stylesheet must style QToolBar");
    QVERIFY2(content.contains("QTabBar"), "Stylesheet must style QTabBar");
    QVERIFY2(content.contains("AddressBar"), "Stylesheet must style AddressBar");
    QVERIFY2(content.contains("QStatusBar"), "Stylesheet must style QStatusBar");
}

void TestTheme::testIconResourcesExist()
{
    QStringList icons = {
        ":/icons/back.svg",
        ":/icons/forward.svg",
        ":/icons/reload.svg",
        ":/icons/new-tab.svg",
        ":/icons/lock-secure.svg",
        ":/icons/lock-insecure.svg",
        ":/icons/scowser.png",
    };

    for (const auto &icon : icons) {
        QVERIFY2(QFile::exists(icon),
                 qPrintable(QString("Icon resource %1 must exist").arg(icon)));
    }
}

QTEST_MAIN(TestTheme)
#include "test_theme.moc"
