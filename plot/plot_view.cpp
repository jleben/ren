#include "plot_view.hpp"
#include "range_bar.hpp"
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

PlotGridView::PlotGridView(QWidget * parent):
    QWidget(parent)
{
    m_grid = new QGridLayout(this);

    m_x_range_ctls.push_back(new PlotRangeController(this));
    m_y_range_ctls.push_back(new PlotRangeController(this));

    auto view = new PlotView2;
    view->setRangeController(m_x_range_ctls.front(), Qt::Horizontal);
    view->setRangeController(m_y_range_ctls.front(), Qt::Vertical);

    m_grid->addWidget(view, 0, 0);

    //printf("Rows: %d, Columns: %d\n", rowCount(), columnCount());
}

void PlotGridView::setRowCount(int count)
{
    //cout << "Setting row count: " << count << endl;

    if (count < 1)
        return;

    if (count < rowCount())
    {
        for (int row = count; row < rowCount(); ++row)
        {
            for (int col = 0; col < columnCount(); ++col)
            {
                auto * item = m_grid->itemAtPosition(row, col);
                delete item;
            }

            delete m_y_range_ctls.back();
            m_y_range_ctls.pop_back();
        }
    }
    else if (count > rowCount())
    {
        for (int row = rowCount(); row < count; ++row)
        {
            auto y_ctl = new PlotRangeController;
            m_y_range_ctls.push_back(y_ctl);

            for (int col = 0; col < columnCount(); ++col)
            {
                auto x_ctl = m_x_range_ctls[col];

                auto view = new PlotView2;
                view->setRangeController(x_ctl, Qt::Horizontal);
                view->setRangeController(y_ctl, Qt::Vertical);

                m_grid->addWidget(view, row, col);
            }
        }
    }
}

void PlotGridView::setColumnCount(int count)
{
    //cout << "Setting column count: " << count << endl;

    if (count < 1)
        return;

    if (count < columnCount())
    {
        for (int col = count; col < columnCount(); ++col)
        {
            for (int row = 0; row < rowCount(); ++row)
            {
                auto item = m_grid->itemAtPosition(row, col);
                delete item;
            }
        }
    }
    else if (count > columnCount())
    {
        for (int col = columnCount(); col < count; ++col)
        {
            for (int row = 0; row < rowCount(); ++row)
            {
                m_grid->addWidget(new PlotView2, row, col);
            }
        }
    }
}

void PlotGridView::addPlot(Plot * plot, int row, int column)
{
    cout << "Adding plot at " << row << ", " << column << endl;

    if (row < 0 or column < 0)
        return;

    //printf("Rows: %d, Columns: %d\n", rowCount(), columnCount());

    if (row+1 > rowCount())
        setRowCount(row+1);

    if (column+1 > columnCount())
        setColumnCount(column+1);

    auto view = viewAtCell(row, column);
    view->setPlot(plot);

    updateDataRange();
}

void PlotGridView::addPlotToColumn(Plot * plot, int column)
{
    int row = rowCount()-1;
    if (plotAtCell(row, column))
        ++row;

    addPlot(plot, row, column);
}

void PlotGridView::removePlot(int row, int column)
{
    auto item = m_grid->itemAtPosition(row, column);
    if (!item)
        return;

    auto plot_view = qobject_cast<PlotView2*>(item->widget());
    if (!plot_view)
        return;

    delete plot_view;

    //removeWidget(plot_view);

    // TODO: disconnecting should be part of plot_view desctructor?
    //disconnect(plot_view->plot());

    // TODO: other stuff, update data range...
}

void PlotGridView::removePlot(Plot * plot)
{
    for (int i = 0; i < m_grid->count(); ++i)
    {
        auto item = m_grid->itemAt(i);
        if (!item)
            continue;

        auto plot_view = qobject_cast<PlotView2*>(item->widget());
        if (!plot_view)
            return;

        if (plot_view->plot() == plot)
        {
            delete plot_view;
            // TODO:  other stuff...

            return;
        }
    }
}

Plot * PlotGridView::plotAt(const QPoint & pos)
{
    for (int row = 0; row < m_grid->rowCount(); ++row)
    {
        for (int col = 0; col < m_grid->columnCount(); ++col)
        {
            if (m_grid->cellRect(row, col).contains(pos))
            {
                return plotAtCell(row, col);
            }
        }
    }

    return nullptr;
}

Plot * PlotGridView::plotAtCell(int row, int column)
{
    auto plot_view = viewAtCell(row, column);
    if (!plot_view)
        return nullptr;

    return plot_view->plot();
}

PlotView2 * PlotGridView::viewAtIndex(int index)
{
    auto item = m_grid->itemAt(index);
    if (!item)
        return nullptr;

    return qobject_cast<PlotView2*>(item->widget());
}

