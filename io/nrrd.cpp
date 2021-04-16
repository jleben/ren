#include "nrrd.hpp"

#include <QFileInfo>

#include <unordered_map>
#include <optional>

using namespace std;

namespace datavis {

NrrdSource::NrrdSource(const string & file_path, DataLibrary *lib):
    DataSource(lib),
    m_file_path(file_path)
{
    m_name = QFileInfo(QString::fromStdString(file_path)).fileName().toStdString();

    readInfo();
}

NrrdSource::~NrrdSource() = default;

namespace {

enum class DataType
{
    Int8,
    Uint8,
    Int16,
    Uint16,
    Int32,
    Uint32,
    Int64,
    Uint64,
    Float,
    Double,
    Block
};

enum class Encoding
{
    Raw,
    Text,
    Hex,
    Gzip,
    Bzip2
};

enum class Endian
{
    Little,
    Big
};

enum class Centering
{
    None,
    Cell,
    Node
};

struct Header
{
    std::int32_t dimension;
    DataType type;
    Encoding encoding;
    optional<Endiand> endian;
    string content;
    optional<string> dataFile;
    optional<std::int32_t> lineSkip;
    optional<std::int32_t> byteSkip;
    optional<string> sampleUnits;
    vector<std::int32_t> sizes;
    optional<vector<double>> spacings;
    optional<vector<double>> axisMins;
    optional<vector<double>> axisMaxs;
    optional<vector<Centering>> centerings;
    optional<vector<string>> labels;
    optional<vector<string>> units;
};

}

virtual vector<string> dataset_ids() const override;
virtual DataSetInfo dataset_info(const string & id) const override;
virtual FutureDataset NrrdSource::dataset(const string & id) override
{
    if (m_dataset) {
        return m_dataset;
    }


}

}
