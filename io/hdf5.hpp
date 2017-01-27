#pragma once

#include "../data/data_source.hpp"

#include <H5Cpp.h>

namespace datavis {

class Hdf5Source : public DataSource
{
public:
    Hdf5Source(const string & file_path);

    string id() const override { return m_file_path; }
    int count() const override;
    int index(const string & id) const override;
    DataSetInfo info(int index) const override;
    DataSet * dataset(int index) const override;

private:
    string m_file_path;
    H5::H5File m_file;
    vector<hsize_t> m_dataset_indices;
};

}
