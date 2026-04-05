#include "app/DownloadManager.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
{
    loadHistory();
}

void DownloadManager::setDownloadDirectory(const QString &path)
{
    m_downloadDirectory = path;
}

QString DownloadManager::downloadDirectory() const
{
    return m_downloadDirectory;
}

QList<DownloadItem> DownloadManager::downloads() const
{
    QList<DownloadItem> items;

    // Active downloads first
    for (auto *dl : m_downloads) {
        DownloadItem item;
        item.id = dl->id();
        item.fileName = QFileInfo(dl->downloadFileName()).fileName();
        item.downloadDirectory = dl->downloadDirectory();
        item.state = dl->state();
        item.receivedBytes = dl->receivedBytes();
        item.totalBytes = dl->totalBytes();
        items.append(item);
    }

    // Then history (already completed, newest first)
    items.append(m_history);

    return items;
}

QList<DownloadItem> DownloadManager::history() const
{
    return m_history;
}

int DownloadManager::activeCount() const
{
    return m_activeCount;
}

void DownloadManager::onDownloadRequested(QWebEngineDownloadRequest *download)
{
    if (!download)
        return;

    if (!m_downloadDirectory.isEmpty())
        download->setDownloadDirectory(m_downloadDirectory);

    download->accept();
    m_downloads.append(download);

    quint32 id = download->id();
    qDebug() << "Download started:" << download->downloadFileName();

    connect(download, &QWebEngineDownloadRequest::receivedBytesChanged, this, [this, id]() {
        emit downloadUpdated(id);
    });

    connect(download, &QWebEngineDownloadRequest::stateChanged, this,
            [this, download, id](QWebEngineDownloadRequest::DownloadState state) {
        emit downloadUpdated(id);
        if (state == QWebEngineDownloadRequest::DownloadCompleted ||
            state == QWebEngineDownloadRequest::DownloadCancelled ||
            state == QWebEngineDownloadRequest::DownloadInterrupted) {

            // Save to persistent history
            DownloadItem item;
            item.id = id;
            item.fileName = QFileInfo(download->downloadFileName()).fileName();
            item.downloadDirectory = download->downloadDirectory();
            item.state = state;
            item.receivedBytes = download->receivedBytes();
            item.totalBytes = download->totalBytes();
            addToHistory(item);

            // Remove from active list
            m_downloads.removeOne(download);

            emit downloadFinished(id);
            qDebug() << "Download finished:" << id << "state:" << state;
        }
        updateActiveCount();
    });

    updateActiveCount();
    emit downloadAdded(id);
}

void DownloadManager::updateActiveCount()
{
    int count = 0;
    for (auto *dl : m_downloads) {
        if (dl->state() == QWebEngineDownloadRequest::DownloadInProgress)
            ++count;
    }
    if (count != m_activeCount) {
        m_activeCount = count;
        emit activeCountChanged(m_activeCount);
    }
}

void DownloadManager::addToHistory(const DownloadItem &item)
{
    m_history.prepend(item);
    while (m_history.size() > MaxHistorySize)
        m_history.removeLast();
    saveHistory();
}

QString DownloadManager::historyFilePath() const
{
    // Use the same directory as QSettings (IniFormat, UserScope, "scowser")
    // which is ~/.config/scowser/ on both macOS and Linux
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
                        + "/scowser";
    QDir().mkpath(configDir);
    return configDir + "/download_history.json";
}

void DownloadManager::loadHistory()
{
    QFile file(historyFilePath());
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray())
        return;

    m_history.clear();
    for (const auto &val : doc.array()) {
        QJsonObject obj = val.toObject();
        DownloadItem item;
        item.id = obj["id"].toInt();
        item.fileName = obj["fileName"].toString();
        item.downloadDirectory = obj["downloadDirectory"].toString();
        item.state = static_cast<QWebEngineDownloadRequest::DownloadState>(obj["state"].toInt());
        item.receivedBytes = obj["receivedBytes"].toVariant().toLongLong();
        item.totalBytes = obj["totalBytes"].toVariant().toLongLong();
        m_history.append(item);
    }

    // Enforce cap in case file was edited
    while (m_history.size() > MaxHistorySize)
        m_history.removeLast();
}

void DownloadManager::saveHistory()
{
    QJsonArray arr;
    for (const auto &item : m_history) {
        QJsonObject obj;
        obj["id"] = static_cast<int>(item.id);
        obj["fileName"] = item.fileName;
        obj["downloadDirectory"] = item.downloadDirectory;
        obj["state"] = static_cast<int>(item.state);
        obj["receivedBytes"] = item.receivedBytes;
        obj["totalBytes"] = item.totalBytes;
        arr.append(obj);
    }

    QFile file(historyFilePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson(QJsonDocument::Compact));
    }
}
