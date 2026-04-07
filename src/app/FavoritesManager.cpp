#include "app/FavoritesManager.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

FavoritesManager::FavoritesManager(QObject *parent)
    : QObject(parent)
{
    load();
}

// --- Items ---

QString FavoritesManager::addFavorite(const QString &url, const QString &title, const QString &groupId)
{
    // Don't duplicate
    if (isFavorited(url))
        return favoriteIdForUrl(url);

    FavoriteItem item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.url = url;
    item.title = title.isEmpty() ? url : title;
    item.groupId = groupId;
    item.pinned = false;
    item.createdAt = QDateTime::currentDateTimeUtc();

    // Position at end of its group
    int maxPos = -1;
    for (const auto &f : m_favorites) {
        if (f.groupId == groupId && f.position > maxPos)
            maxPos = f.position;
    }
    item.position = maxPos + 1;

    m_favorites.append(item);
    save();

    emit favoriteAdded(item.id);
    emit dataChanged();
    qDebug() << "FavoritesManager: Added favorite" << item.title << "(" << item.url << ")";
    return item.id;
}

void FavoritesManager::removeFavorite(const QString &id)
{
    for (int i = 0; i < m_favorites.size(); ++i) {
        if (m_favorites[i].id == id) {
            m_favorites.removeAt(i);
            save();
            emit favoriteRemoved(id);
            emit dataChanged();
            qDebug() << "FavoritesManager: Removed favorite" << id;
            return;
        }
    }
}

void FavoritesManager::updateFavorite(const QString &id, const QString &url, const QString &title)
{
    for (auto &f : m_favorites) {
        if (f.id == id) {
            f.url = url;
            f.title = title;
            save();
            emit favoriteUpdated(id);
            emit dataChanged();
            return;
        }
    }
}

void FavoritesManager::renameFavorite(const QString &id, const QString &title)
{
    for (auto &f : m_favorites) {
        if (f.id == id) {
            f.title = title;
            save();
            emit favoriteUpdated(id);
            emit dataChanged();
            return;
        }
    }
}

void FavoritesManager::setFaviconPng(const QString &id, const QByteArray &pngData)
{
    for (auto &f : m_favorites) {
        if (f.id == id) {
            if (f.faviconPng == pngData)
                return;
            f.faviconPng = pngData;
            save();
            emit favoriteUpdated(id);
            emit dataChanged();
            return;
        }
    }
}

void FavoritesManager::moveFavorite(const QString &id, const QString &groupId)
{
    for (auto &f : m_favorites) {
        if (f.id == id) {
            f.groupId = groupId;
            // Position at end of new group
            int maxPos = -1;
            for (const auto &other : m_favorites) {
                if (other.groupId == groupId && other.id != id && other.position > maxPos)
                    maxPos = other.position;
            }
            f.position = maxPos + 1;
            save();
            emit favoriteUpdated(id);
            emit dataChanged();
            return;
        }
    }
}

void FavoritesManager::pinFavorite(const QString &id, bool pinned)
{
    for (auto &f : m_favorites) {
        if (f.id == id) {
            f.pinned = pinned;
            save();
            emit favoriteUpdated(id);
            emit dataChanged();
            return;
        }
    }
}

void FavoritesManager::reorderFavorite(const QString &id, int newPosition)
{
    for (auto &f : m_favorites) {
        if (f.id == id) {
            QString gid = f.groupId;

            // Collect items in same group, sorted by position
            QList<FavoriteItem *> groupItems;
            for (auto &other : m_favorites) {
                if (other.groupId == gid)
                    groupItems.append(&other);
            }
            std::sort(groupItems.begin(), groupItems.end(),
                      [](const FavoriteItem *a, const FavoriteItem *b) { return a->position < b->position; });

            // Remove the item and re-insert at new position
            groupItems.removeOne(&f);
            if (newPosition < 0) newPosition = 0;
            if (newPosition > groupItems.size()) newPosition = groupItems.size();
            groupItems.insert(newPosition, &f);

            // Renumber
            for (int i = 0; i < groupItems.size(); ++i)
                groupItems[i]->position = i;

            save();
            emit favoriteUpdated(id);
            emit dataChanged();
            return;
        }
    }
}

bool FavoritesManager::isFavorited(const QString &url) const
{
    for (const auto &f : m_favorites) {
        if (f.url == url)
            return true;
    }
    return false;
}

QString FavoritesManager::favoriteIdForUrl(const QString &url) const
{
    for (const auto &f : m_favorites) {
        if (f.url == url)
            return f.id;
    }
    return QString();
}

