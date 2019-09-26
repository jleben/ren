#pragma once

#include "../data/math.hpp"
#include "../data/data_set.hpp"
#include "../data/data_source.hpp"
#include "../json/json.hpp"

#include <QObject>
#include <QPainter>
#include <QPointF>

#include <vector>
#include <tuple>

namespace datavis {

using std::vector;
using std::tuple;
using nlohmann::json;

class Selector;
class PlotView;

class Plot : public QObject
{
    Q_OBJECT

public:
    struct Range
    {
        Range() {}
        Range(double min, double max): min(min), max(max) {}

        double min = 0;
        double max = 0;

        double extent() const { return max - min; }
    };

    Plot(QObject * parent = 0): QObject(parent) {}

    PlotView * view() { return m_view; }

    virtual DataSetPtr dataSet() = 0;
    virtual bool isEmpty() const = 0;
    virtual Range xRange() = 0;
    virtual Range yRange() = 0;
    virtual void plot(QPainter *, const Mapping2d &, const QRectF & region) = 0;
    virtual tuple<vector<double>, vector<double>> dataLocation(const QPointF & point) = 0;

    virtual json save() { return {}; }
    virtual void restore(const FutureDataset &, const json &) {}

signals:
    void xRangeChanged();
    void yRangeChanged();
    void contentChanged();

private:
    friend class PlotView;
    void setView(PlotView *view) { m_view = view; }
    PlotView * m_view = nullptr;
};

inline
QPointF operator* (const Mapping2d & m, const QPointF & v)
{
    return  QPointF(v.x() * m.x_scale + m.x_offset,
                    v.y() * m.y_scale + m.y_offset);
}

}
