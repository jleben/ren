#pragma once

#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

namespace datavis {

using std::vector;

inline
int flat_size(const vector<int> & size)
{
    int fs = 1;
    for (auto & s : size)
        fs *= s;
    return fs;
}

inline
int flat_index(const vector<int> & index, const vector<int> & size)
{
    int fi = index[0];
    for (unsigned d = 1; d < index.size(); ++d)
    {
        fi *= size[d];
        fi += index[d];
    }
    return fi;
}

class abstract_array
{
};

template <typename T>
class array : public abstract_array
{
public:
    using index_t = vector<int>;
    using size_t = vector<int>;

    array(size_t size):
        m_size(size),
        m_data(flat_size(size))
    {}

    const size_t & size() const { return m_size; }

    T & operator()(const index_t & i)
    {
        auto j = flat_index(i, m_size);
        return m_data[j];
    }

    const T & operator()(const index_t & i) const
    {
        auto j = flat_index(i, m_size);
        return m_data[j];
    }

    T * data() { return m_data.data(); }

    const T * data() const { return m_data.data(); }

private:
    size_t m_size;
    vector<T> m_data;
};

template <typename T>
class array_region
{
    T * m_data = nullptr;
    vector<int> m_data_size;
    vector<int> m_region_offset;
    vector<int> m_region_size;

public:
    array_region()
    {}

    array_region(array<T> & a, const vector<int> & offset, const vector<int> & size):
        m_data(a.data()),
        m_data_size(a.size()),
        m_region_offset(offset),
        m_region_size(size)
    {}

    bool is_valid() const { return m_data != nullptr; }

    class iterator
    {
        T * m_data = nullptr;
        vector<int> m_start;
        vector<int> m_end;
        vector<int> m_stride;
        vector<int> m_location;
        int m_index;

    public:
#if 0
        class value_type
        {
        public:
            T & value()
            {
                return m_data[m_index];
            }

            int index() const { return m_index; }

        private:
            friend class iterator;
            T * m_data;
            int m_index;
            vector<int> m_location;
        };
#endif

        iterator(T * data,
                 const vector<int> & start,
                 const vector<int> & end,
                 const vector<int> & stride,
                 int start_index):
            m_data(data),
            m_start(start),
            m_end(end),
            m_stride(stride),
            m_location(start),
            m_index(start_index)
        {
        }

        iterator() {}

        bool operator==(const iterator & other) const
        {
            if (m_data != other.m_data)
                return false;

            if (m_data)
            {
                return m_index == other.m_index;
            }
            else
            {
                return true;
            }
        }

        bool operator!=(const iterator & other) const
        {
            return !(*this == other);
        }

        bool is_valid() const
        {
            return m_data != nullptr;
        }

        const vector<int> & location() const
        {
            return m_location;
        }

        int index() const { return m_index; }

        T & value() const
        {
            return m_data[m_index];
        }

        iterator & operator++()
        {
            int n_dim = m_end.size();
            int d;
            for(d = n_dim - 1; d >= 0; --d)
            {
                ++m_location[d];
                if (m_location[d] < m_end[d])
                {
                    m_index += m_stride[d];
                    break;
                }
                m_location[d] = m_start[d];
            }
            // FIXME: Optimize:
            if (d < 0)
                m_data = nullptr; // Invalidate.

            return *this;
        }

        bool operator<(const iterator & other) const
        {
            return value() < other.value();
        }

        iterator & operator*()
        {
            return *this;
        }
    };

    iterator begin()
    {
        int n_dim = m_data_size.size();

        vector<int> flat_strides(n_dim, 0);

        vector<int> stride(n_dim, 0);

        stride.back() = 1;

        for (int d = n_dim - 1; d >= 0; --d)
        {
            flat_strides[d] = flat_index(stride, m_data_size);

            stride[d] += m_data_size[d] - m_region_size[d];
/*
            subspace_size *= m_size[d+1];
            dim_stride = dim_stride + subspace_size * (m_size[d] - size[d]);
            stride.push_back(dim_stride);
            */
            //cout << "stride " << d << " = " << flat_strides[d] << endl;
        }

        auto & start = m_region_offset;
        vector<int> end = m_region_offset;
        for (int d = 0; d < m_region_size.size(); ++d)
            end[d] += m_region_size[d];

        return iterator(m_data,
                        start,
                        end,
                        flat_strides,
                        flat_index(m_region_offset, m_data_size));
#if 0
        vector<int> it_size;
        vector<int> it_stride;
        for (int d = 0; d < n_dim; ++d)
        {
            if (m_region_size[d] < 2)
                continue;

            it_size.push_back(m_region_size[d]);
            it_stride.push_back(flat_strides[d]);
        }

        return iterator(m_data,
                        it_size,
                        it_stride,
                        flat_index(m_region_offset, m_data_size));
#endif
    }

    iterator end()
    {
        return iterator();
    }
};

template<typename T>
inline
array_region<T>
get_region(array<T> & array, const vector<int> & offset, const vector<int> & size)
{
    return array_region<T>(array, offset, size);
}

template<typename T>
inline
array_region<T>
get_all(array<T> & array)
{
    return array_region<T>(array, vector<int>(array.size().size(), 0), array.size());
}

}
