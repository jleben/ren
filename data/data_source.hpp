#pragma once

#include "../data/array.hpp"
#include "../data/data_set.hpp"

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

class DataSetAccessor //: public QObject
{
    //Q_OBJECT;

    friend class DataSource;

public:
    DataSetAccessor(std::function<void()> destroyCallback):
        d_destroy_cb(destroyCallback) {}

    ~DataSetAccessor()
    {
        if (d_destroy_cb) d_destroy_cb();
    }

    DataSetInfo info() const { return d_info; }
    DataSetPtr dataset() const { return d_dataset; }
    float progress() const { return d_progress; }

//signals:
    //void progressChanged();

    DataSetInfo d_info;
    DataSetPtr d_dataset;
    std::atomic<float> d_progress { 0 };
    std::function<void()> d_destroy_cb;
};

using DataSetAccessPtr = std::shared_ptr<DataSetAccessor>;

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
    virtual DataSetAccessPtr dataset(const string & id) = 0;

private:
    DataLibrary * d_lib = nullptr;
};

}
