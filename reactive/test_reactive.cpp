#include "reactive.hpp"
#include "../testing/testing.h"

#include <QCoreApplication>
#include <QThread>

#include <thread>
#include <chrono>

using namespace Testing;
using namespace Reactive;
using namespace std;

bool test1()
{
    Test test;

    int argc = 0;
    QCoreApplication app(argc, nullptr);

    QThread thread;
    thread.start();

    Value<int> v1 = value(10);

    auto f1 = [](int x) -> int
    {
        this_thread::sleep_for(chrono::milliseconds(200));
        printf("f1: x = %d\n", x); return x + 10;
    };

    auto v2 = Reactive::apply(&thread, f1, v1);

    auto v3 = Reactive::apply(&thread, [](){ printf("f2\n"); return 5; });

    atomic<int> y { 0 };

    auto g = [&](int a, int b)
    {
        y = a + b;
        app.quit();
    };

    auto z = Reactive::apply(g, v2, v3);

    printf("Executing app...\n");

    app.exec();

    printf("Waiting for thread to finish...\n");

    thread.quit();
    thread.wait();

    printf("y = %d\n", y.load());

    test.assert("Result is 25.", y == 25);

    return test.success();
}

Test_Set reactive_tests()
{
    return {
        { "test1", &test1 }
    };
}
