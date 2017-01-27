#include "line_plot.hpp"
#include "selector.hpp"

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

void LinePlot::setDataObject(DataObject * source)
{
    m_data_object = source;

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
    if (!m_data_object)
        return;

    auto size = m_data_object->data()->size();

    if (dim < 0 || dim >= size.size())
        dim = -1;

    if (m_selector_dim == dim)
        m_selector_dim = -1;

    m_dim = dim;

    if (dim >= 0)
    {
        m_start = 0;
        m_end = size[dim];
    }
    else
    {
        m_start = 0;
        m_end = 0;
    }

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
    if (!m_data_object)
    {
        cerr << "No data." << endl;
        return;
    }

    auto n_data_dim = m_data_object->data()->size().size();

    if (new_dim == m_dim)
    {
        m_dim = -1;
    }

    if (new_dim < 0 || new_dim >= n_data_dim)
        new_dim = -1;

    m_selector_dim = new_dim;

    update_selected_region();

    emit dimensionChanged();
    emit selectorDimChanged();
    emit xRangeChanged();
    //emit yRangeChanged();
    emit selectorRangeChanged();
    emit contentChanged();
}

void LinePlot::setRange(int start, int end)
{
    if (!m_data_object)
        return;

    auto size = m_data_object->data()->size();

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
    if (!m_data_object)
        m_value_range = Range(0,0);

    auto region = get_all(*m_data_object->data());

    auto min_it = std::min_element(region.begin(), region.end());
    auto max_it = std::max_element(region.begin(), region.end());

    m_value_range.min = min_it.value();
    m_value_range.max = max_it.value();
}

void LinePlot::update_selected_region()
{
    if (!m_data_object)
    {
        m_data_region = data_region_type();
        return;
    }

    if (m_dim < 0)
    {
        m_data_region = data_region_type();
        return;
    }

    auto data_size = m_data_object->data()->size();
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

    m_data_region = get_region(*m_data_object->data(), offset, size);
}

Plot::Range LinePlot::xRange()
{
    if (!m_data_region.is_valid())
    {
        return Range();
    }

    auto dim = m_data_object->dimension(m_dim);

    return Range { dim.map * m_start, dim.map * m_end };
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
    if (!m_data_object)
        return Range();

    if (m_selector_dim < 0)
        return Range();

    auto data_size = m_data_object->data()->size();

    assert(m_selector_dim >= 0 && m_selector_dim < data_size.size());

    auto dim = m_data_object->dimension(m_selector_dim);

    return Range { dim.map * 0., dim.map * (data_size[m_selector_dim] - 1) };
}

void LinePlot::plot(QPainter * painter,  const Mapping2d & transform)
{
    if (!m_data_region.is_valid())
        return;

    auto dim = m_data_object->dimension(m_dim);

    painter->save();

    QPen line_pen;
    line_pen.setWidth(2);
    line_pen.setColor(m_color);

    painter->setPen(line_pen);
    painter->setBrush(Qt::NoBrush);
    painter->setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    bool first = true;
    for (auto & element : m_data_region)
    {
        double loc = element.location()[m_dim];
        loc = dim.map * loc;

        double value = element.value();

        auto point = transform * QPointF(loc, value);

        if (first)
            path.moveTo(point);
        else
            path.lineTo(point);

        first = false;
    }
    painter->drawPath(path);

    painter->restore();
}

}
