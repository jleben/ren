#pragma once

#include "../data/array.hpp"
#include "../data/data_set.hpp"

#include <string>
#include <memory>

namespace datavis {

using std::string;

class DataSetInfo
{
public:
    string id;
    vector<int> size;
    vector<Dimension> dimensions;
    int dimensionCount() const { return int(size.size()); }
    double minimum(int dim_idx) const { return dimensions[dim_idx].map * 0; }
    double maximum(int dim_idx) const { return dimensions[dim_idx].map * (size[dim_idx] - 1); }
};

class DataSource
{
public:
    virtual ~DataSource() {}
    virtual string id() const = 0;
    virtual int count() const = 0;
    virtual int index(const string & id) const = 0;
    virtual DataSetInfo info(int index) const = 0;
    virtual DataSetPtr dataset(int index) = 0;
};

}
