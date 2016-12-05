#include "settings_view.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>

namespace datavis {

SettingsView::SettingsView(QWidget * parent):
    QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
}

void SettingsView::setPlotSettingsView(QWidget * view)
{
    delete m_plot_settings_view;

    m_plot_settings_view = view;

    if (view)
        m_layout->addWidget(view);
}

}
