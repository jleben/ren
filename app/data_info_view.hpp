#pragma once

#include "../data/data_source.hpp"

#include <QWidget>
#include <QLabel>

namespace datavis {

class DataInfoView : public QWidget
{
    Q_OBJECT
public:
    DataInfoView(QWidget * parent = nullptr);
    void setInfo(const DataSetInfo & info);
private:
    QLabel * m_dimensions;
    QLabel * m_attributes;
};

}
