#pragma once

#include <QTransform>
#include <QImage>
#include <QThread>

#include <array>
#include <future>

#include "plot.hpp"
#include "../data/array.hpp"
#include "../data/data_set.hpp"
#include "../data/data_source.hpp"
#include "../reactive/reactive.hpp"

namespace datavis {

class HeatMap : public Plot
{
public:
    using data_type = array<double>;
    using data_region_type = array_region<double>;
    using vector_t = std::array<int,2>;

    HeatMap(QObject * parent = 0);

    // FIXME: Async dataset loading
    void setDataSet(FutureDataset, const vector_t & dim);
    //void setDataSet(DataSetPtr);
    //void setDataSet(DataSetPtr, const vector_t & dim);
    //void setDimensions(const vector_t & dim);

    vector_t dimensions() { return d_options.dimensions; }

    DataSetPtr dataSet() override { return m_dataset; }

    virtual bool isEmpty() const override { return !m_dataset; }

    virtual Range xRange() override;
    virtual Range yRange() override;
    virtual tuple<vector<double>, vector<double>> dataLocation(const QPointF & point) override;

    virtual void plot(QPainter *,  const Mapping2d &, const QRectF & region) override;

    virtual json save() override;
    virtual void restore(const DataSetPtr &, const json &) override;

private:

    struct PlotData
    {
        vector_t dimensions;
        DataSetPtr dataset;
        data_region_type data_region;
        Range value_range;
        QPixmap pixmap;

        void update_selected_region();
        void update_value_range();
        void generate_image();
    };

    using PlotDataPtr = std::shared_ptr<PlotData>;

    void onSelectionChanged();

    struct
    {
        vector_t dimensions;
    }
    d_options;

    Reactive::Value<PlotDataPtr> d_plot_data;
    Reactive::Value<void> d_prepration;

    DataSetPtr m_dataset = nullptr;
    data_region_type m_data_region;

    QPen m_pen { Qt::black };
};

}
