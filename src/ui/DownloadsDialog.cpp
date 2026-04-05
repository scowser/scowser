#include "ui/DownloadsDialog.h"
#include "app/DownloadManager.h"

#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QFileInfo>

DownloadsDialog::DownloadsDialog(DownloadManager *manager, QWidget *parent)
    : QDialog(parent)
    , m_manager(manager)
{
    setWindowTitle("Downloads");
    setFixedSize(450, 400);

    auto *mainLayout = new QVBoxLayout(this);

    // Scrollable list area
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *listWidget = new QWidget;
    m_listLayout = new QVBoxLayout(listWidget);
    m_listLayout->setAlignment(Qt::AlignTop);
    m_scrollArea->setWidget(listWidget);

    // Empty state label
    m_emptyLabel = new QLabel("No downloads");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setObjectName("downloadsEmpty");

    mainLayout->addWidget(m_scrollArea);
    mainLayout->addWidget(m_emptyLabel);

    // Connect to manager signals for live updates
    connect(m_manager, &DownloadManager::downloadAdded, this, &DownloadsDialog::refreshList);
    connect(m_manager, &DownloadManager::downloadUpdated, this, &DownloadsDialog::refreshList);
    connect(m_manager, &DownloadManager::downloadFinished, this, &DownloadsDialog::refreshList);

    refreshList();
}

void DownloadsDialog::refreshList()
{
    // Clear existing items
    while (auto *item = m_listLayout->takeAt(0)) {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    auto items = m_manager->downloads();

    m_emptyLabel->setVisible(items.isEmpty());
    m_scrollArea->setVisible(!items.isEmpty());

    // Show newest first
    for (int i = items.size() - 1; i >= 0; --i) {
        m_listLayout->addWidget(createDownloadItemWidget(i));
    }
}

QWidget *DownloadsDialog::createDownloadItemWidget(int index)
{
    auto items = m_manager->downloads();
    if (index < 0 || index >= items.size())
        return new QWidget;

    const auto &item = items[index];

    auto *widget = new QWidget;
    widget->setObjectName("downloadItem");
    auto *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(4);

    // File name
    auto *nameLabel = new QLabel(item.fileName);
    nameLabel->setObjectName("downloadFileName");
    layout->addWidget(nameLabel);

    // Progress bar (visible only while downloading)
    if (item.state == QWebEngineDownloadRequest::DownloadInProgress) {
        auto *progressBar = new QProgressBar;
        progressBar->setMinimum(0);
        if (item.totalBytes > 0) {
            progressBar->setMaximum(100);
            int percent = static_cast<int>((item.receivedBytes * 100) / item.totalBytes);
            progressBar->setValue(percent);
        } else {
            progressBar->setMaximum(0); // indeterminate
        }
        layout->addWidget(progressBar);
    }

    // Status text
    QString statusText;
    switch (item.state) {
    case QWebEngineDownloadRequest::DownloadInProgress:
        if (item.totalBytes > 0) {
            statusText = QString("Downloading... %1 / %2")
                .arg(formatBytes(item.receivedBytes), formatBytes(item.totalBytes));
        } else {
            statusText = QString("Downloading... %1").arg(formatBytes(item.receivedBytes));
        }
        break;
    case QWebEngineDownloadRequest::DownloadCompleted:
        statusText = "Complete";
        break;
    case QWebEngineDownloadRequest::DownloadCancelled:
        statusText = "Cancelled";
        break;
    case QWebEngineDownloadRequest::DownloadInterrupted:
        statusText = "Interrupted";
        break;
    default:
        statusText = "Requested";
        break;
    }

    auto *statusLabel = new QLabel(statusText);
    statusLabel->setObjectName("downloadStatus");
    layout->addWidget(statusLabel);

    // Action buttons for completed downloads
    if (item.state == QWebEngineDownloadRequest::DownloadCompleted) {
        auto *btnLayout = new QHBoxLayout;
        btnLayout->setContentsMargins(0, 4, 0, 0);

        QString filePath = QDir(item.downloadDirectory).filePath(item.fileName);

        auto *openFileBtn = new QPushButton("Open File");
        openFileBtn->setObjectName("downloadActionBtn");
        connect(openFileBtn, &QPushButton::clicked, this, [filePath]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        });

        auto *openFolderBtn = new QPushButton("Show in Folder");
        openFolderBtn->setObjectName("downloadActionBtn");
        connect(openFolderBtn, &QPushButton::clicked, this, [filePath]() {
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filePath).absolutePath()));
        });

        btnLayout->addWidget(openFileBtn);
        btnLayout->addWidget(openFolderBtn);
        btnLayout->addStretch();
        layout->addLayout(btnLayout);
    }

    return widget;
}

QString DownloadsDialog::formatBytes(qint64 bytes)
{
    if (bytes < 1024)
        return QString("%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    if (bytes < 1024 * 1024 * 1024)
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
}
