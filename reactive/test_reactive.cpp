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

    auto f1 = [](Status&, int x) -> int
    {
        this_thread::sleep_for(chrono::milliseconds(200));
        printf("f1: x = %d\n", x); return x + 10;
    };

    auto v2 = Reactive::apply(&thread, f1, v1);

    auto v3 = Reactive::apply(&thread, [](Status&){ printf("f2\n"); return 5; });

    atomic<int> y { 0 };

    auto g = [&](Status&, int a, int b)
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


bool test_cancel()
{
    Test test;

    int x = 1;

    int argc = 0;
    QCoreApplication app(argc, nullptr);

    {
        Reactive::apply([&](Status & status)
        {
            x = 2;
        });
    }

    app.sendPostedEvents();

    test.assert(x == 1) << "x (" << x << ") == 1";

    return test.success();
}


// test_cancel_thread:
// Test that discarding the result sets Status::cancelled = true.
bool test_cancel_thread()
{
    Test test;

    int argc = 0;
    QCoreApplication app(argc, nullptr);


    QThread thread;
    thread.start();

    atomic<bool> thread_started { false };

    {
        auto result = Reactive::apply(&thread, [&](Status & status)
        {
            thread_started = true;
            while(!status.cancelled) { this_thread::yield(); }
            thread.quit();
        });

        // Wait until thread is started, to make sure status is observed.
        while(!thread_started)
            this_thread::yield();

        // Now discard the result.
    }

    // Thread will quit only if it observes Status::cancelled = true.
    thread.wait();

    return test.success();
}

Test_Set reactive_tests()
{
    return {
        { "test1", &test1 },
        { "cancel", &test_cancel },
        { "cancel-thread", &test_cancel_thread },
    };
}
