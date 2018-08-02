#include "data_library_view.hpp"
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
    m_lib_tree = new QTreeWidget;

    m_lib_tree->setHeaderLabels(QStringList() << "Name" << "Size");
    m_lib_tree->header()->setSectionHidden(1,true);

    m_dim_tree = new QTreeWidget;
    m_dim_tree->setHeaderLabels(QStringList() << "Name" << "Range");

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_lib_tree);
    layout->addWidget(m_dim_tree);

    connect(m_lib_tree, &QTreeWidget::currentItemChanged,
            this, &DataLibraryView::selectionChanged);
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
            auto id = source->id();
            auto pos = id.rfind('/');
            if (pos != string::npos)
                id = id.substr(pos+1);
            source_name = QString::fromStdString(id);
        }

        auto source_item = new QTreeWidgetItem(QStringList() << source_name);
        source_item->setData(0, Qt::UserRole, QVariant::fromValue(source));

        for (int object_idx = 0; object_idx < source->count(); ++object_idx)
        {
            auto object = source->info(object_idx);

            QString name = QString::fromStdString(object.id);

            QStringList size_texts;
            for (auto & dim : object.dimensions)
                size_texts << QString::number(dim.size);
            QString size_text = size_texts.join(" ");

            QStringList texts;
            texts << name;
            texts << size_text;

            auto object_item = new QTreeWidgetItem(texts);

            source_item->addChild(object_item);
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

DataSource * DataLibraryView::selectedSource()
{
    auto item = m_lib_tree->currentItem();
    if (!item)
        return nullptr;

    if(item->parent())
        item = item->parent();

    return item->data(0, Qt::UserRole).value<DataSource*>();
}

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

}