PlotView2 * PlotGridView::viewAtCell(int row, int column)
{
    auto item = m_grid->itemAtPosition(row, column);
    if (!item)
        return nullptr;

    return qobject_cast<PlotView2*>(item->widget());
}

void PlotGridView::updateDataRange()
{
    printf("updating data range");

    for (int row = 0; row < rowCount(); ++row)
    {
        Plot::Range total_range;

        bool first = true;
        for (int col = 0; col < columnCount(); ++col)
        {
            auto plot = plotAtCell(row, col);
            if (!plot)
                continue;

            auto range = plot->yRange();

            if (first)
            {
                total_range = range;
            }
            else
            {
                total_range.min = std::min(total_range.min, range.min);
                total_range.max = std::max(total_range.max, range.max);
            }
        }

        m_y_range_ctls[row]->setLimit(total_range);
        m_y_range_ctls[row]->setValue(total_range);
    }

    for (int col = 0; col < columnCount(); ++col)
    {
        Plot::Range total_range;

        bool first = true;
        for (int row = 0; row < rowCount(); ++row)
        {
            auto plot = plotAtCell(row, col);
            if (!plot)
                continue;

            auto range = plot->xRange();

            if (first)
            {
                total_range = range;
            }
            else
            {
                total_range.min = std::min(total_range.min, range.min);
                total_range.max = std::max(total_range.max, range.max);
            }
        }

        m_x_range_ctls[col]->setLimit(total_range);
        m_x_range_ctls[col]->setValue(total_range);
    }
}

////

PlotView2::PlotView2(QWidget * parent):
    QWidget(parent)
{
}

PlotView2::~PlotView2()
{
    delete m_plot;
}

void PlotView2::setPlot(Plot *plot)
{
    if (m_plot)
        delete m_plot;

    m_plot = plot;

    update();
}

void PlotView2::setRangeController(PlotRangeController * ctl, Qt::Orientation orientation)
{
    PlotRangeController * old_ctl = nullptr;

    if (orientation == Qt::Horizontal)
    {
        old_ctl = m_x_range;
        m_x_range = ctl;
    }
    else
    {
        old_ctl = m_y_range;
        m_y_range = ctl;
    }

    disconnect(old_ctl);

    connect(ctl, SIGNAL(changed()),
            this, SLOT(update()));

    update();
}

void PlotView2::wheelEvent(QWheelEvent* event)
{
    if (event->phase() != Qt::ScrollUpdate && event->phase() != Qt::NoScrollPhase)
        return;

    double degrees;
    bool vertical;

    if (event->angleDelta().y() != 0)
    {
        degrees = event->angleDelta().y();
        vertical = true;
    }
    else
    {
        degrees = event->angleDelta().x();
        vertical = false;
    }

    degrees /= 8.0;

    bool zoom = event->modifiers() & Qt::ControlModifier;

    //printf("zoom: %d, vertical: %d, degrees: %f\n", zoom, vertical, degrees);

    if (zoom)
    {
        auto * range = vertical ? m_y_range : m_x_range;

        if (!range)
            return;

        auto value = range->value();

        //printf("Value: %f, %f\n", value.min, value.max);

        double extent = value.extent();
        extent *= std::pow(2.0, -degrees / 180.0);

        //printf("Extent %f -> %f\n", value.extent(), extent);


        value.max = value.min + extent;
        value.max = std::min(value.max, range->limit().max);
        value.max = std::max(value.max, value.min);

        //printf("Limit: %f, %f\n", range->limit().min, range->limit().max);
        //printf("New Value: %f, %f\n", value.min, value.max);

        range->setValue(value);
    }
    else
    {
        auto * range = vertical ? m_y_range : m_x_range;

        if (!range)
            return;

        auto value = range->value();

        double visible_extent = value.extent();
        double offset = visible_extent * 0.5 * degrees/360.0;

        if (!vertical)
            offset = -offset;

        value.min += offset;
        value.min = std::min(value.min, range->limit().max - visible_extent);
        value.min = std::max(value.min, range->limit().min);

        value.max = value.min + visible_extent;

        //printf("Limit: %f, %f\n", range->limit().min, range->limit().max);
        //printf("Value: %f, %f\n", value.min, value.max);

        range->setValue(value);
    }
}

