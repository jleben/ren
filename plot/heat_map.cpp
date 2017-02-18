#include "heat_map.hpp"

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

void HeatMap::setRange(const vector_t & start, const vector_t & size)
{
    m_start = start;
    m_size = size;

    update_selected_region();
    update_value_range();
    generate_image();
}

void HeatMap::onSelectionChanged()
{
    // FIXME: Optimize: only update if needed

    update_selected_region();
    generate_image();
}

void HeatMap::update_selected_region()
{
    if (!m_dataset)
    {
        m_data_region = data_region_type();
        return;
    }

    auto data_size = m_dataset->data()->size();
    auto data_dim_count = data_size.size();

    vector<int> offset = m_dataset->selectedIndex();
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

    m_data_region = get_region(*m_dataset->data(), offset, size);
}

void HeatMap::update_value_range()
{
    if (!m_data_region.is_valid())
        return;

    qDebug() << "Computing value range.";

    double min = 0;
    double max = 0;

    auto it = m_data_region.begin();
    if (it != m_data_region.end())
    {
        min = max = (*it).value();
        while(++it != m_data_region.end())
        {
            auto value = (*it).value();
            min = std::min(value, min);
            max = std::max(value, max);
        }
    }

    m_value_range.first = min;
    m_value_range.second = max;

    qDebug() << "Done computing value range.";
}

void HeatMap::generate_image()
{
    qDebug() << "Generating image";

    if (!m_data_region.is_valid())
    {
        m_pixmap = QPixmap();
        return;
    }

    double value_range = m_value_range.second - m_value_range.first;
    double value_scale = value_range != 0 ? 1 / value_range : 1;
    double value_offset = -m_value_range.first;

    QImage image(m_size[0], m_size[1], QImage::Format_RGB888);

    for (auto & element : m_data_region)
    {
        auto loc = element.location();
        int x = loc[m_dim[0]];
        int y = image.height() - 1 - loc[m_dim[1]];

        double v = element.value();
        v += value_offset;
        v *= value_scale;

        int c = 255 * v;

        image.setPixel(x,y,qRgb(c,c,c));
    }

    qDebug() << "Image generated.";

    m_pixmap = QPixmap::fromImage(image);

    qDebug() << "Pixmap generated.";
}

Plot::Range HeatMap::xRange()
{
    if (!m_data_region.is_valid())
    {
        return Range();
    }

    auto x_dim = m_dataset->dimension(m_dim[0]);

    double margin = 0.5;
    double x_min = m_start[0] - margin;
    double x_max = m_start[0] + m_size[0] - 1 + margin;

    return Range(x_dim.map * x_min, x_dim.map * x_max);
}

Plot::Range HeatMap::yRange()
{
    if (!m_data_region.is_valid())
    {
        return Range();
    }

    auto y_dim = m_dataset->dimension(m_dim[1]);

    double margin = 0.5;
    double y_min = m_start[1] - margin;
    double y_max = m_start[1] + m_size[1] - 1 + margin;

    return Range(y_dim.map * y_min, y_dim.map * y_max);
}

vector<double> HeatMap::dataLocation(const QPointF & point)
{
    if (isEmpty())
        return vector<double>();

    auto offset = m_data_region.offset();

    vector<double> location(offset.size());

    for (int d = 0; d < offset.size(); ++d)
    {
        auto dim = m_dataset->dimension(d);
        location[d] = dim.map * offset[d];
    }

    location[m_dim[0]] = point.x();
    location[m_dim[1]] = point.y();

    return location;
}

void HeatMap::plot(QPainter * painter,  const Mapping2d & transform, const QRectF & region)
{
    if (!m_data_region.is_valid())
        return;

    auto x_dim = m_dataset->dimension(m_dim[0]);
    auto y_dim = m_dataset->dimension(m_dim[1]);

    painter->save();

    auto x_range = xRange();
    auto y_range = yRange();

    auto topLeft = transform * QPointF(x_range.min, y_range.max);
    auto bottomRight = transform * QPointF(x_range.max, y_range.min);

    painter->drawPixmap(QRectF(topLeft, bottomRight), m_pixmap, m_pixmap.rect());

    painter->restore();
}


}
