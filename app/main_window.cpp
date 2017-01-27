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
#include <QContextMenuEvent>
#include <QMenu>

namespace datavis {

MainWindow::MainWindow(QWidget * parent):
    QMainWindow(parent)
{
    m_lib = new DataLibrary(this);

    m_lib_view = new DataLibraryView;
    m_lib_view->setLibrary(m_lib);

    m_plot_view = new PlotView;
    m_plot_view->installEventFilter(this);

    m_settings_view = new SettingsView;

    auto lib_action_bar = new QToolBar;
    {
        auto action = lib_action_bar->addAction("Plot");
        connect(action, &QAction::triggered,
                this, &MainWindow::plotSelectedObject);
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

bool MainWindow::hasSelectedObject()
{
    auto object_idx = m_lib_view->selectedObjectIndex();
    return object_idx >= 0;
}

void MainWindow::plotSelectedObject()
{
    auto source = m_lib_view->selectedSource();
    if (!source)
        return;

    auto object_idx = m_lib_view->selectedObjectIndex();

    if (object_idx < 0)
    {
        for (object_idx = 0; object_idx < source->objectCount(); ++object_idx)
        {
            plot(source, object_idx);
        }
        return;
    }
    else
    {
        plot(source, object_idx);
    }
}

void MainWindow::plot(DataSource * source, int index)
{
    auto info = source->objectInfo(index);

    bool is_line = false;

    if (info.dimensionCount() == 1)
    {
        is_line = true;
    }
    else if (info.dimensionCount() == 2)
    {
        is_line = false;
    }
    else
    {
        auto msg = QString("The object %1 has more than 2 dimensions.\n"
                           "Plotting is not supported.").arg(info.id.c_str());

        QMessageBox::warning(this, "Not Implemented", msg);

        return;
    }

    DataObject * data;
    try {
        data = source->object(index);
    } catch (...) {
        QMessageBox::warning(this, "Read Failed",
                             QString("Failed to read data for object %1.").arg(info.id.c_str()));
        return;
    }

    Plot * plot;
    if (is_line)
    {
        auto line = new LinePlot;
        line->setDataObject(data);
        plot = line;
    }
    else
    {
        auto map = new HeatMap;
        map->setDataObject(data);
        plot = map;
    }

    m_data_objects.push_back(data);
    m_plots.push_back(plot);

    m_plot_view->addPlot(plot);
}

void MainWindow::customPlotSelectedObject()
{

}

void MainWindow::removeSelectedPlot()
{
    if (!m_selected_plot)
        return;

    int index = 0;
    for (auto & plot : m_plots)
    {
        if (plot == m_selected_plot)
            break;
        ++index;
    }

    if (index >= (int) m_plots.size())
    {
        cerr << "Warning: Could not find selected plot index." << endl;
        return;
    }

    m_selected_plot = nullptr;

    auto plot = m_plots[index];
    auto data = m_data_objects[index];

    m_plot_view->removePlot(plot);
    delete plot;
    m_plots.erase(m_plots.begin() + index);

    delete data;
    m_data_objects.erase(m_data_objects.begin() + index);
}

void MainWindow::showPlotContextMenu(Plot * plot, const QPoint & pos)
{
    if (!plot)
        return;

    m_selected_plot = plot;

    if (!m_plot_context_menu)
    {
        auto menu = m_plot_context_menu = new QMenu(this);
        {
            auto action = menu->addAction("Remove");
            connect(action, &QAction::triggered,
                    this, &MainWindow::removeSelectedPlot);
        }
    }

    m_plot_context_menu->popup(pos);
}

bool MainWindow::eventFilter(QObject * object, QEvent * event)
{
    if (object == m_plot_view)
    {
        switch(event->type())
        {
        case QEvent::ContextMenu:
        {
            auto menuEvent = static_cast<QContextMenuEvent*>(event);
            if (menuEvent->reason() == QContextMenuEvent::Mouse)
            {
                auto plot = m_plot_view->plotAt(menuEvent->pos());
                if (plot)
                    showPlotContextMenu(plot, menuEvent->globalPos());
            }
            event->accept();
            return true;
        }
        default:;
        }
    }

    return false;
}

}
