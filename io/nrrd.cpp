#include "nrrd.hpp"

#include <QFileInfo>

#include <unordered_map>
#include <optional>
#include <regex>
#include <string_view>
#include <string>

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

namespace Nrrd {

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
    optional<std::int32_t> dimension;
    optional<DataType> type;
    optional<Encoding> encoding;
    optional<Endian> endian;
    optional<string> content;
    optional<string> dataFile;
    optional<std::int32_t> lineSkip;
    optional<std::int32_t> byteSkip;
    optional<string> sampleUnits;
    optional<vector<std::int32_t>> sizes;
    optional<vector<double>> spacings;
    optional<vector<double>> axisMins;
    optional<vector<double>> axisMaxs;
    optional<vector<Centering>> centerings;
    optional<vector<string>> labels;
    optional<vector<string>> units;
};

Header readHeader(istream &stream)
{
    readMagic();

    Header header;

    std::int32_t lineNumber = 1;

    while(stream) {
        string line;
        if (std::getline(stream, line)) {
            if (line.empty()) {
                // End of header
                return header;
            }
            parseLine(header, line, lineNumber);
            ++lineNumber;
        }
        // FIXME: What exactly is an error condition?
    }

    return header;
}

void parseLine(Header &header, const string &line, std::int32_t lineNumber)
{
    if (line[0] == '#') {
        // Comment. Ignore.
        return;
    }

    auto fieldDelimPos = line.find(": ");
    if (fieldDelimPos != string::npos)
    {
        string name = line.substr(0, fieldDelimPos);
        string rest = line.substr(fieldDelimPos + 1);
        string_view value(rest);
        while(!value.empty() && value.back() == ' ') {
            value.remove_suffix(1);
        }
        parseField(Header &header, name, value);
    }

    auto keyValueDelimPos = line.find(":=");
    if (keyValueDelimPos != string::npos)
    {
        // ignore
        return;
    }

    throw std::runtime_error("Line " + to_string(lineNumber) + ": Unexpected line format.");
}

vector<string> parseList(const string &text)
{
    istringstream stream(text);

    vector<string> list;
    while(stream) {
        string word;
        if (stream >> word) {
            list.push_back(word);
        }
        // FIXME: Errors?
    }
    return list;
}

template <typename T>
T parseNumber(const string &text)
{
    T value;
    auto result = std::from_chars(text.begin(), text.end(), value);
    if (result.ec != 0 || result.ptr != text.end()) {
        throw std::runtime_error("Failed to parse number.");
    }
    return value;
}

void parseField(Header &header, const string &name, const string &text)
{
    try {
        if (name == "dimension") {
            header.dimension = parseNumber<std::int32_t>(text);
        }
        else if (name == "type") {
            header.type = parseType(text);
        }
        else if (name == "encoding") {
            //header.encoding =
        }
        else if (name == "sizes") {
            auto list = parseList(text);
            for (auto &elem : list) {
                header.sizes.push_back(parseNumber<std::int32_t>(elem));
            }
        }
    } catch(std::runtime_error const &e) {
        throw std::runtime_error("Failed to parse field " + name + ": " + e.what());
    }
}

DataType parseType(const string &text) {
    std::unordered_map<string, DataType> typeMap{
        {"signed char", DataType::Int8},
        {"int8", DataType::Int8},
        {"int8_t", DataType::Int8},
        {"uchar", DataType::Uint8},
        {"unsigned char", DataType::Uint8},
        {"uint8", DataType::Uint8},
        {"uint8_t", DataType::Uint8},
        {"short", DataType::Int16},
        {"short int", DataType::Int16},
        {"signed short", DataType::Int16},
        {"signed short int", DataType::Int16},
        {"int16", DataType::Int16},
        {"int16_t", DataType::Int16},
        {"ushort", DataType::Uint16},
        {"unsigned short", DataType::Uint16},
        {"unsigned short int", DataType::Uint16},
        {"uint16", DataType::Uint16},
        {"uint16_t", DataType::Uint16},
        {"int", DataType::Int32},
        {"signed int", DataType::Int32},
        {"int32", DataType::Int32},
        {"int32_t", DataType::Int32},

        {"uint", DataType::Uint32},
        {"unsigned int", DataType::Uint32},
        {"uint32", DataType::Uint32},
        {"uint32_t", DataType::Uint32},

        {"longlong", DataType::Int64},
        {"long long", DataType::Int64},
        {"long long int", DataType::Int64},
        {"signed long long", DataType::Int64},
        {"signed long long int", DataType::Int64},
        {"int64", DataType::Int64},
        {"int64_t", DataType::Int64},

        {"ulonglong", DataType::Uint64},
        {"unsigned long long", DataType::Uint64},
        {"unsigned long long int", DataType::Uint64},
        {"uint64", DataType::Uint64},
        {"uint64_t", DataType::Uint64},

        {"float", DataType::Float},
        {"double", DataType::Double},
        {"block", DataType::Block},
    };

    try {
        return typeMap.at(text);
    } catch (std::out_of_range const &) {
        throw std::runtime_error("Invalid data type: " + text);
    }
}


void readMagic(istream &stream)
{
    string line;
    std::readline(stream, line);
    if (!stream) {
        throw std::runtime_error("Failed to read first line.");
    }

    std::regex magic_pattern("NRRD[0-9]{4}");
    if (!std::regex_match(line, magic_pattern)) {
        throw std::runtime_error("Magic pattern not found.");
    }
}

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
