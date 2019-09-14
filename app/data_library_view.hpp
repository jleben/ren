#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QMimeData>
#include <vector>
#include <string>

namespace datavis {

class DataLibrary;
class DataSource;
class DataInfoView;

using std::vector;
using std::string;

class DraggedDatasets : public QMimeData
{
    Q_OBJECT
public:
    struct Item
    {
        string sourceId;
        string datasetId;
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
    //int selectedDatasetIndex();
    string selectedDatasetId();

signals:
    void selectionChanged();

private:
    void updateLibraryTree();
    void updateDimTree();
    void updateDataInfo();

    DataLibrary * m_lib = nullptr;
    QTreeWidget * m_lib_tree = nullptr;
    QTreeWidget * m_dim_tree = nullptr;
    DataInfoView * m_dataset_info = nullptr;
};

class DataSetTree : public QTreeWidget
{
protected:
    virtual QMimeData * mimeData(const QList<QTreeWidgetItem *> items) const;
};


}
