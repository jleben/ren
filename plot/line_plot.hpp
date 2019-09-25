#pragma once

#include <QTransform>

#include "plot.hpp"
#include "../data/array.hpp"
#include "../data/data_set.hpp"
#include "../data/data_source.hpp"

#include <list>
#include <vector>

namespace datavis {

using std::list;
using std::vector;

class LinePlot : public Plot
{
    Q_OBJECT

public:
    using data_type = array<double>;
    using data_region_type = array_region<double>;

    LinePlot(QObject * parent = 0);

    DataSetPtr dataSet() const { return m_dataset; }
    void setDataSet(FutureDataset access, int dimension);

    QColor color() const { return m_color; }
    void setColor(const QColor & c);

    virtual bool isEmpty() const override { return !m_data_region.is_valid(); }
    virtual Range xRange() override;
    virtual Range yRange() override;
    virtual tuple<vector<double>, vector<double>> dataLocation(const QPointF & point) override;
    virtual void plot(QPainter *,  const Mapping2d &, const QRectF & region) override;

    virtual json save() override;
    // FIXME: Async dataset loading
    virtual void restore(const DataSetPtr &, const json &) override;

signals:
    void sourceChanged();
    void dimensionChanged();
    void colorChanged();

private:
    struct DataRange
    {
        DataRange() {}
        DataRange(double min, double max): min(min), max(max) {}
        double min = 0;
        double max = 0;
    };

    struct DataCache
    {
        int block_size = 0;
        vector<DataRange> data;
    };

    void onSelectionChanged();
    static Range findEntireValueRange(DataSetPtr);
    void update_selected_region();
    data_region_type getDataRegion(int start, int size);


    DataCache * getCache(double dataPerPixel);
    void makeCache(DataCache &, int blockSize);

    int m_dim = -1;
    QColor m_color { Qt::black };

    Reactive::Value<void> m_on_dataset;

    DataSetPtr m_dataset = nullptr;
    data_region_type m_data_region;

    Reactive::Value<Range> m_value_range;
    Reactive::Value<void> m_on_value_range;

    double m_cache_use_factor = 5;
    double m_cache_factor = 10;
    list<DataCache> m_cache;
};

}
