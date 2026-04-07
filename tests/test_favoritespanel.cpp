#include <QtTest/QtTest>
#include <QTreeWidget>
#include <QToolButton>
#include <QLineEdit>
#include <QMainWindow>
#include <QSignalSpy>
#include "ui/FavoritesPanel.h"
#include "app/FavoritesManager.h"

class TestFavoritesPanel : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void testPanelCreation();
    void testEmptyState();
    void testShowsFavorites();
    void testShowsGroups();
    void testSearch();
    void testCloseButton();
    void testFavoriteActivatedSignal();

private:
    QString m_tempPath;
};

void TestFavoritesPanel::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_tempPath = QDir::temp().filePath("scowser_test_fav_panel.json");
}

void TestFavoritesPanel::init()
{
    QFile::remove(m_tempPath);
}

void TestFavoritesPanel::testPanelCreation()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);
    QMainWindow mainWin;
    FavoritesPanel panel(&mgr, &mainWin);

    QCOMPARE(panel.objectName(), QString("favoritesPanel"));
    QCOMPARE(panel.windowTitle(), QString("Favorites"));

    auto *tree = panel.findChild<QTreeWidget *>("favTree");
    QVERIFY(tree != nullptr);

    auto *searchBox = panel.findChild<QLineEdit *>("favSearchBox");
    QVERIFY(searchBox != nullptr);
}

void TestFavoritesPanel::testEmptyState()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);
    QMainWindow mainWin;
    FavoritesPanel panel(&mgr, &mainWin);

    auto *tree = panel.findChild<QTreeWidget *>("favTree");
    QCOMPARE(tree->topLevelItemCount(), 0);
}

void TestFavoritesPanel::testShowsFavorites()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);
    mgr.addFavorite("https://example.com", "Example");
    mgr.addFavorite("https://test.com", "Test");

    QMainWindow mainWin;
    FavoritesPanel panel(&mgr, &mainWin);

    auto *tree = panel.findChild<QTreeWidget *>("favTree");
    QCOMPARE(tree->topLevelItemCount(), 2);
}

void TestFavoritesPanel::testShowsGroups()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);
    QString gid = mgr.addGroup("Work");
    mgr.addFavorite("https://work.com", "Work Site", gid);
    mgr.addFavorite("https://ungrouped.com", "Ungrouped");

    QMainWindow mainWin;
    FavoritesPanel panel(&mgr, &mainWin);

    auto *tree = panel.findChild<QTreeWidget *>("favTree");
    // Should have: 1 group item + 1 ungrouped item = 2 top-level items
    QCOMPARE(tree->topLevelItemCount(), 2);

    // Find the group item and check it has a child
    bool foundGroup = false;
    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        auto *item = tree->topLevelItem(i);
        if (item->data(0, Qt::UserRole + 1).toInt() == 1) { // TypeGroup
            QCOMPARE(item->childCount(), 1);
            foundGroup = true;
        }
    }
    QVERIFY(foundGroup);
}

void TestFavoritesPanel::testSearch()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);
    mgr.addFavorite("https://example.com", "Example");
    mgr.addFavorite("https://test.com", "Test Site");

    QMainWindow mainWin;
    FavoritesPanel panel(&mgr, &mainWin);

    auto *tree = panel.findChild<QTreeWidget *>("favTree");
    auto *searchBox = panel.findChild<QLineEdit *>("favSearchBox");

    QCOMPARE(tree->topLevelItemCount(), 2);

    // Search for "Example" — should filter to 1
    searchBox->setText("Example");
    QCOMPARE(tree->topLevelItemCount(), 1);

    // Clear search — should show all
    searchBox->clear();
    QCOMPARE(tree->topLevelItemCount(), 2);
}

void TestFavoritesPanel::testCloseButton()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);
    QMainWindow mainWin;
    FavoritesPanel panel(&mgr, &mainWin);
    panel.show();

    auto *closeBtn = panel.findChild<QToolButton *>("favCloseButton");
    QVERIFY(closeBtn != nullptr);

    closeBtn->click();
    QVERIFY(!panel.isVisible());
}

void TestFavoritesPanel::testFavoriteActivatedSignal()
{
    FavoritesManager mgr;
    mgr.setStoragePath(m_tempPath);
    mgr.addFavorite("https://example.com", "Example");

    QMainWindow mainWin;
    FavoritesPanel panel(&mgr, &mainWin);

    QSignalSpy spy(&panel, &FavoritesPanel::favoriteActivated);

    auto *tree = panel.findChild<QTreeWidget *>("favTree");
    QVERIFY(tree->topLevelItemCount() > 0);

    // Simulate double-click on first item
    auto *item = tree->topLevelItem(0);
    emit tree->itemDoubleClicked(item, 0);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QString("https://example.com"));
}

QTEST_MAIN(TestFavoritesPanel)
#include "test_favoritespanel.moc"
