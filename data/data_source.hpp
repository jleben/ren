#pragma once

#include "../data/array.hpp"
#include "../data/data_object.hpp"

#include <string>

namespace datavis {

using std::string;

class DataObjectInfo
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
    virtual int objectCount() const = 0;
    virtual int objectIndex(const string & id) const = 0;
    virtual DataObjectInfo objectInfo(int index) const = 0;
    virtual DataObject * object(int index) const = 0;
};

}
