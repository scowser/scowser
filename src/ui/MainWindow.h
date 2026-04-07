#pragma once

#include <QMainWindow>
#include <memory>

class TabWidget;
class AddressBar;
class LogPanel;
class FavoritesPanel;
class QToolBar;
class QToolButton;
class QWebEngineView;
class QLabel;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onNavigate(const QUrl &url);
    void onCurrentTabChanged(int index);
    void onTabTitleChanged(const QString &title);
    void onTabUrlChanged(const QUrl &url);
    void onTabIconChanged(const QIcon &icon);
    void onLoadProgress(int progress);
    void onNewTab();
    void onCloseTab(int index);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupShortcuts();
    void connectTab(QWebEngineView *view);
    void showAboutDialog();
    void showSettingsDialog();
    void showDownloadsDialog();
    void onActiveDownloadsChanged(int count);
    void toggleLogPanel();
    void toggleFavoritesPanel();
    void toggleFavoriteForCurrentPage();
    void updateStarButton();
    void onFavoriteActivated(const QString &url);
    void onLinkHovered(const QString &url);
    void showStatusOverlay(const QString &text);
    void hideStatusOverlay();
    void loadNewTabPage(QWebEngineView *view);

    TabWidget *m_tabWidget;
    AddressBar *m_addressBar;
    QToolBar *m_navToolBar;
    QToolButton *m_downloadButton;
    QToolButton *m_starButton;
    LogPanel *m_logPanel;
    FavoritesPanel *m_favoritesPanel;
    QLabel *m_statusOverlay;
    QTimer *m_statusTimer;
};
