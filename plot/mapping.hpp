#pragma once

#include <QPoint>
#include <QPointF>
#include <QPainterPath>

namespace datavis {

struct Point2d
{
    Point2d() {}
    Point2d(double x, double y): x(x), y(y) {}

    double x;
    double y;
};

struct Mapping2d
{
    double x_scale = 1;
    double x_offset = 0;
    double y_scale = 1;
    double y_offset = 0;

    Mapping2d & translate(double x, double y)
    {
        x_offset += x;
        y_offset += y;
        return *this;
    }

    Mapping2d & scale(double x, double y)
    {
        x_scale *= x;
        x_offset *= x;
        y_scale *= y;
        y_offset *= y;
    }

    Mapping2d inverse()
    {
        Mapping2d m;
        m.x_scale = 1.0 / x_scale;
        m.x_offset = - x_offset / x_scale;
        m.y_scale = 1.0 / y_scale;
        m.y_offset = - y_offset / y_scale;
    }

    Point2d operator()(const Point2d & a) const
    {
        Point2d b;
        b.x = a.x * x_scale + x_offset;
        b.y = a.y * y_scale + y_offset;
        return  b;
    }

    QPointF operator()(const QPointF & a) const
    {
        return QPointF(a.x() * x_scale + x_offset,
                       a.y() * y_scale + y_offset);
    }

    QPoint operator()(const QPoint & a) const
    {
        return QPoint(int(a.x() * x_scale + x_offset),
                      int(a.y() * y_scale + y_offset));
    }
};

inline
Mapping2d operator* (const Mapping2d & a, const Mapping2d & b)
{
    // y = k1 x + c1
    // z = k2 y + c2
    // z = k2 (k1 x + c1) + c2
    // z = k2 k1 x + k2 c1 + c2

    Mapping2d c;
    c.x_scale = a.x_scale * b.x_scale;
    c.x_offset = a.x_scale * b.x_offset + a.x_offset;
    return c;
}

}
