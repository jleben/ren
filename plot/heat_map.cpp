#include "heat_map.hpp"
#include "../utility/threads.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>

#include <algorithm>
#include <iostream>

using namespace std;

namespace datavis {

HeatMap::HeatMap(QObject * parent):
    Plot(parent)
{}

json HeatMap::save()
{
    json d;
    d["type"] = "heat_map";
    d["x_dim"] = d_options.dimensions[0];
    d["y_dim"] = d_options.dimensions[1];
    return d;
}

void HeatMap::setDataSet(FutureDataset dataset, const vector_t & dim)
{
    // Clear current data

    d_prepration = nullptr;
    d_plot_data = nullptr;
    m_dataset = nullptr;

    emit xRangeChanged();
    emit yRangeChanged();
    emit contentChanged();

    // Set up new data

    d_options.dimensions = dim;

    auto plot_data = make_shared<PlotData>();
    plot_data->dimensions = dim;

    auto preparePlot = [=](Reactive::Status&, DataSetPtr dataset) -> PlotDataPtr
    {
        printf("HeatMap: Preparing...\n");

        plot_data->dataset = dataset;
        plot_data->update_selected_region();
        plot_data->update_value_range();
        plot_data->generate_image();
        return plot_data;
    };

    d_plot_data = Reactive::apply(background_thread(), preparePlot, dataset);

    d_prepration = Reactive::apply([=](Reactive::Status&, PlotDataPtr plot_data)
    {
        m_dataset = plot_data->dataset;
        connect(m_dataset.get(), &DataSet::selectionChanged,
                this, &HeatMap::onSelectionChanged);

        printf("HeatMap: Range: %f %f, %f %f\n", xRange().min, xRange().max,
               yRange().min, yRange().max);

        auto x_dim = m_dataset->dimension(d_options.dimensions[0]);

        printf("HeatMap: X map: a (%lu) * %f + %f\n",
            x_dim.size, x_dim.map.scale, x_dim.map.offset);

        printf("HeatMap: Ready.\n");

        emit xRangeChanged();
        emit yRangeChanged();
        emit contentChanged();
    },
    d_plot_data);
}

void HeatMap::restore(const FutureDataset & dataset, const json & options)
{
    int x_dim = options.at("x_dim");
    int y_dim = options.at("y_dim");
    setDataSet(dataset, { x_dim, y_dim });
}

#if 0
void HeatMap::setDataSet(DataSetPtr dataset)
{
    setDataSet(dataset, { 0, 1 });
}

void HeatMap::setDataSet(DataSetPtr dataset, const vector_t & dims)
{
    if (m_dataset)
        m_dataset->disconnect(this);

    m_dataset = dataset;

    if (m_dataset)
    {
        connect(m_dataset.get(), &DataSet::selectionChanged,
                this, &HeatMap::onSelectionChanged);

        auto size = dataset->data()->size();

        m_dim = dims;

        m_start = { 0, 0 };

        for (int i = 0; i < 2; ++i)
        {
            int dim = dims[i];
            if (dim >= 0 && dim < m_size.size())
            {
                m_size[i] = size[dims[i]];
            }
            else
            {
                m_size[i] = 0;
                cerr << "HeatMap: Warning: Dimension out of range: " << dims[i] << endl;
            }
        }
    }

    update_selected_region();
    update_value_range();
    generate_image();
}

void HeatMap::setDimensions(const vector_t & dims)
{
    if (m_dim == dims)
        return;

    auto data_size = m_dataset->data()->size();

    m_dim = dims;
    m_start = { 0, 0 };
    for (int i = 0; i < 2; ++i)
    {
        int dim = dims[i];
        if (dim >= 0 && dim < m_size.size())
        {
            m_size[i] = data_size[dims[i]];
        }
        else
        {
            m_size[i] = 0;
            cerr << "HeatMap: Warning: Dimension out of range: " << dims[i] << endl;
        }
    }

    update_selected_region();
    update_value_range();
    generate_image();
}
#endif

void HeatMap::onSelectionChanged()
{
    if (!m_dataset)
        return;

    auto old_region = d_plot_data->value->data_region;

    d_plot_data->value->update_selected_region();

    if (old_region != d_plot_data->value->data_region)
    {
        // FIXME: Do this asynchronously
        d_plot_data->value->generate_image();
        emit contentChanged();
    }
}

void HeatMap::PlotData::update_selected_region()
{
    if (!dataset)
    {
        data_region = data_region_type();
        return;
    }

    auto data_size = dataset->data()->size();
    auto data_dim_count = data_size.size();

    vector<int> offset = dataset->selectedIndex();
    vector<int> size(data_dim_count, 1);

    for (int d = 0; d < 2; ++d)
    {
        int data_dim = dimensions[d];
        if (data_dim < 0 || data_dim >= data_dim_count)
        {
            cerr << "Selected dimension is invalid: " << data_dim << endl;
            data_region = data_region_type();
            return;
        }

        offset[data_dim] = 0;
        size[data_dim] = data_size[data_dim];
    }

    data_region = get_region(*dataset->data(), offset, size);
}

void HeatMap::PlotData::update_value_range()
{
    if (!data_region.is_valid())
        return;

    // qDebug() << "Computing value range.";

    double min = 0;
    double max = 0;

    auto it = data_region.begin();
    if (it != data_region.end())
    {
        min = max = (*it).value();
        while(++it != data_region.end())
        {
            auto value = (*it).value();
            min = std::min(value, min);
            max = std::max(value, max);
        }
    }

    value_range = Range(min, max);

    // qDebug() << "Done computing value range.";
}

void HeatMap::PlotData::generate_image()
{
    // qDebug() << "Generating image";

    if (!data_region.is_valid())
    {
        return;
    }

    double value_extent = value_range.extent();
    double value_scale = value_extent != 0 ? 1 / value_extent : 1;
    double value_offset = -value_range.min;

    int width = dataset->dimension(dimensions[0]).size;
    int height = dataset->dimension(dimensions[1]).size;
    QImage image(width, height, QImage::Format_RGB888);

    for (auto & element : data_region)
    {
        auto loc = element.location();
        int x = loc[dimensions[0]];
        int y = image.height() - 1 - loc[dimensions[1]];

        double v = element.value();
        v += value_offset;
        v *= value_scale;

        int c = 255 * v;

        // FIXME: We may get coordinates out of image if
        // dataset->data() does not agree with dataset->dimension(i).
        image.setPixel(x,y,qRgb(c,c,c));
    }

    // qDebug() << "Image generated.";

    pixmap = QPixmap::fromImage(image);

    // qDebug() << "Pixmap generated.";
}

Plot::Range HeatMap::xRange()
{
    if (!m_dataset)
    {
        return Range();
    }

    auto x_dim = m_dataset->dimension(d_options.dimensions[0]);

    if (x_dim.size < 1) {
        return Range(x_dim.map.offset, x_dim.map.offset);
    }

    double margin = 0.5;
    double x_min = - margin;
    double x_max = x_dim.size - 1 + margin;

    return Range(x_dim.map * x_min, x_dim.map * x_max);
}

Plot::Range HeatMap::yRange()
{
    if (!m_dataset)
    {
        return Range();
    }

    auto y_dim = m_dataset->dimension(d_options.dimensions[1]);

    if (y_dim.size < 1) {
        return Range(y_dim.map.offset, y_dim.map.offset);
    }

    double margin = 0.5;
    double y_min = - margin;
    double y_max = y_dim.size - 1 + margin;

    return Range(y_dim.map * y_min, y_dim.map * y_max);
}

tuple<vector<double>, vector<double>> HeatMap::dataLocation(const QPointF & point)
{
    if (!m_dataset)
        return {};

    auto offset = d_plot_data->value->data_region.offset();

    vector<double> location(m_dataset->dimensionCount(), 0);

    for (int d = 0; d < offset.size(); ++d)
    {
        auto dim = m_dataset->dimension(d);
        location[d] = dim.map * offset[d];
    }

    location[d_options.dimensions[0]] = point.x();
    location[d_options.dimensions[1]] = point.y();

    auto index = m_dataset->indexForPoint(location);

    auto size = m_dataset->data()->size();
    bool in_bounds = true;
    for (int d = 0; d < size.size(); ++d)
        in_bounds &= (index[d] >= 0 && index[d] < size[d]);

    vector<double> attributes(m_dataset->attributeCount(), 0);

    if (in_bounds)
    {
        for (int a = 0; a < m_dataset->attributeCount(); ++a)
        {
            attributes[a] = m_dataset->data(0)(index);
        }
    }

    return { location, attributes };
}

void HeatMap::plot(QPainter * painter,  const Mapping2d & transform, const QRectF & region)
{
    if (!m_dataset)
        return;

    painter->save();

    auto x_range = xRange();
    auto y_range = yRange();

    auto topLeft = transform * QPointF(x_range.min, y_range.max);
    auto bottomRight = transform * QPointF(x_range.max, y_range.min);

    auto & pixmap = d_plot_data->value->pixmap;
    painter->drawPixmap(QRectF(topLeft, bottomRight),
                        pixmap, pixmap.rect());

    painter->restore();
}


}
