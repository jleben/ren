#pragma once

#include <QWidget>
#include <QListWidget>
#include <QMainWindow>

#include "../data/array.hpp"

#include <list>

namespace datavis {

using std::list;

class DataObject;
class PlotView;
class SettingsView;
class LinePlot;
class LinePlotSettingsView;
class DataLibrary;
class DataLibraryView;

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget * parent = 0);

    void openData();
    void openDataFile(const QString & file_path);

private:
    void makeMenu();
    void onOpenFailed(const QString & path);

    DataLibrary * m_lib = nullptr;
    DataLibraryView * m_lib_view = nullptr;
    DataObject * m_data_object = nullptr;
    SettingsView * m_settings_view = nullptr;
    PlotView * m_plot_view = nullptr;
    LinePlot * m_line_plot = nullptr;
    LinePlotSettingsView * m_line_plot_settings_view = nullptr;
};

}
