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

    bool m_changed = false;

    for (int d = 0; d < index.size(); ++d)
    {
        if (index[d] < 0 || index[d] >= m_data.size()[d])
            return;
        m_changed |= (index[d] != m_selection[d]);
    }

    m_selection = index;

    if (m_changed)
        emit selectionChanged();
}

}
