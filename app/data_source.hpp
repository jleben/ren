#pragma once

#include "../data/array.hpp"
#include <QObject>

namespace datavis {

class DataSource : public QObject
{
public:
    DataSource(const QString & filePath, QObject * parent = 0);
    DataSource(const vector<int> & size): m_data(size) {}
    QString path() const { return m_path; }
    array<double> * data() { return & m_data; }
    const array<double> * data() const { return & m_data; }

private:
    QString m_path;
    array<double> m_data;
};

}
