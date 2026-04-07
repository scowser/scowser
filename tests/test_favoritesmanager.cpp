#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QDir>
#include "app/FavoritesManager.h"

class TestFavoritesManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void testAddFavorite();
    void testRemoveFavorite();
    void testIsFavorited();
    void testRenameFavorite();
    void testPinFavorite();
    void testReorderFavorite();
    void testAddGroup();
    void testRemoveGroupKeepItems();
    void testRemoveGroupDeleteItems();
    void testRenameGroup();
    void testPinGroup();
    void testMoveFavoriteToGroup();
    void testGroupCollapsed();
    void testPersistence();
    void testDuplicatePrevention();
    void testPinnedSortOrder();
    void testSignals();

private:
    QString m_tempPath;
};

void TestFavoritesManager::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_tempPath = QDir::temp().filePath("scowser_test_favorites.json");
}

void TestFavoritesManager::cleanupTestCase()
{
    QFile::remove(m_tempPath);
}

void TestFavoritesManager::init()
{
    QFile::remove(m_tempPath);
}

void TestFavoritesManager::testAddFavorite()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString id = mgr.addFavorite("https://example.com", "Example");
    QVERIFY(!id.isEmpty());
    QCOMPARE(mgr.favorites().size(), 1);
    QCOMPARE(mgr.favorites().first().url, QString("https://example.com"));
    QCOMPARE(mgr.favorites().first().title, QString("Example"));
}

void TestFavoritesManager::testRemoveFavorite()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString id = mgr.addFavorite("https://example.com", "Example");
    QCOMPARE(mgr.favorites().size(), 1);

    mgr.removeFavorite(id);
    QCOMPARE(mgr.favorites().size(), 0);
}

void TestFavoritesManager::testIsFavorited()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QVERIFY(!mgr.isFavorited("https://example.com"));
    mgr.addFavorite("https://example.com", "Example");
    QVERIFY(mgr.isFavorited("https://example.com"));
    QVERIFY(!mgr.isFavorited("https://other.com"));
}

void TestFavoritesManager::testRenameFavorite()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString id = mgr.addFavorite("https://example.com", "Old Name");
    mgr.renameFavorite(id, "New Name");

    QCOMPARE(mgr.favorite(id).title, QString("New Name"));
}

void TestFavoritesManager::testPinFavorite()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString id = mgr.addFavorite("https://example.com", "Example");
    QVERIFY(!mgr.favorite(id).pinned);

    mgr.pinFavorite(id, true);
    QVERIFY(mgr.favorite(id).pinned);
    QCOMPARE(mgr.pinnedFavorites().size(), 1);

    mgr.pinFavorite(id, false);
    QVERIFY(!mgr.favorite(id).pinned);
    QCOMPARE(mgr.pinnedFavorites().size(), 0);
}

void TestFavoritesManager::testReorderFavorite()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    mgr.addFavorite("https://a.com", "A");
    mgr.addFavorite("https://b.com", "B");
    QString cId = mgr.addFavorite("https://c.com", "C");

    // Move C to position 0
    mgr.reorderFavorite(cId, 0);

    auto favs = mgr.favorites();
    QCOMPARE(favs.first().title, QString("C"));
}

void TestFavoritesManager::testAddGroup()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString gid = mgr.addGroup("Work");
    QVERIFY(!gid.isEmpty());
    QCOMPARE(mgr.groups().size(), 1);
    QCOMPARE(mgr.groups().first().name, QString("Work"));
}

void TestFavoritesManager::testRemoveGroupKeepItems()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString gid = mgr.addGroup("Work");
    mgr.addFavorite("https://example.com", "Example", gid);
    QCOMPARE(mgr.favoritesInGroup(gid).size(), 1);

    mgr.removeGroup(gid, false);
    QCOMPARE(mgr.groups().size(), 0);
    // Item should now be ungrouped
    QCOMPARE(mgr.favorites().size(), 1);
    QVERIFY(mgr.favorites().first().groupId.isEmpty());
}

