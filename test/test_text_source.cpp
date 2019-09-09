#include "../testing/testing.h"
#include "../io/text.hpp"

#include <QFileInfo>
#include <QDir>
#include <QDebug>

#include <sstream>

using namespace Testing;
using namespace datavis;
using namespace std;

string printSize(const vector<int> & size)
{
    ostringstream text;
    for (int i : size)
        text << ' ' << i;
    return text.str();
}

static bool test_infer_format()
{
    Test test;

    {
        string line = "aa bb cc";
        auto format = TextSourceParser::inferFormat(line);
        test.assert(format.quoted == false) << "Quoted: " << format.quoted;
        test.assert(format.delimiter == ' ') << "Delimiter: '" << format.delimiter << "'";
        test.assert(format.count == 3) << "Count: " << format.count;
    }

    return test.success();
}

static bool test_infer_format_quoted()
{
    Test test;

    {
        string line = "'aa'|'bb'|'cc'";
        auto format = TextSourceParser::inferFormat(line);
        test.assert(format.quoted == true) << "Quoted: " << format.quoted;
        test.assert(format.quote_mark == '\'') << "Quote mark: '" << format.quote_mark << "'";
        test.assert(format.delimiter == '|') << "Delimiter: '" << format.delimiter << "'";
        test.assert(format.count == 3) << "Count: " << format.count;
    }

    return test.success();
}

static bool test_parse()
{
    Test test;

    string line = "aa bb cc";

    TextSourceParser::Format format;
    format.quoted = false;
    format.delimiter = ' ';
    format.count = 3;

    TextSourceParser parser(format);
    auto tokens = parser.parse(line);

    vector<string> expected_tokens = { "aa", "bb", "cc" };
    test.assert("Tokens = <aa,bb,cc>", tokens == expected_tokens);

    return test.success();
}

static bool test_parse_quoted()
{
    Test test;

    string line = "'aa'|'bb'|'cc'";

    TextSourceParser::Format format;
    format.quoted = true;
    format.quote_mark = '\'';
    format.delimiter = '|';
    format.count = 3;

    TextSourceParser parser(format);
    auto tokens = parser.parse(line);

    vector<string> expected_tokens = { "aa", "bb", "cc" };
    test.assert("Tokens = <aa,bb,cc>", tokens == expected_tokens);

    return test.success();
}

static bool test_load_package()
{
    Test test;

    string package_path = QFileInfo(__FILE__).dir().filePath("package").toStdString();

    TextPackageSource source(package_path, nullptr);

    test.assert("Source has id 'package'.", source.id() == "package");

    test.assert("Source has 4 datasets.", source.count() == 4);

    // Test dataset 1
    {
        auto info = source.info(0);

        test.assert("Dataset 1 has name data1.txt", info.id == "data1.txt");

        test.assert("Dataset 1 has 1 dimension.", info.dimensionCount() == 1);
        test.assert("Dataset 1, dim 1 has name 'time'", info.dimensions[0].name == "time");
        test.assert("Dataset 1, dim 1 has size 5", info.dimensions[0].size == 5);
        test.assert("Dataset 1, dim 1 has scale 100", info.dimensions[0].map.scale == 100);
        test.assert("Dataset 1, dim 1 has offset 500", info.dimensions[0].map.offset == 500);

        test.assert("Dataset 1, has 2 attributes.", info.attributes.size() == 2);
        test.assert("Dataset 1, has attribute 'x'.", info.attributes[0].name == "x");
        test.assert("Dataset 1, has attribute 'y'.", info.attributes[1].name == "y");
    }

    // TODO: Test other datasets

    return test.success();
}

static bool test_load_text_file()
{
    Test test;

    auto path = QFileInfo(__FILE__).dir().filePath("package/data1.txt").toStdString();

    TextSource source(path, nullptr);

    test.assert("Source has id 'data1.txt'.", source.id() == "data1.txt");

    test.assert("Source has 1 dataset.", source.count() == 1);

    auto info = source.info(0);

    test.assert("Dataset 1 has id 'data'.", info.id == "data");

    test.assert("Dataset 1 has 1 dimension.", info.dimensionCount() == 1);
    test.assert("Dataset 1, dim 1 has no name.", info.dimensions[0].name.empty());
    test.assert("Dataset 1, dim 1 has size 5", info.dimensions[0].size == 5);
    test.assert("Dataset 1, dim 1 has scale 1", info.dimensions[0].map.scale == 1);
    test.assert("Dataset 1, dim 1 has offset 0", info.dimensions[0].map.offset == 0);

    test.assert("Dataset 1, has 2 attributes.", info.attributes.size() == 2);
    test.assert("Dataset 1, attribute 1 has no name.", info.attributes[0].name.empty());
    test.assert("Dataset 1, attribute 2 has no name.", info.attributes[1].name.empty());

    return test.success();
}

Test_Set text_source_tests()
{
    return {
        { "infer-format", &test_infer_format },
        { "infer-format-quoted", &test_infer_format_quoted },
        { "parse", &test_parse },
        { "parse-quoted", &test_parse_quoted },
        { "load-package", &test_load_package },
        { "load-file", &test_load_text_file },
    };
}
