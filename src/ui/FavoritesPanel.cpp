#include "ui/FavoritesPanel.h"
#include "app/FavoritesManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QIcon>
#include <QFont>
#include <QApplication>
#include <QSize>
#include <QPixmap>

// --- Custom input dialog with proper sizing and dark theme ---

static QString showNameDialog(QWidget *parent, const QString &title,
                               const QString &label, const QString &initial = QString())
{
    QDialog dlg(parent);
    dlg.setWindowTitle(title);
    dlg.setMinimumWidth(360);

    auto *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(20, 16, 20, 16);
    layout->setSpacing(12);

    auto *lbl = new QLabel(label, &dlg);
    lbl->setObjectName("favDialogLabel");
    layout->addWidget(lbl);

    auto *edit = new QLineEdit(&dlg);
    edit->setObjectName("favDialogInput");
    edit->setText(initial);
    edit->selectAll();
    edit->setMinimumHeight(32);
    layout->addWidget(edit);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    buttons->setObjectName("favDialogButtons");
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    layout->addWidget(buttons);

    edit->setFocus();

    if (dlg.exec() == QDialog::Accepted && !edit->text().trimmed().isEmpty())
        return edit->text().trimmed();
    return QString();
}

// --- FavoritesPanel ---

FavoritesPanel::FavoritesPanel(FavoritesManager *manager, QWidget *parent)
    : QDockWidget("Favorites", parent)
    , m_manager(manager)
{
    setObjectName("favoritesPanel");
    setupUI();

    setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea | Qt::LeftDockWidgetArea);
    if (auto *mainWin = qobject_cast<QMainWindow *>(parent)) {
        mainWin->addDockWidget(Qt::LeftDockWidgetArea, this);
    }

    connect(m_manager, &FavoritesManager::dataChanged, this, &FavoritesPanel::rebuild);
    buildTree();
}

void FavoritesPanel::setupUI()
{
    auto *container = new QWidget(this);
    container->setObjectName("favoritesPanelContainer");
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Toolbar
    auto *toolbar = new QWidget(container);
    toolbar->setObjectName("favoritesPanelToolbar");
    auto *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(8, 4, 8, 4);
    toolbarLayout->setSpacing(4);

    auto *titleLabel = new QLabel("Favorites", toolbar);
    titleLabel->setObjectName("favoritesPanelTitle");
    toolbarLayout->addWidget(titleLabel);
    toolbarLayout->addStretch();

    m_addGroupButton = new QToolButton(toolbar);
    m_addGroupButton->setObjectName("favAddGroupButton");
    m_addGroupButton->setText("+ Group");
    m_addGroupButton->setToolTip("Create a new group");
    connect(m_addGroupButton, &QToolButton::clicked, this, &FavoritesPanel::createGroup);
    toolbarLayout->addWidget(m_addGroupButton);

    m_closeButton = new QToolButton(toolbar);
    m_closeButton->setObjectName("favCloseButton");
    m_closeButton->setIcon(QIcon(":/icons/close.svg"));
    m_closeButton->setToolTip("Close panel");
    connect(m_closeButton, &QToolButton::clicked, this, &QDockWidget::hide);
    toolbarLayout->addWidget(m_closeButton);

    layout->addWidget(toolbar);

    // Search box
    m_searchBox = new QLineEdit(container);
    m_searchBox->setObjectName("favSearchBox");
    m_searchBox->setPlaceholderText("Search favorites...");
    m_searchBox->setClearButtonEnabled(true);
    connect(m_searchBox, &QLineEdit::textChanged, this, &FavoritesPanel::onSearchTextChanged);
    layout->addWidget(m_searchBox);

    // Tree widget — no branch decorations, we use text arrows instead
    m_tree = new QTreeWidget(container);
    m_tree->setObjectName("favTree");
    m_tree->setIconSize(QSize(16, 16));
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(false);
    m_tree->setAnimated(true);
    m_tree->setIndentation(16);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setDragDropMode(QAbstractItemView::InternalMove);
    m_tree->setDragEnabled(true);
    m_tree->setAcceptDrops(true);
    m_tree->setDropIndicatorShown(true);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &FavoritesPanel::onItemDoubleClicked);
    connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &FavoritesPanel::onItemContextMenu);
    connect(m_tree, &QTreeWidget::itemClicked, this, [](QTreeWidgetItem *item, int) {
        if (!item) return;
        if (item->data(0, Qt::UserRole + 1).toInt() == 1) { // TypeGroup
            item->setExpanded(!item->isExpanded());
        }
    });
    connect(m_tree->model(), &QAbstractItemModel::rowsMoved, this, &FavoritesPanel::onDropCompleted);

    layout->addWidget(m_tree);

    setWidget(container);
    setTitleBarWidget(new QWidget(this));
}

