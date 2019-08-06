#include "plot_view.hpp"
#include "plot.hpp"
#include "../utility/vector.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
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
    auto vbox = new QVBoxLayout(this);

    auto toolbox = new QHBoxLayout;
    vbox->addLayout(toolbox);

    {
        auto button = new QToolButton;
        button->setText("+Row");
        connect(button, &QAbstractButton::clicked,
                this, &PlotGridView::addRow);
        toolbox->addWidget(button);
    }

    {
        auto button = new QToolButton;
        button->setText("-Row");
        connect(button, &QAbstractButton::clicked,
                this, &PlotGridView::removeRow);
        toolbox->addWidget(button);
    }

    {
        auto button = new QToolButton;
        button->setText("+Col");
        connect(button, &QAbstractButton::clicked,
                this, &PlotGridView::addColumn);
        toolbox->addWidget(button);
    }

    {
        auto button = new QToolButton;
        button->setText("-Col");
        connect(button, &QAbstractButton::clicked,
                this, &PlotGridView::removeColumn);
        toolbox->addWidget(button);
    }

    toolbox->addStretch();

    m_grid = new QGridLayout;
    vbox->addLayout(m_grid);

    addRow();
    addColumn();

    m_reticle = new PlotReticle(this);
    m_reticle->hide();

    selectView(viewAtCell(0,0));

    //printf("Rows: %d, Columns: %d\n", rowCount(), columnCount());
}

PlotView * PlotGridView::makeView()
{
    auto view = new PlotView;
    view->installEventFilter(this);
    return view;
}

void PlotGridView::deleteView(PlotView* view)
{
    if (view && m_selected_view == view)
    {
        selectView(nullptr);
    }

    delete view;
}

void PlotGridView::selectView(PlotView* view)
{
    m_selected_view = view;
    if (view)
    {
        m_selected_cell = findView(view);
    }
    else
    {
        m_selected_cell = QPoint(-1, -1);
    }

    update();
}

void PlotGridView::addRow()
{
    auto y_ctl = new PlotRangeController;
    m_y_range_ctls.push_back(y_ctl);

    auto y_bar = new RangeView(Qt::Vertical, y_ctl);
    m_y_range_views.push_back(y_bar);

    m_grid->addWidget(y_bar, m_rowCount+1, 0);

    for (int col = 0; col < columnCount(); ++col)
    {
        auto x_ctl = m_x_range_ctls[col];

        auto view = makeView();
        view->setRangeController(x_ctl, Qt::Horizontal);
        view->setRangeController(y_ctl, Qt::Vertical);

        m_grid->addWidget(view, m_rowCount+1, col+1);
    }

    ++m_rowCount;

    updateDataRange();

    update();
}

void PlotGridView::removeRow()
{
    if (m_rowCount <= 1)
        return;

    for (int col = 0; col < columnCount(); ++col)
    {
        deleteView(viewAtCell(m_rowCount-1, col));
    }

    delete m_y_range_views.back();
    m_y_range_views.pop_back();

    delete m_y_range_ctls.back();
    m_y_range_ctls.pop_back();

    --m_rowCount;

    updateDataRange();

    update();
}

void PlotGridView::setRowCount(int count)
{
    //cout << "Setting row count: " << count << endl;

    if (count < 1)
        return;

    while(rowCount() > count)
    {
        removeRow();
    }

    while(rowCount() < count)
    {
        addRow();
    }
}

void PlotGridView::addColumn()
{
    auto x_ctl = new PlotRangeController;
    m_x_range_ctls.push_back(x_ctl);

    auto x_bar = new RangeView(Qt::Horizontal, x_ctl);
    m_x_range_views.push_back(x_bar);

    m_grid->addWidget(x_bar, 0, m_columnCount+1);

    for (int row = 0; row < rowCount(); ++row)
    {
        auto y_ctl = m_y_range_ctls[row];

        auto view = makeView();
        view->setRangeController(x_ctl, Qt::Horizontal);
        view->setRangeController(y_ctl, Qt::Vertical);

        m_grid->addWidget(view, row+1, m_columnCount+1);
    }

    ++m_columnCount;

    updateDataRange();

    update();
}

