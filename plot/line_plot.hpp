#pragma once

#include <QTransform>

#include "plot.hpp"
#include "../data/array.hpp"

namespace datavis {

class LinePlot : public Plot
{
public:
    using data_type = array<double>;
    using data_region_type = array_region<double>;

    LinePlot(QObject * parent = 0);
    void setData(data_type * data);
    void setDimension(int dim);
    void setRange(int start, int end);
    void setPen(const QPen &);
    virtual bool isEmpty() const override { return !m_data_region.is_valid(); }
    virtual Range range() override;
    virtual void plot(QPainter *,  const QTransform &) override;

private:
    void update_selected_region();
    data_type * m_data = nullptr;
    data_region_type m_data_region;
    int m_dim = 0;
    int m_start = 0;
    int m_end = 0;

    QPen m_pen { Qt::black };
};

}
