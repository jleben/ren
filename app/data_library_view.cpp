#include "data_library_view.hpp"
#include "data_info_view.hpp"
#include "../data/data_library.hpp"
#include "../data/data_source.hpp"

#include <QTreeWidgetItem>
#include <QStringList>
#include <QVBoxLayout>
#include <QHeaderView>

Q_DECLARE_METATYPE(datavis::DataSource*);

namespace datavis {

DataLibraryView::DataLibraryView(QWidget * parent):
    QWidget(parent)
{
    m_lib_tree = new DataSetTree;
    m_lib_tree->setDragEnabled(true);

    m_lib_tree->setHeaderLabels(QStringList() << "Name" << "Size");
    m_lib_tree->header()->setSectionHidden(1,true);

    m_dataset_info = new DataInfoView;

    m_dim_tree = new QTreeWidget;
    m_dim_tree->setHeaderLabels(QStringList() << "Name" << "Range");

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_lib_tree);
    layout->addWidget(m_dataset_info);
    layout->addWidget(m_dim_tree);

    connect(m_lib_tree, &QTreeWidget::currentItemChanged,
            this, &DataLibraryView::selectionChanged);
    connect(m_lib_tree, &QTreeWidget::currentItemChanged,
            this, &DataLibraryView::updateDataInfo);
}

void DataLibraryView::setLibrary(DataLibrary * lib)
{
    if (m_lib)
        m_lib->disconnect(this);

    m_lib = lib;

    if (lib)
    {
        connect(lib, &DataLibrary::sourcesChanged,
                this, &DataLibraryView::updateLibraryTree);
        connect(lib, &DataLibrary::sourcesChanged,
                this, &DataLibraryView::updateDimTree);
    }

    updateLibraryTree();
}

void DataLibraryView::updateLibraryTree()
{
    m_lib_tree->clear();

    if (!m_lib)
        return;

    for (int source_idx = 0; source_idx < m_lib->sourceCount(); ++source_idx)
    {
        auto source = m_lib->source(source_idx);

        QString source_name;
        {
            source_name = QString::fromStdString(source->id());
        }

        auto source_item = new QTreeWidgetItem(QStringList() << source_name);
        source_item->setData(0, Qt::UserRole, QVariant::fromValue(source));

        auto dataset_ids = source->dataset_ids();
        for (auto & id : dataset_ids)
        {
            auto dataset_info = source->dataset(id)->info();

            QString name = QString::fromStdString(dataset_info.id);

            QStringList size_texts;
            for (auto & dim : dataset_info.dimensions)
                size_texts << QString::number(dim.size);
            QString size_text = size_texts.join(" ");

            QStringList texts;
            texts << name;
            texts << size_text;

            auto dataset_item = new QTreeWidgetItem(texts);

            source_item->addChild(dataset_item);
        }

        m_lib_tree->addTopLevelItem(source_item);
        source_item->setExpanded(true);
    }
}

void DataLibraryView::updateDimTree()
{
    const auto & dimensions = m_lib->dimensions();

    // Remove unused dimensions
#if 0

    int i = 0;
    while(i < m_dim_tree->topLevelItemCount())
    {
        auto * item = m_dim_tree->topLevelItem(i);
        string name = item->text(0).toStdString();
        if (dimensions.find(name) == dimensions.end())
            delete item;
        else
            ++i;
    }
#endif

    m_dim_tree->clear();

    // Update items

    for (const auto & entry : dimensions)
    {
        const string & name = entry.first;
        const auto & dim = entry.second;

        auto range_text = QString("[%1, %2]")
                .arg(minimum(dim->range()))
                .arg(maximum(dim->range()));

        auto item = new QTreeWidgetItem(QStringList() << QString::fromStdString(name) << range_text);

        m_dim_tree->addTopLevelItem(item);
    }
}

void DataLibraryView::updateDataInfo()
{
    auto source = selectedSource();
    string id = selectedDatasetId();

    if (source == nullptr || id.empty())
        m_dataset_info->setInfo(DataSetInfo());
    else
        m_dataset_info->setInfo(source->dataset(id)->info());
}

DataSource * DataLibraryView::selectedSource()
{
    auto item = m_lib_tree->currentItem();
    if (!item)
        return nullptr;

    if(item->parent())
        item = item->parent();

    return item->data(0, Qt::UserRole).value<DataSource*>();
}

#if 0
int DataLibraryView::selectedDatasetIndex()
{
    auto item = m_lib_tree->currentItem();
    if (!item)
        return -1;

    auto parent = item->parent();
    if (!parent)
        return -1;

    return parent->indexOfChild(item);
}
#endif

string DataLibraryView::selectedDatasetId()
{
    auto item = m_lib_tree->currentItem();
    if (!item || !item->parent() || item->parent() == m_lib_tree->invisibleRootItem())
        return string();

    return item->text(0).toStdString();
}

QMimeData * DataSetTree::mimeData(const QList<QTreeWidgetItem *> items) const
{
    vector<DraggedDatasets::Item> data_items;

    for (auto item : items)
    {
        auto source_item = item->parent();
        if (!source_item)
            continue;

        auto source = source_item->data(0, Qt::UserRole).value<DataSource*>();
        string id = item->text(0).toStdString();

        DraggedDatasets::Item data_item;
        data_item.sourceId = source->id();
        data_item.datasetId = id;
        data_items.push_back(data_item);
    }

    if (data_items.empty())
        return nullptr;

    auto data = new DraggedDatasets;
    data->items = data_items;
    return data;
}

}
