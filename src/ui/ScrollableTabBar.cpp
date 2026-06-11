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

    // Trackpads emit many small-delta events per gesture; require two full
    // wheel notches (240 units) of accumulation there so one swipe doesn't
    // fly through the strip. Discrete mouse wheels (no pixel delta, no
    // scroll phase) keep the one-notch-per-step feel.
    const bool isTrackpad = !event->pixelDelta().isNull()
        || event->phase() != Qt::NoScrollPhase;
    const int stepSize = isTrackpad ? 240 : 120;

    if (m_scrollAccumulator >= stepSize) {
        scrollTabs(-1);
        m_scrollAccumulator = 0;
    } else if (m_scrollAccumulator <= -stepSize) {
        scrollTabs(1);
        m_scrollAccumulator = 0;
    }

    event->accept();
}

void ScrollableTabBar::scrollTabs(int direction)
{
    // Scroll the strip without changing the active tab by driving the
    // built-in (visually hidden) scroll buttons. QTabBar keeps their
    // enabled state in sync with the scroll limits.
    const Qt::ArrowType arrow = direction < 0 ? Qt::LeftArrow : Qt::RightArrow;
    for (auto *button : findChildren<QToolButton *>()) {
        if (button->arrowType() == arrow && button->isEnabled()) {
            button->click();
            return;
        }
    }
}
