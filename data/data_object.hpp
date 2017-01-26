#pragma once

#include "array.hpp"
#include "math.hpp"

#include <string>

namespace datavis {

using std::string;

struct Dimension
{
    Mapping1d map;
};

class DataObject
{
public:
    DataObject(const vector<int> & size):
        DataObject(string(), size)
    {}

    DataObject(const string & id, const vector<int> & size):
        m_id(id),
        m_data(size),
        m_dimensions(size.size())
    {}

    DataObject(const string & id, const array<double> & data):
        m_id(id),
        m_data(data),
        m_dimensions(data.size().size())
    {}


    DataObject(const string & id, array<double> && data):
        m_id(id),
        m_data(data),
        m_dimensions(data.size().size())
    {}

    string id() const { return m_id; }

    array<double> * data() { return & m_data; }
    const array<double> * data() const { return & m_data; }

    Dimension dimension(int idx) { return m_dimensions[idx]; }
    void setDimension(int idx, const Dimension & dim) { m_dimensions[idx] = dim; }

private:

    string m_id;
    array<double> m_data;
    vector<Dimension> m_dimensions;

};

}