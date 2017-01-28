#include "plot_view.hpp"
#include "plot.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QDebug>
#include <QCursor>
#include <QMouseEvent>
#include <algorithm>
#include <iostream>
#include <iterator>

using namespace std;

namespace datavis {

PlotView::PlotView(QWidget * parent):
    QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_canvas = new PlotCanvas;
    m_canvas->setMouseTracking(true);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_canvas);
}

void PlotView::addPlot(Plot * plot)
{
    if (!plot)
        return;

    plot->setView(this);

    m_canvas->m_plots.push_back(plot);

    connect(plot, &Plot::xRangeChanged,
            this, &PlotView::onPlotRangeChanged);
    connect(plot, &Plot::yRangeChanged,
            this, &PlotView::onPlotRangeChanged);
    connect(plot, &Plot::contentChanged,
            this, &PlotView::onPlotContentChanged);

    m_canvas->updatePlotMap();
    m_canvas->updateViewMap();

    m_canvas->update();
}

void PlotView::removePlot(Plot * plot)
{
    if (!plot)
        return;

    auto & plots = m_canvas->m_plots;

    auto handle = std::find(plots.begin(), plots.end(), plot);
    if (handle != plots.end())
    {
        disconnect(plot, 0, this, 0);
        m_canvas->m_plots.erase(handle);
    }

    m_canvas->updatePlotMap();
    m_canvas->updateViewMap();

    m_canvas->update();
}

void PlotView::setStacked(bool value)
{
    m_canvas->m_stacked = value;
    m_canvas->update();
}

void PlotView::setCommonX(bool value)
{
    m_canvas->m_common_x = value;
    m_canvas->update();
}

void PlotView::setCommonY(bool value)
{
    m_canvas->m_common_y = value;
    m_canvas->update();
}

void PlotView::onPlotRangeChanged()
{
    m_canvas->updatePlotMap();
    m_canvas->updateViewMap();
    m_canvas->update();
}

void PlotView::onPlotContentChanged()
{
    m_canvas->update();
}

QRect PlotCanvas::plotRect(int index)
{
    int plot_count = (int) m_plots.size();

    double plot_width = width() - 2 * m_margin;

    int plot_left = m_margin;
    int plot_right = width() - m_margin;

    int plot_top;
    int plot_bottom;

    if (m_stacked)
    {
        plot_top = m_margin;
        plot_bottom = height() - m_margin;
    }
    else
    {
        double plot_offset = double(height() - m_margin) / plot_count;
        plot_top = m_margin + index * plot_offset;
        plot_bottom = (index + 1) * plot_offset;
    }

    return QRect(QPoint(plot_left, plot_top), QPoint(plot_right, plot_bottom));
}

QPointF PlotCanvas::mapToPlot(int plotIndex, const QPointF & pos)
{
    if (m_plots.empty())
        return pos;

    auto plot = m_plots[plotIndex];
    auto xRange = m_common_x ? total_x_range : plot->xRange();
    auto yRange = m_common_y ? total_y_range : plot->yRange();

    auto rect = plotRect(plotIndex);
    if (rect.isEmpty())
        return QPointF(0,0);

    double x = (pos.x() - rect.x()) * (xRange.extent() / rect.width());
    double y = (rect.y() + rect.height() - pos.y()) * (yRange.extent() / rect.height());

    return QPointF(x,y);
}

void PlotCanvas::updateViewMap()
{
    auto size = this->size();

    int margin = 10;

    QTransform map;
    map.translate(margin, size.height() - margin);
    map.scale(size.width() - 2 * margin, - (size.height() - 2 * margin));

    m_view_map = m_plot_map * map;
}

void PlotCanvas::updatePlotMap()
{
    {
        bool first = true;
        for (auto plot : m_plots)
        {
            if (plot->isEmpty())
                continue;

            auto x_range = plot->xRange();
            auto y_range = plot->yRange();

            if (first)
            {
                total_x_range = x_range;
                total_y_range = y_range;
            }
            else
            {
                total_x_range.min = min(total_x_range.min, x_range.min);
                total_x_range.max = max(total_x_range.max, x_range.max);

                total_y_range.min = min(total_y_range.min, y_range.min);
                total_y_range.max = max(total_y_range.max, y_range.max);
            }
            first = false;
        }
    }
#if 0
    cout << "View range: "
         << "(" << total_range.min.x() << "," << total_range.max.x() << ")"
         << " -> "
         << "(" << total_range.min.y() << "," << total_range.max.y() << ")"
         << endl;
#endif
    QPointF max(total_x_range.max, total_y_range.max);
    QPointF min(total_x_range.min, total_y_range.min);

    QPointF offset = min;
    QPointF extent = max - min;

#if 0
    qDebug() << "offset:" << offset;
    qDebug() << "extent:" << extent;
#endif
    double x_scale = extent.x() == 0 ? 1 : 1.0 / extent.x();
    double y_scale = extent.y() == 0 ? 1 : 1.0 / extent.y();
#if 0
    qDebug() << "x scale:" << x_scale;
    qDebug() << "y scale:" << y_scale;
#endif
    QTransform transform;
    transform.scale(x_scale, y_scale);
    transform.translate(-offset.x(), -offset.y());

    m_plot_map = transform;
}

