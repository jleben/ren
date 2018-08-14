#pragma once

#include <QTransform>
#include <QImage>
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

    void setDataSet(DataSetPtr);
    void setDataSet(DataSetPtr, const vector_t & dim);
    void setDimensions(const vector_t & dim);
    void setRange(const vector_t & start, const vector_t & size);

    vector_t dimensions() { return m_dim; }

    DataSetPtr dataSet() override { return m_dataset; }

    virtual bool isEmpty() const override { return !m_data_region.is_valid(); }

    virtual Range xRange() override;
    virtual Range yRange() override;
    virtual tuple<vector<double>, vector<double>> dataLocation(const QPointF & point) override;

    virtual void plot(QPainter *,  const Mapping2d &, const QRectF & region) override;

    virtual json save() override;
    virtual void restore(const DataSetPtr &, const json &) override;

private:
    void onSelectionChanged();
    bool update_selected_region();
    void update_value_range();
    void generate_image();

    DataSetPtr m_dataset = nullptr;
    data_region_type m_data_region;
    pair<double,double> m_value_range;
    vector_t m_dim;
    vector_t m_start;
    vector_t m_size;

    QPixmap m_pixmap;

    QPen m_pen { Qt::black };
};

}
