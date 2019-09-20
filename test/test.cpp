#include "../testing/testing.h"

using Testing::Test_Set;

extern Test_Set text_source_tests();
extern Test_Set async_tests();

int main(int argc, char *argv[])
{
    Test_Set tests =
    {
        { "text-source", text_source_tests() },
        { "async", async_tests() }
    };

    return Testing::run(tests, argc, argv);
}
