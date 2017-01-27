#include "plot_view.hpp"
#include "plot.hpp"
#include "selector.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QDebug>
#include <QCursor>
#include <algorithm>
#include <iostream>
#include <iterator>

using namespace std;

namespace datavis {

PlotView::PlotView(QWidget * parent):
    QWidget(parent)
{
    m_canvas = new PlotCanvas;
    m_canvas->setMouseTracking(true);

    m_selector = new Selector;
    m_selector_slider = new QSlider;
    m_selector_slider->setOrientation(Qt::Horizontal);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_canvas);
    layout->addWidget(m_selector_slider);

    connect(m_selector, &Selector::valueChanged,
            m_selector_slider, &QSlider::setValue);
    connect(m_selector_slider, &QSlider::valueChanged,
            m_selector, &Selector::setValue);
}

void PlotView::addPlot(Plot * plot)
{
    if (!plot)
        return;

    plot->setSelector(m_selector);

    m_canvas->m_plots.push_back(plot);

    connect(plot, &Plot::xRangeChanged,
            this, &PlotView::onPlotRangeChanged);
    connect(plot, &Plot::yRangeChanged,
            this, &PlotView::onPlotRangeChanged);
    connect(plot, &Plot::selectorRangeChanged,
            this, &PlotView::onPlotSelectorRangeChanged);
    connect(plot, &Plot::contentChanged,
            this, &PlotView::onPlotContentChanged);

    updateSelectorRange();

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
        plot->setSelector(nullptr);
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

void PlotView::onPlotSelectorRangeChanged()
{
    updateSelectorRange();
}

void PlotView::onPlotContentChanged()
{
    m_canvas->update();
}

void PlotView::updateSelectorRange()
{
    double min;
    double max;
    bool first = true;
    for (auto plot : m_canvas->m_plots)
    {
        auto range = plot->selectorRange();
        if (first)
        {
            min = range.min;
            max = range.max;
        }
        else
        {
            min = std::min(min, range.min);
            max = std::max(max, range.max);
        }
        first = false;
    }

    qDebug() << "Plot selector range:" << min << "," << max;

    m_selector_slider->setRange(min, max);
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

void PlotCanvas::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);

    if (m_plots.empty())
        return;

    int margin = 10;

    int plot_width = width() - 2 * margin;

    int plot_height;
    if (m_stacked)
        plot_height = height() - 2 * margin;
    else
        plot_height = (height() - margin) / int(m_plots.size()) - margin;

    int plot_x = margin;
    int plot_y = margin;

    painter.setBrush(Qt::NoBrush);

    QPen frame_pen;
    frame_pen.setColor(Qt::lightGray);

    for (auto plot : m_plots)
    {
        {
            QRect frame_rect(plot_x, plot_y, plot_width, plot_height);
            painter.setPen(frame_pen);
            painter.drawRect(frame_rect);
        }

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
        map.scale(plot_width, -plot_height);
        map.translate(plot_x, plot_y + plot_height);

        plot->plot(&painter, map);

        if (!m_stacked)
            plot_y += plot_height + margin;
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
