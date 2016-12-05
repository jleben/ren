#include "line_plot.hpp"

#include <QPainter>
#include <QPainterPath>
#include <algorithm>
#include <iostream>

using namespace std;

namespace datavis {

LinePlot::LinePlot(QObject * parent):
    Plot(parent)
{}

void LinePlot::setData(data_type * data)
{
    m_data = data;

    if (!m_data || m_data->size().empty())
    {
        m_dim = -1;
        m_start = 0;
        m_end = 0;
    }
    else
    {
        auto size = data->size();

        m_dim = 0;
        m_start = 0;
        m_end = size[0];
    }

    update_selected_region();

    emit rangeChanged();
    emit contentChanged();
}

void LinePlot::setDimension(int dim)
{
    if (!m_data)
        return;

    auto size = m_data->size();

    if (dim < 0 || dim >= size.size())
        return;

    m_dim = dim;
    m_start = 0;
    m_end = size[dim];

    update_selected_region();

    emit rangeChanged();
    emit contentChanged();
}

void LinePlot::setRange(int start, int end)
{
    if (!m_data)
        return;

    auto size = m_data->size();

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

    emit rangeChanged();
    emit contentChanged();
}

void LinePlot::setPen(const QPen & pen)
{
    m_pen = pen;

    emit contentChanged();
}

void LinePlot::update_selected_region()
{
    if (!m_data)
    {
        m_data_region = data_region_type();
        return;
    }

    auto data_size = m_data->size();
    auto n_dim = data_size.size();
    vector<int> offset(n_dim, 0);
    vector<int> size(n_dim, 1);
    offset[m_dim] = m_start;
    size[m_dim] = m_end - m_start;
    m_data_region = get_region(*m_data, offset, size);
}

Plot::Range LinePlot::range()
{
    if (!m_data_region.is_valid())
    {
        return Range();
    }

    auto min_it = std::min_element(m_data_region.begin(), m_data_region.end());
    auto max_it = std::max_element(m_data_region.begin(), m_data_region.end());
    double min_value = min_it.value();
    double max_value = max_it.value();

    Range range;
    range.min = QPointF(m_start, min_value);
    range.max = QPointF(m_end-1, max_value);
#if 0
    cout << "Plot range: "
         << range.min.x() << " - " << range.max.x()
         << " -> "
         << range.min.y() << " - " << range.max.y()
         << endl;
#endif
    return range;
}

void LinePlot::plot(QPainter * painter,  const QTransform & transform)
{
    if (!m_data_region.is_valid())
        return;

    painter->save();

    painter->setPen(m_pen);
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
