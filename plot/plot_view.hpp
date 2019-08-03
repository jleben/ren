#pragma once

#include "plot.hpp"

#include <QWidget>
#include <QSlider>
#include <QGridLayout>
#include <list>

namespace datavis {

using std::list;
class Plot;
class PlotCanvas;
class RangeBar;

class PlotCanvas : public QWidget
{
    Q_OBJECT

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

    QPoint m_last_mouse_pos;

private:
    void selectDataAt(const QPoint &);

signals:
    void rangeChanged();
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
    void onCanvasRangeChanged();

    PlotCanvas * m_canvas;
    RangeBar * m_range_bar;
};

class PlotRangeController : public QObject
{
    Q_OBJECT

public:
    PlotRangeController(QObject * parent = nullptr): QObject(parent) {}

    void setValue(const Plot::Range & range)
    {
        m_value = range;
        emit changed();
    }

    void setLimit(const Plot::Range & range)
    {
        m_limit = range;
        emit changed();
    }

    const Plot::Range & value() const { return m_value; }
    const Plot::Range & limit() const { return m_limit; }

signals:
    void changed();

private:
    Plot::Range m_value;
    Plot::Range m_limit;
};

class PlotView2 : public QWidget
{
    Q_OBJECT

public:
    PlotView2(QWidget * parent = nullptr);
    ~PlotView2();

    void setPlot(Plot * plot);
    Plot * plot() const { return m_plot; }

    virtual void wheelEvent(QWheelEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

    void setRangeController(PlotRangeController * ctl, Qt::Orientation);

private:
    Plot * m_plot = nullptr;
    PlotRangeController * m_x_range = nullptr;
    PlotRangeController * m_y_range = nullptr;
};

class PlotGridView : public QWidget
{
    Q_OBJECT
public:
    PlotGridView(QWidget * parent = nullptr);

    int columnCount() const { return m_grid->columnCount(); }
    int rowCount() const { return m_grid->rowCount(); }

    // Adding a plot takes ownership
    void setColumnCount(int count);
    void setRowCount(int count);
    void addPlot(Plot*, int row, int column);
    void removePlot(Plot*);
    void removePlot(int row, int column);

    Plot * plotAt(const QPoint & pos);
    Plot * plotAtCell(int row, int column);

    bool hasSelectedCell() const { return m_selected_view != nullptr; }
    QPoint selectedCell() const { return m_selected_cell; }

    QSize sizeHint() const override { return QSize(600,400); }

private:
    PlotView2 * viewAtIndex(int index);
    PlotView2 * viewAtCell(int row, int column);
    PlotView2 * viewAtPoint(const QPoint & pos);
    QPoint findView(PlotView2 * view);
    PlotView2 * makeView();
    void deleteView(PlotView2* view);
    void selectView(PlotView2* view);

    void updateDataRange();

    virtual bool eventFilter(QObject*, QEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

    QGridLayout * m_grid = nullptr;

    vector<PlotRangeController*> m_x_range_ctls;
    vector<PlotRangeController*> m_y_range_ctls;

    QPoint m_selected_cell;
    PlotView2 * m_selected_view = nullptr;
};

}
