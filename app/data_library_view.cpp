#include "data_library_view.hpp"
#include "data_library.hpp"
#include "../data/data_source.hpp"

#include <QTreeWidgetItem>
#include <QStringList>
#include <QVBoxLayout>

Q_DECLARE_METATYPE(datavis::DataSource*);

namespace datavis {

DataLibraryView::DataLibraryView(QWidget * parent):
    QWidget(parent)
{
    m_lib_tree = new QTreeWidget;

    m_lib_tree->setHeaderLabels(QStringList() << "Name" << "Size");

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_lib_tree);

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

        for (int object_idx = 0; object_idx < source->objectCount(); ++object_idx)
        {
            auto object = source->objectInfo(object_idx);

            QString name = QString::fromStdString(object.id);

            QStringList size_texts;
            for (auto & s : object.size)
                size_texts << QString::number(s);
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

DataSource * DataLibraryView::selectedSource()
{
    auto item = m_lib_tree->currentItem();
    if (!item)
        return nullptr;

    if(item->parent())
        item = item->parent();

    return item->data(0, Qt::UserRole).value<DataSource*>();
}

int DataLibraryView::selectedObjectIndex()
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
