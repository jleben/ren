#include "data_source.hpp"
#include "../io/hdf5.hpp"

namespace datavis {

DataSource::DataSource(const QString & filePath, QObject * parent)
{
    // NOTE: May throw!
    m_data = read_hdf5<double>(filePath.toStdString(), "/data");

    if (parent)
        setParent(parent);
}

}
