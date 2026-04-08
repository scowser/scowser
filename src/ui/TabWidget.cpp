#include "ui/TabWidget.h"
#include "ui/ScrollableTabBar.h"

#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QColor>
#include <QToolButton>
#include <QIcon>
#include <QTabBar>
#include <QResizeEvent>
#include <QMenu>

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setTabBar(new ScrollableTabBar(this));
    setTabsClosable(true);
    setMovable(true);
    setDocumentMode(false);
    setElideMode(Qt::ElideRight);
    tabBar()->setExpanding(false);

    m_newTabButton = new QToolButton(this);
    m_newTabButton->setObjectName("newTabButton");
    m_newTabButton->setIcon(QIcon(":/icons/new-tab.svg"));
    m_newTabButton->setToolTip("New Tab");
    m_newTabButton->setAutoRaise(true);
    m_newTabButton->setFixedSize(28, 28);
    connect(m_newTabButton, &QToolButton::clicked, this, &TabWidget::newTabRequested);

    // Tab bar context menu
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), &QTabBar::customContextMenuRequested,
            this, &TabWidget::showTabContextMenu);

    QMetaObject::invokeMethod(this, &TabWidget::repositionNewTabButton, Qt::QueuedConnection);
}

QWebEngineView *TabWidget::createTab()
{
    auto *profile = QWebEngineProfile::defaultProfile();
    auto *page = new QWebEnginePage(profile, this);
    page->setBackgroundColor(QColor("#1e1e2e"));
    auto *view = new QWebEngineView(this);
    view->setPage(page);

    int index = addTab(view, "New Tab");
    setCurrentIndex(index);

    repositionNewTabButton();
    return view;
}

QWebEngineView *TabWidget::currentWebView() const
{
    return qobject_cast<QWebEngineView *>(currentWidget());
}

QWebEngineView *TabWidget::webView(int index) const
{
    return qobject_cast<QWebEngineView *>(widget(index));
}

void TabWidget::closeTab(int index)
{
    auto *view = webView(index);
    if (!view) return;

    removeTab(index);
    view->deleteLater();

    repositionNewTabButton();
}

void TabWidget::resizeEvent(QResizeEvent *event)
{
    QTabWidget::resizeEvent(event);
    repositionNewTabButton();
}

void TabWidget::repositionNewTabButton()
{
    auto *bar = tabBar();
    int x = width() - m_newTabButton->width() - 4;
    int y = bar->y() + (bar->height() - m_newTabButton->height()) / 2;
    m_newTabButton->move(x, y);
    m_newTabButton->raise();
}

void TabWidget::showTabContextMenu(const QPoint &pos)
{
    int index = tabBar()->tabAt(pos);
    if (index < 0) return;

    auto *view = webView(index);
    if (!view) return;

    QUrl url = view->url();
    if (url.isEmpty() || url.scheme() == "about") return;

    QMenu menu(this);

    // Check if this tab is using a persistent profile
    bool isPersistent = view->page()->profile() != QWebEngineProfile::defaultProfile();

    if (isPersistent) {
        auto *unsaveAction = menu.addAction("Remove from Saved Session");
        connect(unsaveAction, &QAction::triggered, this, [this, index]() {
            emit unsaveSessionRequested(index);
        });
    } else {
        auto *saveAction = menu.addAction("Save Session");
        connect(saveAction, &QAction::triggered, this, [this, index]() {
            emit saveSessionRequested(index);
        });
    }

    menu.exec(tabBar()->mapToGlobal(pos));
}
