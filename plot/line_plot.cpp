#include "line_plot.hpp"

#include <cmath>
#include <algorithm>
#include <iostream>
#include <cassert>

#include <QPainter>
#include <QPainterPath>
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
    m_data_region = getDataRegion(m_start, m_end - m_start);
    m_cache.clear();
}

LinePlot::data_region_type LinePlot::getDataRegion(int region_start, int region_size)
{
    if (!m_dataset)
    {
        return data_region_type();
    }

    if (m_dim < 0)
    {
        return data_region_type();
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
            offset[dim] = region_start;
            size[dim] = region_size;
        }
        else
        {
            offset[dim] = selected_index[dim];
        }
    }

    return get_region(*m_dataset->data(), offset, size);
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

tuple<vector<double>, vector<double>> LinePlot::dataLocation(const QPointF & point)
{
    if (isEmpty())
        return {};

    auto offset = m_data_region.offset();

    vector<double> location(m_dataset->dimensionCount());
    for (int d = 0; d < offset.size(); ++d)
    {
      if (d == m_dim)
      {
        location[d] = point.x();
      }
      else
      {
        auto dim = m_dataset->dimension(d);
        location[d] = dim.map * offset[d];
      }
    }

    vector<double> attributes(m_dataset->attributeCount(), 0);
    attributes[0] = point.y();

    return { location, attributes };
}

LinePlot::DataCache * LinePlot::getCache(double dataPerPixel)
{
    //cout << "Requested cache for resolution: " << dataPerPixel << endl;

    double required_data_per_pixel = dataPerPixel / m_cache_use_factor;

    int cache_level = int(std::log(required_data_per_pixel) / std::log(m_cache_factor));

    if (cache_level < 1)
        return nullptr;

    int required_block_size = std::pow(m_cache_factor, cache_level);

    //cout << "Block size: " << required_block_size << endl;

    auto cache_iter = m_cache.begin();
    for(; cache_iter != m_cache.end(); ++cache_iter)
    {
        if (cache_iter->block_size == required_block_size)
            return &(*cache_iter);
        else if (cache_iter->block_size < required_block_size)
            break;
    }

    cache_iter = m_cache.emplace(cache_iter);

    makeCache(*cache_iter, required_block_size);

    return &(*cache_iter);
}

void LinePlot::makeCache(DataCache & cache, int blockSize)
{
    //cout << "Making cache with block size: " << blockSize << endl;

    cache.data.clear();
    cache.block_size = blockSize;

    if (!m_data_region.is_valid())
        return;

    auto it = m_data_region.begin();

    while(it != m_data_region.end())
    {
        double min, max;
        for(int i = 0; i < blockSize && it != m_data_region.end(); ++i, ++it)
        {
            auto & element = *it;
            auto value = element.value();
            if (i == 0)
            {
                min = max = value;
            }
            else
            {
                min = std::min(min, value);
                max = std::max(max, value);
            }
        }
        cache.data.emplace_back(min, max);
    }
}

void LinePlot::plot(QPainter * painter,  const Mapping2d & transform, const QRectF & region)
{
    if (!m_data_region.is_valid())
        return;

    auto dim = m_dataset->dimension(m_dim);

    int region_start = int(region.x() / dim.map);
    int region_size = int(region.width() / dim.map);

    region_start = std::max(region_start, m_start);
    region_size = std::min(region_size, m_end - m_start);

    //cout << "Region: " << region_start << " + " << region_size << endl;
    if (region_size <= 0)
        return;

    auto min_x = transform.x_scale * region.x() + transform.x_offset;
    auto max_x = transform.x_scale * (region.x() + region.width()) + transform.x_offset;

    //cout << "View X range: " << min_x << ", " << max_x << endl;

    if (max_x <= min_x)
        return;

    data_region_type data_region = getDataRegion(region_start, region_size);

    double data_per_pixel = region_size / double(max_x - min_x);

    auto cache = getCache(data_per_pixel);

    painter->save();

    if (cache)
    {
        QPen line_pen;
        line_pen.setWidth(1);
        line_pen.setColor(m_color);

        painter->setPen(line_pen);
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHint(QPainter::Antialiasing, false);

        int cache_index = 0;

        double min_y, max_y;

        for (int x = min_x; x < max_x; ++x)
        {
            bool first = true;

            for(; cache_index < cache->data.size(); ++cache_index)
            {
                int data_index = cache_index * cache->block_size;

                QPointF data_point(dim.map * data_index, 0);

                auto point = transform * data_point;
                if (point.x() >= x + 1)
                    break;

                auto & data = cache->data[cache_index];

                if (first)
                {
                    min_y = data.min;
                    max_y = data.max;
                }
                else
                {
                    min_y = std::min(min_y, data.min);
                    max_y = std::max(max_y, data.max);
                }

                first = false;
            }

            if (!first) // Make sure we got at least one item
            {
                auto min_point = transform * QPointF(0, min_y);
                auto max_point = transform * QPointF(0, max_y);

                painter->drawLine(x, min_point.y(), x, max_point.y());
            }
        }
    }
    else if (max_x - min_x < region_size * 0.8)
    {
        QPen line_pen;
        line_pen.setWidth(1);
        line_pen.setColor(m_color);

        painter->setPen(line_pen);
        painter->setBrush(Qt::NoBrush);
        painter->setRenderHint(QPainter::Antialiasing, false);

        auto it = data_region.begin();

        bool first = true;
        double max_y;
        double min_y;
        double last_y;

        for (int x = min_x; x < max_x; ++x)
        {
            while(it != data_region.end())
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

            if (!first) // Just in case, make sure we got at least one item
            {
                int min_y_pixel = int(round(min_y));
                int max_y_pixel = int(round(max_y));
                if (max_y_pixel == min_y_pixel)
                    max_y_pixel += 1;

                painter->drawLine(x, min_y_pixel, x, max_y_pixel);

                // Include last point to connect old and new line
                max_y = min_y = last_y;
            }
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
        for (auto & element : data_region)
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
