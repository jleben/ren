#include "line_plot.hpp"
#include "selector.hpp"
#include "../app/data_source.hpp"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <QDebug>

using namespace std;

namespace datavis {

LinePlot::LinePlot(QObject * parent):
    Plot(parent)
{}

void LinePlot::setDataSource(DataSource * source)
{
    m_data_source = source;

    if (!source || source->data()->size().empty())
    {
        m_dim = -1;
        m_selector_dim = -1;
        m_start = 0;
        m_end = 0;
    }
    else
    {
        auto size = source->data()->size();

        if (size.size() > 1)
        {
            m_selector_dim = 0;
            m_dim = 1;
        }
        else
        {
            m_selector_dim = -1;
            m_dim = 0;
        }

        m_start = 0;
        m_end = size[m_dim];
    }

    findEntireValueRange();

    update_selected_region();

    emit sourceChanged();
    emit dimensionChanged();
    emit selectorDimChanged();
    emit xRangeChanged();
    emit yRangeChanged();
    emit selectorRangeChanged();
    emit contentChanged();
}

void LinePlot::setSelector(Selector * selector)
{
    if (m_selector)
    {
        disconnect(m_selector, 0, this, 0);
    }

    m_selector = selector;

    if (m_selector)
    {
        connect(m_selector, &Selector::valueChanged,
                this, &LinePlot::onSelectorValueChanged);
    }

    update_selected_region();

    //emit yRangeChanged();
    emit contentChanged();
}

void LinePlot::setDimension(int dim)
{
    if (!m_data_source)
        return;

    auto size = m_data_source->data()->size();

    if (dim < 0 || dim >= size.size())
    {
        cerr << "Invalid dimension: " << dim << endl;
        return;
    }

    if (m_selector_dim == dim)
        m_selector_dim = -1;

    m_dim = dim;
    m_start = 0;
    m_end = size[dim];

    update_selected_region();

    emit dimensionChanged();
    emit selectorDimChanged();
    emit xRangeChanged();
    //emit yRangeChanged();
    emit selectorRangeChanged();
    emit contentChanged();
}

int LinePlot::selectorDim()
{
    return m_selector_dim;
}

void LinePlot::setSelectorDim(int new_dim)
{
    if (!m_data_source)
    {
        cerr << "No data." << endl;
        return;
    }

    auto n_data_dim = m_data_source->data()->size().size();

    if (new_dim < 0 || new_dim >= n_data_dim)
    {
        cerr << "Invalid dimension: " << new_dim << endl;
        return;
    }

    if (new_dim == m_dim)
    {
        m_dim = -1;
    }

    m_selector_dim = new_dim;

    emit dimensionChanged();
    emit selectorDimChanged();
    emit xRangeChanged();
    //emit yRangeChanged();
    emit selectorRangeChanged();
    emit contentChanged();
}

void LinePlot::setRange(int start, int end)
{
    if (!m_data_source)
        return;

    auto size = m_data_source->data()->size();

    if (size.empty())
        return;

    if (m_dim < 0 || m_dim >= size.size())
    {
        cerr << "Unexpected: current dimension is invalid." << endl;
        return;
    }

    if (end < start)
    {
        cerr << "Warning: negative range." << endl;
        return;
    }

    if (start < 0 || start + end >= size[m_dim])
    {
        cerr << "Warning: selected range out of array bounds." << endl;
        return;
    }

    m_start = start;
    m_end = end;

    update_selected_region();

    emit xRangeChanged();
    //emit yRangeChanged();
    emit contentChanged();
}

void LinePlot::setColor(const QColor & color)
{
    m_color = color;

    emit colorChanged();
    emit contentChanged();
}

void LinePlot::onSelectorValueChanged()
{
    update_selected_region();

    //emit yRangeChanged();
    emit contentChanged();
}


void LinePlot::findEntireValueRange()
{
    if (!m_data_source)
        m_value_range = Range(0,0);

    auto region = get_all(*m_data_source->data());

    auto min_it = std::min_element(region.begin(), region.end());
    auto max_it = std::max_element(region.begin(), region.end());

    m_value_range.min = min_it.value();
    m_value_range.max = max_it.value();
}

void LinePlot::update_selected_region()
{
    if (!m_data_source)
    {
        m_data_region = data_region_type();
        return;
    }

    auto data_size = m_data_source->data()->size();
    auto n_dim = data_size.size();
    vector<int> offset(n_dim, 0);
    vector<int> size(n_dim, 1);

    if (m_selector_dim >= 0 && m_selector)
    {
        assert(m_selector_dim < n_dim);
        int value = m_selector->value();
        //qDebug() << "Selector value:" << value;
        //qDebug() << "Data size: " << data_size[m_selector_dim];
        if (value >= 0 && value < data_size[m_selector_dim])
        {
            offset[m_selector_dim] = value;
            size[m_selector_dim] = 1;
        }
        else
        {
            m_data_region = data_region_type();
            return;
        }
    }

    offset[m_dim] = m_start;
    size[m_dim] = m_end - m_start;

    m_data_region = get_region(*m_data_source->data(), offset, size);
}

Plot::Range LinePlot::xRange()
{
    if (!m_data_region.is_valid())
    {
        return Range();
    }

    return Range { double(m_start), double(m_end) };
}

Plot::Range LinePlot::yRange()
{
    return m_value_range;
#if 0
    if (!m_data_region.is_valid())
    {
        return Range();
    }

    auto min_it = std::min_element(m_data_region.begin(), m_data_region.end());
    auto max_it = std::max_element(m_data_region.begin(), m_data_region.end());
    double min_value = min_it.value();
    double max_value = max_it.value();

    return Range { min_value, max_value };
#endif
}

Plot::Range LinePlot::selectorRange()
{
    if (!m_data_source)
        return Range();

    if (m_selector_dim < 0)
        return Range();

    auto data_size = m_data_source->data()->size();

    assert(m_selector_dim >= 0 && m_selector_dim < data_size.size());

    return Range { double(0), double(data_size[m_selector_dim] - 1) };
}

void LinePlot::plot(QPainter * painter,  const QTransform & transform)
{
    if (!m_data_region.is_valid())
        return;

    painter->save();

    painter->setPen(m_color);
    painter->setBrush(Qt::NoBrush);
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    bool first = true;
    for (auto & element : m_data_region)
    {
        int loc = element.location()[m_dim];
        double value = element.value();
        if (first)
            path.moveTo(loc, value);
        else
            path.lineTo(loc, value);
        first = false;
    }
    painter->drawPath(path * transform);

    painter->restore();
}

}
