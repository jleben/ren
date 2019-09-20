#include "../testing/testing.h"
#include "../app/async.hpp"

#include <QObject>
#include <QEventLoop>
#include <thread>
#include <QCoreApplication>

class Tester : public QObject, public Testing::Test
{
    Q_OBJECT

public:
    void run()
    {
        printf("Tester running...\n");

        using namespace datavis;
        using namespace std;

        int x = 1;

        Async<int> task([&](AsyncStatus * status) -> int
        {
            printf("Start.\n");

            int y = x;
            for (int i = 0; i < 3; ++i)
            {
                this_thread::sleep_for(chrono::milliseconds(100));
                printf("Work.\n");
                ++y;
                status->setProgress(y/4.f);
            }

            printf("Done.\n");

            return y;
        });

        QObject::connect(task.status(), &AsyncStatus::progressChanged,
                         this, &Tester::onProgress,
                         Qt::QueuedConnection);
        QObject::connect(task.status(), &AsyncStatus::done,
                         this, &Tester::onDone,
                         Qt::QueuedConnection);

        task.start();

        app.exec();

        assert("At end, status is done.", task.status()->isDone());
        assert("At end, status progress is 1.", task.status()->progress() == 1);
        assert("At end, reported progress is 1.", progress == 1);
        assert("At end, task value is 4.", task.value() == 4);
    }

public slots:
    void onProgress(float value)
    {
        printf("Progress %f\n", value);

        ++count;

        assert("Progress increased", value > progress);

        progress = value;
    }

    void onDone()
    {
        app.quit();
    }

private:
    int argc = 0;
    QCoreApplication app { argc, nullptr };

    int count = 0;
    float progress = 0;
};

