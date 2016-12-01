
#pragma once

#include <QObject>
#include <QPainter>

namespace datavis {

class Plot : public QObject
{
public:
    struct Range
    {
        QPointF min;
        QPointF max;
    };

    Plot(QObject * parent = 0): QObject(parent) {}
    virtual bool isEmpty() const = 0;
    virtual Range range() = 0;
    virtual void plot(QPainter *, const QTransform &) = 0;
};

}
