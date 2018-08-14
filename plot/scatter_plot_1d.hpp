#pragma once

#include <QTransform>

#include "plot.hpp"
#include "../data/data_set.hpp"

#include <list>
#include <vector>

namespace datavis {

using std::list;
using std::vector;

class ScatterPlot1d : public Plot
{
    Q_OBJECT

public:
    enum Orientation {
        Horizontal,
        Vertical
    };

    ScatterPlot1d(QObject * parent = 0);

    void setData(DataSetPtr data, int attribute, Orientation);

    DataSetPtr dataSet() override { return m_dataset; }
    virtual bool isEmpty() const override { return !m_dataset; }
    virtual Range xRange() override;
    virtual Range yRange() override;
    virtual tuple<vector<double>, vector<double>> dataLocation(const QPointF & point) override;
    virtual void plot(QPainter *,  const Mapping2d &, const QRectF & region) override;

public:
    Range find_range();
    double value(int dim, const array_region<double>::iterator &);
    void make_points();

    DataSetPtr m_dataset = nullptr;
    int m_attribute = -1;
    Orientation m_orientation = Horizontal;
    Range m_range;
    vector<Point2d> m_points;
};

}
