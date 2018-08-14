#include "main_window.hpp"

#include <QApplication>
#include <iostream>

using namespace std;
using namespace datavis;

namespace datavis {

enum File_Type
{
    Unknown_File_Type,
    Data_File_Type,
    Project_File_Type
};

}
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto args = app.arguments();

    QString file_path;
    File_Type file_type = Unknown_File_Type;

    if (args.size() > 1)
    {
        file_path = args[1];
        if (file_path.endsWith(".json"))
        {
            file_type = Project_File_Type;
        }
        else
        {
            file_type = Data_File_Type;
        }
    }

    auto main_win = new MainWindow;
    main_win->resize(400,500);
    main_win->move(50,50);

    if (!file_path.isEmpty())
    {
        switch(file_type)
        {
        case Data_File_Type:
        {
            cout << "Opening data file: " << file_path.toStdString() << endl;
            main_win->openDataFile(file_path);
            break;
        }
        case Project_File_Type:
        {
            cout << "Opening project file: " << file_path.toStdString() << endl;
            main_win->openProjectFile(file_path);
            break;
        }
        default:
        {
            cerr << "Error: Unexpected file type " << file_type << " for file: " << file_path.toStdString() << endl;
        }
        }
    }

    main_win->show();

    int status = app.exec();

    delete main_win;

    return status;
}
