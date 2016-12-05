#pragma once

#include <QWidget>

namespace datavis {

class ColorBox : public QWidget
{
public:
    ColorBox(QWidget * parent = 0);
    void setColor(const QColor &);
    QColor color() const { return m_color; }

    virtual void resizeEvent(QResizeEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

private:
    QColor m_color { Qt::black };
};

}
