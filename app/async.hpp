#pragma once

#include <QThread>
#include <QThreadPool>
#include <QRunnable>

#include <atomic>
#include <functional>
#include <memory>

namespace datavis {

class AsyncStatus : public QObject
{
    Q_OBJECT

public slots:
    float progress() const
    {
        return d_progress;
    }

    void setProgress(float value)
    {
        d_progress = value;
        emit progressChanged(value);
    }

    void cancel()
    {
        d_cancelled = true;
    }

    bool isCancelled() const
    {
        return d_cancelled;
    }

    void setDone()
    {
        d_done = true;
        emit done();
    }

    bool isDone() const
    {
        return d_done;
    }

signals:
    void progressChanged(float);
    void cancelled();
    void done();

private:
    std::atomic<bool> d_done { false };
    std::atomic<bool> d_cancelled { false };
    std::atomic<float> d_progress { 0 };
};

template <typename T>
class Async
{

public:
    using WorkFn = std::function<T(AsyncStatus*)>;

    Async(WorkFn fn)
    {
        d_shared_data = std::make_shared<SharedData>();
        d_worker = new Worker(fn, d_shared_data);
    }

    ~Async()
    {
        status()->cancel();
    }

    // Not thread-safe
    void start()
    {
        // NOTE: Prevent double start by setting worker to null after starting

        if (!d_worker) return;

        QThreadPool::globalInstance()->start(d_worker);

        d_worker = nullptr;
    }

    // Thread-safe
    AsyncStatus * status() { return &d_shared_data->status; }

    // Thread-safe only if status()->isDone()
    const T & value() const { return d_shared_data->result; }

    bool isReady() const { return d_shared_data->status.isDone(); }

private:
    struct SharedData
    {
        AsyncStatus status;
        T result;
    };

    using SharedDataPtr = std::shared_ptr<SharedData>;

    class Worker : public QRunnable
    {
    public:
        Worker(WorkFn fn, SharedDataPtr data):
            d_fn(fn),
            d_shared_data(data)
        {}

        void run() override
        {
            d_shared_data->result = d_fn(&d_shared_data->status);
            d_shared_data->status.setDone();
        }

    private:
        WorkFn d_fn;
        SharedDataPtr d_shared_data;
    };

    SharedDataPtr d_shared_data;
    Worker * d_worker = nullptr;
};

}
