#include "main_window.hpp"
#include "../utility/threads.hpp"

#include <QApplication>
#include <iostream>

using namespace std;
using namespace datavis;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto args = app.arguments();

    QString file_path;

    if (args.size() > 1)
    {
        file_path = args[1];
    }

    background_thread()->start();

    auto main_win = new MainWindow;

    {
        auto fm = main_win->fontMetrics();
        main_win->resize(fm.averageCharWidth() * 50, fm.height() * 30);
        main_win->move(50,50);
    }

    if (!file_path.isEmpty())
    {
        main_win->openFile(file_path);
    }

    main_win->show();

    int status = app.exec();

    delete main_win;

    background_thread()->quit();
    background_thread()->wait();

    return status;
}
