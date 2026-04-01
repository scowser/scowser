#pragma once

#include <QTabWidget>

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
};
