#include "hdf5.hpp"
#include "../data/data_library.hpp"

#include <QFileInfo>

#include <stdexcept>

using namespace H5;

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

static
vector<double> readAttribute(const string & name, H5::DataSet & dataset)
{
    vector<double> data;

    if (!dataset.attrExists(name.c_str()))
        return data;

    auto attribute = dataset.openAttribute(name.c_str());
    auto space = attribute.getSpace();

    if (!space.isSimple())
        return data;

    if (space.getSimpleExtentNdims() != 1)
        return data;

    data.resize(space.getSimpleExtentNpoints());

    attribute.read(H5::PredType::NATIVE_DOUBLE, data.data());

    return data;
}

static
vector<string> readStringAttribute(const string & name, H5::DataSet & dataset)
{
    cerr << "Reading string attribute: " << name << endl;

    vector<string> data;

    if (!dataset.attrExists(name.c_str()))
        return data;

    auto attribute = dataset.openAttribute(name.c_str());
    auto space = attribute.getSpace();

    if (!space.isSimple())
        return data;

    if (space.getSimpleExtentNdims() != 1)
        return data;

    int count = space.getSimpleExtentNpoints();

    cerr << "Got " << count << " elements." << endl;

    StrType type;
    try
    {
        type = attribute.getStrType();
    }
    catch (H5::DataTypeIException &)
    {
        cerr << "ERROR: Can't read string attribute. Wrong data type." << endl;
        return data;
    }

    size_t maxLen = type.getSize();

    cerr << "Max string length is " << maxLen << endl;

    vector<char> buf(count * maxLen + 1, 0);

    attribute.read(type, buf.data());

    for (int i = 0; i < count; ++i)
    {
        size_t len = 0;
        while(len < maxLen && buf[i+len] != 0)
            ++len;
        string text(buf.data() + i, len);
        cerr << "An element: " << text << endl;
        data.push_back(text);
    }

    return data;
}

static
vector<DataSet::Dimension> readDimensions(H5::DataSet & dataset)
{
    auto dataspace = dataset.getSpace();
    if (!dataspace.isSimple())
        throw std::runtime_error("Data space is not simple.");

    int dim_count = dataspace.getSimpleExtentNdims();

    vector<hsize_t> size(dim_count);
    dataspace.getSimpleExtentDims(size.data());

    auto dim_offset = readAttribute("dimension-offset", dataset);
    auto dim_step = readAttribute("dimension-step", dataset);
    auto dim_name = readStringAttribute("dimension-name", dataset);

    vector<DataSet::Dimension> dims;
    dims.reserve(dim_count);

    for (int d = 0; d < dim_count; ++d)
    {
        DataSet::Dimension dim;
        dim.size = size[d];
        if (d < dim_offset.size())
            dim.map.offset = dim_offset[d];
        if (d < dim_step.size())
            dim.map.scale = dim_step[d];
        if (d < dim_name.size())
            dim.name = dim_name[d];
        dims.push_back(dim);
    }

    return dims;
}

Hdf5Source::Hdf5Source(const string & file_path, DataLibrary * lib):
    DataSource(lib),
    m_file_path(file_path),
    m_file(file_path.c_str(), H5F_ACC_RDONLY)
{
    m_name = QFileInfo(QString::fromStdString(file_path)).fileName().toStdString();

    auto child_count = m_file.getNumObjs();
    for (hsize_t i = 0; i < child_count; ++i)
    {
        auto type = m_file.getObjTypeByIdx(i);

        auto dataset_name = m_file.getObjnameByIdx(i);

        if (type != H5G_DATASET)
        {
            cerr << "Warning: Ignoring object " << dataset_name << "."
                 << " It is not a dataset." << endl;
            continue;
        }

        auto dataset = m_file.openDataSet(dataset_name);

        auto dataspace = dataset.getSpace();
        if (!dataspace.isSimple())
        {
            cerr << "Warning: Ignoring dataset " << dataset_name << "."
                 << " Space is not simple." << endl;
            continue;
        }

        m_dataset_indices.push_back(i);
    }

    m_datasets.resize(m_dataset_indices.size());
}

int Hdf5Source::count() const
{
    return m_dataset_indices.size();
}

int Hdf5Source::index(const string & id) const
{
    for (int i = 0; i < m_dataset_indices.size(); ++i)
    {
        auto dataset_index = m_dataset_indices[i];
        auto dataset_name = m_file.getObjnameByIdx(dataset_index);
        if (id == dataset_name)
            return i;
    }
    return -1;
}

DataSetInfo Hdf5Source::info(int index) const
{
    auto dataset_index = m_dataset_indices[index];

    auto dataset_name = m_file.getObjnameByIdx(dataset_index);

    auto dataset = m_file.openDataSet(dataset_name);

    DataSetInfo info;

    info.id = dataset_name;
    info.dimensions = readDimensions(dataset);
    info.attributes.resize(1);

    return info;
}

DataSetPtr Hdf5Source::dataset(int index)
{
    auto client_dataset = m_datasets[index].lock();
    if (client_dataset)
        return client_dataset;

    auto dataset_index = m_dataset_indices[index];

    auto dataset_name = m_file.getObjnameByIdx(dataset_index);

    auto dataset = m_file.openDataSet(dataset_name);

    auto dataspace = dataset.getSpace();
    if (!dataspace.isSimple())
        throw std::runtime_error("Data space is not simple.");

    auto dimensions = readDimensions(dataset);

    vector<int> object_size;
    for (auto & dim : dimensions)
        object_size.push_back(dim.size);

    client_dataset = make_shared<DataSet>(dataset_name, object_size);
    client_dataset->setSource(this);

    dataset.read(client_dataset->data()->data(), hdf5_type<double>::native_type());

    for (int d = 0; d < dimensions.size(); ++d)
    {
        client_dataset->setDimension(d, dimensions[d]);
        string name = dimensions[d].name;
        DimensionPtr gdim = library()->dimension(name);
        if (gdim)
            client_dataset->setGlobalDimension(d, gdim);
    }

    m_datasets[index] = client_dataset;

    return client_dataset;
}

}
