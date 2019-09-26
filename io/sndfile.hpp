#pragma once

#include "../data/data_source.hpp"

#include <memory>

namespace datavis {

class DataLibrary;

class SoundFileSource : public DataSource
{
public:
    SoundFileSource(const string & file_path, DataLibrary *);

    string path() const override { return m_file_path; }
    string id() const override { return m_name; }
    //int count() const override { return 1; }

    virtual int count() const override { return 1; }
    virtual vector<string> dataset_ids() const override { return { "data" }; }
    DataSetInfo dataset_info(const string & id) const { return m_info; }
    virtual FutureDataset dataset(const string & id) override;

private:
    void getInfo();

    struct Read_Result
    {
        DataSetInfo info;
        DataSetPtr dataset;
    };

    static Read_Result read_file(const string & file_path);

    string m_file_path;
    string m_name;
    DataSetInfo m_info;
    FutureDataset m_dataset;
};

}