FavoriteItem FavoritesManager::favorite(const QString &id) const
{
    for (const auto &f : m_favorites) {
        if (f.id == id)
            return f;
    }
    return FavoriteItem();
}

QList<FavoriteItem> FavoritesManager::favorites() const
{
    auto sorted = m_favorites;
    std::sort(sorted.begin(), sorted.end(), [](const FavoriteItem &a, const FavoriteItem &b) {
        if (a.pinned != b.pinned) return a.pinned > b.pinned;
        return a.position < b.position;
    });
    return sorted;
}

QList<FavoriteItem> FavoritesManager::favoritesInGroup(const QString &groupId) const
{
    QList<FavoriteItem> result;
    for (const auto &f : m_favorites) {
        if (f.groupId == groupId)
            result.append(f);
    }
    std::sort(result.begin(), result.end(), [](const FavoriteItem &a, const FavoriteItem &b) {
        if (a.pinned != b.pinned) return a.pinned > b.pinned;
        return a.position < b.position;
    });
    return result;
}

QList<FavoriteItem> FavoritesManager::pinnedFavorites() const
{
    QList<FavoriteItem> result;
    for (const auto &f : m_favorites) {
        if (f.pinned)
            result.append(f);
    }
    std::sort(result.begin(), result.end(), [](const FavoriteItem &a, const FavoriteItem &b) {
        return a.position < b.position;
    });
    return result;
}

// --- Groups ---

QString FavoritesManager::addGroup(const QString &name)
{
    FavoriteGroup grp;
    grp.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    grp.name = name;
    grp.pinned = false;
    grp.collapsed = false;
    grp.createdAt = QDateTime::currentDateTimeUtc();

    int maxPos = -1;
    for (const auto &g : m_groups) {
        if (g.position > maxPos)
            maxPos = g.position;
    }
    grp.position = maxPos + 1;

    m_groups.append(grp);
    save();

    emit groupAdded(grp.id);
    emit dataChanged();
    qDebug() << "FavoritesManager: Added group" << grp.name;
    return grp.id;
}

void FavoritesManager::removeGroup(const QString &id, bool deleteItems)
{
    for (int i = 0; i < m_groups.size(); ++i) {
        if (m_groups[i].id == id) {
            m_groups.removeAt(i);

            if (deleteItems) {
                m_favorites.erase(
                    std::remove_if(m_favorites.begin(), m_favorites.end(),
                                   [&id](const FavoriteItem &f) { return f.groupId == id; }),
                    m_favorites.end());
            } else {
                // Move items to ungrouped
                for (auto &f : m_favorites) {
                    if (f.groupId == id)
                        f.groupId.clear();
                }
            }

            save();
            emit groupRemoved(id);
            emit dataChanged();
            qDebug() << "FavoritesManager: Removed group" << id;
            return;
        }
    }
}

void FavoritesManager::renameGroup(const QString &id, const QString &name)
{
    for (auto &g : m_groups) {
        if (g.id == id) {
            g.name = name;
            save();
            emit groupUpdated(id);
            emit dataChanged();
            return;
        }
    }
}

void FavoritesManager::pinGroup(const QString &id, bool pinned)
{
    for (auto &g : m_groups) {
        if (g.id == id) {
            g.pinned = pinned;
            save();
            emit groupUpdated(id);
            emit dataChanged();
            return;
        }
    }
}

void FavoritesManager::reorderGroup(const QString &id, int newPosition)
{
    QList<FavoriteGroup *> sorted;
    for (auto &g : m_groups)
        sorted.append(&g);
    std::sort(sorted.begin(), sorted.end(),
              [](const FavoriteGroup *a, const FavoriteGroup *b) { return a->position < b->position; });

    FavoriteGroup *target = nullptr;
    for (auto *g : sorted) {
        if (g->id == id) { target = g; break; }
    }
    if (!target) return;

    sorted.removeOne(target);
    if (newPosition < 0) newPosition = 0;
    if (newPosition > sorted.size()) newPosition = sorted.size();
    sorted.insert(newPosition, target);

    for (int i = 0; i < sorted.size(); ++i)
        sorted[i]->position = i;

    save();
    emit groupUpdated(id);
    emit dataChanged();
}

void FavoritesManager::setGroupCollapsed(const QString &id, bool collapsed)
{
    for (auto &g : m_groups) {
        if (g.id == id) {
            g.collapsed = collapsed;
            save();
            emit groupUpdated(id);
            emit dataChanged();
            return;
        }
    }
}

