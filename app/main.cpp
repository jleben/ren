#include "main_window.hpp"

#include <QApplication>
#include <iostream>

using namespace std;
using namespace datavis;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto main_win = new MainWindow;

    auto args = app.arguments();

    if (args.size() > 1)
    {
        auto file_path = args[1];
        cout << "Opening file: " << file_path.toStdString() << endl;
        main_win->openDataFile(file_path);
    }

    main_win->resize(800,600);
    main_win->show();

    return app.exec();
}
