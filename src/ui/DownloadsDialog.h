#pragma once

#include <QDialog>
#include <QVBoxLayout>

class DownloadManager;
class QLabel;
class QScrollArea;

class DownloadsDialog : public QDialog {
    Q_OBJECT

public:
    explicit DownloadsDialog(DownloadManager *manager, QWidget *parent = nullptr);

private slots:
    void refreshList();

private:
    QWidget *createDownloadItemWidget(int index);
    static QString formatBytes(qint64 bytes);

    DownloadManager *m_manager;
    QVBoxLayout *m_listLayout;
    QLabel *m_emptyLabel;
    QScrollArea *m_scrollArea;
};
