#include "scatter_plot_1d.hpp"

namespace datavis {

ScatterPlot1d::ScatterPlot1d(QObject * parent):
    Plot(parent)
{}

json ScatterPlot1d::save()
{
    json d;
    d["type"] = "scatter_1d";
    d["attribute"] = m_attribute;
    d["orientation"] = m_orientation == Horizontal ? "horizontal" : "vertical";
    return d;
}

void ScatterPlot1d::restore(const DataSetPtr & dataset, const json & options)
{
    int attribute = options.at("attribute");
    string orientation_text = options.at("orientation");
    Orientation orientation = orientation_text == "horizontal" ? Horizontal : Vertical;

    setData(dataset, attribute, orientation);
}

void ScatterPlot1d::setData(DataSetPtr data, int attribute, Orientation orientation)
{
    m_dataset = data;
    m_attribute = attribute;
    m_orientation = orientation;

    m_range = find_range();

    //cerr << "X range: " << m_x_range.min << ", " << m_x_range.max << endl;
    //cerr << "Y range: " << m_y_range.min << ", " << m_y_range.max << endl;

    make_points();
}

void ScatterPlot1d::make_points()
{
    // FIXME: Implement selection in other dimensions

    auto data_region = get_region(m_dataset->data(m_attribute),
                                  vector<int>(m_dataset->dimensionCount(), 0),
                                  m_dataset->data()->size());

    for(auto item : data_region)
    {
        double v =  m_dataset->data(m_attribute).data()[item.index()];

        Point2d p;
        if (m_orientation == Horizontal)
            p.x = v;
        else
            p.y = v;

        m_points.push_back(p);
    }
}

Plot::Range ScatterPlot1d::xRange()
{
    if (m_orientation == Horizontal)
        return m_range;
    else
        return Range(0,1);
}

Plot::Range ScatterPlot1d::yRange()
{
    if (m_orientation == Vertical)
        return m_range;
    else
        return Range(0,1);
}

tuple<vector<double>, vector<double>> ScatterPlot1d::dataLocation(const QPointF & point)
{
    if (m_dataset)
    {
        double value = m_orientation == Horizontal ? point.x() : point.y();
        vector<double> loc(m_dataset->dimensionCount(), 0);
        vector<double> att(m_dataset->attributeCount(), 0);
        att[m_attribute] = value;
        return { loc, att };
    }
    else
    {
        return {};
    }
}

Plot::Range ScatterPlot1d::find_range()
{
    int ndim = m_dataset->dimensionCount();
    auto data_region = get_region(m_dataset->data(m_attribute),
                                  vector<int>(ndim, 0),
                                  m_dataset->data()->size());
    auto min = std::min_element(data_region.begin(), data_region.end());
    auto max = std::max_element(data_region.begin(), data_region.end());
    return Range(min.value(), max.value());
}

void ScatterPlot1d::plot(QPainter * painter,  const Mapping2d & view_map, const QRectF & region)
{
    QColor c;

    QRect r;

    if (m_orientation == Horizontal)
    {
        auto bottom = view_map * Point2d(0,0);
        auto top = view_map * Point2d(0,1);
        r.setTop(top.y);
        r.setBottom(bottom.y);
        r.setWidth(2);

        for (auto & point : m_points)
        {
            auto p = view_map * point;
            r.moveLeft(p.x);
            painter->fillRect(r, c);
        }
    }
    else
    {
        auto left = view_map * Point2d(0,0);
        auto right = view_map * Point2d(1,0);
        r.setLeft(left.x);
        r.setRight(right.x);
        r.setHeight(2);

        for (auto & point : m_points)
        {
            auto p = view_map * point;
            r.moveTop(p.y);
            painter->fillRect(r, c);
        }
    }
}

}
