#pragma once

#include <QWidget>
#include <QTreeWidget>

namespace datavis {

class DataLibrary;
class DataSource;

class DataLibraryView : public QWidget
{
public:
    DataLibraryView(QWidget * parent = 0);

    void setLibrary(DataLibrary *);
    DataLibrary * library() { return m_lib; }

    DataSource * selectedSource();
    int selectedObjectIndex();

private:
    void updateLibraryTree();

    DataLibrary * m_lib = nullptr;
    QTreeWidget * m_lib_tree = nullptr;
};

}
