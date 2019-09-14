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

    virtual int count() const override { return 0; }
    virtual vector<string> dataset_ids() const override { return {}; }
    DataSetInfo dataset_info(const string & id) const { return {}; }
    virtual DataSetAccessPtr dataset(const string & id) override { return nullptr; }

    int index(const string & id) const { return 0; }
    DataSetInfo info(int index) const { return m_info; }
    DataSetPtr dataset(int index);

private:
    void getInfo();

    string m_file_path;
    string m_name;
    DataSetInfo m_info;
    DataSetPtr m_dataset;
};

}


