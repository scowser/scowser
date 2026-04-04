#include "ui/ScrollableTabBar.h"

#include <QToolButton>
#include <QWheelEvent>

ScrollableTabBar::ScrollableTabBar(QWidget *parent)
    : QTabBar(parent)
{
    setUsesScrollButtons(true);
}

void ScrollableTabBar::showEvent(QShowEvent *event)
{
    QTabBar::showEvent(event);
    hideScrollButtons();
}

void ScrollableTabBar::hideScrollButtons()
{
    for (auto *button : findChildren<QToolButton *>()) {
        button->setFixedSize(0, 0);
        button->setVisible(false);
    }
}

void ScrollableTabBar::wheelEvent(QWheelEvent *event)
{
    int delta = event->angleDelta().y();
    if (delta == 0)
        delta = -event->angleDelta().x();

    if (delta > 0 && currentIndex() > 0) {
        setCurrentIndex(currentIndex() - 1);
    } else if (delta < 0 && currentIndex() < count() - 1) {
        setCurrentIndex(currentIndex() + 1);
    }

    event->accept();
}
