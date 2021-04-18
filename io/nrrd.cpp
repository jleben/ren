#include "nrrd.hpp"
#include "../utility/error.hpp"
#include "../utility/threads.hpp"

#include <QFileInfo>
#include <QDir>

#include <unordered_map>
#include <optional>
#include <regex>
#include <string_view>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>

using namespace std;

namespace datavis {

namespace {

template <typename T>
struct TypeToken {};

template <auto V>
struct ValueToken {};

constexpr bool platformIsBigEndian()
{
    std::uint32_t i = 1;
    return ((char*)(&i))[0] == 0;
}

}

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

template <typename F>
auto visit(DataType type, F func)
{
    switch(type)
    {
        case DataType::Int8:
            return func(ValueToken<DataType::Int8>{});
        case DataType::Uint8:
            return func(ValueToken<DataType::Uint8>{});
        case DataType::Int16:
            return func(ValueToken<DataType::Uint16>{});
        case DataType::Uint16:
            return func(ValueToken<DataType::Uint16>{});
        case DataType::Int32:
            return func(ValueToken<DataType::Int32>{});
        case DataType::Uint32:
            return func(ValueToken<DataType::Uint32>{});
        case DataType::Int64:
            return func(ValueToken<DataType::Int64>{});
        case DataType::Uint64:
            return func(ValueToken<DataType::Uint64>{});
        case DataType::Double:
            return func(ValueToken<DataType::Double>{});
        case DataType::Float:
            return func(ValueToken<DataType::Float>{});
        default:
            throw std::logic_error("Unexpected DataType.");
    }
}

void readMagic(istream &stream)
{
    string line;
    std::getline(stream, line);
    if (!stream) {
        throw std::runtime_error("Failed to read first line.");
    }

    std::regex magic_pattern("NRRD[0-9]{4}");
    if (!std::regex_match(line, magic_pattern)) {
        throw std::runtime_error("Magic pattern not found.");
    }
}

void parseNumber(std::int32_t &value, string_view text)
{
    char *end;
    value = std::int32_t(std::strtol(text.data(), &end, 10));
    if (end != text.data() + text.size()) {
        throw Error("Failed to parse integer.");
    }
}

void parseNumber(double &value, string_view text)
{
    char *end;
    value = std::strtod(text.data(), &end);
    if (end != text.data() + text.size()) {
        throw Error("Failed to parse floating point number.");
    }
}

vector<string> parseList(string_view text)
{
    // FIXME:
    string text_str(text);
    std::istringstream stream(text_str);
    vector<string> list;

    while(!stream.fail())
    {
        string word;
        if (stream >> word) {
            list.push_back(word);
        }
        // FIXME: Errors?
    }

    return list;
}

template <typename T>
void parseNumList(vector<T> &list, string_view text)
{
    vector<string> elems = parseList(text);
    std::int32_t index = 0;
    for (string const &elem : elems) {
        T value;
        try {
            parseNumber(value, elem);
        } catch (std::runtime_error const &e) {
            throw std::runtime_error("Element " + to_string(index) + ": " + e.what());
        }
        list.push_back(value);
    }
}

DataType parseType(string_view text) {
    std::unordered_map<string_view, DataType> typeMap{
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
        throw Error(string("Invalid data type: ") + string(text));
    }
}

Encoding parseEncoding(string_view text)
{
    std::unordered_map<string_view, Encoding> encodingMap{
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
        throw std::runtime_error(string("Invalid encoding: ") + string(text));
    }
}

Centering parseCentering(string_view text)
{
    std::unordered_map<string_view, Centering> map{
        {"cell", Centering::Cell},
        {"node", Centering::Node},
        {"???", Centering::None},
        {"none", Centering::None}
    };

    try {
        return map.at(text);
    } catch (std::out_of_range const &) {
        throw std::runtime_error("Invalid centering: " + string(text));
    }
}

void parseField(Header &header, const string &name, string_view text)
{
    try {
        if (name == "dimension") {
            parseNumber(header.dimension.emplace(), text);
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
            parseNumber(header.lineSkip.emplace(), text);
        }
        else if (name == "byteskip" or name == "byte skip") {
            parseNumber(header.byteSkip.emplace(), text);
        }
        else if (name == "sample units" or name == "sampleunits") {
            header.sampleUnits = text;
        }
        else if (name == "sizes") {
            parseNumList(header.sizes.emplace(), text);
        }
        else if (name == "spacings") {
            parseNumList(header.spacings.emplace(), text);
        }
        else if (name == "axis mins" or name == "axismins") {
            parseNumList(header.axisMins.emplace(), text);
        }
        else if (name == "axis maxs" or name == "axismaxs") {
            parseNumList(header.axisMaxs.emplace(), text);
        }
        else if (name == "centers" or name == "centerings") {
            auto list = parseList(text);
            auto &centerings = header.centerings.emplace();
            for (string const &elem : list) {
                centerings.push_back(parseCentering(elem));
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
        parseField(header, name, value);
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
        throw Error(string("Missing field: ") + string(name));
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

template <DataType D> using CppType = typename CppTypeHelper<D>::Type;

struct DataTypeSize
{
    template <DataType D>
    auto operator()(ValueToken<D>) { return sizeof(CppType<D>); }
};

size_t data_type_size(DataType type)
{
    return visit(type, DataTypeSize{});
}

template<typename T>
void validate_list_size(vector<T> const &list, string_view name, std::size_t expected)
{
    if (list.size() != expected)
    {
        throw Error(string("Field '") + string(name) + "' has an invalid number of elements - "
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

    validate_field_present(header.sizes, "sizes");
    validate_list_size(*header.sizes, "sizes", *header.dimension);

    if (header.spacings.has_value() && header.axisMins.has_value() && header.axisMaxs.has_value()) {
        throw Error("The field 'spacings' is not allowed "
            "when both fields 'axis mins' and 'axis maxs' are present.");
    }
    if (header.spacings.has_value()) {
        validate_list_size(*header.spacings, "spacings", *header.dimension);
    }

    if (header.axisMins.has_value()) {
        validate_list_size(*header.axisMins, "axis mins", *header.dimension);
    }

    if (header.axisMaxs.has_value()) {
        validate_list_size(*header.axisMaxs, "axis maxs", *header.dimension);
    }

    if (header.centerings.has_value()) {
        validate_list_size(*header.centerings, "centers", *header.dimension);
    }

    if (header.labels.has_value()) {
        validate_list_size(*header.labels, "labels", *header.dimension);
    }

    if (header.units.has_value()) {
        validate_list_size(*header.units, "units", *header.dimension);
    }
}

std::string resolve_data_path(std::string const &header_path, std::string const &data_path)
{
    if (data_path.empty()) {
        return {};
    }

    if (data_path[0] == '/') {
        return data_path;
    }

    QFileInfo info(QString::fromStdString(header_path));
    auto dir = info.dir();
    return dir.filePath(QString::fromStdString(data_path)).toStdString();
}


void readText(istream & input, array<double> & output)
{
    for (std::size_t i = 0; i < output.flat_size(); ++i)
    {
        std::string word;
        input >> word;
        if (!input or word.empty()) {
            throw Error("Failed to read value at index " + to_string(i));
        }

        try {
            // FIXME: value representable in double?
            Nrrd::parseNumber(output.data()[i], word);
        } catch (std::exception const &) {
            throw Error("Malformed value at index " + to_string(i));
        }
    }
}

struct RawReader
{
    istream &input;
    array<double> &output;
    optional<Nrrd::Endian> endian;

    template <Nrrd::DataType T>
    void operator()(ValueToken<T>)
    {
        using CppType = Nrrd::CppType<T>;

        constexpr std::size_t elem_size = sizeof(CppType);
        std::array<char, elem_size> bytes;

        for (std::size_t i = 0; i < output.flat_size(); ++i)
        {
            input.read(bytes.data(), elem_size);
            if (!input) {
                throw Error("Failed to read element at index " + to_string(i));
            }

            if (elem_size > 1)
            {
                if ((*endian == Nrrd::Endian::Little && platformIsBigEndian()) ||
                    (*endian == Nrrd::Endian::Big && !platformIsBigEndian()))
                {
                    std::reverse(bytes.begin(), bytes.end());
                }
            }

            CppType value = *reinterpret_cast<CppType*>(bytes.data());
            // FIXME: Check unrepresentable values
            output.data()[i] = double(value);
        }
    }
};

void readRaw(istream & input, array<double> & data,
    Nrrd::DataType type, optional<Nrrd::Endian> endian)
{
    visit(type, RawReader{input, data, endian});
}

array<double> readData(istream & input, Nrrd::Header const &header)
{
    vector<int> size(header.sizes->begin(), header.sizes->end());
    array<double> data(size);

    switch(*header.encoding)
    {
    case Nrrd::Encoding::Text:
        readText(input, data);
        break;
    case Nrrd::Encoding::Raw:
        readRaw(input, data, *header.type, header.endian);
        break;
    default:
        throw Error("Unsupported encoding.");
    }

    return data;
}

}

namespace {

vector<DataSet::Dimension> get_dimensions(Nrrd::Header const &header)
{
    vector<DataSet::Dimension> dimensions(*header.dimension);

    for (std::int32_t i = 0; i < *header.dimension; ++i)
    {
        DataSet::Dimension & dimension = dimensions[i];

        if (header.labels.has_value()) {
            dimension.name = header.labels->at(i);
        }
        dimension.size = std::size_t(header.sizes->at(i));

        bool hasNodeCentering =
            header.centerings.has_value() && header.centerings->at(i) == Nrrd::Centering::Node;

        if (header.axisMins.has_value() && header.axisMaxs.has_value())
        {
            double range = header.axisMaxs->at(i) - header.axisMins->at(i);
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
            dimension.map.scale = header.spacings->at(i);
        }

        if (header.axisMins.has_value())
        {
            dimension.map.offset = header.axisMins->at(i);
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
                    dimension.map.offset = header.axisMaxs->at(i) -
                        dimension.map.scale * (dimension.size - 1);
                 else
                    dimension.map.offset = header.axisMaxs->at(i) -
                        dimension.map.scale * (double(dimension.size) + 0.5);
            }
        }
    }

    return dimensions;
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

vector<string> NrrdSource::dataset_ids() const
{
    return { "data" };
}

DataSetInfo NrrdSource::dataset_info(const string &) const
{
    return m_info;
}

FutureDataset NrrdSource::dataset(const string &)
{
    if (m_dataset) {
        return m_dataset;
    }

    // FIXME: Make async

    try
    {
        ifstream file(m_file_path);
        if (!file.is_open()) {
            throw Error("Failed to open file: " + m_file_path);
        }

        auto header = Nrrd::readHeader(file);

        array<double> data;

        if (header.dataFile.has_value())
        {
            string data_path = Nrrd::resolve_data_path(m_file_path, *header.dataFile);
            ifstream data_file(data_path);

            if (!data_file.is_open()) {
                // FIXME: Support exceptions
                throw Error("Failed to open data file: " + data_path);
            }

            data = Nrrd::readData(data_file, header);
        }
        else
        {
            data = Nrrd::readData(file, header);
        }

        auto dimensions = get_dimensions(header);
        auto attributes = get_attributes(header);

        auto dataset = make_shared<DataSet>("data", *header.sizes, attributes.size());

        dataset->setSource(this);

        dataset->data(0) = std::move(data);

        for (int i = 0; i < dimensions.size(); ++i) {
            dataset->dimension(i) = dimensions[i];
        }
        for (int i = 0; i < dimensions.size(); ++i) {
            dataset->attribute(i) = attributes[i];
        }

        return Reactive::value(dataset);
    }
    catch (std::exception const &e)
    {
        // FIXME: Propagate error to UI?
        cerr << "Failed to read data: " << e.what() << endl;
        return nullptr;
    }

#if 0
    auto data = Reactive::apply(background_thread(),
        [m_file_path](Reactive::Status &)
        {
            ifstream file(m_file_path);
            if (!file.is_open()) {
                // FIXME: Support exceptions
                cerr << ""
                throw Error("Failed to open file: " + m_file_path);
            }

            auto header = Nrrd::readHeader(
        });
#endif
}

DataSetInfo NrrdSource::readInfo()
{
    ifstream file(m_file_path);
    if (!file.is_open()) {
        throw Error("Failed to open file: " + m_file_path);
    }

    Nrrd::Header header;
    try {
        header = Nrrd::readHeader(file);
        Nrrd::validate_header(header);
    } catch (std::exception &e) {
        throw Error(string("Failed to read header: ") + e.what());
    }

    DataSetInfo info;
    info.id = "data";
    info.dimensions = get_dimensions(header);
    info.attributes = get_attributes(header);
    return info;
}

}