FavoriteGroup FavoritesManager::group(const QString &id) const
{
    for (const auto &g : m_groups) {
        if (g.id == id)
            return g;
    }
    return FavoriteGroup();
}

QList<FavoriteGroup> FavoritesManager::groups() const
{
    auto sorted = m_groups;
    std::sort(sorted.begin(), sorted.end(), [](const FavoriteGroup &a, const FavoriteGroup &b) {
        if (a.pinned != b.pinned) return a.pinned > b.pinned;
        return a.position < b.position;
    });
    return sorted;
}

QList<FavoriteGroup> FavoritesManager::pinnedGroups() const
{
    QList<FavoriteGroup> result;
    for (const auto &g : m_groups) {
        if (g.pinned)
            result.append(g);
    }
    std::sort(result.begin(), result.end(), [](const FavoriteGroup &a, const FavoriteGroup &b) {
        return a.position < b.position;
    });
    return result;
}

// --- Persistence ---

void FavoritesManager::setStoragePath(const QString &path)
{
    m_storagePath = path;
}

QString FavoritesManager::storagePath() const
{
    if (!m_storagePath.isEmpty())
        return m_storagePath;

    QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
                        + "/scowser";
    QDir().mkpath(configDir);
    return configDir + "/favorites.json";
}

void FavoritesManager::load()
{
    QFile file(storagePath());
    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject())
        return;

    QJsonObject root = doc.object();

    // Load groups
    m_groups.clear();
    for (const auto &val : root["groups"].toArray()) {
        QJsonObject obj = val.toObject();
        FavoriteGroup g;
        g.id = obj["id"].toString();
        g.name = obj["name"].toString();
        g.pinned = obj["pinned"].toBool();
        g.position = obj["position"].toInt();
        g.collapsed = obj["collapsed"].toBool();
        g.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        if (!g.id.isEmpty())
            m_groups.append(g);
    }

    // Load favorites
    m_favorites.clear();
    for (const auto &val : root["favorites"].toArray()) {
        QJsonObject obj = val.toObject();
        FavoriteItem f;
        f.id = obj["id"].toString();
        f.url = obj["url"].toString();
        f.title = obj["title"].toString();
        f.faviconPng = QByteArray::fromBase64(obj["favicon"].toString().toLatin1());
        f.groupId = obj["groupId"].toString();
        f.pinned = obj["pinned"].toBool();
        f.position = obj["position"].toInt();
        f.createdAt = QDateTime::fromString(obj["createdAt"].toString(), Qt::ISODate);
        if (!f.id.isEmpty())
            m_favorites.append(f);
    }
}

void FavoritesManager::save()
{
    QJsonObject root;

    // Save groups
    QJsonArray groupsArr;
    for (const auto &g : m_groups) {
        QJsonObject obj;
        obj["id"] = g.id;
        obj["name"] = g.name;
        obj["pinned"] = g.pinned;
        obj["position"] = g.position;
        obj["collapsed"] = g.collapsed;
        obj["createdAt"] = g.createdAt.toString(Qt::ISODate);
        groupsArr.append(obj);
    }
    root["groups"] = groupsArr;

    // Save favorites
    QJsonArray favsArr;
    for (const auto &f : m_favorites) {
        QJsonObject obj;
        obj["id"] = f.id;
        obj["url"] = f.url;
        obj["title"] = f.title;
        if (!f.faviconPng.isEmpty())
            obj["favicon"] = QString::fromLatin1(f.faviconPng.toBase64());
        obj["groupId"] = f.groupId;
        obj["pinned"] = f.pinned;
        obj["position"] = f.position;
        obj["createdAt"] = f.createdAt.toString(Qt::ISODate);
        favsArr.append(obj);
    }
    root["favorites"] = favsArr;

    QFile file(storagePath());
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    }
}

void FavoritesManager::normalizePositions(QList<FavoriteItem> &items)
{
    std::sort(items.begin(), items.end(), [](const FavoriteItem &a, const FavoriteItem &b) {
        return a.position < b.position;
    });
    for (int i = 0; i < items.size(); ++i)
        items[i].position = i;
}

void FavoritesManager::normalizeGroupPositions()
{
    std::sort(m_groups.begin(), m_groups.end(), [](const FavoriteGroup &a, const FavoriteGroup &b) {
        return a.position < b.position;
    });
    for (int i = 0; i < m_groups.size(); ++i)
        m_groups[i].position = i;
}
