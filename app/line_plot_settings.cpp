#include "line_plot_settings.hpp"
#include "data_source.hpp"

#include <QDebug>

namespace datavis {

LinePlotSettings::LinePlotSettings(QObject * parent):
    QObject(parent)
{

}

void LinePlotSettings::setDataSource(DataSource * source)
{
    m_source = source;

    justSetDefault();

    emit sourceChanged();
    emit dimensionXChanged(m_dim_x);
    emit colorChanged();
}

void LinePlotSettings::setDimensionX(int dim)
{
    m_dim_x = dim;
    emit dimensionXChanged(dim);
}

void LinePlotSettings::setColor(const QColor & color)
{
    m_color = color;
    emit colorChanged();
}

void LinePlotSettings::setDefault()
{
    justSetDefault();

    emit dimensionXChanged(m_dim_x);
    emit colorChanged();
}

void LinePlotSettings::justSetDefault()
{
    m_dim_x = 0;
    m_color = Qt::black;
}

}
