#include "nrrd.hpp"
#include "../utility/error.hpp"

#include <QFileInfo>

#include <unordered_map>
#include <optional>
#include <regex>
#include <string_view>
#include <string>

using namespace std;

namespace datavis {

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

template <typename T>
void parseNumber(T &value, const string &text)
{
    auto result = std::from_chars(text.begin(), text.end(), value);
    if (result.ec != 0 || result.ptr != text.end()) {
        throw std::runtime_error("Failed to parse number.");
    }
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
void parseNumList(vector<T> &list, const string &text)
{
    vector<string> elems = parseList(text);
    std::int32_t index = 0;
    for (string const &elem : elems) {
        T value;
        try {
            parseNumber(value);
        } catch (std::runtime_error const &e) {
            throw std::runtime_error("Element " + to_string(index) + ": " + e.what());
        }
        list.push_back(value);
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

Encoding parseEncoding(const string &text)
{
    std::unordered_map<string, Encoding> encodingMap{
        {"raw", Encoding::Raw},
        {"txt", Encoding::Text},
        {"text", Encoding::Text},
        {"ascii", Encoding::Text},
        {"hex", Encoding::Hex},
        {"gz", Encoding::Gzip},
        {"gzip", Encoding::Gzip},
        {"bz2", Encoding::Bzip2},
        {"bzip2", Encoding::Bzip2}
    };

    try {
        return encodingMap.at(text);
    } catch (std::out_of_range const &) {
        throw std::runtime_error("Invalid encoding: " + text);
    }
}

Centering parseCentering(const string &text)
{
    std::unordered_map<string, Centering> map{
        {"cell", Centering::Cell},
        {"node", Centering::Node},
        {"???", Centering::None},
        {"none", Centering::None}
    };

    try {
        return map.at(text);
    } catch (std::out_of_range const &) {
        throw std::runtime_error("Invalid centering: " + text);
    }
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
            header.encoding = parseEncoding(text);
        }
        else if (name == "content") {
            header.content = text;
        }
        else if (name == "datafile") {
            header.dataFile = text;
        }
        else if (name == "lineskip" or name == "line skip") {
            parseNumber(header.lineSkip, text);
        }
        else if (name == "byteskip" or name == "byte skip") {
            parseNumber(header.byteSkip, text);
        }
        else if (name == "sample units" or name == "sampleunits") {
            header.sampleUnits = text;
        }
        else if (name == "sizes") {
            parseNumList(header.sizes, text);
        }
        else if (name == "spacings") {
            parseNumList(header.spacings, text);
        }
        else if (name == "axis mins" or name == "axismins") {
            parseNumList(header.axisMins, text);
        }
        else if (name == "axis maxs" or name == "axismaxs") {
            parseNumList(header.axisMaxs, text);
        }
        else if (name == "centers" or name == "centerings") {
            auto list = parseList(text);
            for (string const &elem : list) {
                header.centerings.push_back(parseCentering(elem));
            }
        }
        else if (name == "labels") {
            header.labels = parseList(text);
        }
        else if (name == "units") {
            header.units = parseList(text);
        }
        else {
            cerr << "NRRD: Ignoring field '" << name << "'." << endl;
        }
    } catch(std::runtime_error const &e) {
        throw std::runtime_error("Failed to parse field " + name + ": " + e.what());
    }
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

Header readHeader(istream &stream)
{
    readMagic(stream);

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

template <typename T>
void validate_field_present(optional<T> const &field, string_view name)
{
    if (!field.has_value()) {
        throw Error("Missing field: " + name);
    }
}

template <DataType>
struct CppTypeHelper;

template <> struct CppTypeHelper<DataType::Int8> { using Type = std::int8_t; };
template <> struct CppTypeHelper<DataType::Uint8> { using Type = std::uint8_t; };
template <> struct CppTypeHelper<DataType::Int16> { using Type = std::int16_t; };
template <> struct CppTypeHelper<DataType::Uint16> { using Type = std::uint16_t; };
template <> struct CppTypeHelper<DataType::Int32> { using Type = std::int32_t; };
template <> struct CppTypeHelper<DataType::Uint32> { using Type = std::uint32_t; };
template <> struct CppTypeHelper<DataType::Int64> { using Type = std::int64_t; };
template <> struct CppTypeHelper<DataType::Uint64> { using Type = std::uint64_t; };
template <> struct CppTypeHelper<DataType::Float> { using Type = float; };
template <> struct CppTypeHelper<DataType::Double> { using Type = double; };

template <DataType D> using CppType = CppTypeHelper<D>::Type;

template <DataType D> struct DataTypeSize
{
    auto visit() { return sizeof(CppType<D>); }
};

template <typename V>
auto visit(DataType type)
{
    switch(type)
    {
        case DataType::Int8:
            return V<DataType::Int8>::visit();
        case DataType::Uint8:
            return V<DataType::Uint8>::visit();
        case DataType::Int16:
            return V<DataType::Uint16>::visit();
        case DataType::Uint16:
            return V<DataType::Uint16>::visit();
        case DataType::Int32:
            return V<DataType::Int32>::visit();
        case DataType::Uint32:
            return V<DataType::Uint32>::visit();
        case DataType::Int64:
            return V<DataType::Int64>::visit();
        case DataType::Uint64:
            return V<DataType::Uint64>::visit();
        case DataType::Double:
            return V<DataType::Double>::visit();
        case DataType::Float:
            return V<DataType::Float>::visit();
        default:
            throw std::logic_error("Unexpected DataType.");
    }
}

size_t data_type_size(DataType type)
{
    return visit<DataTypeSize>(type);
}

template<typename T>
void validate_list_size(vector<T> const &list, string_view name, std::size_t expected)
{
    if (list.size() != expected)
    {
        throw Error("Field '" + name + "' has an invalid number of elements - "
            + to_string(expected) + " required.");
    }
}

void validate_header(Header const &header)
{
    validate_field_present(header.dimension, "dimension");
    if (*header.dimension < 1) {
        throw Error("Field 'dimension' must be larger or equal 1.");
    }

    validate_field_present(header.type, "type");

    if (*header.type == DataType::Block)
    {
        throw Error("Data type not supported.");
    }

    validate_field_present(header.encoding, "encoding");
    if (*header.encoding != Encoding::Raw || *header.encoding != Encoding::Text) {
        throw Error("Encoding not supported.");
    }

    if (*header.encoding != Encoding::Text && data_type_size(*header.type) > 1) {
        validate_field_present(header.endian, "endian");
    }

    validate_field_present(header.sizes);
    validate_list_size(*header.sizes, "sizes", *header.dimension);

    if (header.spacings.has_value && header.axisMins.has_value && header.axisMaxs.has_value) {
        throw Error("The field 'spacings' is not allowed "
            "when both fields 'axis mins' and 'axis maxs' are present.");
    }
    if (header.spacings.has_value) {
        validate_list_size(*header.spacings, "spacings", *header.dimension);
    }

    if (header.axisMins.has_value) {
        validate_list_size(*header.axisMins, "axis mins", *header.dimension);
    }

    if (header.axisMaxs.has_value) {
        validate_list_size(*header.axisMaxs, "axis maxs", *header.dimension);
    }

    if (header.centerings.has_value) {
        validate_list_size(*header.centerings, "centers", *header.dimension);
    }

    if (header.labels.has_value) {
        validate_list_size(*header.labels, "labels", *header.dimension);
    }

    if (header.units.has_value) {
        validate_list_size(*header.units, "units", *header.dimension);
    }
}

}

namespace {

vector<DataSet::Dimension> get_dimensions(Nrrd::Header const &header)
{
    vector<DataSet::Dimension> dimensions;

    dimensions.resize(header.dimension);

    for (std::int32_t i = 0; i < header.dimension; ++i)
    {
        DataSet::Dimension & dimension = dimensions[i];

        if (header.labels.has_value()) {
            dimension.name = header.labels[i];
        }
        dimension.size = std::size_t(header.sizes[i]);

        bool hasNodeCentering =
            header.centerings.has_value() && header.centerings[i] == Centering::Node;

        if (header.axisMins.has_value() && header.axisMaxs.has_value())
        {
            double range = (*header.axisMaxs)[i] - (*header.axisMins)[i];
            if (range == 0)
            {
                dimension.map.scale = 0;
            }
            else if (hasNodeCentering)
            {
                if (dimension.size < 2)
                    dimension.map.scale = 0;
                else
                    dimension.map.scale = range / (dimension.size - 1);
            }
            else
            {
                if (dimension.size < 1)
                    dimension.map.scale = 0;
                else
                    dimension.map.scale = range / dimension.size;
            }
        }

        if (header.spacings.has_value()) {
            dimension.map.scale = header.spacings[i];
        }

        if (header.axisMins.has_value())
        {
            dimension.map.offset = header.axisMins[i];
            if (!hasNodeCentering) {
                dimension.map.offset += dimension.map.scale / 0.5;
            }
        }
        else if (header.axisMaxs.has_value())
        {
            if (dimension.size < 1)
            {
                dimension.map.offset = 0;
            }
            else
            {
                if (hasNodeCentering)
                    dimension.map.offset = header.axisMaxs[i] -
                        dimension.map.scale * (dimension.size - 1);
                 else
                    dimension.map.offset = header.axisMaxs[i] -
                        dimension.map.scale * (double(dimension.size) + 0.5);
            }
        }
    }
}

vector<DataSet::Attribute> get_attributes(Nrrd::Header const &header)
{
    DataSet::Attribute attribute;
    attribute.name = "value";
    return { attribute };
}

}

NrrdSource::NrrdSource(const string & file_path, DataLibrary *lib):
    DataSource(lib),
    m_file_path(file_path)
{
    m_name = QFileInfo(QString::fromStdString(file_path)).fileName().toStdString();

    m_info = readInfo();
}

NrrdSource::~NrrdSource() = default;

virtual vector<string> NrrdSource::dataset_ids() const override
{
    return {"data"};
}

virtual DataSetInfo NrrdSource::dataset_info(const string & id) const override
{
    return m_info;
}

virtual FutureDataset NrrdSource::dataset(const string & id) override
{
    if (m_dataset) {
        return m_dataset;
    }
}

DataSetInfo NrrdSource::readInfo()
{
    ifstream file(m_file_path);
    if (!file.is_open()) {
        throw Error("Failed to open file: " + m_file_path);
    }

    Header header;
    try {
        header = Nrrd::readHeader(file);
        Nrrd::validate_header(header);
    } catch (std::exception &e) {
        throw Error(string("Failed to read header: ") + e.what());
    }

    DataSetInfo info;
    info.dimensions = get_dimensions(header);
    info.attributes = get_attributes(header);
    return info;
}

}
