#pragma once

#include "../data/data_source.hpp"

#include <memory>

namespace datavis {

class DataLibrary;

class TextSource : public DataSource
{
public:
    TextSource(const string & file_path, DataLibrary *);

    string id() const override { return m_file_path; }
    int count() const override { return 1; }
    int index(const string & id) const override { return 0; }
    DataSetInfo info(int index) const override;
    DataSetPtr dataset(int index) { return m_dataset; }

private:
    string m_file_path;
    DataSetPtr m_dataset;
};

}