void PlotView2::paintEvent(QPaintEvent*)
{
    //auto mouse_pos = mapFromGlobal(QCursor::pos());

    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);

    painter.setBrush(Qt::NoBrush);

    QPen frame_pen;
    frame_pen.setColor(Qt::lightGray);

    auto plot_rect = this->rect().adjusted(10,10,-10,-10);

    painter.setPen(frame_pen);
    painter.drawRect(plot_rect);

    auto plot = m_plot;

    if (!plot or plot->isEmpty())
        return;

    Mapping2d map;

    //Plot::Range x_range = m_common_x ? view_x_range : plot->xRange();
    //Plot::Range y_range = m_common_y ? total_y_range : plot->yRange();

    Plot::Range x_range = m_x_range ? m_x_range->value() : plot->xRange();
    Plot::Range y_range = m_y_range ? m_y_range->value() : plot->yRange();

    cout << "X range: " << x_range.min << ", " << x_range.max << endl;
    cout << "Y range: " << y_range.min << ", " << y_range.max << endl;

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

    painter.setClipping(false);

#if 0
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

            QString text;

            QStringList dim_strings;
            for (double v : get<0>(dataPos))
            {
                dim_strings << QString::number(v, 'f', 2);
            }

            QStringList att_string;
            for (auto & v : get<1>(dataPos))
                att_string << QString::number(v);

            text = att_string.join(" ") + " @ " + dim_strings.join(" ");

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
#endif
}



////

PlotView::PlotView(QWidget * parent):
    QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_canvas = new PlotCanvas;
    m_canvas->setMouseTracking(true);

    m_range_bar = new RangeBar;

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->addWidget(m_canvas);
    layout->addWidget(m_range_bar);

    connect(m_canvas, &PlotCanvas::rangeChanged,
            this, &PlotView::onCanvasRangeChanged);
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

void PlotView::onCanvasRangeChanged()
{
    m_range_bar->setRange(m_canvas->position(), m_canvas->range());
}

QRect PlotCanvas::plotRect(int index)
{
    int plot_count = (int) m_plots.size();

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

    emit rangeChanged();
}

void PlotCanvas::selectDataAt(const QPoint & pos)
{
    for (int i = 0; i < m_plots.size(); ++i)
    {
        auto plot = m_plots[i];
        auto plotPos = mapToPlot(i, pos);
        auto dataPos = plot->dataLocation(plotPos);
        DataSetPtr dataset = plot->dataSet();
        for (int d = 0; d < dataset->dimensionCount(); ++d)
        {
            DimensionPtr dim = dataset->globalDimension(d);
            if (!dim) continue;
            dim->setFocus(get<0>(dataPos)[d]);
        }
    }
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

void PlotCanvas::mouseMoveEvent(QMouseEvent * event)
{
    if ( event->modifiers() & Qt::ControlModifier &&
         event->buttons() & Qt::LeftButton &&
         !m_plots.empty() )
    {
        auto plot_rect = plotRect(0);
        auto mouse_distance = event->pos() - m_last_mouse_pos;
        auto rel_distance = plot_rect.width() > 0 ?
                    mouse_distance.x() / double(plot_rect.width()) : 0;
        setOffset(view_x_range.min - rel_distance * view_x_range.extent());
    }
    else if (event->buttons() & Qt::LeftButton)
    {
        selectDataAt(event->pos());
    }

    m_last_mouse_pos = event->pos();

    update();
}

void PlotCanvas::mousePressEvent(QMouseEvent* event)
{
    selectDataAt(event->pos());
}

void PlotCanvas::wheelEvent(QWheelEvent* event)
{
    if (event->phase() != Qt::ScrollUpdate && event->phase() != Qt::NoScrollPhase)
        return;

    if (m_plots.empty())
        return;

    double degrees = event->angleDelta().y() / 8.0;

    if (event->modifiers() & Qt::ControlModifier)
    {
        auto plot_rect = plotRect(0);
        auto mouse_rel_x = plot_rect.width() > 0
                ? (event->pos().x() - plot_rect.x()) / double(plot_rect.width())
                : 0;
        auto mouse_x =  mouse_rel_x * view_x_range.extent() + view_x_range.min;

        double new_size = view_x_range.extent() * pow(2.0, -1.0 * degrees/360.0);
        setSize(new_size);

        auto new_x = mouse_x - mouse_rel_x * view_x_range.extent();
        setOffset(new_x);
    }
    else if (event->modifiers() & Qt::ShiftModifier)
    {
        double new_offset = view_x_range.min + 0.5 * view_x_range.extent() * degrees/360.0;
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

            QString text;

            QStringList dim_strings;
            for (double v : get<0>(dataPos))
            {
                dim_strings << QString::number(v, 'f', 2);
            }

            QStringList att_string;
            for (auto & v : get<1>(dataPos))
                att_string << QString::number(v);

            text = att_string.join(" ") + " @ " + dim_strings.join(" ");

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

    emit rangeChanged();

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

    emit rangeChanged();

    update();
}

}
