#pragma once

#include <QWidget>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>

#include "../data/array.hpp"
#include "../data/data_source.hpp"
#include "../json/json.hpp"

#include <list>

namespace datavis {

using std::list;
using nlohmann::json;

class DataSet;
class PlotView;
class PlotGridView;
class Plot;
class LinePlot;
class DataLibrary;
class DataLibraryView;
class PlotDataSettingsView;

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget * parent = 0);

    void openFile(const QString & path);

    void openData();
    void openDataFile(const QString & file_path);

    void saveProject();
    void saveProjectAs();
    void saveProjectFile(const QString & path);
    void openProject();
    void openProjectFile(const QString & file_path);
    bool closeProject();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void closeEvent(QCloseEvent *event);

private:
    void makeMenu();
    void onOpenFailed(const QString & path, const QString & reason);
    void onSelectedDataChanged();
    bool hasSelectedObject();
    void plotSelectedObject();
    PlotGridView * addPlotView();
    void removePlotView(PlotGridView*);
    void plot(DataSource *, const string & id);
    Plot * makePlot(DataSource *, const string & datasetId);

    void restorePlot(PlotGridView *, const json & state);
    void removeSelectedPlot();
    void onDatasetDroppedOnPlotView(const QVariant &);
    bool eventFilter(QObject*, QEvent*) override;
    void showPlotContextMenu(Plot*, const QPoint & pos);

    QString m_project_file_path;

    DataLibrary * m_lib = nullptr;
    DataLibraryView * m_lib_view = nullptr;

    list<PlotGridView*> m_plot_views;

    PlotGridView * m_selected_plot_view = nullptr;
    Plot * m_selected_plot = nullptr;

    QMenu * m_plot_context_menu = nullptr;
};

}
