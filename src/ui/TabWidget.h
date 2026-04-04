#pragma once

#include <QTabWidget>

class QToolButton;
class QWebEngineView;

class TabWidget : public QTabWidget {
    Q_OBJECT

public:
    explicit TabWidget(QWidget *parent = nullptr);

    QWebEngineView *createTab();
    QWebEngineView *currentWebView() const;
    QWebEngineView *webView(int index) const;
    void closeTab(int index);

signals:
    void newTabRequested();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void repositionNewTabButton();
    QToolButton *m_newTabButton;
};
