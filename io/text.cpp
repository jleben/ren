#include "text.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

namespace datavis {

static int find_column_count(const string & line)
{
    istringstream stream(line);
    double value;
    int count = 0;
    while(!stream.eof() && (stream >> value))
    {
        cout << value << endl;
        ++count;
    }
    if (stream.fail())
        throw std::runtime_error("Invalid format.");

    return count;
}

TextSource::TextSource(const string & file_path, DataLibrary * lib):
    DataSource(lib),
    m_file_path(file_path)
{
    vector<string> lines;

    {
        ifstream file(file_path);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file.");

        string line;
        while(std::getline(file, line))
            lines.push_back(line);
    }

    int row_count = lines.size();
    if (row_count < 0)
        throw std::runtime_error("No data.");

    int column_count = find_column_count(lines[0]);

    cout << "Got column count: " << column_count << endl;

    vector<int> data_size = { row_count };
    auto dataset = make_shared<DataSet>("data", data_size, column_count );

    DataSet::Dimension dim;
    dim.size = row_count;
    dataset->setDimension(0, dim);

    for (int col = 0; col < column_count; ++col)
    {
        dataset->attribute(col).name = "attribute " + to_string(col+1);
    }
#if 1
    for (int row = 0; row < row_count; ++row)
    {
        auto & line = lines[row];
        istringstream stream(line);
        double value;
        for (int col = 0; col < column_count; ++col)
        {
            if (!(stream >> value))
                throw std::runtime_error("Invalid format.");
            dataset->data(col).data()[row] = value;
        }
    }
#endif
    m_dataset = dataset;
}

DataSetInfo TextSource::info(int) const
{
    DataSetInfo info;
    info.id = "data";
    info.dimensions.push_back(m_dataset->dimension(0));
    info.attributes = m_dataset->attributes();

    return info;
}

}

