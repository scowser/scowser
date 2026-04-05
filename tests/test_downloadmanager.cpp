#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include "app/DownloadManager.h"

class TestDownloadManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testDefaultState();
    void testSetDownloadDirectory();
    void testEmptyDownloadsList();
    void testActiveCountInitiallyZero();
    void testMaxHistorySize();
    void testHistoryEmptyOnFreshStart();
};

void TestDownloadManager::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    // Clean any leftover test history
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QFile::remove(configDir + "/download_history.json");
}

void TestDownloadManager::cleanupTestCase()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QFile::remove(configDir + "/download_history.json");
}

void TestDownloadManager::testDefaultState()
{
    DownloadManager mgr;
    QCOMPARE(mgr.activeCount(), 0);
    QVERIFY(mgr.downloads().isEmpty());
    QVERIFY(mgr.downloadDirectory().isEmpty());
}

void TestDownloadManager::testSetDownloadDirectory()
{
    DownloadManager mgr;
    QString dir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    mgr.setDownloadDirectory(dir);
    QCOMPARE(mgr.downloadDirectory(), dir);
}

void TestDownloadManager::testEmptyDownloadsList()
{
    DownloadManager mgr;
    auto items = mgr.downloads();
    QCOMPARE(items.size(), 0);
}

void TestDownloadManager::testActiveCountInitiallyZero()
{
    DownloadManager mgr;
    QSignalSpy spy(&mgr, &DownloadManager::activeCountChanged);

    QCOMPARE(mgr.activeCount(), 0);
    QCOMPARE(spy.count(), 0);
}

void TestDownloadManager::testMaxHistorySize()
{
    QCOMPARE(DownloadManager::MaxHistorySize, 20);
}

void TestDownloadManager::testHistoryEmptyOnFreshStart()
{
    DownloadManager mgr;
    QVERIFY(mgr.history().isEmpty());
}

QTEST_MAIN(TestDownloadManager)
#include "test_downloadmanager.moc"
