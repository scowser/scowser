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
    if (delta == 0) {
        event->accept();
        return;
    }

    if ((delta > 0) != (m_scrollAccumulator > 0))
        m_scrollAccumulator = 0;

    m_scrollAccumulator += delta;

    // Trackpads emit many small-delta events per gesture; require a full
    // wheel notch (120 units) before switching so one swipe doesn't skip
    // through several tabs.
    constexpr int stepSize = 120;

    if (m_scrollAccumulator >= stepSize) {
        if (currentIndex() > 0)
            setCurrentIndex(currentIndex() - 1);
        m_scrollAccumulator = 0;
    } else if (m_scrollAccumulator <= -stepSize) {
        if (currentIndex() < count() - 1)
            setCurrentIndex(currentIndex() + 1);
        m_scrollAccumulator = 0;
    }

    event->accept();
}
