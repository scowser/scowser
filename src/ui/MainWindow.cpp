#include "ui/MainWindow.h"
#include "ui/TabWidget.h"
#include "ui/AddressBar.h"
#include "ui/LogPanel.h"
#include "ui/FavoritesPanel.h"
#include "ui/DevToolsPanel.h"

#include <QToolBar>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QIcon>
#include <QSize>
#include <QToolButton>
#include <QMenuBar>
#include <QLabel>
#include <QTimer>
#include <QBuffer>
#include <QFile>
#include <QPixmap>
#include <QMessageBox>
#include "ui/AboutDialog.h"
#include "ui/SettingsDialog.h"
#include "ui/DownloadsDialog.h"
#include "app/Application.h"
#include "app/Settings.h"
#include "app/DownloadManager.h"
#include "app/FavoritesManager.h"
#include "security/SessionManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupShortcuts();

    // Apply initial search engine URL and connect for future changes
    auto *settings = Application::instance()->settings();
    m_addressBar->setSearchEngineUrl(settings->searchEngineUrl());
    connect(settings, &Settings::searchEngineUrlChanged,
            m_addressBar, &AddressBar::setSearchEngineUrl);

    restoreSavedSessions();

    // If no saved sessions were restored, open a new tab
    if (m_tabWidget->count() == 0)
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
    connect(m_tabWidget, &TabWidget::saveSessionRequested, this, &MainWindow::onSaveSessionRequested);
    connect(m_tabWidget, &TabWidget::unsaveSessionRequested, this, &MainWindow::onUnsaveSessionRequested);

    m_logPanel = new LogPanel(this);
    m_logPanel->hide();

    m_favoritesPanel = new FavoritesPanel(Application::instance()->favoritesManager(), this);
    m_favoritesPanel->hide();
    connect(m_favoritesPanel, &FavoritesPanel::favoriteActivated,
            this, &MainWindow::onFavoriteActivated);

    m_devToolsPanel = new DevToolsPanel(this);
    m_devToolsPanel->hide();

    // Floating status overlay (like Firefox) — no permanent status bar
    setStatusBar(nullptr);
    m_statusOverlay = new QLabel(this);
    m_statusOverlay->setObjectName("statusOverlay");
    m_statusOverlay->hide();
    m_statusOverlay->setWordWrap(false);

    m_statusTimer = new QTimer(this);
    m_statusTimer->setSingleShot(true);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::hideStatusOverlay);
}

void MainWindow::setupMenuBar()
{
    // On macOS, "About scowser" automatically goes into the app menu
    auto *aboutAction = new QAction("About scowser", this);
    aboutAction->setMenuRole(QAction::AboutRole);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // On macOS, PreferencesRole places this in the app menu automatically
    auto *settingsAction = new QAction("Preferences...", this);
    settingsAction->setMenuRole(QAction::PreferencesRole);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettingsDialog);

    // View menu
    auto *viewMenu = menuBar()->addMenu("View");

    auto *toggleFavoritesAction = new QAction("Show Favorites", this);
    toggleFavoritesAction->setCheckable(true);
    toggleFavoritesAction->setChecked(false);
    connect(toggleFavoritesAction, &QAction::triggered, this, &MainWindow::toggleFavoritesPanel);
    connect(m_favoritesPanel, &QDockWidget::visibilityChanged, toggleFavoritesAction, &QAction::setChecked);
    viewMenu->addAction(toggleFavoritesAction);

    auto *toggleLogAction = new QAction("Show Logs", this);
    toggleLogAction->setCheckable(true);
    toggleLogAction->setChecked(false);
    connect(toggleLogAction, &QAction::triggered, this, &MainWindow::toggleLogPanel);
    connect(m_logPanel, &QDockWidget::visibilityChanged, toggleLogAction, &QAction::setChecked);
    viewMenu->addAction(toggleLogAction);

    auto *toggleDevToolsAction = new QAction("Show DevTools", this);
    toggleDevToolsAction->setCheckable(true);
    toggleDevToolsAction->setChecked(false);
    toggleDevToolsAction->setShortcut(QKeySequence(Qt::Key_F12));
    connect(toggleDevToolsAction, &QAction::triggered, this, &MainWindow::toggleDevTools);
    connect(m_devToolsPanel, &QDockWidget::visibilityChanged, toggleDevToolsAction, &QAction::setChecked);
    viewMenu->addAction(toggleDevToolsAction);

    auto *devToolsDockAction = new QAction("Move DevTools to Right", this);
    connect(devToolsDockAction, &QAction::triggered, this, [this, devToolsDockAction]() {
        m_devToolsPanel->toggleDockOrientation();
        devToolsDockAction->setText(m_devToolsPanel->isDockedBottom()
            ? "Move DevTools to Right" : "Move DevTools to Bottom");
    });
    viewMenu->addAction(devToolsDockAction);

    auto *helpMenu = menuBar()->addMenu("Help");
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(settingsAction);
}

