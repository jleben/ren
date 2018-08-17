#include "../testing/testing.h"
#include "../io/text.hpp"

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

Test_Set text_source_tests()
{
    return {
        { "infer-format", &test_infer_format },
        { "infer-format-quoted", &test_infer_format_quoted },
        { "parse", &test_parse },
        { "parse-quoted", &test_parse_quoted },
    };
}
