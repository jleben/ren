#pragma once

#include "../data/array.hpp"
#include "../data/data_set.hpp"
#include "../reactive/reactive.hpp"

#include <string>
#include <memory>
#include <atomic>
#include <functional>

namespace datavis {

using std::string;

class DataLibrary;
class DataSource;

class DataSetInfo
{
public:
    string id;
    vector<DataSet::Dimension> dimensions;
    vector<DataSet::Attribute> attributes;
    int dimensionCount() const { return int(dimensions.size()); }
};

using FutureDataset = Reactive::Value<DataSetPtr>;

class DataSource
{
public:
    DataSource(DataLibrary * lib): d_lib(lib) {}
    DataLibrary * library() const { return d_lib; }
    virtual ~DataSource() {}
    virtual string path() const = 0;
    virtual string id() const = 0;

    virtual int count() const = 0;
    virtual vector<string> dataset_ids() const = 0;
    virtual DataSetInfo dataset_info(const string & id) const = 0;
    virtual FutureDataset dataset(const string & id) = 0;

private:
    DataLibrary * d_lib = nullptr;
};

}
