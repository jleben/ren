#include "line_plot_settings_view.hpp"
#include "../plot/line_plot.hpp"
#include "color_box.hpp"
#include "data_source.hpp"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <cassert>

namespace datavis {

LinePlotSettingsView::LinePlotSettingsView(QWidget * parent):
    QWidget(parent)
{
    m_dim_x = new QSpinBox;

    m_color = new ColorBox;

    auto form = new QFormLayout(this);
    form->addRow(new QLabel("X dim:"), m_dim_x);
    form->addRow(new QLabel("Color:"), m_color);

    connect(m_dim_x, &QSpinBox::editingFinished,
            this, &LinePlotSettingsView::onDimXEdited);
}

void LinePlotSettingsView::setPlot(LinePlot * plot)
{
    if (m_plot)
    {
        disconnect(m_plot, 0, this, 0);
    }

    m_plot = plot;

    if (m_plot)
    {
        connect(m_plot, &LinePlot::sourceChanged,
                this, &LinePlotSettingsView::onSourceChanged);
        connect(m_plot, &LinePlot::dimensionChanged,
                this, &LinePlotSettingsView::onDimXChanged);
        connect(m_plot, &LinePlot::colorChanged,
                this, &LinePlotSettingsView::onColorChanged);
    }

    updateAll();
}

void LinePlotSettingsView::updateAll()
{
    auto source = m_plot ? m_plot->dataSource() : nullptr;
    auto data = source ? source->data() : nullptr;

    int max_dim = data ? data->size().size() - 1 : 0;
    //qDebug() << "Max dimension: " << max_dim << endl;

    m_dim_x->setRange(0, max_dim);
    m_dim_x->setValue(m_plot ? m_plot->dimension() : 0);

    m_color->setColor(m_plot ? m_plot->color() : QColor(Qt::black));
}

void LinePlotSettingsView::onSourceChanged()
{
    assert(m_plot);

    auto source = m_plot->dataSource();
    auto data = source ? source->data() : nullptr;

    int max_dim = data ? data->size().size() - 1 : 0;
    m_dim_x->setRange(0, max_dim);
}

void LinePlotSettingsView::onDimXChanged()
{
    assert(m_plot);
    m_dim_x->setValue(m_plot->dimension());
}

void LinePlotSettingsView::onDimXEdited()
{
    if (!m_plot)
        return;

    int dim = m_dim_x->value();
    m_plot->setDimension(dim);
}

void LinePlotSettingsView::onColorChanged()
{
    assert(m_plot);
    m_color->setColor(m_plot->color());
}

}