void MainWindow::showAboutDialog()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::showSettingsDialog()
{
    SettingsDialog dialog(Application::instance()->settings(), this);
    dialog.exec();
}

void MainWindow::showDownloadsDialog()
{
    DownloadsDialog dialog(Application::instance()->downloadManager(), this);
    dialog.exec();
}

void MainWindow::onActiveDownloadsChanged(int count)
{
    if (count > 0) {
        m_downloadButton->setToolTip(QString("Downloads (%1 active)").arg(count));
    } else {
        m_downloadButton->setToolTip("Downloads");
    }
}

void MainWindow::setupToolBar()
{
    m_navToolBar = addToolBar("Navigation");
    m_navToolBar->setMovable(false);
    m_navToolBar->setIconSize(QSize(16, 16));
    m_navToolBar->setFloatable(false);

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

    // Star button (right of address bar, before downloads)
    m_starButton = new QToolButton(this);
    m_starButton->setIcon(QIcon(":/icons/star.svg"));
    m_starButton->setToolTip("Add to favorites");
    m_starButton->setObjectName("starButton");
    connect(m_starButton, &QToolButton::clicked, this, &MainWindow::toggleFavoriteForCurrentPage);
    m_navToolBar->addWidget(m_starButton);

    // Download button (right of star)
    m_downloadButton = new QToolButton(this);
    m_downloadButton->setIcon(QIcon(":/icons/download.svg"));
    m_downloadButton->setToolTip("Downloads");
    m_downloadButton->setObjectName("downloadButton");
    connect(m_downloadButton, &QToolButton::clicked, this, &MainWindow::showDownloadsDialog);
    m_navToolBar->addWidget(m_downloadButton);

    // Update button when active download count changes
    auto *mgr = Application::instance()->downloadManager();
    connect(mgr, &DownloadManager::activeCountChanged, this, &MainWindow::onActiveDownloadsChanged);

    // Update star when favorites change
    auto *favMgr = Application::instance()->favoritesManager();
    connect(favMgr, &FavoritesManager::dataChanged, this, &MainWindow::updateStarButton);
}

void MainWindow::setupShortcuts()
{
    auto *newTabShortcut = new QShortcut(QKeySequence::AddTab, this);
    connect(newTabShortcut, &QShortcut::activated, this, &MainWindow::onNewTab);

    auto *closeTabShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this);
    connect(closeTabShortcut, &QShortcut::activated, [this]() {
        onCloseTab(m_tabWidget->currentIndex());
    });

    auto *focusUrlShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_L), this);
    connect(focusUrlShortcut, &QShortcut::activated, [this]() {
        m_addressBar->setFocus();
        m_addressBar->selectAll();
    });

    auto *reloadShortcut = new QShortcut(QKeySequence::Refresh, this);
    connect(reloadShortcut, &QShortcut::activated, [this]() {
        if (auto *view = m_tabWidget->currentWebView())
            view->reload();
    });

    // Ctrl+D to toggle favorite for current page
    auto *favShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_D), this);
    connect(favShortcut, &QShortcut::activated, this, &MainWindow::toggleFavoriteForCurrentPage);

    // Ctrl+B to toggle favorites panel
    auto *favPanelShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_B), this);
    connect(favPanelShortcut, &QShortcut::activated, this, &MainWindow::toggleFavoritesPanel);

    // Ctrl+Shift+I (Cmd+Shift+I on macOS) to toggle DevTools; F12 lives on the menu action
    auto *devToolsShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I), this);
    connect(devToolsShortcut, &QShortcut::activated, this, &MainWindow::toggleDevTools);
}

