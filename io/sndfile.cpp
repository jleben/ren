#include "sndfile.hpp"
#include "../data/data_library.hpp"
#include "../utility/error.hpp"

#include <QFileInfo>

#include <sndfile.h>

namespace datavis {

SoundFileSource::SoundFileSource(const string & file_path, DataLibrary * lib):
    DataSource(lib),
    m_file_path(file_path)
{
    m_name = QFileInfo(QString::fromStdString(file_path)).fileName().toStdString();

    getInfo();
}

static DataSetInfo getInfo(const SF_INFO & sf_info)
{
    DataSetInfo info;
    info.id = "data";

    DataSet::Dimension dim;
    dim.name = "time";
    dim.size = { size_t(sf_info.frames) };
    dim.map.scale = 1.0 / sf_info.samplerate;
    info.dimensions.push_back(dim);

    for (int c = 0; c < sf_info.channels; ++c)
    {
        info.attributes.push_back({ string("channel ") + to_string(c+1) });
    }

    return info;
}

void SoundFileSource::getInfo()
{
    SF_INFO sf_info;
    sf_info.format = 0;

    SNDFILE * file = sf_open(m_file_path.c_str(), SFM_READ, &sf_info);
    if (!file)
    {
        throw Error("Failed to open file.");
    }

    m_info = datavis::getInfo(sf_info);

    sf_close(file);
}

DataSetPtr SoundFileSource::dataset(int index)
{
    if (index != 0)
        return nullptr;

    if (m_dataset)
        return m_dataset;

    SF_INFO sf_info;
    sf_info.format = 0;

    SNDFILE * file = sf_open(m_file_path.c_str(), SFM_READ, &sf_info);
    if (!file)
    {
        throw Error("Failed to open file.");
    }

    m_info = datavis::getInfo(sf_info);

    vector<int> data_size = { int(sf_info.frames) };
    int attribute_count = sf_info.channels;

    auto dataset = make_shared<DataSet>(m_info.id, data_size, attribute_count);
    dataset->setSource(this);

    for (int d = 0; d < m_info.dimensionCount(); ++d)
    {
        const auto & dim = m_info.dimensions[d];

        dataset->setDimension(d, dim);

        DimensionPtr gdim = library()->dimension(dim.name);
        if (gdim)
            dataset->setGlobalDimension(d, gdim);
    }

    for (int a = 0; a < m_info.attributes.size(); ++a)
    {
        dataset->attribute(a) = m_info.attributes[a];
    }

    int batch_size = 1024;
    vector<double> buffer(batch_size * sf_info.channels);

    size_t dest_frame = 0;

    for (sf_count_t f = 0; f < sf_info.frames; f += batch_size)
    {
        auto read_frames = sf_readf_double(file, buffer.data(), batch_size);
        if (read_frames < batch_size && dest_frame + read_frames < sf_info.frames)
        {
            cerr << "ERROR: Reading file at frame " << f << endl;
            goto end;
        }

        size_t buffer_index = 0;
        for (int f = 0; f < read_frames; ++f, ++dest_frame)
        {
            for (int c = 0; c < sf_info.channels; ++c, ++buffer_index)
            {
                dataset->data(c).data()[dest_frame] = buffer[buffer_index];
            }
        }
    }

end:
    sf_close(file);

    m_dataset = dataset;

    return dataset;
}

}
