#pragma once

#include <QWidget>

namespace datavis {

class RangeBar : public QWidget
{
public:
    RangeBar(QWidget * parent = 0);
    double extent() { return m_extent; }
    double position() { return m_position; }
    void setExtent(double value);
    void setPosition(double value);
    void setRange(double position, double extent);

    virtual void paintEvent(QPaintEvent*) override;

    virtual QSize sizeHint() const { return QSize(50,10); }

private:
    double m_position = 0;
    double m_extent = 1;
    QColor m_background_color;
    QColor m_foreground_color;
};

}
