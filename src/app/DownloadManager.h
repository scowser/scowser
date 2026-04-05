#pragma once

#include <QObject>
#include <QList>
#include <QWebEngineDownloadRequest>

struct DownloadItem {
    quint32 id;
    QString fileName;
    QString downloadDirectory;
    QWebEngineDownloadRequest::DownloadState state;
    qint64 receivedBytes;
    qint64 totalBytes;
};

class DownloadManager : public QObject {
    Q_OBJECT

public:
    static constexpr int MaxHistorySize = 20;

    explicit DownloadManager(QObject *parent = nullptr);

    void setDownloadDirectory(const QString &path);
    QString downloadDirectory() const;

    QList<DownloadItem> downloads() const;
    QList<DownloadItem> history() const;
    int activeCount() const;

public slots:
    void onDownloadRequested(QWebEngineDownloadRequest *download);

signals:
    void downloadAdded(quint32 id);
    void downloadUpdated(quint32 id);
    void downloadFinished(quint32 id);
    void activeCountChanged(int count);

private:
    void updateActiveCount();
    void addToHistory(const DownloadItem &item);
    void loadHistory();
    void saveHistory();
    QString historyFilePath() const;

    QString m_downloadDirectory;
    QList<QWebEngineDownloadRequest *> m_downloads;
    QList<DownloadItem> m_history;
    int m_activeCount = 0;
};
