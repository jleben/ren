#include "plot_view.hpp"
#include "plot.hpp"

#include <QPainter>
#include <QDebug>
#include <algorithm>
#include <iostream>

using namespace std;

namespace datavis {

PlotView::PlotView(QWidget * parent):
    QWidget(parent)
{}

void PlotView::addPlot(Plot * plot)
{
    m_plots.push_back(plot);

    updatePlotMap();
    updateViewMap();
}

void PlotView::updateViewMap()
{
    auto size = this->size();

    int margin = 10;

    QTransform map;
    map.translate(margin, size.height() - margin);
    map.scale(size.width() - 2 * margin, - (size.height() - 2 * margin));

    m_view_map = m_plot_map * map;
}

void PlotView::updatePlotMap()
{
    Plot::Range total_range;

    {
        bool first = true;
        for (auto plot : m_plots)
        {
            if (plot->isEmpty())
                continue;

            auto range = plot->range();

            if (first)
                total_range = range;
            else
            {
                double min_x = min(total_range.min.x(), range.min.x());
                double min_y = min(total_range.min.y(), range.min.y());
                double max_x = max(total_range.max.x(), range.max.x());
                double max_y = max(total_range.max.y(), range.max.y());

                total_range.min = QPointF(min_x, min_y);
                total_range.max = QPointF(max_x, max_y);
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
    QPointF extent = total_range.max - total_range.min;
    QPointF offset = total_range.min;
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

void PlotView::resizeEvent(QResizeEvent*)
{
    updateViewMap();
}

void PlotView::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);

    for (auto plot : m_plots)
    {
        if (plot->isEmpty())
            continue;

        plot->plot(&painter, m_view_map);
    }
}

}