void MainWindow::connectTab(QWebEngineView *view)
{
    connect(view, &QWebEngineView::titleChanged, this, &MainWindow::onTabTitleChanged);
    connect(view, &QWebEngineView::urlChanged, this, &MainWindow::onTabUrlChanged);
    connect(view, &QWebEngineView::loadProgress, this, &MainWindow::onLoadProgress);
    connect(view, &QWebEngineView::iconChanged, this, &MainWindow::onTabIconChanged);
    connect(view->page(), &QWebEnginePage::linkHovered, this, &MainWindow::onLinkHovered);
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
        updateStarButton();

        // DevTools follow the active tab
        if (m_devToolsPanel->isVisible())
            m_devToolsPanel->attachToPage(view->page());
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

    // Update saved session title if this tab is saved
    auto *sm = Application::instance()->sessionManager();
    QString urlStr = view->url().toDisplayString();
    if (sm->isTabSaved(urlStr) && !title.isEmpty()) {
        sm->unsaveTab(urlStr);
        sm->saveTab(urlStr, title);
    }
}

void MainWindow::onTabIconChanged(const QIcon &icon)
{
    auto *view = qobject_cast<QWebEngineView *>(sender());
    if (!view) return;

    int index = m_tabWidget->indexOf(view);
    if (index >= 0) {
        m_tabWidget->setTabIcon(index, icon);
    }

    // Update favicon data on matching favorite
    QUrl pageUrl = view->url();
    if (!icon.isNull() && !pageUrl.isEmpty() && pageUrl.scheme() != "about") {
        auto *favMgr = Application::instance()->favoritesManager();
        QString urlStr = pageUrl.toDisplayString();
        if (favMgr->isFavorited(urlStr)) {
            QPixmap px = icon.pixmap(16, 16);
            QByteArray data;
            QBuffer buf(&data);
            buf.open(QIODevice::WriteOnly);
            px.save(&buf, "PNG");
            favMgr->setFaviconPng(favMgr->favoriteIdForUrl(urlStr), data);
        }
    }
}

void MainWindow::onTabUrlChanged(const QUrl &url)
{
    auto *view = qobject_cast<QWebEngineView *>(sender());
    if (view && view == m_tabWidget->currentWebView()) {
        m_addressBar->setUrl(url);
        updateStarButton();
    }
}

void MainWindow::onLoadProgress(int progress)
{
    auto *view = qobject_cast<QWebEngineView *>(sender());
    if (view && view != m_tabWidget->currentWebView())
        return;

    if (progress < 100) {
        showStatusOverlay(QString("Loading... %1%").arg(progress));
    } else {
        // Brief flash then hide
        m_statusTimer->start(800);
    }
}

void MainWindow::onNewTab()
{
    auto *view = m_tabWidget->createTab();
    connectTab(view);
    loadNewTabPage(view);
}

void MainWindow::onCloseTab(int index)
{
    if (m_tabWidget->count() <= 1) {
        if (auto *view = m_tabWidget->webView(0)) {
            loadNewTabPage(view);
        }
        return;
    }
    m_tabWidget->closeTab(index);
}

void MainWindow::onSaveSessionRequested(int index)
{
    auto *view = m_tabWidget->webView(index);
    if (!view) return;

    QUrl url = view->url();
    if (url.isEmpty() || url.scheme() == "about") return;

    // Confirmation dialog
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Save Session");
    msgBox.setText("Save this tab's session?");
    msgBox.setInformativeText(
        "This will enable cookies, cache, and local storage for this tab. "
        "Your browsing data will persist between sessions.\n\n"
        "The tab will reload with a persistent profile.");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);

    if (msgBox.exec() != QMessageBox::Ok)
        return;

    // Save to session manager
    auto *sm = Application::instance()->sessionManager();
    QString urlStr = url.toDisplayString();
    sm->saveTab(urlStr, view->title());

    // Switch this tab to the persistent profile by creating a new page
    auto *persistentProfile = sm->persistentProfile();
    auto *newPage = new QWebEnginePage(persistentProfile, view);
    newPage->setBackgroundColor(QColor("#1e1e2e"));

    view->setPage(newPage);
    view->load(url);

    // Re-attach DevTools if they were inspecting this tab's old page
    if (m_devToolsPanel->isVisible() && view == m_tabWidget->currentWebView())
        m_devToolsPanel->attachToPage(newPage);

    qDebug() << "MainWindow: Tab switched to persistent profile:" << urlStr;
}

