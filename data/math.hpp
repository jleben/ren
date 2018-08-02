#pragma once

#include <tuple>

namespace datavis {

template <size_t D>
struct Vector
{
    double operator[] (size_t d) const { return d_values[d]; }
    double d_values[D];
};

using Vector2d = Vector<2>;

struct Point2d
{
    Point2d() {}
    Point2d(double x, double y): x(x), y(y) {}

    double x;
    double y;
};

struct Mapping1d
{
    double scale = 1;
    double offset = 0;
};

inline
double operator*(const Mapping1d & m, double value)
{
    return value * m.scale + m.offset;
}

inline
double operator/(double value, const Mapping1d & m)
{
    return (value - m.offset) / m.scale;
}

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
        return *this;
    }

    Mapping2d inverse()
    {
        Mapping2d m;
        m.x_scale = 1.0 / x_scale;
        m.x_offset = - x_offset / x_scale;
        m.y_scale = 1.0 / y_scale;
        m.y_offset = - y_offset / y_scale;
        return m;
    }
};

inline
Mapping2d operator* (const Mapping2d & a, const Mapping2d & b)
{
    Mapping2d c;
    c.x_scale = a.x_scale * b.x_scale;
    c.x_offset = a.x_scale * b.x_offset + a.x_offset;
    return c;
}

inline
Point2d operator* (const Mapping2d & m, const Point2d & v)
{
    return  Point2d(v.x * m.x_scale + m.x_offset,
                    v.y * m.y_scale + m.y_offset);
}

using Range = std::tuple<double, double>;
inline auto & minimum(Range & r) { return std::get<0>(r); }
inline auto & maximum(Range & r) { return std::get<1>(r); }
inline Range join(Range & a, Range & b)
{
    return { std::min(minimum(a), minimum(b)), std::max(maximum(a), maximum(b)) };
}

}
