#pragma once

#include <QTransform>
#include <array>

#include "plot.hpp"
#include "../data/array.hpp"
#include "../data/data_set.hpp"

namespace datavis {

class HeatMap : public Plot
{
public:
    using data_type = array<double>;
    using data_region_type = array_region<double>;
    using vector_t = std::array<int,2>;

    HeatMap(QObject * parent = 0);

    virtual void setSelector(Selector *) {}

    void setDataSet(DataSet *);
    void setDimensions(const vector_t & dim);
    void setRange(const vector_t & start, const vector_t & size);

    virtual bool isEmpty() const override { return !m_data_region.is_valid(); }

    virtual Range xRange() override;
    virtual Range yRange() override;
    virtual Range selectorRange() override;

    virtual void plot(QPainter *,  const Mapping2d &) override;

private:
    void update_selected_region();
    void update_value_range();
    DataSet * m_data_object = nullptr;
    data_region_type m_data_region;
    pair<double,double> m_value_range;
    vector_t m_dim;
    vector_t m_start;
    vector_t m_size;

    QPen m_pen { Qt::black };
};

}
