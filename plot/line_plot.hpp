#pragma once

#include <QTransform>

#include "plot.hpp"
#include "../data/array.hpp"
#include "../data/data_set.hpp"

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
    void setDataSet(DataSetPtr data);
    void setDataSet(DataSetPtr data, int dimension);

    int dimension() const { return m_dim; }
    void setDimension(int dim);

    void setRange(int start, int end);

    QColor color() const { return m_color; }
    void setColor(const QColor & c);

    DataSetPtr dataSet() override { return m_dataset; }
    virtual bool isEmpty() const override { return !m_data_region.is_valid(); }
    virtual Range xRange() override;
    virtual Range yRange() override;
    virtual tuple<vector<double>, vector<double>> dataLocation(const QPointF & point) override;
    virtual void plot(QPainter *,  const Mapping2d &, const QRectF & region) override;

    virtual json save() override;
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
    void findEntireValueRange();
    void update_selected_region();
    data_region_type getDataRegion(int start, int size);


    DataCache * getCache(double dataPerPixel);
    void makeCache(DataCache &, int blockSize);

    DataSetPtr m_dataset = nullptr;
    data_region_type m_data_region;
    int m_dim = -1;
    int m_start = 0;
    int m_end = 0;

    QColor m_color { Qt::black };

    Range m_value_range;

    double m_cache_use_factor = 5;
    double m_cache_factor = 10;
    list<DataCache> m_cache;
};

}
