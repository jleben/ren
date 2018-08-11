#pragma once

#include "../data/data_source.hpp"

#include <QWidget>
#include <QListWidget>
#include <QComboBox>

namespace datavis {

class PlotSettingsView : public QWidget
{
public:
    PlotSettingsView(QWidget * parent = 0);
    void setDataInfo(const DataSetInfo & info);
    int x() { return m_x_source->currentIndex(); }
    int y() { return m_y_source->currentIndex(); }
private:
    DataSetInfo m_info;
    QComboBox * m_x_source = nullptr;
    QComboBox * m_y_source = nullptr;
};

}