void TestFavoritesManager::testRemoveGroupDeleteItems()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString gid = mgr.addGroup("Work");
    mgr.addFavorite("https://example.com", "Example", gid);

    mgr.removeGroup(gid, true);
    QCOMPARE(mgr.groups().size(), 0);
    QCOMPARE(mgr.favorites().size(), 0);
}

void TestFavoritesManager::testRenameGroup()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString gid = mgr.addGroup("Old Name");
    mgr.renameGroup(gid, "New Name");
    QCOMPARE(mgr.group(gid).name, QString("New Name"));
}

void TestFavoritesManager::testPinGroup()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString gid = mgr.addGroup("Work");
    QVERIFY(!mgr.group(gid).pinned);

    mgr.pinGroup(gid, true);
    QVERIFY(mgr.group(gid).pinned);
    QCOMPARE(mgr.pinnedGroups().size(), 1);
}

void TestFavoritesManager::testMoveFavoriteToGroup()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString gid = mgr.addGroup("Work");
    QString fid = mgr.addFavorite("https://example.com", "Example");

    QVERIFY(mgr.favorite(fid).groupId.isEmpty());
    QCOMPARE(mgr.favoritesInGroup(gid).size(), 0);

    mgr.moveFavorite(fid, gid);
    QCOMPARE(mgr.favorite(fid).groupId, gid);
    QCOMPARE(mgr.favoritesInGroup(gid).size(), 1);
}

void TestFavoritesManager::testGroupCollapsed()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString gid = mgr.addGroup("Work");
    QVERIFY(!mgr.group(gid).collapsed);

    mgr.setGroupCollapsed(gid, true);
    QVERIFY(mgr.group(gid).collapsed);
}

void TestFavoritesManager::testPersistence()
{
    // Write
    {
        FavoritesManager mgr;
        mgr.setStoragePath(m_tempPath);
        QString gid = mgr.addGroup("Saved Group");
        mgr.addFavorite("https://saved.com", "Saved Site", gid);
        mgr.addFavorite("https://other.com", "Other Site");
    }

    // Read
    {
        FavoritesManager mgr;
        mgr.setStoragePath(m_tempPath);
        mgr.load();

        QCOMPARE(mgr.groups().size(), 1);
        QCOMPARE(mgr.groups().first().name, QString("Saved Group"));
        QCOMPARE(mgr.favorites().size(), 2);
        QVERIFY(mgr.isFavorited("https://saved.com"));
        QVERIFY(mgr.isFavorited("https://other.com"));
    }
}

void TestFavoritesManager::testDuplicatePrevention()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QString id1 = mgr.addFavorite("https://example.com", "First");
    QString id2 = mgr.addFavorite("https://example.com", "Second");

    // Should return existing ID, not create duplicate
    QCOMPARE(id1, id2);
    QCOMPARE(mgr.favorites().size(), 1);
}

void TestFavoritesManager::testPinnedSortOrder()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    mgr.addFavorite("https://a.com", "A");
    QString bId = mgr.addFavorite("https://b.com", "B");
    mgr.addFavorite("https://c.com", "C");

    mgr.pinFavorite(bId, true);

    auto favs = mgr.favorites();
    // Pinned item should be first
    QCOMPARE(favs.first().title, QString("B"));
}

void TestFavoritesManager::testSignals()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);

    QSignalSpy addedSpy(&mgr, &FavoritesManager::favoriteAdded);
    QSignalSpy removedSpy(&mgr, &FavoritesManager::favoriteRemoved);
    QSignalSpy updatedSpy(&mgr, &FavoritesManager::favoriteUpdated);
    QSignalSpy dataChangedSpy(&mgr, &FavoritesManager::dataChanged);
    QSignalSpy groupAddedSpy(&mgr, &FavoritesManager::groupAdded);

    QString id = mgr.addFavorite("https://example.com", "Example");
    QCOMPARE(addedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    mgr.renameFavorite(id, "New Name");
    QCOMPARE(updatedSpy.count(), 1);

    mgr.removeFavorite(id);
    QCOMPARE(removedSpy.count(), 1);

    mgr.addGroup("Test Group");
    QCOMPARE(groupAddedSpy.count(), 1);
}

QTEST_MAIN(TestFavoritesManager)
#include "test_favoritesmanager.moc"
