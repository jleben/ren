#include "main_window.hpp"
#include "../data/data_library.hpp"
#include "data_library_view.hpp"
#include "plot_data_settings_view.hpp"
#include "plot_settings_view2.hpp"
#include "../plot/plot_view.hpp"
#include "../plot/line_plot.hpp"
#include "../plot/heat_map.hpp"
#include "../plot/scatter_plot_1d.hpp"
#include "../plot/scatter_plot_2d.hpp"
#include "../io/hdf5.hpp"
#include "../json/json.hpp"
#include "../json/utils.hpp"
#include "../utility/error.hpp"
#include <sstream>

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
#include <QMimeData>
#include <QGuiApplication>
#include <QScreen>
#include <QShortcut>

#include <fstream>
#include <algorithm>

namespace datavis {

MainWindow::MainWindow(QWidget * parent):
    QMainWindow(parent)
{
    setWindowTitle("Ren");

    m_lib = new DataLibrary(this);

    m_lib_view = new DataLibraryView;
    m_lib_view->setLibrary(m_lib);

    auto lib_action_bar = new QToolBar;


    {
        auto action = lib_action_bar->addAction("Add Window");
        connect(action, &QAction::triggered,
                this, &MainWindow::addPlotView);
    }

    lib_action_bar->addSeparator();

    {
        auto action = lib_action_bar->addAction("Plot");
        connect(action, &QAction::triggered,
                this, &MainWindow::plotSelectedObject);
    }

    auto tool_layout = new QVBoxLayout;
    tool_layout->addWidget(lib_action_bar);
    tool_layout->addWidget(m_lib_view);

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

    setAcceptDrops(true);

    {
        auto shortcut = new QShortcut(QString("F2"), this);
        connect(shortcut, &QShortcut::activated,
                this, &QWidget::raise);
        shortcut->setContext(Qt::ApplicationShortcut);
    }
}

void MainWindow::makeMenu()
{
    //auto menuBar = new QMenuBar();
    auto menuBar = this->menuBar();

    auto fileMenu = menuBar->addMenu("File");

    {
        auto action = fileMenu->addAction("Open Project...");
        connect(action, &QAction::triggered,
                this, &MainWindow::openProject);
    }
    {
        auto action = fileMenu->addAction("Save Project");
        connect(action, &QAction::triggered,
                this, &MainWindow::saveProject);
    }
    {
        auto action = fileMenu->addAction("Save Project As...");
        connect(action, &QAction::triggered,
                this, &MainWindow::saveProjectAs);
    }
    {
        auto action = fileMenu->addAction("Close Project.");
        connect(action, &QAction::triggered,
                this, &MainWindow::closeProject);
    }
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

    {
        auto menu = menuBar->addMenu("Window");

        {
            auto action = menu->addAction("New Plot Window");
            action->setShortcut(QKeySequence::New);
            action->setShortcutContext(Qt::ApplicationShortcut);
            connect(action, &QAction::triggered,
                    this, &MainWindow::addPlotView);
        }
    }
}

void MainWindow::openFile(const QString & path)
{
    if (path.endsWith(".json"))
    {
        openProjectFile(path);
    }
    else
    {
        openDataFile(path);
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

void MainWindow::onOpenFailed(const QString & path, const QString & reason)
{
    auto message = QString("Failed to open file:\n") + path;
    if (!reason.isEmpty())
    {
        message += "\n\n";
        message += reason;
    }

    QMessageBox::warning(this, "Open Failed", message);
}

void MainWindow::onSelectedDataChanged()
{

}

bool MainWindow::hasSelectedObject()
{
    auto object_id = m_lib_view->selectedDatasetId();
    return !object_id.empty();
}

void MainWindow::plotSelectedObject()
{
    auto source = m_lib_view->selectedSource();
    if (!source)
        return;

    auto object_id = m_lib_view->selectedDatasetId();
    if (object_id.empty())
        return;

    plot(source, object_id);
}

PlotGridView * MainWindow::addPlotView()
{
    auto view = new PlotGridView;
    view->setWindowTitle("Ren - Plot View");
    view->setAcceptDrops(true);
    view->installEventFilter(this);
    connect(view, &PlotGridView::datasetDropped,
            this, &MainWindow::onDatasetDroppedOnPlotView);
    m_plot_views.push_back(view);

    auto * screen = qGuiApp->primaryScreen();

    if (screen)
    {
        auto screenRect = screen->availableGeometry();
        //int margin = std::min(screenRect.width(), screenRect.height()) * 0.15;
        int xMargin = screenRect.width() * 0.15;
        int yMargin = screenRect.height() * 0.08;
        view->setGeometry(screenRect.adjusted(xMargin, yMargin, -xMargin, -yMargin));
    }

    view->show();

    return view;
}

void MainWindow::removePlotView(PlotGridView * view)
{
    if (m_selected_plot_view == view)
        m_selected_plot_view = nullptr;

    m_plot_views.remove(view);
    view->removeEventFilter(this);
    view->deleteLater();
}

void MainWindow::onDatasetDroppedOnPlotView(const QVariant & data)
{
    auto dropped_data = data.value<PlotGridView::DroppedDataset>();

    auto source = m_lib->source(QString::fromStdString(dropped_data.source_id));
    if (!source)
        return;

    auto plot = makePlot(source, dropped_data.dataset_id);
    if (!plot)
        return;

    auto * view = dropped_data.view;

    //printf("Dropping at %d %d\n", dropped_data.row, dropped_data.column);

    if (dropped_data.insert_row)
    {
        //printf("Inserting row %d\n", dropped_data.row);
        view->insertRow(dropped_data.row);
    }

    if (dropped_data.insert_col)
    {
        //printf("Inserting column %d\n", dropped_data.column);
        view->insertColumn(dropped_data.column);
    }

    view->addPlot(plot, dropped_data.row, dropped_data.column);
}

Plot * MainWindow::makePlot(DataSource * source, const string & datasetId)
{
    auto info = source->dataset_info(datasetId);

    auto dialog = new QDialog;
    dialog->setWindowTitle("Select Plot Data");

    auto settings = new PlotSettingsView(info);

    auto buttons = new QDialogButtonBox
            ( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

    auto layout = new QVBoxLayout(dialog);
    layout->addWidget(settings);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted,
            dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected,
            dialog, &QDialog::reject);

    auto result = dialog->exec();
    if (result != QDialog::Accepted)
        return nullptr;

    auto dataset = source->dataset(datasetId);
    auto plot = settings->makePlot(dataset);

    if (!plot)
    {
        QMessageBox::warning(this, "Creating Plot Failed",
                             QString("Invalid options."));
    }

    return plot;
}

void MainWindow::plot(DataSource * source, const string & id)
{
    auto plot = makePlot(source, id);

    // Add plot to view

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

    {
        auto * view = m_selected_plot_view;

        if (view->hasSelectedCell())
        {
            auto cell = view->selectedCell();
            view->addPlot(plot, cell.y(), cell.x());
        }
        else
        {
            view->addPlot(plot, view->rowCount(), 0);
        }

        view->show();
    }

    //m_selected_plot_view->addPlot(plot);
    //m_selected_plot_view->show();
}

void MainWindow::removeSelectedPlot()
{
    if (!m_selected_plot_view || !m_selected_plot)
        return;

    m_selected_plot_view->removePlot(m_selected_plot);

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
    auto plot_view = qobject_cast<PlotGridView*>(object);

    if (plot_view)
    {
        switch(event->type())
        {
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

void MainWindow::saveProject()
{
    if (m_project_file_path.isEmpty())
    {
        saveProjectAs();
    }
    else
    {
        saveProjectFile(m_project_file_path);
    }
}

void MainWindow::saveProjectAs()
{
    auto file_path = QFileDialog::getSaveFileName(this, "Save Project");

    if (file_path.isEmpty())
        return;

    m_project_file_path = file_path;

    qDebug() << "Setting project file path: " << file_path;

    saveProjectFile(file_path);
}

void MainWindow::saveProjectFile(const QString & path)
{
    ofstream file(path.toStdString());
    if (!file.is_open())
    {
        QMessageBox::warning(this, "Save Project Failed",
                             "Failed to save the project"
                             " because the file could not be accessed.");
        return;
    }

    bool has_errors = false;

    json project_json;

    project_json["main_window_position"] << geometry();

    auto & sources_json = project_json["sources"];

    for (int source_idx = 0; source_idx < m_lib->sourceCount(); ++source_idx)
    {
        auto source = m_lib->source(source_idx);

        json source_json;
        source_json["path"] = source->path();

        sources_json.push_back(source_json);
    }

    auto & plot_views_json = project_json["views"];

    for (auto plot_view : m_plot_views)
    {
        json plot_view_json;

        auto geometry = plot_view->geometry();
        plot_view_json["position"] << geometry;

        for (int row = 0; row < plot_view->rowCount(); ++row)
        {
            for(int col = 0; col < plot_view->columnCount(); ++col)
            {
                auto * plot = plot_view->plotAtCell(row, col);
                if (!plot)
                    continue;

                auto data_set = plot->dataSet();

                json plot_json;
                plot_json["row"] = row;
                plot_json["column"] = col;
                plot_json["data_source"] = data_set->source()->id();
                plot_json["data_set"] = data_set->id();

                try {
                    plot_json["options"] = plot->save();
                } catch (json::exception & e) {
                    cerr << "Failed to save plot: "
                         << data_set->source()->id() << " : " << data_set->id()
                         << endl;
                    has_errors = true;
                    continue;
                }

                plot_view_json["plots"].push_back(plot_json);
            }
        }

        plot_views_json.push_back(plot_view_json);
    }

    try
    {
        file << std::setw(4) << project_json << endl;
    }
    catch (json::exception & e)
    {
        QMessageBox::warning(this, "Save Project Failed",
                             QString("Failed to save the project: ")
                             + e.what());
        return;
    }

    if (has_errors)
    {
        QMessageBox::warning(this, "Save Project",
                             QString("Something went wrong when saving one of the plots."));
        return;
    }
    else
    {
        QMessageBox::information(this, "Save Project", "Project saved successfully.");
    }
}

void MainWindow::openProject()
{
    auto file_path = QFileDialog::getOpenFileName(this, "Open Project");

    if (file_path.isEmpty())
        return;

    openProjectFile(file_path);
}

void MainWindow::openProjectFile(const QString & file_path)
{
    ifstream file(file_path.toStdString());
    if (!file.is_open())
    {
        QMessageBox::warning(this, "Open Project Failed",
                             "Failed to open the project"
                             " because the file could not be accessed:\n"
                             + file_path);
        return;
    }

    json project_json;

    try
    {
        file >> project_json;
    }
    catch (json::exception & e)
    {
        QMessageBox::warning(this, "Open Project",
                             QString("Failed to read the project file: ")
                             + e.what());
        return;
    }

    bool has_errors = false;

    try
    {
        QRect geometry;
        project_json.at("main_window_position") >> geometry;
        setGeometry(geometry);
    }
    catch (json::exception & e)
    {
        cerr << "JSON exception while restoring main window geometry: "
             << e.what() << endl;
        has_errors = true;
    }

    try
    {
        for (auto & source_json : project_json.at("sources"))
        {
            try
            {
                string path = source_json.at("path");
                m_lib->open(QString::fromStdString(path));
            }
            catch (json::exception & e)
            {
                cerr << "JSON exception while restoring a data source: "
                     << e.what() << endl;
                has_errors = true;
            }
        }
    }
    catch (json::exception & e)
    {
        cerr << "JSON exception while restoring data sources: "
             << e.what() << endl;
        has_errors = true;
    }

    try
    {
        for (auto & plot_view_json : project_json.at("views"))
        {
            auto plot_view = addPlotView();

            try
            {
                QRect geometry;
                plot_view_json.at("position") >> geometry;
                plot_view->setGeometry(geometry);
            }
            catch (json::exception & e)
            {
                cerr << "JSON exception while restoring plot view: "
                     << e.what() << endl;
                has_errors = true;
            }

            m_selected_plot_view = plot_view;

            for (auto & plot_json : plot_view_json.at("plots"))
            {
                try
                {
                    restorePlot(plot_view, plot_json);
                }
                catch(json::exception & e)
                {
                    cerr << "JSON exception while restoring a plot: "
                         << e.what() << endl;
                    has_errors = true;
                }
                catch(Error & e)
                {
                    cerr << "Error while restoring a plot: "
                         << e.what() << endl;
                    has_errors = true;
                }
            }
        }
    }
    catch (json::exception & e)
    {
        has_errors = true;
    }

    if (has_errors)
    {
        QMessageBox::warning(this, "Open Project",
                             QString("Something went wrong while restoring plots."));
    }
    else
    {
        qDebug() << "Setting project file path: " << file_path;
        m_project_file_path = file_path;
    }
}

void MainWindow::restorePlot(PlotGridView * view, const json & state)
{
    string plot_type = state.at("options").at("type");

    string source_path = state.at("data_source");
    DataSource * source = m_lib->source(QString::fromStdString(source_path));
    if (!source)
    {
        throw Error(string("Failed to find data source: ") + source_path);
    }

    auto dataset_id = state.at("data_set");
    auto dataset = source->dataset(dataset_id);
    if (!dataset)
    {
        Error e;
        e.reason() << "Failed to find data set " << dataset_id
                   << " in source " << source_path;
        throw e;
    }

    Plot * plot = nullptr;

    if (plot_type == "line")
    {
        plot = new LinePlot;
    }
    else if (plot_type == "heat_map")
    {
        plot = new HeatMap;
    }
    else if (plot_type == "scatter_1d")
    {
        plot = new ScatterPlot1d;
    }
    else if (plot_type == "scatter_2d")
    {
        plot = new ScatterPlot2d;
    }

    try
    {
        plot->restore(dataset, state.at("options"));
    }
    catch (json::exception &)
    {
        delete plot;
        throw Error("Invalid plot state.");
    }

    int row = state.at("row");
    int column = state.at("column");

    view->addPlot(plot, row, column);
}

bool MainWindow::closeProject()
{
    auto msg = QString("Would you like to save the project before closing?");
    auto button = QMessageBox::question(this, "Close Project",
                                        msg, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
    if (button == QMessageBox::Cancel)
        return false;

    if (button == QMessageBox::Yes)
        saveProject();

    m_selected_plot = nullptr;
    m_selected_plot_view = nullptr;

    for (auto * view : m_plot_views)
    {
        delete view;
    }

    m_plot_views.clear();

    m_lib->closeAll();

    m_project_file_path.clear();

    return true;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    auto urls = event->mimeData()->urls();

    for (auto & url : urls)
    {
        if (url.isLocalFile())
        {
            auto path = url.toLocalFile();
            openFile(path);
        }
        else
        {
            QMessageBox::warning(this, "Open Failed",
                                 QString("The URL is not a local file:\n")
                                 + url.toString());
        }
    }

    event->acceptProposedAction();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    bool closed = closeProject();
    if (closed)
        event->accept();
    else
        event->ignore();
}

}