void MainWindow::onUnsaveSessionRequested(int index)
{
    auto *view = m_tabWidget->webView(index);
    if (!view) return;

    QUrl url = view->url();
    if (url.isEmpty()) return;

    // Remove from saved sessions
    auto *sm = Application::instance()->sessionManager();
    sm->unsaveTab(url.toDisplayString());

    // Switch back to ephemeral (default) profile
    auto *defaultProfile = QWebEngineProfile::defaultProfile();
    auto *newPage = new QWebEnginePage(defaultProfile, view);
    newPage->setBackgroundColor(QColor("#1e1e2e"));

    view->setPage(newPage);
    view->load(url);

    // Re-attach DevTools if they were inspecting this tab's old page
    if (m_devToolsPanel->isVisible() && view == m_tabWidget->currentWebView())
        m_devToolsPanel->attachToPage(newPage);

    qDebug() << "MainWindow: Tab switched back to ephemeral profile:" << url.toDisplayString();
}

void MainWindow::restoreSavedSessions()
{
    auto *sm = Application::instance()->sessionManager();
    const auto savedTabs = sm->savedTabs();

    for (const auto &tab : savedTabs) {
        createPersistentTab(tab.url, tab.title);
    }

    if (!savedTabs.isEmpty()) {
        m_tabWidget->setCurrentIndex(0);
    }
}

QWebEngineView *MainWindow::createPersistentTab(const QString &url, const QString &title)
{
    auto *sm = Application::instance()->sessionManager();
    auto *persistentProfile = sm->persistentProfile();

    auto *page = new QWebEnginePage(persistentProfile, m_tabWidget);
    page->setBackgroundColor(QColor("#1e1e2e"));

    auto *view = new QWebEngineView(m_tabWidget);
    view->setPage(page);

    int index = m_tabWidget->addTab(view, title.isEmpty() ? "New Tab" : title);
    m_tabWidget->setCurrentIndex(index);

    connectTab(view);
    view->load(QUrl(url));

    return view;
}

void MainWindow::loadNewTabPage(QWebEngineView *view)
{
    // If a custom homepage is set, navigate there instead
    QString homepage = Application::instance()->settings()->homepage();
    if (!homepage.isEmpty()) {
        if (!homepage.startsWith("http://") && !homepage.startsWith("https://"))
            homepage = "https://" + homepage;
        view->load(QUrl::fromUserInput(homepage));
        return;
    }

    // Load logo from resources and encode as data URI
    static QString cachedHtml;
    if (cachedHtml.isEmpty()) {
        QFile logoFile(":/icons/scowser.png");
        QString logoDataUri;
        if (logoFile.open(QIODevice::ReadOnly)) {
            logoDataUri = "data:image/png;base64," + QString::fromLatin1(logoFile.readAll().toBase64());
        }

        cachedHtml = QStringLiteral(R"(
<!DOCTYPE html>
<html>
<head>
<style>
  * { margin: 0; padding: 0; box-sizing: border-box; }
  html { height: 100%%; }
  body {
    min-height: 100vh;
    background-color: #1e1e2e;
    color: #cdd6f4;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
    display: flex;
    align-items: center;
    justify-content: center;
    user-select: none;
    -webkit-user-select: none;
  }
  .container {
    text-align: center;
    opacity: 0;
    animation: fadeIn 0.3s ease forwards;
  }
  @keyframes fadeIn { to { opacity: 1; } }
  .icon { margin-bottom: 16px; }
  .icon img { width: 128px; height: 128px; }
  .logo {
    font-size: 48px;
    font-weight: 700;
    letter-spacing: -1px;
    color: #cdd6f4;
    margin-bottom: 8px;
  }
  .tagline {
    font-size: 14px;
    color: #6c7086;
    font-weight: 400;
  }
  .shortcuts {
    margin-top: 24px;
    font-size: 11px;
    color: #585b70;
  }
  kbd {
    background: #313244;
    padding: 2px 6px;
    border-radius: 4px;
    font-family: inherit;
    font-size: 11px;
    color: #a6adc8;
  }
</style>
</head>
<body>
  <div class="container">
    <div class="icon"><img src="%1" alt="scowser"></div>
    <div class="logo">scowser</div>
    <div class="tagline">private by default</div>
    <div class="shortcuts">
      <kbd>Ctrl+L</kbd> address bar &nbsp;&middot;&nbsp;
      <kbd>Ctrl+T</kbd> new tab &nbsp;&middot;&nbsp;
      <kbd>Ctrl+B</kbd> favorites
    </div>
  </div>
</body>
</html>
)").arg(logoDataUri);
    }
    view->setHtml(cachedHtml, QUrl("about:blank"));
}

