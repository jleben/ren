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
    //std::function<int(int)> f = [](int x) -> int
    std::function<int(int)> f = [](int x) -> int
    {
        this_thread::sleep_for(chrono::milliseconds(200));
        printf("x = %d\n", x); return x + 10;
    };

    auto v2 = apply(v1, f, &thread);

    atomic<int> y { 0 };

    std::function<void(int)> f2 = [&](int a)
    {
        y = a;
        app.quit();
    };

    auto v3 = apply(v2, f2);

    printf("Executing app...\n");

    app.exec();

    printf("Waiting for thread to finish...\n");

    thread.quit();
    thread.wait();

    printf("y = %d\n", y.load());

    test.assert("Result is 20.", y == 20);

    //thread.quit();


    return test.success();
}

Test_Set reactive_tests()
{
    return {
        { "test1", &test1 }
    };
}
