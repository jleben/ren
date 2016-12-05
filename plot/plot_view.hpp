#pragma once

#include <QWidget>
#include <list>

namespace datavis {

using std::list;
class Plot;

class PlotView : public QWidget
{
public:
    PlotView(QWidget * parent = 0);

    void addPlot(Plot*);
    void removePlot(Plot*);

    virtual void resizeEvent(QResizeEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

private:
    void onPlotRangeChanged();
    void onPlotContentChanged();

    void updateViewMap();
    void updatePlotMap();

    list<Plot*> m_plots;
    QTransform m_view_map;
    QTransform m_plot_map;
};

}
