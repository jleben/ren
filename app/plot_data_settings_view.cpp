#include "plot_data_settings_view.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidgetItem>

namespace datavis {

PlotDataSettingsView::PlotDataSettingsView(QWidget *parent):
    QWidget(parent)
{

    m_dimension_list = new QListWidget;
    m_dimension_list->setSelectionRectVisible(false);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_dimension_list);
}

void PlotDataSettingsView::setDataInfo(const DataSetInfo & info)
{
    m_dimension_list->clear();

    for (int i = 0; i < info.dimensionCount(); ++i)
    {
        auto & dim = info.dimensions[i];

        auto item = new QListWidgetItem;

        m_dimension_list->addItem(item);

        auto flags =
                Qt::ItemIsEnabled |
                Qt::ItemIsUserCheckable;

        item->setFlags(flags);

        item->setCheckState(Qt::Unchecked);

        QString text;

        text += QString::number(i);
        text += ": ";
        text += QString::number(dim.size);

        if (dim.size > 0)
        {
            text += QString(" (%1, %2) @ %3")
                    .arg(dim.minimum())
                    .arg(dim.maximum())
                    .arg(dim.map.scale);
        }

        item->setText(text);
    }
}

vector<int> PlotDataSettingsView::selectedDimensions()
{
    vector<int> dims;

    for (int i = 0; i < m_dimension_list->count(); ++i)
    {
        auto item = m_dimension_list->item(i);
        if (item->checkState() == Qt::Checked)
            dims.push_back(i);
    }

    return dims;
}

}
