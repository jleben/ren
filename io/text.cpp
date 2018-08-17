#include "text.hpp"
#include "../utility/error.hpp"
#include "../json/json.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

namespace datavis {

string TextSourceParser::m_possible_delimiters = " \t;:|/\\";

bool TextSourceParser::isPossibleDelimiter(char c)
{
    return m_possible_delimiters.find(c) != string::npos;
}

TextSourceParser::Format TextSourceParser::inferFormat(const string &line)
{
    Format format;

    if (line.empty())
        throw Error("Empty line.");

    if (line[0] == '"' || line[0] == '\'')
    {
        format.quoted = true;
        format.quote_mark = line[0];
    }

    int index = 0;

    string::size_type pos = 0;
    while(pos < line.size())
    {
        ++index;

        // Skip a field (all characters until a possible delimiter)

        if (format.quoted)
        {
            if (line[pos] != format.quote_mark)
                throw Error("Field without opening quotation mark.");
            pos = line.find(format.quote_mark, pos+1);
            if (pos == string::npos)
                throw Error("Field without closing quotation mark.");
            ++pos;
            if (pos < line.size())
            {
                if (index == 1)
                {
                        if (!isPossibleDelimiter(line[pos]))
                            throw Error("Quoted field is not followed by a delimiter.");
                        format.delimiter = line[pos];
                 }
                else
                {
                    if (line[pos] != format.delimiter)
                        throw Error("Quoted field is not followed by the delimiter.");
                }
                ++pos;
            }
        }
        else
        {
            if (index == 1)
            {
                for(; pos < line.size(); ++pos)
                {
                    if (isPossibleDelimiter(line[pos]))
                    {
                        format.delimiter = line[pos];
                        ++pos;
                        break;
                    }
                }
            }
            else
            {
                pos = line.find(format.delimiter, pos);
                if (pos != string::npos)
                    ++pos;
            }
        }
    }

    format.count = index;

    return format;
}

TextSourceParser::TextSourceParser(const Format & format):
    m_format(format)
{
#if 0
    cerr << "Created parser with format: " << endl
         << "- Quoted: " << m_format.quoted << endl
         << "- Quote mark: '" << m_format.quote_mark << "'" << endl
         << "- Delimiter: '" << m_format.delimiter << "'" << endl
         << "- Count: " << m_format.count << endl;
#endif
}

vector<string> TextSourceParser::parse(const string & line)
{
    vector<string> fields;

    if (line.empty())
        throw Error("Empty line.");

    string::size_type pos = 0;
    int index = 0;

    if (m_format.quoted)
    {
        while (index < m_format.count && pos < line.size())
        {
            if (index > 0)
            {
                if (line[pos] != m_format.delimiter)
                    throw Error("Missing delimiter after field.");
                ++pos;
                if (pos >= line.size())
                    throw Error("Missing field after delimiter.");
            }

            if (line[pos] != m_format.quote_mark)
                throw Error("Field without opening quotation mark.");

            ++pos;

            string::size_type start = pos;

            pos = line.find(m_format.quote_mark, pos);
            if (pos == string::npos)
                throw Error("Field without closing quotation mark.");

            string::size_type size = pos - start;

            auto field = line.substr(start, size);
            fields.push_back(field);

            ++pos;
            ++index;
        }
    }
    else
    {
        while (index < m_format.count && pos < line.size())
        {
            string::size_type start = pos;
            pos = line.find(m_format.delimiter, pos);

            string field;
            if (pos == string::npos)
                field = line.substr(start);
            else
                field = line.substr(start, pos - start);

            fields.push_back(field);

            if (pos != string::npos)
                ++pos;

           ++index;
        }
    }

    if (index < m_format.count)
        throw Error("Too few fields.");
    if (pos < line.size())
        throw Error("Unexpected data at end of line.");

    return fields;
}

TextSource::TextSource(const string & file_path, DataLibrary * lib):
    DataSource(lib),
    m_file_path(file_path)
{}

DataSetInfo TextSource::info(int) const
{
    return inferInfo();
}

DataSetInfo TextSource::inferInfo() const
{
    DataSetInfo info;
    info.id = "data";

    ifstream file(m_file_path);
    if (!file.is_open())
        throw Error("Failed to open file.");

    string first_line;
    std::getline(file, first_line);
    if (!file)
        throw Error("Failed to read a line.");

    auto format = TextSourceParser::inferFormat(first_line);
    if (format.count < 1)
        throw Error("No fields.");

    info.attributes.resize(format.count);

    TextSourceParser parser(format);
    auto fields = parser.parse(first_line);

    bool has_field_names = false;
    if (std::isalpha(fields.front()[0]))
    {
        has_field_names = true;
        for (int i = 0; i < format.count; ++i)
            info.attributes[i].name = fields[i];
    }

    {
        int record_count = has_field_names ? 0 : 1;
        string line;
        while(std::getline(file, line))
            ++record_count;

        info.dimensions.resize(1);
        info.dimensions.front().size = record_count;
    }

    return info;
}

DataSetPtr TextSource::dataset(int)
{
    if (!m_dataset)
        m_dataset = getData();

    return m_dataset;
}

DataSetPtr TextSource::getData()
{
    vector<string> lines;

    {
        ifstream file(m_file_path);
        if (!file.is_open())
            throw Error("Failed to open file.");

        string line;
        while(std::getline(file, line))
            lines.push_back(line);
    }

    if (lines.empty())
        throw Error("No lines.");

    auto format = TextSourceParser::inferFormat(lines.front());
    if (format.count < 1)
        throw Error("No fields.");

    TextSourceParser parser(format);

    auto fields = parser.parse(lines.front());
    bool has_field_names = std::isalpha(fields.front()[0]);

    size_t record_count = lines.size();
    if (has_field_names) record_count -= 1;

    vector<int> data_size { int(record_count) };
    auto dataset = make_shared<DataSet>("data", data_size, format.count);

    if (has_field_names)
    {
        for (int i = 0; i < format.count; ++i)
            dataset->attribute(i).name = fields[i];
    }

    DataSet::Dimension dim;
    dim.size = record_count;
    dataset->setDimension(0, dim);

    int line_index = has_field_names ? 1 : 0;
    for (size_t record_index = 0; record_index < record_count; ++record_index, ++line_index)
    {
        auto fields = parser.parse(lines[line_index]);
        for (int i = 0; i < format.count; ++i)
        {
            double value = stof(fields[i]);
            dataset->data(i).data()[record_index] = value;
        }
    }

    return dataset;
}

TextPackageSource::TextPackageSource(const string & path, DataLibrary * lib):
    DataSource(lib),
    m_dir_path(path),
    // FIXME:
    m_file_path(path + "/datapackage.json")
{
    parseDescriptor();
}

int TextPackageSource::index(const string & id) const
{
    for(int i = 0; i < m_members.size(); ++i)
    {
        if (m_members[i].info.id == id)
            return i;
    }

    return -1;
}

DataSetInfo TextPackageSource::info(int index) const
{
    return m_members[index].info;
}

DataSetPtr TextPackageSource::dataset(int index)
{
    if (!m_members[index].dataset)
        loadDataSet(index);

    return m_members[index].dataset;
}

using nlohmann::json;

static void parseResourceDescription(TextPackageSource::Member & member, const json & data)
{
    string path = data.at("path");

    member.path = path;

    member.info.id = path;

    if (data.find("format") != data.end())
    {
        auto format_data = data["format"];

        member.format.quoted = format_data.value("quoted", false);

        string quote_mark = format_data.value("quote_mark", "'");
        if (quote_mark.size() != 1)
            throw Error("Quote mark is not a single character.");
        member.format.quote_mark = quote_mark[0];

        string delimiter = format_data.at("delimiter");
        if (delimiter.size() != 1)
            throw Error("Delimiter is not a single character.");

        member.format.delimiter = delimiter[0];
    }

    if (data.find("fields") != data.end())
    {
        for (const auto & field : data["fields"])
        {
            DataSet::Attribute attribute;
            attribute.name = field.value("name", "");
            member.info.attributes.push_back(attribute);
        }
    }
    else
    {
        member.info.attributes.resize(1);
    }

    member.format.count = member.info.attributes.size();

    for (const auto & dimension_data : data.at("dimensions"))
    {
        DataSet::Dimension dimension;

        dimension.name = dimension_data.value("name", "");
        dimension.size = dimension_data.at("size");
        dimension.map.scale = dimension_data.value("scale", 1);
        dimension.map.offset = dimension_data.value("offset", 0);

        member.info.dimensions.push_back(dimension);
    }

    if (member.info.dimensions.empty())
    {
        throw Error("Descriptor does not declare any dimensions.");
    }
}

#if 0
static void inferResource(TextPackageSource::Member & member, const string & base_path)
{
    // FIXME:
    string path = base_path + '/' + member.path;

    ifstream file(path);
    if (!file.is_open())
        throw Error("Failed to open descriptor file.");

    string first_line;
    std::getline(file, first_line);
    if (!file)
        throw Error("Failed to read a line.");

    if (member.format.delimiter == 0)
    {
        auto format = TextSourceParser::inferFormat(first_line);
        member.format = format;
        if (member.info.attributes.empty())
            member.info.attributes.resize(format.count);
        else if (member.info.attributes.size() != format.count)
            throw Error("Number of attributes does not match number of fields.");
    }
    else
    {
        if (member.info.attributes.empty())
        {
            member.info.attributes.resize(1);
            member.format.count = 1;
        }
    }

    if (member.info.dimensions.empty())
    {
        int line_count = 1;

        string line;
        while(std::getline(file, line))
            ++line_count;

        DataSet::Dimension dim;
        dim.size = line_count;
        member.info.dimensions.push_back(dim);
    }
}
#endif

void TextPackageSource::parseDescriptor()
{
    using nlohmann::json;

    ifstream file(m_file_path);
    if (!file.is_open())
        throw Error("Failed to open descriptor file.");

    json data;

    try
    {
        file >> data;

        auto resources = data.at("resources");

        m_members.reserve(resources.size());

        for(auto resource : resources)
        {
            m_members.emplace_back();
            parseResourceDescription(m_members.back(), resource);
        }
    }
    catch (json::exception & e)
    {
        throw Error(string("Failed to parse descriptor: ") + e.what());
    }
}

void TextPackageSource::loadDataSet(int index)
{
    Member & member = m_members[index];

    // FIXME:
    string path = m_dir_path + '/' + member.path;

    vector<string> lines;

    // Read lines from file
    {
        ifstream file(path);
        if (!file.is_open())
            throw Error("Failed to open resource file: " + path);

        string line;
        while(std::getline(file, line))
            lines.push_back(line);
    }

    // Confirm total size of dataset
    {
        size_t total_size = 1;
        for (const auto & dim : member.info.dimensions)
        {
            total_size *= dim.size;
        }

        if (lines.size() != total_size)
        {
            throw Error("Number of records does not match data space size.");
        }
    }

    // Create DataSet

    vector<int> data_size;
    for (int i = 0; i < member.info.dimensions.size(); ++i)
    {
        data_size.push_back(member.info.dimensions[i].size);
    }

    auto dataset = make_shared<DataSet>(member.path, data_size, member.info.attributes.size());

    for (int i = 0; i < member.info.attributes.size(); ++i)
    {
        dataset->attribute(i) = member.info.attributes[i];
    }
    for (int i = 0; i < member.info.dimensions.size(); ++i)
    {
        dataset->setDimension(i, member.info.dimensions[i]);
    }

    // Fill DataSet with data

    TextSourceParser parser(member.format);

    for (size_t line_index = 0; line_index < lines.size(); ++line_index)
    {
        auto fields = parser.parse(lines[line_index]);
        for (int i = 0; i < fields.size(); ++i)
        {
            double value = stof(fields[i]);
            dataset->data(i).data()[line_index] = value;
        }
    }

    member.dataset = dataset;
}

}

