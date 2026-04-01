#pragma once

#include <QMainWindow>
#include <memory>

class TabWidget;
class AddressBar;
class QToolBar;
class QWebEngineView;

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
    void onLoadProgress(int progress);
    void onNewTab();
    void onCloseTab(int index);

private:
    void setupUI();
    void setupToolBar();
    void setupShortcuts();
    void connectTab(QWebEngineView *view);

    TabWidget *m_tabWidget;
    AddressBar *m_addressBar;
    QToolBar *m_navToolBar;
};
