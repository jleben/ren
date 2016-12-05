#include "line_plot_settings_view.hpp"
#include "line_plot_settings.hpp"
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

void LinePlotSettingsView::setSettings(LinePlotSettings * settings)
{
    if (m_settings)
    {
        disconnect(m_settings, 0, this, 0);
    }

    m_settings = settings;

    if (m_settings)
    {
        connect(m_settings, &LinePlotSettings::sourceChanged,
                this, &LinePlotSettingsView::onSourceChanged);
        connect(m_settings, &LinePlotSettings::dimensionXChanged,
                this, &LinePlotSettingsView::onDimXChanged);
        connect(m_settings, &LinePlotSettings::colorChanged,
                this, &LinePlotSettingsView::onColorChanged);
    }

    updateAll();
}

void LinePlotSettingsView::updateAll()
{
    auto source = m_settings ? m_settings->source() : nullptr;
    auto data = source ? source->data() : nullptr;

    int max_dim = data ? data->size().size() - 1 : 0;
    //qDebug() << "Max dimension: " << max_dim << endl;

    m_dim_x->setRange(0, max_dim);
    m_dim_x->setValue(m_settings ? m_settings->dimensionX() : 0);

    m_color->setColor(m_settings ? m_settings->color() : QColor(Qt::black));
}

void LinePlotSettingsView::onSourceChanged()
{
    assert(m_settings);

    auto source = m_settings->source();
    auto data = source ? source->data() : nullptr;

    int max_dim = data ? data->size().size() - 1 : 0;
    m_dim_x->setRange(0, max_dim);
}

void LinePlotSettingsView::onDimXChanged()
{
    assert(m_settings);
    m_dim_x->setValue(m_settings->dimensionX());
}

void LinePlotSettingsView::onDimXEdited()
{
    assert(m_settings);
    int dim = m_dim_x->value();
    m_settings->setDimensionX(dim);
}

void LinePlotSettingsView::onColorChanged()
{
    assert(m_settings);
    m_color->setColor(m_settings->color());
}

}
