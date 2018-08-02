#include "data_set.hpp"

namespace datavis {


void DataSet::selectIndex(int dim, int index)
{
    if (m_selection[dim] != index)
    {
        m_selection[dim] = index;
        emit selectionChanged();
    }
}

void DataSet::selectIndex(const vector<int> & index)
{
#if 0
    cout << "DataSet: selecting index: ";
    for (auto & i : index)
        cout << i << " ";
    cout << endl;
#endif
    if (index.size() != m_selection.size())
        throw std::runtime_error("Invalid index: wrong number of dimensions.");

    bool changed = false;

    for (int d = 0; d < index.size(); ++d)
    {
        if (index[d] < 0 || index[d] >= m_data.size()[d])
            continue;
        changed |= (index[d] != m_selection[d]);
        m_selection[d] = index[d];
    }

    if (changed)
        emit selectionChanged();
}

void DataSet::setGlobalDimension(int idx, const DimensionPtr & dim)
{
    auto & my_dim = m_global_dimensions[idx];

    if (my_dim)
        my_dim->disconnect(this);

    my_dim = dim;

    connect(dim.get(), &datavis::Dimension::focusChanged,
            this, &DataSet::onDimensionFocusChanged);
}

void DataSet::onDimensionFocusChanged()
{
    bool changed = false;

    for (int d = 0; d < dimensionCount(); ++d)
    {
        auto gdim = m_global_dimensions[d];
        if (!gdim) continue;

        double focus = gdim->focus();

        int index = std::round(focus / m_dimensions[d].map);
        if (index < 0 || index >= m_data.size()[d])
            continue;

        changed |= (index != m_selection[d]);
        m_selection[d] = index;
    }

    if (changed)
        emit selectionChanged();
}

}
