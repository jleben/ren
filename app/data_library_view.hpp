#pragma once

#include <QWidget>
#include <QTreeWidget>

namespace datavis {

class DataLibrary;
class DataSource;

class DataLibraryView : public QWidget
{
    Q_OBJECT
public:
    DataLibraryView(QWidget * parent = 0);

    void setLibrary(DataLibrary *);
    DataLibrary * library() { return m_lib; }

    DataSource * selectedSource();
    int selectedObjectIndex();

signals:
    void selectionChanged();

private:
    void updateLibraryTree();

    DataLibrary * m_lib = nullptr;
    QTreeWidget * m_lib_tree = nullptr;
};

}
