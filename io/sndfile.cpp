#include "sndfile.hpp"
#include "../data/data_library.hpp"
#include "../utility/error.hpp"
#include "../utility/threads.hpp"

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

SoundFileSource::Read_Result SoundFileSource::read_file(const string & file_path)
{
    SF_INFO sf_info;
    sf_info.format = 0;

    SNDFILE * file = sf_open(file_path.c_str(), SFM_READ, &sf_info);
    if (!file)
    {
        throw Error("Failed to open file.");
    }

    auto info = datavis::getInfo(sf_info);

    vector<int> data_size = { int(sf_info.frames) };
    int attribute_count = sf_info.channels;

    auto dataset = make_shared<DataSet>(info.id, data_size, attribute_count);
    //dataset->setSource(this);

    for (int d = 0; d < info.dimensionCount(); ++d)
    {
        const auto & dim = info.dimensions[d];
        dataset->setDimension(d, dim);
    }

    for (int a = 0; a < info.attributes.size(); ++a)
    {
        dataset->attribute(a) = info.attributes[a];
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

    Read_Result result;
    result.info = info;
    result.dataset = dataset;
    return result;
}

//DataSetPtr SoundFileSource::dataset(int index)
FutureDataset SoundFileSource::dataset(const string & id)
{
    if (m_dataset)
        return m_dataset;

    auto reading = Reactive::apply(background_thread(), [=](Reactive::Status&)
    {
        return read_file(m_file_path);
    });

    m_dataset = Reactive::apply([=](Reactive::Status&, const Read_Result & result)
    {
        // FIXME: Notify anyone about potentially updated info?
        m_info = result.info;

        auto & dataset = result.dataset;

        dataset->setSource(this);

        for (int d = 0; d < dataset->dimensionCount(); ++d)
        {
            const auto & name = dataset->dimension(d).name;
            DimensionPtr gdim = library()->dimension(name);
            if (gdim)
                dataset->setGlobalDimension(d, gdim);
        }

        printf("SoundFileSource: dataset ready.\n");

        return dataset;
    },
    reading);

    return m_dataset;
}

}
