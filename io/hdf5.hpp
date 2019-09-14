#pragma once

#include "../data/data_source.hpp"

#include <H5Cpp.h>
#include <memory>
#include <unordered_map>

namespace datavis {

class DataLibrary;

class Hdf5Source : public DataSource
{
public:
    Hdf5Source(const string & file_path, DataLibrary *);

    string path() const override { return m_file_path; }
    string id() const override { return m_name; }

    virtual int count() const override;
    virtual vector<string> dataset_ids() const override;
    virtual DataSetInfo dataset_info(const string & id) const override;
    virtual DataSetAccessPtr dataset(const string & id) override;

private:
    static DataSetPtr readDataset(string id, H5::DataSet & dataset, Hdf5Source *, DataLibrary *);

    string m_file_path;
    string m_name;
    H5::H5File m_file;
    std::unordered_map<string, DataSetInfo> d_infos;
    std::unordered_map<string, std::weak_ptr<DataSetAccessor>> d_datasets;
};

}
