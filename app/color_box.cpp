#include "color_box.hpp"

#include <QPainter>
#include <QDebug>

namespace datavis {

ColorBox::ColorBox(QWidget * parent):
    QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ColorBox::setColor(const QColor & color)
{
    m_color = color;
    update();
}

void ColorBox::resizeEvent(QResizeEvent*)
{

}

void ColorBox::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.setPen(Qt::black);
    painter.setBrush(m_color);
    painter.drawRect(rect().adjusted(0,0,-1,-1));
}

}
