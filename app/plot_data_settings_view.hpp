#pragma once

#include "../data/data_source.hpp"

#include <QWidget>
#include <QListWidget>

namespace datavis {

class PlotDataSettingsView : public QWidget
{
public:
    PlotDataSettingsView(QWidget * parent = 0);
    void setDataInfo(const DataSetInfo & info);
    vector<int> selectedDimensions();
private:
    DataSetInfo m_info;
    QListWidget * m_dimension_list = nullptr;
};

}
