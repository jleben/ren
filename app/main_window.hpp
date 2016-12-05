#pragma once

#include <QWidget>
#include <QListWidget>
#include <QMainWindow>

#include "../data/array.hpp"

#include <list>

namespace datavis {

using std::list;

class DataSource;
class PlotView;
class SettingsView;
class LinePlot;
class LinePlotSettings;
class LinePlotSettingsView;

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget * parent = 0);

    void openData();
    void openDataFile(const QString & file_path);

private:
    void makeMenu();

    DataSource * m_data_source = nullptr;
    SettingsView * m_settings_view = nullptr;
    PlotView * m_plot_view = nullptr;
    LinePlot * m_line_plot = nullptr;
    LinePlotSettings * m_line_plot_settings = nullptr;
    LinePlotSettingsView * m_line_plot_settings_view = nullptr;
};

}
