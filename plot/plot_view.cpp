#include "plot_view.hpp"
#include "plot.hpp"
#include "../utility/vector.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QDebug>
#include <QCursor>
#include <QMouseEvent>
#include <QWheelEvent>
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

    m_canvas->updateDataRange();

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

    m_canvas->updateDataRange();

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
    m_canvas->updateDataRange();
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

    auto xRange = m_common_x ? view_x_range : plot->xRange();
    auto yRange = m_common_y ? total_y_range : plot->yRange();

    auto rect = plotRect(plotIndex);
    if (rect.isEmpty())
        return QPointF(0,0);

    double x = (pos.x() - rect.x()) * (xRange.extent() / rect.width()) + xRange.min;
    double y = (rect.y() + rect.height() - pos.y()) * (yRange.extent() / rect.height()) + yRange.min;

    return QPointF(x,y);
}

void PlotCanvas::updateDataRange()
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

    // Reset view
    view_x_range = total_x_range;
}

void PlotCanvas::resizeEvent(QResizeEvent*)
{
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

void PlotCanvas::wheelEvent(QWheelEvent* event)
{
    if (event->phase() != Qt::ScrollUpdate)
        return;

    double degrees = event->angleDelta().y() / 8.0;

    if (event->modifiers() & Qt::ControlModifier)
    {
        double new_size = view_x_range.extent() * pow(2.0, -degrees/90.0);
        //qDebug() << "new size = " << new_size;
        setSize(new_size);
    }
    else if (event->modifiers() & Qt::ShiftModifier)
    {
        double new_offset = view_x_range.min + view_x_range.extent() * degrees/180.0;
        //qDebug() << "new offset = " << new_offset;
        setOffset(new_offset);
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

        Plot::Range x_range = m_common_x ? view_x_range : plot->xRange();
        Plot::Range y_range = m_common_y ? total_y_range : plot->yRange();

        QRectF region(x_range.min, y_range.min, x_range.extent(), y_range.extent());

        double x_extent = x_range.extent();
        double y_extent = y_range.extent();
        double x_scale = x_extent == 0 ? 1 : 1.0 / x_extent;
        double y_scale = y_extent == 0 ? 1 : 1.0 / y_extent;

        map.translate(-x_range.min, -y_range.min);
        map.scale(x_scale, y_scale);
        map.scale(plot_rect.width(), -plot_rect.height());
        map.translate(plot_rect.x(), plot_rect.y() + plot_rect.height());

        painter.setClipRect(plot_rect);

        painter.save();

        plot->plot(&painter, map, region);

        painter.restore();

        ++plot_index;
    }

    painter.setClipping(false);

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
            rect.setWidth(rect.width() + 10);

            int x, y;

            if (pos.x() < width() / 2)
                x = pos.x() + 20;
            else
                x = pos.x() - 20 - rect.width();

            if (pos.y() < height() / 2)
                y = pos.y() + 20 + fm.ascent();
            else
                y = pos.y() - 20 - fm.descent();

            rect.translate(x, y);

            painter.fillRect(rect.adjusted(-2,0,2,0), QColor(255,255,255,230));

            painter.drawText(QPoint(x, y), text);
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

double PlotCanvas::position()
{
    double data_extent = total_x_range.extent();
    double offset = view_x_range.min - total_x_range.min;
    if (data_extent > 0)
        return offset / data_extent;
    else
        return 0;
}

void PlotCanvas::setPosition(double value)
{
    double extent = total_x_range.extent();
    double offset = value * extent + total_x_range.min;
    setOffset(offset);
}

void PlotCanvas::setOffset(double offset)
{
    double extent = view_x_range.extent();
    double max_offset = total_x_range.max - extent;
    offset = std::max(total_x_range.min, offset);
    offset = std::min(max_offset, offset);

    if (offset == view_x_range.min)
        return;

    view_x_range.min = offset;
    view_x_range.max = offset + extent;

    update();
}

double PlotCanvas::range()
{
    double data_extent = total_x_range.extent();
    double view_extent = view_x_range.extent();
    if (data_extent > 0)
        return view_extent / data_extent;
    else
        return 0;
}

void PlotCanvas::setRange(double value)
{
    double extent = total_x_range.extent();
    double size = value * extent;
    setSize(size);
}

void PlotCanvas::setSize(double size)
{
    double extent = total_x_range.extent();

    size = std::min(extent, size);
    size = std::max(0.0, size);

    if (size == view_x_range.extent())
        return;

    double max_offset = total_x_range.max - size;
    view_x_range.min = std::min(view_x_range.min, max_offset);
    view_x_range.max = view_x_range.min + size;

    update();
}

}
