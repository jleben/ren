#include "heat_map.hpp"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <iostream>

using namespace std;

namespace datavis {

HeatMap::HeatMap(QObject * parent):
    Plot(parent)
{}

void HeatMap::setData(data_type * data)
{
    m_data = data;
    auto size = data->size();

    m_dim = { 0, 1 };
    m_start = { 0, 0 };
    if (size.size() >= 2)
        m_size = { size[0], size[1] };
    else
        m_size = { 0, 0 };

    update_selected_region();
    update_value_range();
}

void HeatMap::setDimensions(const vector_t & dim)
{
    auto data_size = m_data->size();

    m_dim = dim;
    m_start = { 0, 0 };
    for (int d = 0; d < 2; ++d)
    {
        int src_d = dim[d];
        if (src_d < 0 || src_d >= data_size.size())
            m_size[d] = 0;
        else
            m_size[d] = data_size[src_d];
    }

    update_selected_region();
    update_value_range();
}

void HeatMap::setRange(const vector_t & start, const vector_t & size)
{
    m_start = start;
    m_size = size;

    update_selected_region();
    update_value_range();
}

void HeatMap::update_selected_region()
{
    auto data_size = m_data->size();
    auto data_dim_count = data_size.size();

    vector<int> offset(data_dim_count, 0);
    vector<int> size(data_dim_count, 1);

    for (int d = 0; d < 2; ++d)
    {
        int data_dim = m_dim[d];
        if (data_dim < 0 || data_dim >= data_dim_count)
        {
            cerr << "Selected dimension is invalid: " << data_dim << endl;
            m_data_region = data_region_type();
            return;
        }

        if (m_start[d] < 0 || m_size[d] < 0 || m_start[d] + m_size[d] > data_size[data_dim])
        {
            cerr << "Selected range for dimension " << data_dim << " is invalid:"
                 << m_start[d] << "," << (m_start[d] + m_size[d])
                 << endl;
            m_data_region = data_region_type();
            return;
        }

        offset[data_dim] = m_start[d];
        size[data_dim] = m_size[d];
    }

    m_data_region = get_region(*m_data, offset, size);
}

void HeatMap::update_value_range()
{
    if (!m_data_region.is_valid())
        return;

    auto min_it = std::min_element(m_data_region.begin(), m_data_region.end());
    auto max_it = std::max_element(m_data_region.begin(), m_data_region.end());
    m_value_range.first = min_it.value();
    m_value_range.second = max_it.value();
}

Plot::Range HeatMap::range()
{
    if (!m_data_region.is_valid())
    {
        return Range();
    }

    double margin = 0.5;

    Range range;
    range.min = QPointF
            (m_start[0] - margin,
            m_start[1] - margin);
    range.max = QPointF
            (m_start[0] + m_size[0] - 1 + margin,
            m_start[1] + m_size[1] - 1 + margin);

    return range;
}

void HeatMap::plot(QPainter * painter,  const QTransform & transform)
{
    if (!m_data_region.is_valid())
        return;

    painter->save();

    painter->setPen(Qt::black);

    double value_range = m_value_range.second - m_value_range.first;
    double value_scale = value_range != 0 ? 1 / value_range : 1;
    double value_offset = -m_value_range.first;

    for (auto & element : m_data_region)
    {
        auto & loc = element.location();
        double x = loc[m_dim[0]];
        double y = loc[m_dim[1]];

        double v = element.value();
        v += value_offset;
        v *= value_scale;

        int c = 255 * v;

        auto a = QPointF(x-0.5,y-0.5) * transform;
        auto b = QPointF(x+0.5,y+0.5) * transform;

        painter->fillRect(QRectF(a,b), QColor(c,c,c));
    }

    painter->restore();
}


}
