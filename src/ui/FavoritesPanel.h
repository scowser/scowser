#pragma once

#include <QDockWidget>
#include <QTreeWidget>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>

class FavoritesManager;

class FavoritesPanel : public QDockWidget {
    Q_OBJECT

public:
    explicit FavoritesPanel(FavoritesManager *manager, QWidget *parent = nullptr);

public slots:
    void rebuild();

signals:
    void favoriteActivated(const QString &url);

private:
    void setupUI();
    void buildTree();
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onItemContextMenu(const QPoint &pos);
    void onSearchTextChanged(const QString &text);

    // Context menu actions
    void openFavorite(QTreeWidgetItem *item);
    void renameFavorite(QTreeWidgetItem *item);
    void deleteFavorite(QTreeWidgetItem *item);
    void pinFavorite(QTreeWidgetItem *item);
    void moveFavoriteToGroup(QTreeWidgetItem *item, const QString &groupId);

    void createGroup();
    void renameGroup(QTreeWidgetItem *item);
    void deleteGroup(QTreeWidgetItem *item);
    void pinGroup(QTreeWidgetItem *item);

    // Drag-and-drop reordering
    void onDropCompleted();

    void setFavicon(QTreeWidgetItem *item, const QByteArray &pngData);

    FavoritesManager *m_manager;
    QTreeWidget *m_tree;
    QToolButton *m_closeButton;
    QToolButton *m_addGroupButton;
    QLineEdit *m_searchBox;
    QString m_searchFilter;
    bool m_building = false;

    static constexpr int RoleItemId = Qt::UserRole;
    static constexpr int RoleItemType = Qt::UserRole + 1;
    static constexpr int RoleUrl = Qt::UserRole + 2;
    enum ItemType { TypeFavorite = 0, TypeGroup = 1 };
};
