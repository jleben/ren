#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QMimeData>
#include <vector>
#include <string>

namespace datavis {

class DataLibrary;
class DataSource;

using std::vector;
using std::string;

class DraggedDatasets : public QMimeData
{
    Q_OBJECT
public:
    struct Item
    {
        string sourceId;
        int datasetIndex;
    };

    vector<Item> items;
};

class DataLibraryView : public QWidget
{
    Q_OBJECT
public:
    DataLibraryView(QWidget * parent = 0);

    void setLibrary(DataLibrary *);
    DataLibrary * library() { return m_lib; }

    DataSource * selectedSource();
    int selectedDatasetIndex();

signals:
    void selectionChanged();

private:
    void updateLibraryTree();
    void updateDimTree();

    DataLibrary * m_lib = nullptr;
    QTreeWidget * m_lib_tree = nullptr;
    QTreeWidget * m_dim_tree = nullptr;
};

class DataSetTree : public QTreeWidget
{
protected:
    virtual QMimeData * mimeData(const QList<QTreeWidgetItem *> items) const;
};


}