void PlotGridView::removeColumn()
{
    if (m_columnCount <= 1)
        return;

    for (int row = 0; row < rowCount(); ++row)
    {
        deleteView(viewAtCell(row, m_columnCount-1));
    }

    delete m_x_range_views.back();
    m_x_range_views.pop_back();

    delete m_x_range_ctls.back();
    m_x_range_ctls.pop_back();

    --m_columnCount;

    updateDataRange();

    update();
}

void PlotGridView::setColumnCount(int count)
{
    //cout << "Setting column count: " << count << endl;

    if (count < 1)
        return;

    while(columnCount() > count)
    {
        removeColumn();
    }

    while(columnCount() < count)
    {
        addColumn();
    }
}

void PlotGridView::addPlot(Plot * plot, int row, int column)
{
    //cout << "Adding plot at " << row << ", " << column << endl;

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

void PlotGridView::removePlot(int row, int column)
{
    auto view = viewAtCell(row, column);

    if (view)
        view->setPlot(nullptr);

    updateDataRange();
}

void PlotGridView::removePlot(Plot * plot)
{
    for (int i = 0; i < m_grid->count(); ++i)
    {
        auto view = viewAtIndex(i);
        if (view->plot() == plot)
        {
            view->setPlot(nullptr);
            updateDataRange();
            return;
        }
    }
}

Plot * PlotGridView::plotAt(const QPoint & pos)
{
    auto * view = viewAtPoint(pos);

    if (view)
        return view->plot();

    return nullptr;
}

Plot * PlotGridView::plotAtCell(int row, int column)
{
    auto plot_view = viewAtCell(row, column);
    if (!plot_view)
        return nullptr;

    return plot_view->plot();
}

PlotView * PlotGridView::viewAtIndex(int index)
{
    auto item = m_grid->itemAt(index);
    if (!item)
        return nullptr;

    return qobject_cast<PlotView*>(item->widget());
}

PlotView * PlotGridView::viewAtCell(int row, int column)
{
    auto item = m_grid->itemAtPosition(row+1, column+1);
    if (!item)
        return nullptr;

    return qobject_cast<PlotView*>(item->widget());
}

PlotView * PlotGridView::viewAtPoint(const QPoint & pos)
{
    for (int row = 0; row < rowCount(); ++row)
    {
        for (int col = 0; col < columnCount(); ++col)
        {
            if (m_grid->cellRect(row, col).contains(pos))
            {
                return viewAtCell(row, col);
            }
        }
    }

    return nullptr;
}

QPoint PlotGridView::findView(PlotView * view)
{
    for (int row = 0; row < rowCount(); ++row)
    {
        for (int col = 0; col < columnCount(); ++col)
        {
            if (viewAtCell(row, col) == view)
                return QPoint(col, row);
        }
    }

    return QPoint(-1, -1);
}

void PlotGridView::updateDataRange()
{
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

bool PlotGridView::eventFilter(QObject * object, QEvent * event)
{
    switch(event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto view = qobject_cast<PlotView*>(object);
        if (!view)
            return false;

        selectView(view);

        break;
    }
    case QEvent::MouseMove:
    {
        m_reticle->update();
        break;
    }
    case QEvent::Enter:
    {
        m_reticle->show();
        break;
    }
    case QEvent::Leave:
    {
        m_reticle->hide();
        break;
    }
    default:
        break;
    }

    return false;
}

bool PlotGridView::event(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::ChildAdded:
    {
        if (m_reticle)
            m_reticle->raise();
        break;
    }
    default:
        break;
    }

    return QWidget::event(event);
}

void PlotGridView::resizeEvent(QResizeEvent*)
{
    m_reticle->setGeometry(m_grid->geometry());
}

void PlotGridView::enterEvent(QEvent*)
{
    update();
}

void PlotGridView::leaveEvent(QEvent*)
{
    update();
}

void PlotGridView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    if (m_selected_view)
    {
        const auto & cell = m_selected_cell;
        painter.drawRect(m_grid->cellRect(cell.y()+1, cell.x()+1).adjusted(-5,-5,5,5));
    }
}