void FavoritesPanel::buildTree()
{
    // Guard against re-entrant rebuilds
    if (m_building)
        return;
    m_building = true;

    m_tree->clear();

    auto groups = m_manager->groups();
    auto ungrouped = m_manager->favoritesInGroup(QString());

    // Pinned ungrouped items first
    for (const auto &fav : ungrouped) {
        if (!fav.pinned) continue;
        if (!m_searchFilter.isEmpty() &&
            !fav.title.contains(m_searchFilter, Qt::CaseInsensitive) &&
            !fav.url.contains(m_searchFilter, Qt::CaseInsensitive))
            continue;

        auto *item = new QTreeWidgetItem(m_tree);
        item->setText(0, QString::fromUtf8("\xf0\x9f\x93\x8c ") + fav.title);
        item->setToolTip(0, fav.url);
        item->setData(0, RoleItemId, fav.id);
        item->setData(0, RoleItemType, TypeFavorite);
        item->setData(0, RoleUrl, fav.url);
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
        setFavicon(item, fav.faviconPng);
    }

    // Groups
    for (const auto &grp : groups) {
        auto favs = m_manager->favoritesInGroup(grp.id);

        // If searching, skip groups with no matching items
        bool hasMatch = m_searchFilter.isEmpty();
        if (!hasMatch) {
            for (const auto &fav : favs) {
                if (fav.title.contains(m_searchFilter, Qt::CaseInsensitive) ||
                    fav.url.contains(m_searchFilter, Qt::CaseInsensitive)) {
                    hasMatch = true;
                    break;
                }
            }
            if (grp.name.contains(m_searchFilter, Qt::CaseInsensitive))
                hasMatch = true;
        }
        if (!hasMatch) continue;

        bool expanded = !grp.collapsed;
        // When searching, always expand groups to show results
        if (!m_searchFilter.isEmpty())
            expanded = true;

        auto *groupItem = new QTreeWidgetItem(m_tree);
        QString arrow = expanded ? QString::fromUtf8("\xe2\x96\xbe ") : QString::fromUtf8("\xe2\x96\xb8 ");
        QString pin = grp.pinned ? QString::fromUtf8("\xf0\x9f\x93\x8c ") : QString();
        groupItem->setText(0, arrow + pin + grp.name + QString("  (%1)").arg(favs.size()));
        groupItem->setData(0, RoleItemId, grp.id);
        groupItem->setData(0, RoleItemType, TypeGroup);
        groupItem->setFlags(groupItem->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

        QFont groupFont = groupItem->font(0);
        groupFont.setBold(true);
        groupItem->setFont(0, groupFont);

        for (const auto &fav : favs) {
            if (!m_searchFilter.isEmpty() &&
                !fav.title.contains(m_searchFilter, Qt::CaseInsensitive) &&
                !fav.url.contains(m_searchFilter, Qt::CaseInsensitive))
                continue;

            auto *favItem = new QTreeWidgetItem(groupItem);
            QString favPin = fav.pinned ? QString::fromUtf8("\xf0\x9f\x93\x8c ") : QString();
            favItem->setText(0, favPin + fav.title);
            favItem->setToolTip(0, fav.url);
            favItem->setData(0, RoleItemId, fav.id);
            favItem->setData(0, RoleItemType, TypeFavorite);
            favItem->setData(0, RoleUrl, fav.url);
            favItem->setFlags(favItem->flags() | Qt::ItemIsDragEnabled);
            setFavicon(favItem, fav.faviconPng);
        }

        groupItem->setExpanded(expanded);
    }

    // Non-pinned ungrouped items
    for (const auto &fav : ungrouped) {
        if (fav.pinned) continue;
        if (!m_searchFilter.isEmpty() &&
            !fav.title.contains(m_searchFilter, Qt::CaseInsensitive) &&
            !fav.url.contains(m_searchFilter, Qt::CaseInsensitive))
            continue;

        auto *item = new QTreeWidgetItem(m_tree);
        item->setText(0, fav.title);
        item->setToolTip(0, fav.url);
        item->setData(0, RoleItemId, fav.id);
        item->setData(0, RoleItemType, TypeFavorite);
        item->setData(0, RoleUrl, fav.url);
        item->setFlags(item->flags() | Qt::ItemIsDragEnabled);
        setFavicon(item, fav.faviconPng);
    }

    m_building = false;
}

void FavoritesPanel::rebuild()
{
    buildTree();
}

void FavoritesPanel::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item) return;

    int type = item->data(0, RoleItemType).toInt();
    if (type == TypeFavorite) {
        openFavorite(item);
    }
    // Groups toggle on single click (handled in setupUI)
}

