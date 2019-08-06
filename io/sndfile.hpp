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
    string id() const override { return m_file_path; }
    int count() const override { return 1; }
    int index(const string & id) const override { return 0; }
    DataSetInfo info(int index) const override { return m_info; }
    DataSetPtr dataset(int index) override;

private:
    void getInfo();

    string m_file_path;
    DataSetInfo m_info;
    DataSetPtr m_dataset;
};

}