PlotReticle::PlotReticle(QWidget * parent):
    QWidget(parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void PlotReticle::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    auto pos = mapFromGlobal(QCursor::pos());

    QPen cursor_pen;
    cursor_pen.setColor(Qt::red);
    cursor_pen.setWidth(1);

    painter.setPen(cursor_pen);

    painter.drawLine(pos.x(), 0, pos.x(), height());
    painter.drawLine(0, pos.y(), width(), pos.y());
}

RangeView::RangeView(Qt::Orientation orientation, PlotRangeController * ctl, QWidget * parent):
    QWidget(parent),
    m_ctl(ctl),
    m_orientation(orientation)
{
    if (orientation == Qt::Horizontal)
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    else
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    m_background_color = QColor(100,100,100);
    m_foreground_color = QColor(200,200,200);

    connect(ctl, SIGNAL(changed()),
            this, SLOT(update()));
}

void RangeView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    painter.setPen(Qt::NoPen);

    painter.fillRect(rect(), m_background_color);

    float position, size;

    if (m_ctl->limit().extent() > 0)
    {
        position = (m_ctl->value().min - m_ctl->limit().min) / m_ctl->limit().extent();
        size = m_ctl->value().extent() / m_ctl->limit().extent();
    }
    else
    {
        position = size = 0;
    }

    QRect rangeRect;

    if (m_orientation == Qt::Horizontal)
    {
        int thumb_x = int(position * width());
        int thumb_width = std::max(5, int(size * width()));

        rangeRect = rect();
        rangeRect.setX(thumb_x);
        rangeRect.setWidth(thumb_width);
    }
    else
    {
        int thumb_height = std::max(5, int(size * height()));
        int thumb_y = height() - thumb_height - int(position * height());

        rangeRect = rect();
        rangeRect.setY(thumb_y);
        rangeRect.setHeight(thumb_height);
    }

    painter.fillRect(rangeRect, m_foreground_color);

    auto stripe_color = m_background_color;
    stripe_color.setAlpha(100);
    painter.setBrush(QBrush(stripe_color, Qt::BDiagPattern));
    painter.drawRect(rangeRect);
}

////

PlotView::PlotView(QWidget * parent):
    QWidget(parent)
{
    setMouseTracking(true);
}

PlotView::~PlotView()
{
    delete m_plot;
}

void PlotView::setPlot(Plot *plot)
{
    if (m_plot)
        delete m_plot;

    m_plot = plot;

    if (plot)
    {
        connect(plot, SIGNAL(contentChanged()),
                this, SLOT(update()));
    }

    update();
}

void PlotView::setRangeController(PlotRangeController * ctl, Qt::Orientation orientation)
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

QRect PlotView::plotRect() const
{
    return this->rect().adjusted(10,10,-10,-10);
}

QPointF PlotView::mapToPlot(const QPointF & pos)
{
    const auto & xRange = m_x_range ? m_x_range->value() : m_plot->xRange();
    const auto & yRange = m_y_range ? m_y_range->value() : m_plot->yRange();

    auto rect = plotRect();
    if (rect.isEmpty())
        return QPointF(0,0);

    double x = (pos.x() - rect.x()) * (xRange.extent() / rect.width()) + xRange.min;
    double y = (rect.y() + rect.height() - pos.y()) * (yRange.extent() / rect.height()) + yRange.min;

    return QPointF(x,y);
}

QPointF PlotView::mapDistanceToPlot(const QPointF & distance)
{
    const auto & xRange = m_x_range ? m_x_range->value() : m_plot->xRange();
    const auto & yRange = m_y_range ? m_y_range->value() : m_plot->yRange();

    auto rect = plotRect();
    if (rect.isEmpty())
        return QPointF(0,0);

    double x = distance.x() * (xRange.extent() / rect.width());
    double y = -distance.y() * (yRange.extent() / rect.height());

    return QPointF(x,y);
}

