#include <QObject>
#include <QEvent>
#include <QCoreApplication>

#include <memory>
#include <mutex>
#include <unordered_set>
#include <vector>
#include <functional>
#include <atomic>

namespace Reactive {

template <typename QObjectType>
struct QObject_Pointer : public std::shared_ptr<QObjectType>
{
    struct Deleter
    {
        void operator() (QObjectType * object) { object->deleteLater(); }
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

#if 0
class Item
{
public:
    virtual ~Item() {}
};

using ItemPtr = std::shared_pointer<Item>;
#endif

template <typename T>
class TypedItem : public QEvent
{
public:
    TypedItem(): QEvent(QEvent::User) {}
    TypedItem(T v): QEvent(QEvent::User), data(v) {}
    T data;
};

using Worker_Pointer = QObject_Pointer<QObject>;

template <typename T>
struct Value_Data
{
    std::mutex mutex;
    Worker_Pointer worker;
    std::vector<std::weak_ptr<QObject>> subscribers;
    bool ready = false;
    T value;
};

template <>
struct Value_Data<void>
{
    Worker_Pointer worker;
    std::atomic<bool> done { false };
};

template <typename T>
using Value = std::shared_ptr<Value_Data<T>>;

template <typename R, typename A>
class Function_Worker : public QObject
{
    bool event(QEvent * event) override
    {
        if (event->type() != QEvent::User)
            return QObject::event(event);

        auto item = static_cast<TypedItem<A>*>(event);
        printf("Item data = %d\n", item->data);
        auto r = fn(item->data);
        printf("Function result = %d\n", r);

        std::vector<std::weak_ptr<QObject>> subscribers;

        auto real_result = result.lock();
        if (real_result)
        {
            std::lock_guard<std::mutex> lock(real_result->mutex);
            real_result->value = r;
            real_result->ready = true;
            subscribers = real_result->subscribers;
        }

        printf("Result stored\n");

        for (auto & potential_subscriber : subscribers)
        {
            auto subscriber = potential_subscriber.lock();
            if (!subscriber)
                continue;
            auto item = new TypedItem<R>(r);
            QCoreApplication::postEvent(subscriber.get(), item);
        }

        printf("Worker done\n");

        return true;
    }

public:
    std::function<R(A)> fn;
    std::shared_ptr<Value_Data<A>> arg;
    std::weak_ptr<Value_Data<R>> result;
};

template <typename A>
class Function_Worker<void,A> : public QObject
{
    bool event(QEvent * event) override
    {
        if (event->type() != QEvent::User)
            return QObject::event(event);

        auto item = static_cast<TypedItem<A>*>(event);
        printf("Item data = %d\n", item->data);
        fn(item->data);
        printf("Function done.\n");

        auto real_result = result.lock();
        if (real_result)
            real_result->done = true;

        return true;
    }

public:
    std::function<void(A)> fn;
    std::shared_ptr<Value_Data<A>> arg;
    std::weak_ptr<Value_Data<void>> result;
};

template <typename A>
void subscribe(Value<A> arg, Worker_Pointer worker)
{
    std::lock_guard<std::mutex> lock(arg->mutex);

    if (arg->ready)
    {
        printf("Immediately posting function arg: %d\n", arg->value);
        auto item = new TypedItem<A>(arg->value);
        QCoreApplication::postEvent(worker.get(), item);
    }
    else
    {
        printf("Subscribing to function arg.\n");
        arg->subscribers.push_back(worker);
    }
}

template <typename R, typename A> inline
Value<R> apply(Value<A> arg, std::function<R(A)> fn, QThread * thread = nullptr)
{
    auto result = std::make_shared<Value_Data<R>>();

    using Worker = Function_Worker<R,A>;

    auto worker = QObject_Pointer<Worker>(new Worker);
    worker->fn = fn;
    worker->result = result;
    worker->arg = arg; // FIXME: Discard arg when worker is done.

    // This makes worker's lifetime depend on
    // the lifetime of it's result.
    result->worker = worker;

    if (thread)
        worker->moveToThread(thread);

    subscribe(arg, worker);

    printf("Apply done.\n");

    return result;
}

template <typename T> inline
Value<T> value(T v)
{
    auto result = std::make_shared<Value_Data<T>>();
    result->ready = true;
    result->value = v;
    return result;
}

// Wait for Value to be ready and return it's content.
template <typename T> inline
T get(Value<T>);

}
