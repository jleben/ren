#include "range_bar.hpp"

#include <QPainter>

namespace datavis {

RangeBar::RangeBar(QWidget * parent):
    QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_background_color = QColor(100,100,100);
    m_foreground_color = QColor(200,200,200);
}

void RangeBar::setExtent(double value)
{
    m_extent = std::max(0.0, std::min(1.0 - m_position, value));
    update();
}

void RangeBar::setPosition(double value)
{
    m_position = std::max(0.0, std::min(1.0 - m_extent, value));
    update();
}

void RangeBar::setRange(double position, double extent)
{
    m_position = std::max(0.0, std::min(1.0, position));
    m_extent = std::max(0.0, std::min(1.0 - position, extent));
    update();
}

void RangeBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.setPen(Qt::NoPen);

    painter.fillRect(rect(), m_background_color);

    int thumb_x = int(m_position * width());
    int thumb_width = std::max(5, int(m_extent * width()));

    QRect rangeRect = rect();
    rangeRect.setX(thumb_x);
    rangeRect.setWidth(thumb_width);

    painter.fillRect(rangeRect, m_foreground_color);

    auto stripe_color = m_background_color;
    stripe_color.setAlpha(100);
    painter.setBrush(QBrush(stripe_color, Qt::BDiagPattern));
    painter.drawRect(rangeRect);
}

}
