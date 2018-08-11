#include "scatter_plot.hpp"

namespace datavis {

ScatterPlot::ScatterPlot(QObject * parent):
    Plot(parent)
{}

void ScatterPlot::setData(DataSetPtr data, int xDim, int yDim)
{
    m_dataset = data;

    m_x_dim = xDim;
    m_y_dim = yDim;

    m_x_range = range(m_x_dim);
    m_y_range = range(m_y_dim);

    cerr << "X range: " << m_x_range.min << ", " << m_x_range.max << endl;
    cerr << "Y range: " << m_y_range.min << ", " << m_y_range.max << endl;
}

Plot::Range ScatterPlot::xRange()
{
    return m_x_range;
}

Plot::Range ScatterPlot::yRange()
{
    return m_y_range;
}

vector<double> ScatterPlot::dataLocation(const QPointF & point)
{
    if (m_dataset)
    {
        return vector<double>(m_dataset->dimensionCount(), 0);
    }
    else
    {
        return vector<double>();
    }
}

Plot::Range ScatterPlot::range(int dim_index)
{
    if (dim_index < m_dataset->dimensionCount())
    {
        const auto & dim = m_dataset->dimension(dim_index);
        return Range(dim.minimum(), dim.maximum());
    }
    else
    {
        int att_idx = dim_index - m_dataset->dimensionCount();
        int ndim = m_dataset->dimensionCount();
        auto data_region = get_region(m_dataset->data(att_idx),
                                      vector<int>(ndim, 0),
                                      m_dataset->data()->size());
        auto min = std::min_element(data_region.begin(), data_region.end());
        auto max = std::max_element(data_region.begin(), data_region.end());
        return Range(min.value(), max.value());
    }
}

inline double ScatterPlot::value(int dim_index,  const array_region<double>::iterator & iter)
{
    if (dim_index < m_dataset->dimensionCount())
    {
        const auto & dim = m_dataset->dimension(dim_index);
        return dim.map * iter.location()[dim_index];
    }
    else
    {
        int att_idx = dim_index - m_dataset->dimensionCount();
        return m_dataset->data(att_idx).data()[iter.index()];
    }
}

void ScatterPlot::plot(QPainter * painter,  const Mapping2d & view_map, const QRectF & region)
{
    int ndim = m_dataset->dimensionCount();

    auto data_region = get_region(*m_dataset->data(), vector<int>(ndim, 0),  m_dataset->data()->size());

    QColor c;

    for(auto item : data_region)
    {
        item.index();
        Point2d p;
        p.x = value(m_x_dim, item);
        p.y = value(m_y_dim, item);

        p = view_map * p;

        QRectF r(0, 0, 4, 4);
        r.moveCenter(QPointF(p.x, p.y));
        painter->fillRect(r, c);
    }
}

}
