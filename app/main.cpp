#include "main_window.hpp"

#include <QApplication>

using namespace std;
using namespace datavis;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto main_win = new MainWindow;
    main_win->resize(800,600);
    main_win->show();

    return app.exec();
}
