#pragma once

#include <QTransform>

#include "plot.hpp"
#include "../data/array.hpp"
#include "../data/data_object.hpp"

namespace datavis {

class Selector;

class LinePlot : public Plot
{
    Q_OBJECT

public:
    using data_type = array<double>;
    using data_region_type = array_region<double>;

    LinePlot(QObject * parent = 0);

    DataObject * dataObject() const { return m_data_object; }
    void setDataObject(DataObject * data);

    virtual void setSelector(Selector *);

    int dimension() const { return m_dim; }
    void setDimension(int dim);

    int selectorDim();
    void setSelectorDim(int dim);

    void setRange(int start, int end);

    QColor color() const { return m_color; }
    void setColor(const QColor & c);

    virtual bool isEmpty() const override { return !m_data_region.is_valid(); }
    virtual Range xRange() override;
    virtual Range yRange() override;
    virtual Range selectorRange() override;
    virtual void plot(QPainter *,  const Mapping2d &) override;

signals:
    void sourceChanged();
    void dimensionChanged();
    void selectorDimChanged();
    void colorChanged();

private:
    void onSelectorValueChanged();
    void findEntireValueRange();
    void update_selected_region();

    DataObject * m_data_object = nullptr;
    Selector * m_selector = nullptr;
    data_region_type m_data_region;
    int m_dim = -1;
    int m_selector_dim = -1;
    int m_start = 0;
    int m_end = 0;

    QColor m_color { Qt::black };

    Range m_value_range;
};

}
