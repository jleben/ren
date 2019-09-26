#pragma once

#include "../data/data_source.hpp"

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace datavis {

using std::string;
using std::vector;
using std::unordered_map;

class DataLibrary;

class TextSource : public DataSource
{
public:
    TextSource(const string & file_path, DataLibrary *);

    string path() const override { return m_file_path; }
    string id() const override { return m_name; }

    virtual int count() const override { return 1; }
    virtual vector<string> dataset_ids() const override { return { "data" }; }
    DataSetInfo dataset_info(const string & id) const override;
    virtual FutureDataset dataset(const string & id) override;

private:
    DataSetInfo inferInfo() const;
    DataSetPtr getData();

    string m_file_path;
    string m_name;
    DataSetPtr m_dataset;
};

class TextSourceParser
{
public:
    struct Format
    {
        bool quoted = false;
        char quote_mark = '"';
        char delimiter = ' ';
        int count = 0;
    };

    static Format inferFormat(const string & line);
    static bool isPossibleDelimiter(char c);

    TextSourceParser(const Format & format);
    vector<string> parse(const string & line);

private:
    static string m_possible_delimiters;

    Format m_format;
};

class TextPackageSource : public DataSource
{
public:
    TextPackageSource(const string & path, DataLibrary *);

    string path() const override { return m_dir_path; }
    string id() const override { return m_name; }

    virtual int count() const override { return m_members.size(); }
    virtual vector<string> dataset_ids() const override;
    DataSetInfo dataset_info(const string & id) const;
    virtual FutureDataset dataset(const string & id) override;

    struct Member
    {
        string path;
        TextSourceParser::Format format;
        DataSetInfo info;
        DataSetPtr dataset = nullptr;
    };

private:
    void parseDescriptor();
    void loadDataSet(Member&);

    string m_dir_path;
    string m_file_path;
    string m_name;
    // Store IDs in vector to preserve declared order of members
    vector<string> m_member_ids;
    unordered_map<string, Member> m_members;
};

}

