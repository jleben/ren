#pragma once

#include "../io/data_source.hpp"

#include <QWidget>
#include <QListWidget>

namespace datavis {

class PlotDataSettingsView : public QWidget
{
public:
    PlotDataSettingsView(QWidget * parent = 0);
    void setDataInfo(const DataObjectInfo & info);
    vector<int> selectedDimensions();
private:
    DataObjectInfo m_info;
    QListWidget * m_dimension_list = nullptr;
};

}
