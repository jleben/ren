#include "data_library.hpp"
#include "../io/hdf5.hpp"
#include "../io/text.hpp"
#include "../io/sndfile.hpp"

#include <algorithm>

namespace datavis {

DataLibrary::DataLibrary(QObject * parent):
    QObject(parent)
{}

void DataLibrary::open(const QString & path)
{
    DataSource * source = nullptr;

    auto std_path = path.toStdString();

    try
    {
        if (path.endsWith(".h5"))
        {
            cerr << "Opening data file " << std_path << " as HDF5." << endl;
            source = new Hdf5Source(std_path, this);
        }
        else if (path.endsWith(".wav"))
        {
            cerr << "Opening data file " << std_path << " as sound." << endl;
            source = new SoundFileSource(std_path, this);
        }
        else
        {
            cerr << "Opening data file " << std_path << " as text." << endl;
            source = new TextSource(std_path, this);
        }
    }
    catch (std::runtime_error & e)
    {
        cerr << "ERROR: Failed to open file " << path.toStdString() << ": " << e.what() << endl;
        emit openFailed(path);
        return;
    }
    catch (...)
    {
        emit openFailed(path);
        return;
    }

    m_sources.push_back(source);

    updateDimensions();

    emit sourcesChanged();
}

void DataLibrary::close(DataSource * source)
{
    if (!source)
        return;

    auto pos = std::find(m_sources.begin(), m_sources.end(), source);
    if (pos == m_sources.end())
        return;

    delete *pos;

    m_sources.erase(pos);

    updateDimensions();

    emit sourcesChanged();
}

void DataLibrary::closeAll()
{
    for(DataSource * source : m_sources)
        delete source;

    m_sources.clear();

    updateDimensions();

    emit sourcesChanged();
}

DataSource * DataLibrary::source(const QString & path)
{
    auto std_path = path.toStdString();

    auto it = find_if(m_sources.begin(), m_sources.end(), [&std_path](DataSource * source) {
        return source->id() == std_path;
    });

    if (it == m_sources.end())
        return nullptr;

    return *it;
}

DimensionPtr DataLibrary::dimension(const string & name)
{
    auto it = d_dimensions.find(name);
    if (it == d_dimensions.end())
        return nullptr;
    else
        return it->second;
}

void DataLibrary::updateDimensions()
{
    unordered_map<string, Range> ranges;

    for (DataSource * source : m_sources)
    {
        int data_count = source->count();
        for (int i = 0; i < data_count; ++i)
        {
            auto info = source->info(i);
            for (int d = 0; d < info.dimensionCount(); ++d)
            {
                const auto & dim = info.dimensions[d];
                Range source_range = { dim.minimum(), dim.maximum() };
                string name = dim.name;
                if (name.empty())
                    continue;

                auto range_it = ranges.find(name);
                if (range_it == ranges.end())
                {
                    ranges[name] = source_range;
                }
                else
                {
                    range_it->second = join(range_it->second, source_range);
                }
            }
        }
    }

    // Erase unused dimensions
    {
        auto it = d_dimensions.begin();
        while(it != d_dimensions.end())
        {
            const auto & name = it->first;
            if (ranges.find(name) == ranges.end())
                it = d_dimensions.erase(it);
            else
                ++it;
        }
    }

    // Store new dimension ranges
    for (auto & entry : ranges)
    {
        auto & name = entry.first;
        auto & range = entry.second;

        auto & dim = d_dimensions[name];
        if (!dim)
            dim = make_shared<Dimension>();

        dim->range() = range;
    }
}


}
