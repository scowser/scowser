#include <QtTest/QtTest>
#include <QLabel>
#include <QScrollArea>
#include "app/DownloadManager.h"
#include "ui/DownloadsDialog.h"

class TestDownloadsDialog : public QObject {
    Q_OBJECT

private slots:
    void testDialogCreation();
    void testEmptyState();
};

void TestDownloadsDialog::testDialogCreation()
{
    DownloadManager mgr;
    DownloadsDialog dialog(&mgr);

    QCOMPARE(dialog.windowTitle(), QString("Downloads"));
    QCOMPARE(dialog.isVisible(), false);
}

void TestDownloadsDialog::testEmptyState()
{
    DownloadManager mgr;
    DownloadsDialog dialog(&mgr);

    // Empty label should not be hidden, scroll area should be hidden
    // (use isHidden/isVisibleTo since dialog itself isn't shown)
    auto *emptyLabel = dialog.findChild<QLabel *>("downloadsEmpty");
    QVERIFY(emptyLabel != nullptr);
    QVERIFY(!emptyLabel->isHidden());

    auto *scrollArea = dialog.findChild<QScrollArea *>();
    QVERIFY(scrollArea != nullptr);
    QVERIFY(scrollArea->isHidden());
}

QTEST_MAIN(TestDownloadsDialog)
#include "test_downloadsdialog.moc"