void FavoritesPanel::onItemContextMenu(const QPoint &pos)
{
    auto *item = m_tree->itemAt(pos);
    auto *menu = new QMenu(this);

    if (!item) {
        // Right-click on empty space
        menu->addAction("New Group", this, &FavoritesPanel::createGroup);
        menu->exec(m_tree->viewport()->mapToGlobal(pos));
        menu->deleteLater();
        return;
    }

    int type = item->data(0, RoleItemType).toInt();

    if (type == TypeFavorite) {
        menu->addAction("Open", [this, item]() { openFavorite(item); });
        menu->addSeparator();
        menu->addAction("Rename", [this, item]() { renameFavorite(item); });

        // Pin/unpin
        QString fid = item->data(0, RoleItemId).toString();
        auto fav = m_manager->favorite(fid);
        menu->addAction(fav.pinned ? "Unpin" : "Pin", [this, item]() { pinFavorite(item); });

        // Move to group submenu
        auto groups = m_manager->groups();
        if (!groups.isEmpty() || !fav.groupId.isEmpty()) {
            auto *moveMenu = menu->addMenu("Move to...");
            if (!fav.groupId.isEmpty()) {
                moveMenu->addAction("Ungrouped", [this, item]() {
                    moveFavoriteToGroup(item, QString());
                });
                moveMenu->addSeparator();
            }
            for (const auto &grp : groups) {
                if (grp.id == fav.groupId) continue;
                moveMenu->addAction(grp.name, [this, item, gid = grp.id]() {
                    moveFavoriteToGroup(item, gid);
                });
            }
        }

        menu->addSeparator();
        menu->addAction("Delete", [this, item]() { deleteFavorite(item); });

    } else if (type == TypeGroup) {
        menu->addAction("Rename", [this, item]() { renameGroup(item); });

        QString gid = item->data(0, RoleItemId).toString();
        auto grp = m_manager->group(gid);
        menu->addAction(grp.pinned ? "Unpin" : "Pin", [this, item]() { pinGroup(item); });

        menu->addSeparator();
        menu->addAction("Delete Group (keep items)", [this, item]() {
            deleteGroup(item);
        });
        menu->addAction("Delete Group and Items", [this, item]() {
            QString gid = item->data(0, RoleItemId).toString();
            auto grp = m_manager->group(gid);
            auto reply = QMessageBox::question(this, "Delete Group",
                QString("Delete group \"%1\" and all its favorites?").arg(grp.name),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes)
                m_manager->removeGroup(gid, true);
        });
    }

    menu->exec(m_tree->viewport()->mapToGlobal(pos));
    menu->deleteLater();
}

