#include "main_window.hpp"
#include "settings_view.hpp"
#include "line_plot_settings_view.hpp"
#include "../plot/plot_view.hpp"
#include "../plot/line_plot.hpp"
#include "../io/hdf5.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QDebug>

namespace datavis {

MainWindow::MainWindow(QWidget * parent):
    QMainWindow(parent)
{
    m_plot_view = new PlotView;

    m_settings_view = new SettingsView;

    auto content_view = new QWidget;

    auto layout = new QHBoxLayout(content_view);
    layout->addWidget(m_settings_view, 0);
    layout->addWidget(m_plot_view, 1);

    setCentralWidget(content_view);

    m_line_plot_settings_view = new LinePlotSettingsView;

    m_settings_view->setPlotSettingsView(m_line_plot_settings_view);

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
    m_line_plot_settings_view->setPlot(nullptr);

    m_plot_view->removePlot(m_line_plot);
    delete m_line_plot;
    m_line_plot = nullptr;

    delete m_data_object;
    m_data_object = nullptr;

    try {
        auto data = read_hdf5<double>(file_path.toStdString(), "/data");
        m_data_object = new DataObject("/data", std::move(data));
    } catch (...) {
        qCritical() << "Failed to open file: " << file_path;
    }

    m_line_plot = new LinePlot;
    m_line_plot->setDataObject(m_data_object);

    m_line_plot_settings_view->setPlot(m_line_plot);

    m_plot_view->addPlot(m_line_plot);
}



}
