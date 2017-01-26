#pragma once

#include "../data/array.hpp"
#include "../data/data_object.hpp"

#include <H5Cpp.h>
#include <stdexcept>

namespace datavis {

template <typename T>
struct hdf5_type;

template<> struct hdf5_type<int>
{
    static const H5::PredType & file_type() { return H5::PredType::STD_I32BE; }
    static const H5::PredType & native_type() { return H5::PredType::NATIVE_INT; }
};
template<> struct hdf5_type<float>
{
    static const H5::PredType & file_type() { return H5::PredType::IEEE_F32BE; }
    static const H5::PredType & native_type() { return H5::PredType::NATIVE_FLOAT; }
};
template<> struct hdf5_type<double>
{
    static const H5::PredType & file_type() { return H5::PredType::IEEE_F64BE; }
    static const H5::PredType & native_type() { return H5::PredType::NATIVE_DOUBLE; }
};

template<typename T>
array<T> read_hdf5(const string & file_path, const string & location)
{
    using namespace H5;

    H5File file(file_path.c_str(), H5F_ACC_RDONLY);
    auto dataset = file.openDataSet(location.c_str());
    auto dataspace = dataset.getSpace();
    if (!dataspace.isSimple())
        throw std::runtime_error("Data space is not simple.");

    int dim_count = dataspace.getSimpleExtentNdims();

    vector<hsize_t> file_data_size(dim_count);
    dataspace.getSimpleExtentDims(file_data_size.data());

    vector<int> size(file_data_size.begin(), file_data_size.end());
    datavis::array<T> array(size);

    dataset.read(array.data(), hdf5_type<T>::native_type());

    return array;
}

}
