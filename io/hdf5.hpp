#pragma once

#include "../data/data_source.hpp"

#include <H5Cpp.h>
#include <memory>

namespace datavis {

class DataLibrary;

class Hdf5Source : public DataSource
{
public:
    Hdf5Source(const string & file_path, DataLibrary *);

    string path() const override { return m_file_path; }
    string id() const override { return m_file_path; }
    int count() const override;
    int index(const string & id) const override;
    DataSetInfo info(int index) const override;
    DataSetPtr dataset(int index) override;

private:
    using PrivateDataSetPtr = std::weak_ptr<DataSet>;

    string m_file_path;
    H5::H5File m_file;
    vector<hsize_t> m_dataset_indices;
    vector<PrivateDataSetPtr> m_datasets;
};

}
