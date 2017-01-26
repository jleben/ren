#include "line_plot_settings_view.hpp"
#include "../plot/line_plot.hpp"
#include "color_box.hpp"

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
    m_x_dim = new QComboBox;
    m_selector_dim = new QComboBox;
    m_selector_dim->setModel(m_x_dim->model());

    m_color = new ColorBox;

    auto form = new QFormLayout(this);
    form->addRow(new QLabel("X dim:"), m_x_dim);
    form->addRow(new QLabel("Selector dim:"), m_selector_dim);
    form->addRow(new QLabel("Color:"), m_color);

    connect(m_x_dim, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, &LinePlotSettingsView::onXDimEdited);
    connect(m_selector_dim, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
            this, &LinePlotSettingsView::onSelectorDimEdited);
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
                this, &LinePlotSettingsView::onXDimChanged);
        connect(m_plot, &LinePlot::selectorDimChanged,
                this, &LinePlotSettingsView::onSelectorDimChanged);
        connect(m_plot, &LinePlot::colorChanged,
                this, &LinePlotSettingsView::onColorChanged);
    }

    updateAll();
}

void LinePlotSettingsView::updateAll()
{
    updateDimensionList();

    if (!m_plot)
        return;

    m_x_dim->setCurrentIndex(m_plot->dimension() + 1);

    m_selector_dim->setCurrentIndex(m_plot->selectorDim() + 1);

    m_color->setColor(m_plot ? m_plot->color() : QColor(Qt::black));
}

void LinePlotSettingsView::updateDimensionList()
{
    m_x_dim->clear();

    if (!m_plot)
        return;

    if (!m_plot->dataObject())
        return;

    auto n_dim = m_plot->dataObject()->data()->size().size();

    m_x_dim->clear();
    m_x_dim->addItem("None", int(-1));
    for (int i = 0; i < n_dim; ++i)
        m_x_dim->addItem(QString::number(i), i);
}

void LinePlotSettingsView::onSourceChanged()
{
    updateDimensionList();

    assert(m_plot);

    m_x_dim->setCurrentIndex(m_plot->dimension() + 1);

    m_selector_dim->setCurrentIndex(m_plot->selectorDim() + 1);
}

void LinePlotSettingsView::onXDimChanged()
{
    assert(m_plot);
    m_x_dim->setCurrentIndex(m_plot->dimension() + 1);
}

void LinePlotSettingsView::onXDimEdited()
{
    if (!m_plot)
        return;

    int dim = m_x_dim->currentData().toInt();

    m_plot->setDimension(dim);
}

void LinePlotSettingsView::onSelectorDimChanged()
{
    assert(m_plot);
    m_selector_dim->setCurrentIndex(m_plot->selectorDim() + 1);
}

void LinePlotSettingsView::onSelectorDimEdited()
{
    if (!m_plot)
        return;

    int dim = m_selector_dim->currentData().toInt();

    m_plot->setSelectorDim(dim);
}

void LinePlotSettingsView::onColorChanged()
{
    assert(m_plot);

    m_color->setColor(m_plot->color());
}

}
