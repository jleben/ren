#pragma once

#include <QObject>
#include <QEvent>
#include <QCoreApplication>

#include <memory>
#include <mutex>
#include <unordered_set>
#include <vector>
#include <functional>
#include <atomic>
#include <tuple>
#include <utility>

namespace Reactive {

template <typename F>
inline
auto for_each(F)
{}

template <typename F, typename A>
inline
auto for_each(F fn, A arg)
{
    fn(arg);
}

template <typename F, typename A, typename ...As>
inline
auto for_each(F fn, A arg, As ... args)
{
    fn(arg);
    for_each(fn, args...);
}

template <typename QObjectType>
struct QObject_Pointer : public std::shared_ptr<QObjectType>
{
    struct Deleter
    {
        void operator() (QObjectType * object)
        {
            QCoreApplication::removePostedEvents(object, QEvent::User);
            object->deleteLater();
        }
    };

    QObject_Pointer() {}
    template<typename U>
    QObject_Pointer(const QObject_Pointer<U> & u): std::shared_ptr<QObjectType>(u) {}
    QObject_Pointer(QObjectType* object): std::shared_ptr<QObjectType>(object, Deleter()) {}
};

}

namespace std
{
    template <typename T> struct hash<Reactive::QObject_Pointer<T>>
        : public std::hash<std::shared_ptr<QObject>>
    {};
}


namespace Reactive {

template <typename T>
class ReadyArg : public QEvent
{
public:
    ReadyArg(): QEvent(QEvent::User) {}
    ReadyArg(T v): QEvent(QEvent::User), value(v) {}
    T value;
};

struct Worker : public QObject
{
    virtual void cancel() = 0;
};

using Worker_Pointer = QObject_Pointer<Worker>;

template <typename T>
struct Value_Data
{
    ~Value_Data()
    {
        if (worker)
            worker->cancel();
    }

    std::mutex mutex;
    Worker_Pointer worker;
    std::vector<std::weak_ptr<QObject>> subscribers;
    std::atomic<bool> ready { false };
    T value;
};

template <>
struct Value_Data<void>
{
    ~Value_Data()
    {
        if (worker)
            worker->cancel();
    }

    Worker_Pointer worker;
    std::atomic<bool> done { false };
};

template <typename T>
using Value = std::shared_ptr<Value_Data<T>>;

struct Status
{
    std::atomic<bool> cancelled;
};

template <typename ...A>
struct Function_Worker_Base : public Worker
{
    bool allArgsReady() const
    {
        bool ready = true;

        std::apply([&](Value<A> ... arg)
        {
            std::vector<bool> states = { arg->ready.load() ... };
            for (bool v : states)
                ready &= v;
        },
        args);

        return ready;
    }

    void cancel() override { status.cancelled = true; }

    std::tuple<Value<A>...> args;
    Status status;
};

template <typename R, typename ... A>
class Function_Worker : public Function_Worker_Base<A...>
{
    bool event(QEvent * event) override
    {
        if (event->type() != QEvent::User)
            return QObject::event(event);

        bool ready = Function_Worker_Base<A...>::allArgsReady();
        //printf("Worker: All args ready: %d\n", ready);

        if (!ready)
            return true;

        auto r = std::apply([this](Value<A> ... arg)
        {
                return fn(this->status, arg->value...);
        },
        Function_Worker_Base<A...>::args);

        //printf("Worker: Function done. Result = %d\n", r);

        std::vector<std::weak_ptr<QObject>> subscribers;

        auto real_result = result.lock();
        if (real_result)
        {
            std::lock_guard<std::mutex> lock(real_result->mutex);
            real_result->value = r;
            real_result->ready = true;
            subscribers = real_result->subscribers;
        }

        //printf("Worker: Result stored\n");

        for (auto & potential_subscriber : subscribers)
        {
            auto subscriber = potential_subscriber.lock();
            if (!subscriber)
                continue;
            auto item = new ReadyArg<R>(r);
            QCoreApplication::postEvent(subscriber.get(), item);
            //printf("Worker: Subscriber notified.\n");
        }

        //printf("Worker: Done\n");

        return true;
    }

public:
    std::function<R(Status&, A...)> fn;
    std::weak_ptr<Value_Data<R>> result;
};

template <typename ... A>
class Function_Worker<void,A...> : public Function_Worker_Base<A...>
{
    bool event(QEvent * event) override
    {
        if (event->type() != QEvent::User)
            return QObject::event(event);

        bool ready = Function_Worker_Base<A...>::allArgsReady();
        //printf("Ready: %d\n", ready);

        if (!ready)
            return true;

        std::apply([this](Value<A> ... arg)
        {
                return fn(this->status, arg->value...);
        },
        Function_Worker_Base<A...>::args);

        //printf("Function done.\n");

        auto real_result = result.lock();
        if (real_result)
            real_result->done = true;

        return true;
    }

public:
    std::function<void(Status&, A...)> fn;
    std::weak_ptr<Value_Data<void>> result;
};

template <typename A>
void subscribe(Value<A> arg, Worker_Pointer worker)
{
    std::lock_guard<std::mutex> lock(arg->mutex);

    if (arg->ready)
    {
        //printf("Immediately posting function arg: %d\n", arg->value);
        auto item = new ReadyArg<A>(arg->value);
        QCoreApplication::postEvent(worker.get(), item);
    }
    else
    {
        //printf("Subscribing to function arg.\n");
        arg->subscribers.push_back(worker);
    }
}

template <typename F, typename ... A> inline
auto apply(QThread * thread, F fn, Value<A> ...arg)
-> Value<typename std::result_of<F(Status&,A...)>::type>
{
    using R = typename std::result_of<F(Status&,A...)>::type;

    auto result = std::make_shared<Value_Data<R>>();

    using Worker = Function_Worker<R,A...>;

    auto worker = QObject_Pointer<Worker>(new Worker);
    worker->fn = fn;
    worker->result = result;
     // FIXME: Discard arg when worker is done.
    worker->args = std::make_tuple(arg...);

    // This makes worker's lifetime depend on
    // the lifetime of it's result.
    result->worker = worker;

    if (thread)
        worker->moveToThread(thread);

    if (sizeof...(arg))
    {
        for_each([&](auto arg){ subscribe(arg, worker); }, arg...);
    }
    else
    {
        //printf("Worker has no arg. Scheduling immediately.\n");
        QCoreApplication::postEvent(worker.get(), new QEvent(QEvent::User));
    }

    //printf("Apply done.\n");

    return result;
}

template <typename F, typename ... A> inline
auto apply(F fn, Value<A> ...arg) -> decltype(apply((QThread*)nullptr, fn, arg...))
{
    return apply((QThread*)nullptr, fn, arg...);
}

template <typename T> inline
Value<T> value(T v)
{
    auto result = std::make_shared<Value_Data<T>>();
    result->ready = true;
    result->value = v;
    return result;
}

}
