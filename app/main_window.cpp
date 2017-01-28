#include "main_window.hpp"
#include "data_library.hpp"
#include "data_library_view.hpp"
#include "plot_data_settings_view.hpp"
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
#include <QDialog>
#include <QDialogButtonBox>

namespace datavis {

MainWindow::MainWindow(QWidget * parent):
    QMainWindow(parent)
{
    m_lib = new DataLibrary(this);

    m_lib_view = new DataLibraryView;
    m_lib_view->setLibrary(m_lib);

    auto lib_action_bar = new QToolBar;
    {
        auto action = lib_action_bar->addAction("Plot");
        connect(action, &QAction::triggered,
                this, &MainWindow::plotSelectedObject);
    }
    {
        auto action = lib_action_bar->addAction("Custom...");
        connect(action, &QAction::triggered,
                this, &MainWindow::customPlotSelectedObject);
    }

    lib_action_bar->addSeparator();

    {
        auto action = lib_action_bar->addAction("Add View");
        connect(action, &QAction::triggered,
                this, &MainWindow::addPlotView);
    }

    auto tool_layout = new QVBoxLayout;
    tool_layout->addWidget(m_lib_view);
    tool_layout->addWidget(lib_action_bar);

    auto content_view = new QWidget;

    auto layout = new QHBoxLayout(content_view);
    layout->addLayout(tool_layout, 0);

    setCentralWidget(content_view);

    connect(m_lib, &DataLibrary::openFailed,
            this, &MainWindow::onOpenFailed,
            Qt::QueuedConnection);

    connect(m_lib_view, &DataLibraryView::selectionChanged,
            this, &MainWindow::onSelectedDataChanged);

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

void MainWindow::onSelectedDataChanged()
{

}

bool MainWindow::hasSelectedObject()
{
    auto object_idx = m_lib_view->selectedDatasetIndex();
    return object_idx >= 0;
}

void MainWindow::plotSelectedObject()
{
    auto source = m_lib_view->selectedSource();
    if (!source)
        return;

    auto object_idx = m_lib_view->selectedDatasetIndex();

    if (object_idx < 0)
    {
        for (object_idx = 0; object_idx < source->count(); ++object_idx)
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

void MainWindow::customPlotSelectedObject()
{
    auto source = m_lib_view->selectedSource();
    if (!source)
        return;

    auto object_idx = m_lib_view->selectedDatasetIndex();
    if (object_idx < 0)
        return;

    plotCustom(source, object_idx);
}

PlotView * MainWindow::addPlotView()
{
    auto view = new PlotView;
    view->installEventFilter(this);

    m_plot_views.push_back(view);

    view->show();

    return view;
}

void MainWindow::removePlotView(PlotView * view)
{
    if (m_selected_plot_view = view)
        m_selected_plot_view = nullptr;

    m_plot_views.remove(view);
    view->removeEventFilter(this);
    view->deleteLater();
}

void MainWindow::plot(DataSource * source, int index)
{
    auto info = source->info(index);

    if (info.dimensionCount() < 1)
    {
        return;
    }
    else if (info.dimensionCount() == 1)
    {
        plot(source, index, {0});
    }
    else
    {
        plot(source, index, {0, 1});
    }
}

void MainWindow::plotCustom(DataSource * source, int index)
{
    auto info = source->info(index);

    auto dialog = new QDialog;
    dialog->setWindowTitle("Select Plot Data");

    auto settings = new PlotDataSettingsView;
    settings->setDataInfo(info);

    auto buttons = new QDialogButtonBox
            ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    auto layout = new QVBoxLayout(dialog);
    layout->addWidget(settings);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
            dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected,
            dialog, &QDialog::reject);

    bool ok = false;

    vector<int> selected_dimensions;

    while (!ok)
    {
        auto result = dialog->exec();
        if (result == QDialog::Rejected)
            return;

        selected_dimensions = settings->selectedDimensions();
        if ( selected_dimensions.size() < 1 ||
             selected_dimensions.size() > 2 )
        {
            QMessageBox::warning(this, "Invalid Selection.",
                                 "Please select at least 1 and at most 2 dimensions.");
        }
        else
        {
            ok = true;
        }
    }

    plot(source, index, selected_dimensions);
}

void MainWindow::plot(DataSource * source, int index, vector<int> dimensions)
{
    if (dimensions.size() < 1 || dimensions.size() > 2)
    {
        cerr << "Unexpected: Invalid number of dimension selected for plotting: "
             << dimensions.size() << endl;
        return;
    }

    DataSetPtr data;
    try {
        data = source->dataset(index);
    } catch (...) {
        QMessageBox::warning(this, "Read Failed",
                             QString("Failed to read data for object %1.")
                             .arg(data->id().c_str()));
        return;
    }

    Plot * plot;

    if (dimensions.size() == 1)
    {
        auto line = new LinePlot;
        line->setDataSet(data);
        line->setDimension(dimensions[0]);
        plot = line;
    }
    else
    {
        auto map = new HeatMap;
        map->setDataSet(data);
        map->setDimensions({dimensions[0], dimensions[1]});
        plot = map;
    }

    if (!m_selected_plot_view)
    {
        if(m_plot_views.empty())
        {
            m_selected_plot_view = addPlotView();
        }
        else
        {
            m_selected_plot_view = m_plot_views.front();
        }
    }

    m_selected_plot_view->addPlot(plot);

    m_selected_plot_view->show();
}

void MainWindow::removeSelectedPlot()
{
    if (!m_selected_plot)
        return;

    auto plot = m_selected_plot;
    plot->view()->removePlot(plot);
    delete plot;

    m_selected_plot = nullptr;
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
    auto plot_view = qobject_cast<PlotView*>(object);

    if (plot_view)
    {
        switch(event->type())
        {
        case QEvent::ContextMenu:
        {
            auto menuEvent = static_cast<QContextMenuEvent*>(event);
            if (menuEvent->reason() == QContextMenuEvent::Mouse)
            {
                auto plot = plot_view->plotAt(menuEvent->pos());
                if (plot)
                    showPlotContextMenu(plot, menuEvent->globalPos());
            }
            event->accept();
            return true;
        }
        case QEvent::WindowActivate:
        {
            m_selected_plot_view = plot_view;
            return false;
        }
        case QEvent::Close:
        {
            removePlotView(plot_view);
            return false;
        }
        default:;
        }
    }

    return false;
}

}
