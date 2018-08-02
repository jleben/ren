#pragma once

#include <memory>
#include <list>
#include <vector>

#include <QObject>
#include <QString>

namespace datavis {

class DataSource;

using std::list;
using std::vector;

class DataLibrary : public QObject
{
    Q_OBJECT
public:
    DataLibrary(QObject * parent = 0);
    void open(const QString & path);
    void close(DataSource * source);
    int sourceCount() const { return m_sources.size(); }
    DataSource * source(int index) { return m_sources[index]; }
    DataSource * source(const QString & path);

signals:
    void sourcesChanged();
    void openFailed(const QString & path);

private:
    vector<DataSource*> m_sources;
};

}
