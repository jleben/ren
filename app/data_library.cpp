#include "data_library.hpp"
#include "../io/hdf5.hpp"

namespace datavis {

DataLibrary::DataLibrary(QObject * parent):
    QObject(parent)
{}

void DataLibrary::open(const QString & path)
{
    DataSource * source = nullptr;

    try {
        source = new Hdf5Source(path.toStdString());
    } catch (...) {
        emit openFailed(path);
        return;
    }

    m_sources.push_back(source);

    emit sourcesChanged();
}

void DataLibrary::close(DataSource * source)
{
    if (!source)
        return;

    auto pos = std::find(m_sources.begin(), m_sources.end(), source);
    if (pos == m_sources.end())
        return;

    m_sources.erase(pos);

    emit sourcesChanged();
}

}