void PlotCanvas::resizeEvent(QResizeEvent*)
{
    updateViewMap();
}

void PlotCanvas::enterEvent(QEvent*)
{
    update();
}

void PlotCanvas::leaveEvent(QEvent*)
{
    update();
}

void PlotCanvas::mouseMoveEvent(QMouseEvent*)
{
    update();
}

void PlotCanvas::mousePressEvent(QMouseEvent* event)
{
    auto pos = event->pos();
    for (int i = 0; i < m_plots.size(); ++i)
    {
        auto plot = m_plots[i];
        if (plotRect(i).contains(pos))
        {
            auto plotPos = mapToPlot(i, pos);
            auto dataPos = plot->dataLocation(plotPos);
            auto dataIndex = plot->dataSet()->indexForPoint(dataPos);
            plot->dataSet()->selectIndex(dataIndex);
            return;
        }
    }
}

void PlotCanvas::paintEvent(QPaintEvent* event)
{
    auto mouse_pos = mapFromGlobal(QCursor::pos());

    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);

    if (m_plots.empty())
        return;

    painter.setBrush(Qt::NoBrush);

    QPen frame_pen;
    frame_pen.setColor(Qt::lightGray);

    int plot_index = 0;

    int plot_under_mouse_index = -1;

    for (auto plot : m_plots)
    {
        auto plot_rect = this->plotRect(plot_index);

        if (plot_rect.contains(mouse_pos))
            plot_under_mouse_index = plot_index;

        painter.setPen(frame_pen);
        painter.drawRect(plot_rect);

        if (plot->isEmpty())
            continue;

        Mapping2d map;

        Plot::Range x_range = m_common_x ? total_x_range : plot->xRange();
        Plot::Range y_range = m_common_y ? total_y_range : plot->yRange();

        double x_extent = x_range.max - x_range.min;
        double y_extent = y_range.max - y_range.min;
        double x_scale = x_extent == 0 ? 1 : 1.0 / x_extent;
        double y_scale = y_extent == 0 ? 1 : 1.0 / y_extent;

        map.translate(-x_range.min, -y_range.min);
        map.scale(x_scale, y_scale);
        map.scale(plot_rect.width(), -plot_rect.height());
        map.translate(plot_rect.x(), plot_rect.y() + plot_rect.height());

        plot->plot(&painter, map);

        ++plot_index;
    }

    if (underMouse())
    {
        auto pos = mapFromGlobal(QCursor::pos());

        QPen cursor_pen;
        cursor_pen.setColor(Qt::red);
        cursor_pen.setWidth(1);

        painter.setPen(cursor_pen);

        painter.drawLine(pos.x(), 0, pos.x(), height());
        painter.drawLine(0, pos.y(), width(), pos.y());

        if (plot_under_mouse_index >= 0)
        {
            auto plot = m_plots[plot_under_mouse_index];
            auto plotPos = mapToPlot(plot_under_mouse_index, pos);
            auto dataPos = plot->dataLocation(plotPos);
            auto dataIndex = plot->dataSet()->indexForPoint(dataPos);

            vector<int> dataSize = plot->dataSet()->data()->size();

            bool in_bounds = true;
            for (int d = 0; d < dataSize.size(); ++d)
                in_bounds &= (dataIndex[d] >= 0 && dataIndex[d] < dataSize[d]);

            QString text;

            if (in_bounds)
            {
                double value = (*plot->dataSet()->data())(dataIndex);

                text += QString::number(value);
                text += " ";
            }

            QStringList coord_strings;
            for (auto & i : dataPos)
                coord_strings << QString::number(i, 'f', 2);

            text += "@ ";
            text += coord_strings.join(" ");

            auto fm = fontMetrics();
            auto rect = fm.boundingRect(text);

            int x, y;

            if (pos.x() < width() / 2)
                x = pos.x() + 20;
            else
                x = pos.x() - 20 - rect.width();

            if (pos.y() < height() / 2)
                y = pos.y() + 20;
            else
                y = pos.y() - 20 - fm.descent();

            painter.drawText(QPoint(x,y), text);
        }
    }
}

Plot * PlotView::plotAt(const QPoint & view_pos)
{
    QPoint pos = m_canvas->mapFrom(this, view_pos);

    if (m_canvas->m_plots.empty())
        return nullptr;

    if (m_canvas->m_stacked)
        return nullptr;

    int plot_count = (int) m_canvas->m_plots.size();
    float plot_height = float(m_canvas->height()) / plot_count;
    int plot_index = float(pos.y()) / plot_height;
    if (plot_index < 0 || plot_index >= plot_count)
        return nullptr;

    auto iter = m_canvas->m_plots.begin();
    advance(iter, plot_index);
    return *iter;
}

}
