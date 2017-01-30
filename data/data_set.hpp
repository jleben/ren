#pragma once

#include "array.hpp"
#include "math.hpp"

#include <string>
#include <memory>
#include <QObject>

namespace datavis {

using std::string;

struct Dimension
{
    Mapping1d map;
};

class DataSource;

class DataSet : public QObject
{
    Q_OBJECT

public:
    DataSet(const vector<int> & size):
        DataSet(string(), size)
    {}

    DataSet(const string & id, const vector<int> & size):
        m_id(id),
        m_data(size),
        m_dimensions(size.size()),
        m_selection(size.size(), 0)
    {}

    DataSet(const string & id, const array<double> & data):
        m_id(id),
        m_data(data),
        m_dimensions(data.size().size()),
        m_selection(data.size().size(), 0)
    {}


    DataSet(const string & id, array<double> && data):
        m_id(id),
        m_data(data),
        m_dimensions(data.size().size()),
        m_selection(data.size().size(), 0)
    {}

    DataSource * source() { return m_source; }
    void setSource(DataSource * source) { m_source = source; }

    string id() const { return m_id; }

    array<double> * data() { return & m_data; }
    const array<double> * data() const { return & m_data; }

    Dimension dimension(int idx) const { return m_dimensions[idx]; }
    void setDimension(int idx, const Dimension & dim) { m_dimensions[idx] = dim; }

    vector<int> indexForPoint(const vector<double> & point)
    {
        vector<int> index(point.size());
        for (int d = 0; d < point.size(); ++d)
            index[d] = int(point[d] / m_dimensions[d].map);
        return index;
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
    DataSource * m_source;
    string m_id;
    array<double> m_data;
    vector<Dimension> m_dimensions;
    vector<int> m_selection;

};

using DataSetPtr = std::shared_ptr<DataSet>;

}
