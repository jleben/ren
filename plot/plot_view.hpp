#pragma once

#include <QWidget>
#include <QSlider>
#include <list>

namespace datavis {

using std::list;
class Plot;
class Selector;
class PlotCanvas;

class PlotView : public QWidget
{
public:
    PlotView(QWidget * parent = 0);

    void addPlot(Plot*);
    void removePlot(Plot*);

private:
    void onPlotRangeChanged();
    void onPlotSelectorRangeChanged();
    void onPlotContentChanged();

    void updateSelectorRange();

    PlotCanvas * m_canvas;
    Selector * m_selector;
    QSlider * m_selector_slider;
};

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
};

}
