#pragma once

#include <QWidget>
#include <QTreeWidget>

namespace datavis {

class DataLibrary;

class DataLibraryView : public QWidget
{
public:
    DataLibraryView(QWidget * parent = 0);

    void setLibrary(DataLibrary *);
    DataLibrary * library() { return m_lib; }

private:
    void updateLibraryTree();

    DataLibrary * m_lib = nullptr;
    QTreeWidget * m_lib_tree = nullptr;
};

}
