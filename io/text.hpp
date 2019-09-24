#pragma once

#include "../data/data_source.hpp"

#include <memory>
#include <vector>
#include <string>

namespace datavis {

using std::string;
using std::vector;

class DataLibrary;

class TextSource : public DataSource
{
public:
    TextSource(const string & file_path, DataLibrary *);

    string path() const override { return m_file_path; }
    string id() const override { return m_name; }

    virtual int count() const override { return 0; }
    virtual vector<string> dataset_ids() const override { return {}; }
    DataSetInfo dataset_info(const string & id) const { return {}; }
    virtual FutureDataset dataset(const string & id) override { return nullptr; }


    //int count() const override { return 1; }
    int index(const string & id) const { return 0; }
    DataSetInfo info(int index) const;
    DataSetPtr dataset(int index);

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

    virtual int count() const override { return 0; }
    virtual vector<string> dataset_ids() const override { return {}; }
    DataSetInfo dataset_info(const string & id) const { return {}; }
    virtual FutureDataset dataset(const string & id) override { return nullptr; }


    //int count() const override { return m_members.size(); }
    int index(const string & id) const;
    DataSetInfo info(int index) const;
    DataSetPtr dataset(int index);

    struct Member
    {
        string path;
        TextSourceParser::Format format;
        DataSetInfo info;
        DataSetPtr dataset = nullptr;
    };

private:
    void parseDescriptor();
    void loadDataSet(int index);

    string m_dir_path;
    string m_file_path;
    string m_name;
    vector<Member> m_members;
};

}

