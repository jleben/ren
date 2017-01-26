#include "main_window.hpp"
#include "data_library.hpp"
#include "data_library_view.hpp"
#include "settings_view.hpp"
#include "line_plot_settings_view.hpp"
#include "../plot/plot_view.hpp"
#include "../plot/line_plot.hpp"
#include "../plot/heat_map.hpp"
#include "../io/hdf5.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QToolButton>
#include <QToolBar>

namespace datavis {

MainWindow::MainWindow(QWidget * parent):
    QMainWindow(parent)
{
    m_lib = new DataLibrary(this);

    m_lib_view = new DataLibraryView;
    m_lib_view->setLibrary(m_lib);

    m_plot_view = new PlotView;

    m_settings_view = new SettingsView;

    auto lib_action_bar = new QToolBar;
    {
        auto action = lib_action_bar->addAction("Line");
        connect(action, &QAction::triggered,
                this, &MainWindow::addLinePlotForSelectedObject);
    }
    {
        auto action = lib_action_bar->addAction("Heatmap");
        connect(action, &QAction::triggered,
                this, &MainWindow::addHeatPlotForSelectedObject);
    }

    auto tool_layout = new QVBoxLayout;
    tool_layout->addWidget(m_lib_view);
    tool_layout->addWidget(lib_action_bar);
    tool_layout->addWidget(m_settings_view);

    auto content_view = new QWidget;

    auto layout = new QHBoxLayout(content_view);
    layout->addLayout(tool_layout, 0);
    layout->addWidget(m_plot_view, 1);

    setCentralWidget(content_view);

    m_line_plot_settings_view = new LinePlotSettingsView;

    m_settings_view->setPlotSettingsView(m_line_plot_settings_view);

    connect(m_lib, &DataLibrary::openFailed,
            this, &MainWindow::onOpenFailed,
            Qt::QueuedConnection);

    makeMenu();
}

void MainWindow::makeMenu()
{
    //auto menuBar = new QMenuBar();
    auto menuBar = this->menuBar();

    auto fileMenu = menuBar->addMenu("File");

    {
        auto action = fileMenu->addAction("Open Data...");
        connect(action, &QAction::triggered,
                this, &MainWindow::openData);
    }
    fileMenu->addSeparator();
    {
        auto action = fileMenu->addAction("Quit");
        action->setMenuRole(QAction::QuitRole);
        action->setShortcut(QKeySequence::Quit);
        connect(action, &QAction::triggered,
                this, &MainWindow::close);
    }
}

void MainWindow::openData()
{
    auto file = QFileDialog::getOpenFileName(this);
    if (file.isEmpty())
        return;

    openDataFile(file);
}

void MainWindow::openDataFile(const QString & file_path)
{
    m_lib->open(file_path);
}

void MainWindow::onOpenFailed(const QString & path)
{
    QMessageBox::warning(this, "Open Failed",
                         QString("Failed to open file:\n")
                         + path);
}

DataObject * MainWindow::loadSelectedObject()
{
    auto source = m_lib_view->selectedSource();
    if (!source)
        return nullptr;

    auto object_idx = m_lib_view->selectedObjectIndex();
    if (object_idx < 0)
        return nullptr;

    DataObject * object;

    try {
        object = source->object(object_idx);
    } catch (...) {
        QMessageBox::warning(this, "Read Failed",
                             QString("Failed to read data."));
        return nullptr;
    }

    return object;
}

void MainWindow::addLinePlotForSelectedObject()
{
    auto object = loadSelectedObject();

    if (!object)
        return;

    m_data_objects.push_back(object);

    auto plot = new LinePlot(this);
    plot->setDataObject(object);

    m_plots.push_back(plot);

    m_plot_view->addPlot(plot);
}

void MainWindow::addHeatPlotForSelectedObject()
{
    auto object = loadSelectedObject();

    if (!object)
        return;

    m_data_objects.push_back(object);

    auto plot = new HeatMap(this);
    plot->setDataObject(object);

    m_plots.push_back(plot);

    m_plot_view->addPlot(plot);
}

}
