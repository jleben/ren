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

void PlotDataSettingsView::setDataInfo(const DataObjectInfo & info)
{
    m_dimension_list->clear();

    for (int i = 0; i < info.dimensionCount(); ++i)
    {
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
        text += QString::number(info.size[i]);

        auto dim = info.dimensions[i];
        if (info.size[i] > 0)
        {
            text += QString(" (%1, %2) @ %3")
                    .arg(dim.map * 0)
                    .arg(dim.map * (info.size[i]-1))
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
