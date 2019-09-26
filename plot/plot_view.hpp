#pragma once

#include "plot.hpp"

#include <QWidget>
#include <QSlider>
#include <QGridLayout>
#include <list>

namespace datavis {

using std::list;
class Plot;

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

    void moveTo(double pos);
    void scaleTo(double extent);

    const Plot::Range & value() const { return m_value; }
    const Plot::Range & limit() const { return m_limit; }

signals:
    void changed();

private:
    Plot::Range m_value;
    Plot::Range m_limit;
};

class PlotView : public QWidget
{
    Q_OBJECT

public:
    PlotView(QWidget * parent = nullptr);
    ~PlotView();

    void setPlot(Plot * plot);
    Plot * plot() const { return m_plot; }
    QRect plotRect() const;
    QPointF mapToPlot(const QPointF & pos);
    QPointF mapDistanceToPlot(const QPointF & distance);
    void selectDataAt(const QPoint &);

    void setRangeController(PlotRangeController * ctl, Qt::Orientation);

protected:
    virtual void enterEvent(QEvent*) override;
    virtual void leaveEvent(QEvent*) override;
    virtual void mouseMoveEvent(QMouseEvent*) override;
    virtual void mousePressEvent(QMouseEvent*) override;
    virtual void wheelEvent(QWheelEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

private:
    enum MouseInteraction
    {
        NoMouseInteraction,
        MouseZoom,
        MouseShift,
        MouseFocusData
    };

    Plot * m_plot = nullptr;
    PlotRangeController * m_x_range = nullptr;
    PlotRangeController * m_y_range = nullptr;

    MouseInteraction m_mouse_interaction = NoMouseInteraction;
    QPoint m_mouse_press_point;
    Plot::Range m_x_range_start;
    Plot::Range m_y_range_start;
    QPointF m_mouse_press_plot_point;
};

class PlotReticle : public QWidget
{
public:
    PlotReticle(QWidget * parent = nullptr);
    virtual void paintEvent(QPaintEvent*) override;
};

class RangeView : public QWidget
{
    Q_OBJECT
public:
    RangeView(Qt::Orientation, PlotRangeController *, QWidget * parent = nullptr);
    Qt::Orientation orientation() const { return m_orientation; }
    virtual void paintEvent(QPaintEvent*) override;
    virtual QSize sizeHint() const override { return QSize(10,10); }

private:
    PlotRangeController * m_ctl;
    Qt::Orientation m_orientation;
    QColor m_background_color;
    QColor m_foreground_color;
};


class PlotGridView : public QWidget
{
    Q_OBJECT
public:
    class Cell
    {
    public:
        Cell() {}
        Cell(int r, int c): row(r), column(c) {}
        int row;
        int column;
    };

    struct DroppedDataset
    {
        string source_id;
        string dataset_id;
        PlotGridView * view = nullptr;
        int row;
        int column;
        bool insert_row = false;
        bool insert_col = false;
    };

    PlotGridView(QWidget * parent = nullptr);

    int columnCount() const { return m_columnCount; }
    int rowCount() const { return m_rowCount; }

    void addRow();
    void insertRow(int row);
    void removeRow(int row);
    void removeSelectedRow();
    void setRowCount(int count);

    void addColumn();
    void insertColumn(int column);
    void removeColumn(int column);
    void removeSelectedColumn();
    void setColumnCount(int count);

    void swapRows(int a, int b);
    void swapColumns(int a, int b);
    void moveSelectedRowDown();
    void moveSelectedRowUp();
    void moveSelectedColumnLeft();
    void moveSelectedColumnRight();

    // Adding a plot takes ownership
    void addPlot(Plot*, int row, int column);
    void removePlot(Plot*);
    void removePlot(int row, int column);
    void removeSelectedPlot();

    Plot * plotAt(const QPoint & pos);
    Plot * plotAtCell(int row, int column);

    bool hasSelectedCell() const { return m_selected_view != nullptr; }
    QPoint selectedCell() const { return m_selected_cell; }

    QSize sizeHint() const override { return QSize(600,400); }

signals:
    // QVariant = DroppedDataset
    void datasetDropped(QVariant);

private:
    PlotView * viewAtIndex(int index);
    PlotView * viewAtCell(int row, int column);
    PlotView * viewAtPoint(const QPoint & pos);
    QPoint findView(PlotView * view);
    Cell findView2(PlotView * view);
    void prepareDrop(PlotView * view, const QPoint &);
    PlotView * makeView();
    void deleteView(PlotView* view);
    void selectView(PlotView* view);

    void updateDataRange();

    virtual bool eventFilter(QObject*, QEvent*) override;
    virtual bool event(QEvent *event) override;
    virtual void enterEvent(QEvent*) override;
    virtual void leaveEvent(QEvent*) override;
    virtual void resizeEvent(QResizeEvent*) override;
    virtual void paintEvent(QPaintEvent*) override;

    void printState();

    QGridLayout * m_grid = nullptr;
    int m_rowCount = 0;
    int m_columnCount = 0;

    vector<PlotRangeController*> m_x_range_ctls;
    vector<PlotRangeController*> m_y_range_ctls;
    vector<RangeView*> m_x_range_views;
    vector<RangeView*> m_y_range_views;

    QPoint m_selected_cell;
    PlotView * m_selected_view = nullptr;

    struct
    {
        Cell cell { -1, -1 };
        Cell offset { 0, 0 };
    }
    m_drop;

    PlotReticle * m_reticle = nullptr;
};

}

Q_DECLARE_METATYPE(datavis::PlotGridView::DroppedDataset);
