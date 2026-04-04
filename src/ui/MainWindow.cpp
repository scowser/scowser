#include "ui/MainWindow.h"
#include "ui/TabWidget.h"
#include "ui/AddressBar.h"

#include <QToolBar>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QIcon>
#include <QSize>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupToolBar();
    setupShortcuts();

    // Open a blank tab on startup
    onNewTab();

    resize(1280, 800);
    setWindowTitle("scowser");
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    m_tabWidget = new TabWidget(this);
    setCentralWidget(m_tabWidget);

    connect(m_tabWidget, &TabWidget::currentChanged, this, &MainWindow::onCurrentTabChanged);
    connect(m_tabWidget, &TabWidget::tabCloseRequested, this, &MainWindow::onCloseTab);
    connect(m_tabWidget, &TabWidget::newTabRequested, this, &MainWindow::onNewTab);

    statusBar()->showMessage("Ready");
}

void MainWindow::setupToolBar()
{
    m_navToolBar = addToolBar("Navigation");
    m_navToolBar->setMovable(false);
    m_navToolBar->setIconSize(QSize(16, 16));

    auto *backAction = m_navToolBar->addAction(QIcon(":/icons/back.svg"), "Back", [this]() {
        if (auto *view = m_tabWidget->currentWebView())
            view->back();
    });
    backAction->setToolTip("Back");

    auto *forwardAction = m_navToolBar->addAction(QIcon(":/icons/forward.svg"), "Forward", [this]() {
        if (auto *view = m_tabWidget->currentWebView())
            view->forward();
    });
    forwardAction->setToolTip("Forward");

    auto *reloadAction = m_navToolBar->addAction(QIcon(":/icons/reload.svg"), "Reload", [this]() {
        if (auto *view = m_tabWidget->currentWebView())
            view->reload();
    });
    reloadAction->setToolTip("Reload");

    m_addressBar = new AddressBar(this);
    m_navToolBar->addWidget(m_addressBar);
    connect(m_addressBar, &AddressBar::urlEntered, this, &MainWindow::onNavigate);
}

void MainWindow::setupShortcuts()
{
    // New tab
    auto *newTabShortcut = new QShortcut(QKeySequence::AddTab, this);
    connect(newTabShortcut, &QShortcut::activated, this, &MainWindow::onNewTab);

    // Close tab
    auto *closeTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this);
    connect(closeTabShortcut, &QShortcut::activated, [this]() {
        onCloseTab(m_tabWidget->currentIndex());
    });

    // Focus address bar
    auto *focusUrlShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);
    connect(focusUrlShortcut, &QShortcut::activated, [this]() {
        m_addressBar->setFocus();
        m_addressBar->selectAll();
    });

    // Reload
    auto *reloadShortcut = new QShortcut(QKeySequence::Refresh, this);
    connect(reloadShortcut, &QShortcut::activated, [this]() {
        if (auto *view = m_tabWidget->currentWebView())
            view->reload();
    });
}

void MainWindow::connectTab(QWebEngineView *view)
{
    connect(view, &QWebEngineView::titleChanged, this, &MainWindow::onTabTitleChanged);
    connect(view, &QWebEngineView::urlChanged, this, &MainWindow::onTabUrlChanged);
    connect(view, &QWebEngineView::loadProgress, this, &MainWindow::onLoadProgress);
}

void MainWindow::onNavigate(const QUrl &url)
{
    if (auto *view = m_tabWidget->currentWebView()) {
        view->load(url);
    }
}

void MainWindow::onCurrentTabChanged(int index)
{
    if (auto *view = m_tabWidget->webView(index)) {
        m_addressBar->setUrl(view->url());
        setWindowTitle(view->title().isEmpty() ? "scowser" : view->title() + " — scowser");
    }
}

void MainWindow::onTabTitleChanged(const QString &title)
{
    auto *view = qobject_cast<QWebEngineView *>(sender());
    if (!view) return;

    int index = m_tabWidget->indexOf(view);
    if (index >= 0) {
        m_tabWidget->setTabText(index, title.isEmpty() ? "New Tab" : title);
    }

    if (m_tabWidget->currentWebView() == view) {
        setWindowTitle(title.isEmpty() ? "scowser" : title + " — scowser");
    }
}

void MainWindow::onTabUrlChanged(const QUrl &url)
{
    auto *view = qobject_cast<QWebEngineView *>(sender());
    if (view && view == m_tabWidget->currentWebView()) {
        m_addressBar->setUrl(url);
    }
}

void MainWindow::onLoadProgress(int progress)
{
    if (progress < 100) {
        statusBar()->showMessage(QString("Loading... %1%").arg(progress));
    } else {
        statusBar()->showMessage("Ready");
    }
}

void MainWindow::onNewTab()
{
    auto *view = m_tabWidget->createTab();
    connectTab(view);
    view->load(QUrl("about:blank"));
}

void MainWindow::onCloseTab(int index)
{
    if (m_tabWidget->count() <= 1) {
        // Don't close the last tab, just clear it
        if (auto *view = m_tabWidget->webView(0)) {
            view->load(QUrl("about:blank"));
        }
        return;
    }
    m_tabWidget->closeTab(index);
}
