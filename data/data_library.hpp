#pragma once

#include <memory>
#include <list>
#include <vector>
#include <unordered_map>

#include <QObject>
#include <QString>

#include "dimension.hpp"

namespace datavis {

class DataSource;

using std::list;
using std::vector;
using std::unordered_map;

class DataLibrary : public QObject
{
    Q_OBJECT
public:
    using Dimensions = unordered_map<string, DimensionPtr>;

    DataLibrary(QObject * parent = 0);
    void open(const QString & path);
    void close(DataSource * source);
    void closeAll();
    int sourceCount() const { return m_sources.size(); }
    DataSource * source(int index) { return m_sources[index]; }
    DataSource * source(const QString & path);
    DimensionPtr dimension(const string & name);
    const Dimensions & dimensions() const { return d_dimensions; }

signals:
    void sourcesChanged();
    void openFailed(const QString & path, const QString & reason = QString());

private:
    void updateDimensions();

    vector<DataSource*> m_sources;
    unordered_map<string, DimensionPtr> d_dimensions;
};

}