void FavoritesPanel::onSearchTextChanged(const QString &text)
{
    m_searchFilter = text.trimmed();
    buildTree();
}

void FavoritesPanel::openFavorite(QTreeWidgetItem *item)
{
    QString url = item->data(0, RoleUrl).toString();
    if (!url.isEmpty())
        emit favoriteActivated(url);
}

void FavoritesPanel::renameFavorite(QTreeWidgetItem *item)
{
    QString fid = item->data(0, RoleItemId).toString();
    auto fav = m_manager->favorite(fid);

    QString newName = showNameDialog(this, "Rename Favorite", "Name:", fav.title);
    if (!newName.isEmpty())
        m_manager->renameFavorite(fid, newName);
}

void FavoritesPanel::deleteFavorite(QTreeWidgetItem *item)
{
    QString fid = item->data(0, RoleItemId).toString();
    m_manager->removeFavorite(fid);
}

void FavoritesPanel::pinFavorite(QTreeWidgetItem *item)
{
    QString fid = item->data(0, RoleItemId).toString();
    auto fav = m_manager->favorite(fid);
    m_manager->pinFavorite(fid, !fav.pinned);
}

void FavoritesPanel::moveFavoriteToGroup(QTreeWidgetItem *item, const QString &groupId)
{
    QString fid = item->data(0, RoleItemId).toString();
    m_manager->moveFavorite(fid, groupId);
}

void FavoritesPanel::createGroup()
{
    QString name = showNameDialog(this, "New Group", "Group name:");
    if (!name.isEmpty())
        m_manager->addGroup(name);
}

void FavoritesPanel::renameGroup(QTreeWidgetItem *item)
{
    QString gid = item->data(0, RoleItemId).toString();
    auto grp = m_manager->group(gid);

    QString newName = showNameDialog(this, "Rename Group", "Name:", grp.name);
    if (!newName.isEmpty())
        m_manager->renameGroup(gid, newName);
}

void FavoritesPanel::deleteGroup(QTreeWidgetItem *item)
{
    QString gid = item->data(0, RoleItemId).toString();
    m_manager->removeGroup(gid, false);
}

void FavoritesPanel::pinGroup(QTreeWidgetItem *item)
{
    QString gid = item->data(0, RoleItemId).toString();
    auto grp = m_manager->group(gid);
    m_manager->pinGroup(gid, !grp.pinned);
}

void FavoritesPanel::setFavicon(QTreeWidgetItem *item, const QByteArray &pngData)
{
    if (pngData.isEmpty())
        return;

    QPixmap px;
    if (px.loadFromData(pngData, "PNG"))
        item->setIcon(0, QIcon(px));
}

void FavoritesPanel::onDropCompleted()
{
    // After drag-and-drop, read the new tree order and update the manager
    for (int i = 0; i < m_tree->topLevelItemCount(); ++i) {
        auto *item = m_tree->topLevelItem(i);
        int type = item->data(0, RoleItemType).toInt();
        QString id = item->data(0, RoleItemId).toString();

        if (type == TypeGroup) {
            m_manager->reorderGroup(id, i);

            // Update children positions within this group
            for (int j = 0; j < item->childCount(); ++j) {
                auto *child = item->child(j);
                QString childId = child->data(0, RoleItemId).toString();
                m_manager->moveFavorite(childId, id);
                m_manager->reorderFavorite(childId, j);
            }
        } else if (type == TypeFavorite) {
            // Top-level favorite — ungrouped
            m_manager->moveFavorite(id, QString());
            m_manager->reorderFavorite(id, i);
        }
    }
}
