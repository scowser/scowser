#include "ui/TabWidget.h"

#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QToolButton>

TabWidget::TabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    setTabsClosable(true);
    setMovable(true);
    setDocumentMode(true);
    setElideMode(Qt::ElideRight);

    // "New tab" button
    auto *newTabButton = new QToolButton(this);
    newTabButton->setText("+");
    newTabButton->setAutoRaise(true);
    setCornerWidget(newTabButton, Qt::TopRightCorner);
    connect(newTabButton, &QToolButton::clicked, this, &TabWidget::newTabRequested);
}

QWebEngineView *TabWidget::createTab()
{
    // Use off-the-record profile for ephemeral browsing
    auto *profile = QWebEngineProfile::defaultProfile();
    auto *page = new QWebEnginePage(profile, this);
    auto *view = new QWebEngineView(this);
    view->setPage(page);

    int index = addTab(view, "New Tab");
    setCurrentIndex(index);

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
}
