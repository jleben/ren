#pragma once

#include <QWidget>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>

#include "../data/array.hpp"

#include <list>

namespace datavis {

using std::list;

class DataObject;
class PlotView;
class Plot;
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
    DataObject * loadSelectedObject();
    void addLinePlotForSelectedObject();
    void addHeatPlotForSelectedObject();
    void removeSelectedPlot();
    bool eventFilter(QObject*, QEvent*) override;
    void showPlotContextMenu(Plot*, const QPoint & pos);

    DataLibrary * m_lib = nullptr;
    DataLibraryView * m_lib_view = nullptr;

    vector<DataObject*> m_data_objects;
    vector<Plot*> m_plots;
    Plot * m_selected_plot = nullptr;

    SettingsView * m_settings_view = nullptr;
    PlotView * m_plot_view = nullptr;

    QMenu * m_plot_context_menu = nullptr;
};

}
