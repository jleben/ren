#pragma once

#include "../data/array.hpp"
#include "../data/data_set.hpp"

#include <string>
#include <memory>

namespace datavis {

using std::string;

class DataLibrary;

class DataSetInfo
{
public:
    string id;
    vector<DataSet::Dimension> dimensions;
    vector<DataSet::Attribute> attributes;
    int dimensionCount() const { return int(dimensions.size()); }
};

class DataSource
{
public:
    DataSource(DataLibrary * lib): d_lib(lib) {}
    DataLibrary * library() const { return d_lib; }
    virtual ~DataSource() {}
    virtual string path() const = 0;
    virtual string id() const = 0;
    virtual int count() const = 0;
    virtual int index(const string & id) const = 0;
    virtual DataSetInfo info(int index) const = 0;
    virtual DataSetPtr dataset(int index) = 0;

private:
    DataLibrary * d_lib = nullptr;
};

}
