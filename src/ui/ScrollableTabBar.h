#pragma once

#include <QTabBar>

class ScrollableTabBar : public QTabBar {
    Q_OBJECT

public:
    explicit ScrollableTabBar(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void hideScrollButtons();
};
