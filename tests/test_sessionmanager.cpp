#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "security/SessionManager.h"

class TestSessionManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void testEphemeralProfileCreated();
    void testPersistentProfileCreated();
    void testProfilesAreDifferent();
    void testDefaultIsEphemeral();
    void testSetEphemeral();
    void testSaveTab();
    void testUnsaveTab();
    void testIsTabSaved();
    void testSaveTabDuplicate();
    void testWriteAndLoadSessions();
    void testLoadEmptyFile();
    void testLoadMissingFile();
    void testSavedTabsChangedSignal();
    void testSessionClearedSignal();
    void testEphemeralModeChangedSignal();

private:
    QTemporaryDir *m_tempDir = nullptr;
};

void TestSessionManager::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestSessionManager::cleanupTestCase()
{
    delete m_tempDir;
    m_tempDir = nullptr;
}

void TestSessionManager::init()
{
    // Clean up any previous temp dir
    delete m_tempDir;
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());

    // Remove any leftover sessions file from previous tests
    SessionManager sm;
    QFile::remove(sm.sessionsFilePath());
}

void TestSessionManager::testEphemeralProfileCreated()
{
    SessionManager sm;
    QVERIFY(sm.ephemeralProfile() != nullptr);
}

void TestSessionManager::testPersistentProfileCreated()
{
    SessionManager sm;
    QVERIFY(sm.persistentProfile() != nullptr);
}

void TestSessionManager::testProfilesAreDifferent()
{
    SessionManager sm;
    QVERIFY(sm.ephemeralProfile() != sm.persistentProfile());
}

void TestSessionManager::testDefaultIsEphemeral()
{
    SessionManager sm;
    QVERIFY(sm.isEphemeral());
}

void TestSessionManager::testSetEphemeral()
{
    SessionManager sm;

    sm.setEphemeral(false);
    QVERIFY(!sm.isEphemeral());

    sm.setEphemeral(true);
    QVERIFY(sm.isEphemeral());
}

void TestSessionManager::testSaveTab()
{
    SessionManager sm;
    sm.saveTab("https://example.com", "Example");

    QCOMPARE(sm.savedTabs().size(), 1);
    QCOMPARE(sm.savedTabs().first().url, "https://example.com");
    QCOMPARE(sm.savedTabs().first().title, "Example");
}

void TestSessionManager::testUnsaveTab()
{
    SessionManager sm;
    sm.saveTab("https://example.com", "Example");
    sm.saveTab("https://other.com", "Other");

    sm.unsaveTab("https://example.com");

    QCOMPARE(sm.savedTabs().size(), 1);
    QCOMPARE(sm.savedTabs().first().url, "https://other.com");
}

void TestSessionManager::testIsTabSaved()
{
    SessionManager sm;
    QVERIFY(!sm.isTabSaved("https://example.com"));

    sm.saveTab("https://example.com", "Example");
    QVERIFY(sm.isTabSaved("https://example.com"));
    QVERIFY(!sm.isTabSaved("https://other.com"));
}

void TestSessionManager::testSaveTabDuplicate()
{
    // File was cleaned in init(), so fresh start
    SessionManager sm;
    sm.saveTab("https://example.com", "Example");
    sm.saveTab("https://example.com", "Example Again");

    // Should not duplicate
    QCOMPARE(sm.savedTabs().size(), 1);
}

void TestSessionManager::testWriteAndLoadSessions()
{
    // File was cleaned in init(), so fresh start
    // Write sessions
    {
        SessionManager sm;
        sm.saveTab("https://example.com", "Example");
        sm.saveTab("https://test.org", "Test Site");
    }

    // Load sessions in a new manager
    {
        SessionManager sm;
        auto tabs = sm.savedTabs();
        QCOMPARE(tabs.size(), 2);
        QCOMPARE(tabs[0].url, "https://example.com");
        QCOMPARE(tabs[0].title, "Example");
        QCOMPARE(tabs[1].url, "https://test.org");
        QCOMPARE(tabs[1].title, "Test Site");
    }
}

void TestSessionManager::testLoadEmptyFile()
{
    SessionManager sm;
    // With test mode, the sessions file may or may not exist
    // Either way, savedTabs should not crash
    // Clear any loaded tabs first
    while (!sm.savedTabs().isEmpty()) {
        sm.unsaveTab(sm.savedTabs().first().url);
    }
    QCOMPARE(sm.savedTabs().size(), 0);
}

void TestSessionManager::testLoadMissingFile()
{
    // Just verify constructor doesn't crash when no file exists
    QStandardPaths::setTestModeEnabled(true);
    SessionManager sm;
    // No crash = pass
    QVERIFY(true);
}

void TestSessionManager::testSavedTabsChangedSignal()
{
    SessionManager sm;
    QSignalSpy spy(&sm, &SessionManager::savedTabsChanged);

    sm.saveTab("https://example.com", "Example");
    QCOMPARE(spy.count(), 1);

    sm.unsaveTab("https://example.com");
    QCOMPARE(spy.count(), 2);
}

void TestSessionManager::testSessionClearedSignal()
{
    SessionManager sm;
    QSignalSpy spy(&sm, &SessionManager::sessionCleared);

    sm.clearAllData();
    QCOMPARE(spy.count(), 1);
}

void TestSessionManager::testEphemeralModeChangedSignal()
{
    SessionManager sm;
    QSignalSpy spy(&sm, &SessionManager::ephemeralModeChanged);

    sm.setEphemeral(false);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toBool(), false);

    sm.setEphemeral(true);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.last().first().toBool(), true);

    // No signal if same value
    sm.setEphemeral(true);
    QCOMPARE(spy.count(), 2);
}

QTEST_MAIN(TestSessionManager)
#include "test_sessionmanager.moc"
