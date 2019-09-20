#include "test_async.hpp"
#include "../testing/testing.h"
#include "../app/async.hpp"

#include <QObject>

#include <chrono>
#include <thread>

using namespace Testing;
using namespace std;
using namespace datavis;

static
bool test_async()
{
    Tester tester;
    tester.run();
    return tester.success();
}

Test_Set async_tests()
{
    return {
        { "async", &test_async },
    };
}
 
