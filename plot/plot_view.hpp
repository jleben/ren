#pragma once

#include "plot.hpp"

#include <QWidget>
#include <QSlider>
#include <list>

namespace datavis {

using std::list;
class Plot;
class Selector;
class PlotCanvas;

class PlotCanvas : public QWidget
{
public:
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

    void updateViewMap();
    void updatePlotMap();

    list<Plot*> m_plots;
    QTransform m_view_map;
    QTransform m_plot_map;
    bool m_stacked = false;
    bool m_common_x = true;
    bool m_common_y = true;

    Plot::Range total_x_range;
    Plot::Range total_y_range;
};

class PlotView : public QWidget
{
public:
    PlotView(QWidget * parent = 0);

    void addPlot(Plot*);
    void removePlot(Plot*);

    bool isStacked() const { return m_canvas->m_stacked; }
    void setStacked(bool value);

    bool hasCommonX() const { return m_canvas->m_common_x; }
    void setCommonX(bool value);

    bool hasCommonY() const { return m_canvas->m_common_y; }
    void setCommonY(bool value);

private:
    void onPlotRangeChanged();
    void onPlotSelectorRangeChanged();
    void onPlotContentChanged();

    void updateSelectorRange();

    PlotCanvas * m_canvas;
    Selector * m_selector;
    QSlider * m_selector_slider;
};

}
