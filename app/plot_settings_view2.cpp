#include "plot_settings_view2.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListWidgetItem>
#include <QLabel>

namespace datavis {

PlotSettingsView::PlotSettingsView(QWidget *parent):
    QWidget(parent)
{
    auto form = new QFormLayout(this);

    m_x_source = new QComboBox;
    m_y_source = new QComboBox;

    m_y_source->setModel(m_x_source->model());

    form->addRow(new QLabel("X:"), m_x_source);
    form->addRow(new QLabel("Y:"), m_y_source);
}


void PlotSettingsView::setDataInfo(const DataSetInfo & info)
{
    m_info = info;

    for(int i = 0; i < m_info.dimensions.size(); ++i)
    {
        string name = m_info.dimensions[i].name;
        if (name.empty())
            name = "Dimension " + to_string(i);
        cerr << "Adding dim " << name << endl;
        m_x_source->addItem(QString::fromStdString(name));
    }

    for (int i = 0; i < m_info.attributes.size(); ++i)
    {
        string name = m_info.attributes[i].name;
        if (name.empty())
            name = "Attribute " + to_string(i);
        cerr << "Adding att " << name << endl;
        m_x_source->addItem(QString::fromStdString(name));
    }

    m_x_source->setCurrentIndex(0);
    m_y_source->setCurrentIndex(1);
}

}
