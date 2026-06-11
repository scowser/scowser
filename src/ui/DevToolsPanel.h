#pragma once

#include <QDockWidget>

class QWebEngineView;
class QWebEnginePage;

class DevToolsPanel : public QDockWidget {
    Q_OBJECT

public:
    explicit DevToolsPanel(QWidget *parent = nullptr);
    ~DevToolsPanel() override;

    void attachToPage(QWebEnginePage *page);
    void detach();
    QWebEnginePage *inspectedPage() const;
    bool isDockedBottom() const;

public slots:
    void toggleDockOrientation();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void setupUI();
    void ensureDevToolsView();
    void ensureUsableSize();
    void onInspectedPageDestroyed();

    QWebEngineView *m_devToolsView = nullptr;
    QWebEnginePage *m_inspectedPage = nullptr;
    QWidget *m_container;
};
