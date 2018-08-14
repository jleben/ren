#pragma once

#include <QTransform>

#include "plot.hpp"
#include "../data/data_set.hpp"

#include <list>
#include <vector>

namespace datavis {

using std::list;
using std::vector;

class ScatterPlot2d : public Plot
{
    Q_OBJECT

public:
    enum Mark
    {
        Dot,
        Line
    };

    ScatterPlot2d(QObject * parent = 0);

    void setData(DataSetPtr data, int xDim, int yDim);
    void setShowDot(bool value);
    void setShowLine(bool value);

    DataSetPtr dataSet() override { return m_dataset; }
    virtual bool isEmpty() const override { return !m_dataset; }
    virtual Range xRange() override;
    virtual Range yRange() override;
    virtual tuple<vector<double>, vector<double>> dataLocation(const QPointF & point) override;
    virtual void plot(QPainter *,  const Mapping2d &, const QRectF & region) override;

    virtual json save() override;
    virtual void restore(const DataSetPtr &, const json &) override;

public:
    Range range(int dim);
    double value(int dim, const array_region<double>::iterator &);
    void make_points();

    DataSetPtr m_dataset = nullptr;
    int m_x_dim;
    int m_y_dim;
    bool m_show_dot;
    bool m_show_line;
    Range m_x_range;
    Range m_y_range;
    vector<Point2d> m_points;
};

}