void MainWindow::toggleLogPanel()
{
    m_logPanel->setVisible(!m_logPanel->isVisible());
}

void MainWindow::toggleFavoritesPanel()
{
    m_favoritesPanel->setVisible(!m_favoritesPanel->isVisible());
}

void MainWindow::toggleDevTools()
{
    if (m_devToolsPanel->isVisible()) {
        m_devToolsPanel->hide();
        m_devToolsPanel->detach();
    } else {
        attachDevToolsToCurrentTab();
        m_devToolsPanel->show();
    }
}

void MainWindow::attachDevToolsToCurrentTab()
{
    if (auto *view = m_tabWidget->currentWebView()) {
        m_devToolsPanel->attachToPage(view->page());
    }
}

void MainWindow::toggleFavoriteForCurrentPage()
{
    auto *view = m_tabWidget->currentWebView();
    if (!view) return;

    QUrl url = view->url();
    if (url.isEmpty() || url.scheme() == "about")
        return;

    auto *favMgr = Application::instance()->favoritesManager();
    QString urlStr = url.toDisplayString();

    if (favMgr->isFavorited(urlStr)) {
        QString id = favMgr->favoriteIdForUrl(urlStr);
        favMgr->removeFavorite(id);
    } else {
        QString id = favMgr->addFavorite(urlStr, view->title());
        QIcon icon = view->icon();
        if (!icon.isNull()) {
            QPixmap px = icon.pixmap(16, 16);
            QByteArray data;
            QBuffer buf(&data);
            buf.open(QIODevice::WriteOnly);
            px.save(&buf, "PNG");
            favMgr->setFaviconPng(id, data);
        }
    }
}

void MainWindow::updateStarButton()
{
    auto *view = m_tabWidget->currentWebView();
    if (!view) return;

    QUrl url = view->url();
    auto *favMgr = Application::instance()->favoritesManager();

    bool isFav = !url.isEmpty() && url.scheme() != "about" &&
                 favMgr->isFavorited(url.toDisplayString());

    if (isFav) {
        m_starButton->setIcon(QIcon(":/icons/star-filled.svg"));
        m_starButton->setToolTip("Remove from favorites");
    } else {
        m_starButton->setIcon(QIcon(":/icons/star.svg"));
        m_starButton->setToolTip("Add to favorites");
    }
}

void MainWindow::onFavoriteActivated(const QString &url)
{
    if (auto *view = m_tabWidget->currentWebView()) {
        view->load(QUrl(url));
    }
}

void MainWindow::onLinkHovered(const QString &url)
{
    if (url.isEmpty()) {
        hideStatusOverlay();
    } else {
        showStatusOverlay(url);
    }
}

void MainWindow::showStatusOverlay(const QString &text)
{
    m_statusTimer->stop();
    m_statusOverlay->setText(text);
    m_statusOverlay->adjustSize();

    // Cap width to 60% of window width
    int maxWidth = static_cast<int>(width() * 0.6);
    if (m_statusOverlay->width() > maxWidth)
        m_statusOverlay->setFixedWidth(maxWidth);
    else
        m_statusOverlay->setMaximumWidth(maxWidth);

    // Position at bottom-left, above the window edge
    m_statusOverlay->move(0, height() - m_statusOverlay->height());
    m_statusOverlay->show();
    m_statusOverlay->raise();
}

void MainWindow::hideStatusOverlay()
{
    m_statusOverlay->hide();
}
