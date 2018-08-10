#pragma once

#include "array.hpp"
#include "math.hpp"
#include "dimension.hpp"

#include <string>
#include <memory>
#include <QObject>
#include <cmath>

namespace datavis {

using std::string;

class DataSource;

class DataSet : public QObject
{
    Q_OBJECT

public:
    struct Dimension
    {
        string name;
        size_t size;
        Mapping1d map;
        Range range() const { return { minimum(), maximum() }; }
        double minimum() const { return map * 0; }
        double maximum() const { return map * (size - 1); }
    };

    struct Attribute
    {
        string name;
    };

    DataSet(const vector<int> & size):
        DataSet(string(), size, 1)
    {}

    DataSet(const string & id, const vector<int> & size, int attribute_count = 1):
        m_id(id),
        m_dimensions(size.size()),
        m_attributes(attribute_count),
        m_global_dimensions(size.size() + attribute_count),
        m_selection(size.size(), 0)
    {
        m_data.reserve(attribute_count);
        for (int i = 0; i < attribute_count; ++i)
            m_data.emplace_back(size);
    }

    DataSet(const string & id, const array<double> & data):
        m_id(id),
        m_dimensions(data.size().size()),
        m_attributes(1),
        m_global_dimensions(data.size().size() + 1),
        m_selection(data.size().size(), 0)
    {
        m_data.emplace_back(data);
    }

    DataSource * source() { return m_source; }
    void setSource(DataSource * source) { m_source = source; }

    string id() const { return m_id; }

    array<double> * data() { return & m_data[0]; }
    const array<double> * data() const { return & m_data[0]; }

    double * data(int idx) { return m_data[idx].data(); }
    const double * data(int idx) const { return m_data[idx].data(); }

    int dimensionCount() const { return m_dimensions.size(); }
    Dimension dimension(int idx) const { return m_dimensions[idx]; }
    void setDimension(int idx, const Dimension & dim) { m_dimensions[idx] = dim; }

    Attribute & attribute(int idx) { return m_attributes[idx]; }
    const Attribute & attribute(int idx) const { return m_attributes[idx]; }

    DimensionPtr globalDimension(int idx) const { return m_global_dimensions[idx]; }
    void setGlobalDimension(int idx, const DimensionPtr & dim);

    vector<int> indexForPoint(const vector<double> & point)
    {
        vector<int> index(point.size());
        for (int d = 0; d < point.size(); ++d)
            index[d] = std::round(point[d] / m_dimensions[d].map);
        return index;
    }

    vector<double> pointForIndex(const vector<int> & index)
    {
        vector<double> point(index.size());
        for (int d = 0; d < index.size(); ++d)
            point[d] = m_dimensions[d].map * index[d];
        return point;
    }

    void selectIndex(int dim, int index);
    void selectIndex(const vector<int> & index);

    int selectedIndex(int dim) const { return m_selection[dim]; }

    vector<int> selectedIndex() const { return m_selection; }

    double selectedPoint(int dim) const
    {
        return m_dimensions[dim].map * m_selection[dim];
    }

    vector<double> selectedPoint() const
    {
        vector<double> point(m_selection.size());
        for (int d = 0; d < m_selection.size(); ++d)
        {
            point[d] = m_dimensions[d].map * m_selection[d];
        }
        return point;
    }

signals:
    void selectionChanged();

private:
    void onDimensionFocusChanged();

    DataSource * m_source = nullptr;
    string m_id;
    vector<array<double>> m_data;
    vector<Dimension> m_dimensions;
    vector<Attribute> m_attributes;
    vector<DimensionPtr> m_global_dimensions;
    vector<int> m_selection;

};

using DataSetPtr = std::shared_ptr<DataSet>;

}
