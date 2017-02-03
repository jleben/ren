#pragma once

#include "plot.hpp"

#include <QWidget>
#include <QSlider>
#include <list>

namespace datavis {

using std::list;
class Plot;
class PlotCanvas;

class PlotCanvas : public QWidget
{
public:
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void enterEvent(QEvent*) override;
    virtual void leaveEvent(QEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void wheelEvent(QWheelEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

    void updateDataRange();
    QRect plotRect(int index);
    QPointF mapToPlot(int plotIndex, const QPointF & pos);

    double position();
    void setPosition(double value);
    void setOffset(double value);

    double range();
    void setRange(double value);
    void setSize(double value);

    vector<Plot*> m_plots;

    int m_margin = 10;
    bool m_stacked = false;
    bool m_common_x = true;
    bool m_common_y = false;

    Plot::Range total_x_range;
    Plot::Range total_y_range;

    Plot::Range view_x_range;
};

class PlotView : public QWidget
{
    Q_OBJECT

public:
    PlotView(QWidget * parent = 0);

    void addPlot(Plot*);
    void removePlot(Plot*);

    const vector<Plot*> plots() { return m_canvas->m_plots; }

    bool isStacked() const { return m_canvas->m_stacked; }
    void setStacked(bool value);

    bool hasCommonX() const { return m_canvas->m_common_x; }
    void setCommonX(bool value);

    bool hasCommonY() const { return m_canvas->m_common_y; }
    void setCommonY(bool value);

    Plot * plotAt(const QPoint & pos);

    double position() { return m_canvas->position(); }
    void setPosition(double value) { m_canvas->setPosition(value); }

    double range() { return m_canvas->range(); }
    void setRange(double value) { m_canvas->setRange(value); }

    QSize sizeHint() const override { return QSize(600,400); }

private:
    void onPlotRangeChanged();
    void onPlotContentChanged();

    PlotCanvas * m_canvas;
};

}
