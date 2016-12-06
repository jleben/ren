#pragma once

#include <QTransform>

#include "plot.hpp"
#include "../data/array.hpp"

namespace datavis {

class DataSource;

class LinePlot : public Plot
{
    Q_OBJECT

public:
    using data_type = array<double>;
    using data_region_type = array_region<double>;

    LinePlot(QObject * parent = 0);

    DataSource * dataSource() const { return m_data_source; }
    void setDataSource(DataSource * data);

    int dimension() const { return m_dim; }
    void setDimension(int dim);

    void setRange(int start, int end);

    QColor color() const { return m_color; }
    void setColor(const QColor & c);

    virtual bool isEmpty() const override { return !m_data_region.is_valid(); }
    virtual Range range() override;
    virtual void plot(QPainter *,  const QTransform &) override;

signals:
    void sourceChanged();
    void dimensionChanged();
    void colorChanged();

private:
    void update_selected_region();
    DataSource * m_data_source = nullptr;
    data_region_type m_data_region;
    int m_dim = 0;
    int m_start = 0;
    int m_end = 0;

    QColor m_color { Qt::black };
};

}
