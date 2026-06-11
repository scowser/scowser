#include "ui/DevToolsPanel.h"

#include <QVBoxLayout>
#include <QMainWindow>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QDebug>

DevToolsPanel::DevToolsPanel(QWidget *parent)
    : QDockWidget("DevTools", parent)
{
    setObjectName("devToolsPanel");
    setupUI();

    // Default to bottom dock (horizontal pane, like browser devtools)
    setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    if (auto *mainWin = qobject_cast<QMainWindow *>(parent)) {
        mainWin->addDockWidget(Qt::BottomDockWidgetArea, this);
    }
}

DevToolsPanel::~DevToolsPanel()
{
    detach();
}

void DevToolsPanel::setupUI()
{
    // Chrome-less: the panel is just the inspector view, no toolbar or title.
    // The DevTools frontend has its own close/dock controls; the panel is
    // toggled via F12 / Ctrl+Shift+I / View menu.
    m_container = new QWidget(this);
    m_container->setObjectName("devToolsPanelContainer");
    auto *layout = new QVBoxLayout(m_container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    setWidget(m_container);

    // Hide the default dock widget title bar
    setTitleBarWidget(new QWidget(this));
}

void DevToolsPanel::ensureDevToolsView()
{
    if (m_devToolsView)
        return;

    // The DevTools frontend (devtools:// scheme) is created lazily so that
    // no extra renderer process exists until the panel is first used.
    m_devToolsView = new QWebEngineView(m_container);
    m_devToolsView->setObjectName("devToolsView");

    auto *page = new QWebEnginePage(QWebEngineProfile::defaultProfile(), m_devToolsView);
    page->setBackgroundColor(QColor("#1e1e2e"));

    // The inspector frontend requires JavaScript and local storage even when
    // the user has disabled them globally for web content.
    page->settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    page->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);

    m_devToolsView->setPage(page);
    m_container->layout()->addWidget(m_devToolsView);
}

void DevToolsPanel::attachToPage(QWebEnginePage *page)
{
    if (m_inspectedPage == page)
        return;

    detach();
    if (!page)
        return;

    ensureDevToolsView();

    m_inspectedPage = page;
    m_devToolsView->page()->setInspectedPage(page);
    connect(page, &QObject::destroyed, this, &DevToolsPanel::onInspectedPageDestroyed);

    qDebug() << "DevToolsPanel: Inspecting" << page->url().toDisplayString();
}

void DevToolsPanel::detach()
{
    if (!m_inspectedPage)
        return;

    disconnect(m_inspectedPage, &QObject::destroyed, this, nullptr);
    if (m_devToolsView && m_devToolsView->page())
        m_devToolsView->page()->setInspectedPage(nullptr);
    m_inspectedPage = nullptr;
}

void DevToolsPanel::onInspectedPageDestroyed()
{
    // The page is already being destroyed — do not touch it, just drop the
    // reference so a later attach starts clean.
    m_inspectedPage = nullptr;
    if (m_devToolsView && m_devToolsView->page())
        m_devToolsView->page()->setInspectedPage(nullptr);
}

QWebEnginePage *DevToolsPanel::inspectedPage() const
{
    return m_inspectedPage;
}

bool DevToolsPanel::isDockedBottom() const
{
    auto *mainWin = qobject_cast<QMainWindow *>(parentWidget());
    if (!mainWin) return false;
    return mainWin->dockWidgetArea(const_cast<DevToolsPanel *>(this)) == Qt::BottomDockWidgetArea;
}

void DevToolsPanel::toggleDockOrientation()
{
    auto *mainWin = qobject_cast<QMainWindow *>(parentWidget());
    if (!mainWin) return;

    if (isDockedBottom()) {
        mainWin->addDockWidget(Qt::RightDockWidgetArea, this);
    } else {
        mainWin->addDockWidget(Qt::BottomDockWidgetArea, this);
    }
    // Defer until the new dock layout has settled, otherwise the size check
    // reads the pre-move geometry and skips the expansion
    QMetaObject::invokeMethod(this, &DevToolsPanel::ensureUsableSize, Qt::QueuedConnection);
}

void DevToolsPanel::showEvent(QShowEvent *event)
{
    QDockWidget::showEvent(event);
    // Defer until the dock layout has settled, then expand if collapsed
    QMetaObject::invokeMethod(this, &DevToolsPanel::ensureUsableSize, Qt::QueuedConnection);
}

void DevToolsPanel::ensureUsableSize()
{
    auto *mainWin = qobject_cast<QMainWindow *>(parentWidget());
    if (!mainWin || !isVisible()) return;

    // The empty container gives the dock a near-zero size hint, so the first
    // show would allocate a sliver. Bottom pane: expand a collapsed pane to
    // ~40% of the window height; deliberate user sizes above the threshold
    // are left alone. Right pane: Qt re-docks it narrow, so always grow to
    // 30% of the window width (larger user sizes are kept).
    if (isDockedBottom()) {
        constexpr int collapsedThreshold = 150;
        if (height() < collapsedThreshold)
            mainWin->resizeDocks({this}, {mainWin->height() * 2 / 5}, Qt::Vertical);
    } else {
        int target = mainWin->width() * 3 / 10;
        if (width() < target)
            mainWin->resizeDocks({this}, {target}, Qt::Horizontal);
    }
}
