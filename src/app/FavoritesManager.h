#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QDateTime>
#include <QUuid>

struct FavoriteItem {
    QString id;
    QString url;
    QString title;
    QByteArray faviconPng;  // PNG data for offline favicon storage
    QString groupId;  // empty = ungrouped
    bool pinned = false;
    int position = 0;
    QDateTime createdAt;

    bool operator==(const FavoriteItem &other) const { return id == other.id; }
};

struct FavoriteGroup {
    QString id;
    QString name;
    bool pinned = false;
    int position = 0;
    bool collapsed = false;
    QDateTime createdAt;

    bool operator==(const FavoriteGroup &other) const { return id == other.id; }
};

class FavoritesManager : public QObject {
    Q_OBJECT

public:
    explicit FavoritesManager(QObject *parent = nullptr);

    // --- Items ---
    QString addFavorite(const QString &url, const QString &title, const QString &groupId = QString());
    void removeFavorite(const QString &id);
    void updateFavorite(const QString &id, const QString &url, const QString &title);
    void renameFavorite(const QString &id, const QString &title);
    void setFaviconPng(const QString &id, const QByteArray &pngData);
    void moveFavorite(const QString &id, const QString &groupId);
    void pinFavorite(const QString &id, bool pinned);
    void reorderFavorite(const QString &id, int newPosition);

    bool isFavorited(const QString &url) const;
    QString favoriteIdForUrl(const QString &url) const;
    FavoriteItem favorite(const QString &id) const;
    QList<FavoriteItem> favorites() const;
    QList<FavoriteItem> favoritesInGroup(const QString &groupId) const;
    QList<FavoriteItem> pinnedFavorites() const;

    // --- Groups ---
    QString addGroup(const QString &name);
    void removeGroup(const QString &id, bool deleteItems = false);
    void renameGroup(const QString &id, const QString &name);
    void pinGroup(const QString &id, bool pinned);
    void reorderGroup(const QString &id, int newPosition);
    void setGroupCollapsed(const QString &id, bool collapsed);

    FavoriteGroup group(const QString &id) const;
    QList<FavoriteGroup> groups() const;
    QList<FavoriteGroup> pinnedGroups() const;

    // --- Persistence ---
    void load();
    void save();

    // For testing
    void setStoragePath(const QString &path);

signals:
    void favoriteAdded(const QString &id);
    void favoriteRemoved(const QString &id);
    void favoriteUpdated(const QString &id);
    void groupAdded(const QString &id);
    void groupRemoved(const QString &id);
    void groupUpdated(const QString &id);
    void dataChanged();

private:
    QString storagePath() const;
    void normalizePositions(QList<FavoriteItem> &items);
    void normalizeGroupPositions();

    QList<FavoriteItem> m_favorites;
    QList<FavoriteGroup> m_groups;
    QString m_storagePath;
};
