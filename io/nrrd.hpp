#pragma once

#include "../data/data_source.hpp"

namespace datavis {

class DataLibrary;

class NrrdSource : public DataSource
{
public:
    NrrdSource(const string & file_path, DataLibrary *);
    ~NrrdSource();

    string path() const override { return m_file_path; }
    string id() const override { return m_name; }

    virtual int count() const override { return 1; }
    virtual vector<string> dataset_ids() const override { return {"data"}; }
    virtual DataSetInfo dataset_info(const string & id) const override;
    virtual FutureDataset dataset(const string & id) override;

private:
    void readInfo();

    string m_file_path;
    string m_name;
    DataSetInfo m_info;
    FutureDataset m_dataset;
};

}