void PlotView::selectDataAt(const QPoint & pos)
{
    auto plot = this->plot();
    if (!plot)
        return;

    auto plotPos = mapToPlot(pos);
    auto dataPos = plot->dataLocation(plotPos);
    DataSetPtr dataset = plot->dataSet();
    for (int d = 0; d < dataset->dimensionCount(); ++d)
    {
        DimensionPtr dim = dataset->globalDimension(d);
        if (!dim) continue;
        dim->setFocus(get<0>(dataPos)[d]);
    }
}

void PlotView::enterEvent(QEvent*)
{
    update();
}

void PlotView::leaveEvent(QEvent*)
{
    update();
}

void PlotView::mousePressEvent(QMouseEvent* event)
{
    m_mouse_interaction = NoMouseInteraction;

    bool left_button_pressed = event->buttons() & Qt::LeftButton;
    bool shift_pressed = event->modifiers() & Qt::ShiftModifier;
    if (left_button_pressed and shift_pressed)
    {
        m_mouse_press_point = event->pos();
        m_mouse_press_plot_point = mapToPlot(event->pos());

        if (m_x_range)
            m_x_range_start = m_x_range->value();
        if (m_y_range)
            m_y_range_start = m_y_range->value();

        m_mouse_interaction = MouseShift;
    }
    else if (left_button_pressed and (!event->modifiers()))
    {
        selectDataAt(event->pos());

        m_mouse_interaction = MouseFocusData;
    }
}

void PlotView::mouseMoveEvent(QMouseEvent * event)
{
    if (m_mouse_interaction == MouseShift and event->buttons() & Qt::LeftButton)
    {
        auto distance = event->pos() - m_mouse_press_point;
        auto plot_distance = mapDistanceToPlot(distance);

        if (m_x_range)
            m_x_range->moveTo(m_x_range_start.min - plot_distance.x());
        if (m_y_range)
            m_y_range->moveTo(m_y_range_start.min - plot_distance.y());
    }
    else if (m_mouse_interaction == MouseFocusData and (event->buttons() & Qt::LeftButton))
    {
        selectDataAt(event->pos());
    }
}



void PlotView::wheelEvent(QWheelEvent* event)
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

        auto plot_rect = plotRect();

        auto mouse_plot_pos = mapToPlot(event->pos());

        double new_extent = range->value().extent() * pow(2.0, -degrees/360.0);
        range->scaleTo(new_extent);

        auto plot_origin = plot_rect.topLeft() + QPoint(0, plot_rect.height());
        auto mouse_plot_offset = mapDistanceToPlot(event->pos() - plot_origin);

        if (vertical)
        {
            range->moveTo(mouse_plot_pos.y() - mouse_plot_offset.y());
        }
        else
        {
            range->moveTo(mouse_plot_pos.x() - mouse_plot_offset.x());
        }
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

void PlotView::paintEvent(QPaintEvent*)
{
    //auto mouse_pos = mapFromGlobal(QCursor::pos());

    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);

    painter.setBrush(Qt::NoBrush);

    QPen frame_pen;
    frame_pen.setColor(Qt::lightGray);

    auto plot_rect = plotRect();

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

    //cout << "X range: " << x_range.min << ", " << x_range.max << endl;
    //cout << "Y range: " << y_range.min << ", " << y_range.max << endl;

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

    if (underMouse())
    {
        auto pos = mapFromGlobal(QCursor::pos());

        QPen cursor_pen;
        cursor_pen.setColor(Qt::red);
        cursor_pen.setWidth(1);

        painter.setPen(cursor_pen);

        auto plot = this->plot();
        auto plotPos = mapToPlot(pos);
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



void PlotRangeController::moveTo(double pos)
{
    double extent = m_value.extent();
    double p = pos;
    p = std::min(p, m_limit.max - extent);
    p = std::max(p, m_limit.min);
    m_value.min = p;
    m_value.max = p + extent;

    emit changed();
}

void PlotRangeController::scaleTo(double extent)
{
    extent = std::min(extent, m_limit.extent());
    extent = std::max(extent, 0.0);

    m_value.max = m_value.min + extent;
    if (m_value.max > m_limit.max)
    {
        m_value.max = m_limit.max;
        m_value.min = m_value.max - extent;
    }

    emit changed();
}

}
