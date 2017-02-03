#include "line_plot.hpp"

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

void LinePlot::setDataSet(DataSetPtr dataset)
{
    setDataSet(dataset, 0);
}

void LinePlot::setDataSet(DataSetPtr dataset, int dim)
{
    if (m_dataset)
        m_dataset->disconnect(this);

    m_dataset = dataset;

    if (m_dataset)
    {
        connect(m_dataset.get(), &DataSet::selectionChanged,
                this, &LinePlot::onSelectionChanged);
    }

    if (!dataset || dataset->data()->size().empty())
    {
        m_dim = -1;
        m_start = 0;
        m_end = 0;
    }
    else
    {
        auto size = dataset->data()->size();

        if (size.size() > dim)
        {
            m_dim = dim;
        }
        else
        {
            m_dim = 0;
        }

        m_start = 0;
        m_end = size[dim];
    }

    findEntireValueRange();

    update_selected_region();

    emit sourceChanged();
    emit dimensionChanged();
    emit xRangeChanged();
    emit yRangeChanged();
    emit contentChanged();
}

void LinePlot::setDimension(int dim)
{
    if (!m_dataset)
        return;

    auto size = m_dataset->data()->size();

    if (dim < 0 || dim >= size.size())
        dim = -1;

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
    emit xRangeChanged();
    //emit yRangeChanged();
    emit contentChanged();
}

void LinePlot::setRange(int start, int end)
{
    if (!m_dataset)
        return;

    auto size = m_dataset->data()->size();

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

void LinePlot::onSelectionChanged()
{
    // FIXME: Optimize: only update if needed

    update_selected_region();

    emit contentChanged();
}

void LinePlot::findEntireValueRange()
{
    if (!m_dataset)
        m_value_range = Range(0,0);

    auto region = get_all(*m_dataset->data());

    auto min_it = std::min_element(region.begin(), region.end());
    auto max_it = std::max_element(region.begin(), region.end());

    m_value_range.min = min_it.value();
    m_value_range.max = max_it.value();
}

void LinePlot::update_selected_region()
{
    if (!m_dataset)
    {
        m_data_region = data_region_type();
        return;
    }

    if (m_dim < 0)
    {
        m_data_region = data_region_type();
        return;
    }

    auto data_size = m_dataset->data()->size();
    auto n_dim = data_size.size();

    vector<int> offset(n_dim, 0);
    vector<int> size(n_dim, 1);

    auto selected_index = m_dataset->selectedIndex();
#if 0
    cout << "LinePlot: selected index: ";
    for (auto & i : selected_index)
        cout << i << " ";
    cout << endl;
#endif
    for (int dim = 0; dim < n_dim; ++dim)
    {
        if (dim == m_dim)
        {
            offset[dim] = m_start;
            size[dim] = m_end - m_start;
        }
        else
        {
            offset[dim] = selected_index[dim];
        }
    }

    m_data_region = get_region(*m_dataset->data(), offset, size);
}

Plot::Range LinePlot::xRange()
{
    if (!m_data_region.is_valid())
    {
        return Range();
    }

    auto dim = m_dataset->dimension(m_dim);

    return Range { dim.map * m_start, dim.map * (m_end - 1) };
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

vector<double> LinePlot::dataLocation(const QPointF & point)
{
    if (isEmpty())
        return vector<double>();

    auto offset = m_data_region.offset();

    vector<double> location(offset.begin(), offset.end());
    location[m_dim] = point.x();
    return location;
}

void LinePlot::plot(QPainter * painter,  const Mapping2d & transform, const QRectF & region)
{
    if (!m_data_region.is_valid())
        return;

    auto dim = m_dataset->dimension(m_dim);

    auto x_range = xRange();
    auto min_x = transform.x_scale * x_range.min + transform.x_offset;
    auto max_x = transform.x_scale * x_range.max + transform.x_offset;

    int elem_count = m_end - m_start;

    painter->save();

    if (max_x - min_x < elem_count * 0.8)
    {
        QPen line_pen;
        line_pen.setWidth(1);
        line_pen.setColor(m_color);

        painter->setPen(line_pen);
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHint(QPainter::Antialiasing, false);

        auto it = m_data_region.begin();

        bool first = true;
        double max_y;
        double min_y;
        double last_y;

        for (int x = min_x; x <= max_x; ++x)
        {
            while(it != m_data_region.end())
            {
                const auto & element = *it;

                double loc = element.location()[m_dim];
                loc = dim.map * loc;

                double value = element.value();

                auto point = transform * QPointF(loc, value);

                if (point.x() >= x + 1)
                    break;

                if (first)
                {
                    min_y = max_y = point.y();
                }
                else
                {
                    if (point.y() > max_y)
                        max_y = point.y();
                    else if (point.y() < min_y)
                        min_y = point.y();
                }

                last_y = point.y();

                ++it;
                first = false;
            }

            int min_y_pixel = round(min_y);
            int max_y_pixel = round(max_y);
            if (max_y_pixel == min_y_pixel)
                max_y_pixel += 1;

            painter->drawLine(x, min_y_pixel, x, max_y_pixel);

            // Include last point to connect old and new line
            max_y = min_y = last_y;
        }
    }
    else
    {
        QPen line_pen;
        line_pen.setWidth(1);
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
    }

    painter->restore();
}

}
