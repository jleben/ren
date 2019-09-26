#include "plot_settings_view2.hpp"
#include "../plot/line_plot.hpp"
#include "../plot/heat_map.hpp"
#include "../plot/scatter_plot_1d.hpp"
#include "../plot/scatter_plot_2d.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListWidgetItem>
#include <QLabel>

namespace datavis {

PlotSettingsView::PlotSettingsView(const DataSetInfo & info, QWidget *parent):
    QWidget(parent)
{
    auto layout = new QVBoxLayout(this);

    m_type = new QComboBox(this);
    m_type->addItem("Line");
    m_type->addItem("Heat Map");
    m_type->addItem("Scatter 1D");
    m_type->addItem("Scatter 2D");
    layout->addWidget(m_type);

    m_settings.push_back(new LinePlotSettings(info));
    m_settings.push_back(new HeatMapPlotSettings(info));
    m_settings.push_back(new ScatterPlot1dSettings(info));
    m_settings.push_back(new ScatterPlot2dSettings(info));

    m_settings_stack = new QStackedLayout;
    for (auto widget : m_settings)
    {
        m_settings_stack->addWidget(widget);
    }
    layout->addLayout(m_settings_stack);

    connect(m_type, SIGNAL(currentIndexChanged(int)),
            m_settings_stack, SLOT(setCurrentIndex(int)));
}

Plot * PlotSettingsView::makePlot(const FutureDataset & dataset)
{
    auto settings = m_settings[m_type->currentIndex()];
    return settings->makePlot(dataset);
}

PlotSettings::PlotSettings(const DataSetInfo & info, QWidget * parent):
    QWidget(parent),
    m_info(info)
{
}

void PlotSettings::fillDimensionsAndAttributes(QComboBox * box)
{
    fillDimensions(box);
    fillAttributes(box);
}

void PlotSettings::fillDimensions(QComboBox * box)
{
    for(int i = 0; i < m_info.dimensions.size(); ++i)
    {
        string name = m_info.dimensions[i].name;
        if (name.empty())
            name = "Dimension " + to_string(i);
        cerr << "Adding dim " << name << endl;
        box->addItem(QString::fromStdString(name));
    }
}

void PlotSettings::fillAttributes(QComboBox * box)
{
    for (int i = 0; i < m_info.attributes.size(); ++i)
    {
        string name = m_info.attributes[i].name;
        if (name.empty())
            name = "Attribute " + to_string(i);
        cerr << "Adding att " << name << endl;
        box->addItem(QString::fromStdString(name));
    }
}

LinePlotSettings::LinePlotSettings(const DataSetInfo & info, QWidget * parent):
    PlotSettings(info, parent)
{
    auto form = new QFormLayout(this);

    m_dimension = new QComboBox;
    fillDimensions(m_dimension);

    form->addRow("Dimension:", m_dimension);
}

Plot * LinePlotSettings::makePlot(const FutureDataset & dataset)
{
    int dim = m_dimension->currentIndex();
    if (dim < 0)
        return nullptr;

    auto plot = new LinePlot;
    plot->setDataSet(dataset, m_dimension->currentIndex());
    return plot;
}

HeatMapPlotSettings::HeatMapPlotSettings(const DataSetInfo & info, QWidget * parent):
    PlotSettings(info, parent)
{
    auto form = new QFormLayout(this);

    m_x_dim = new QComboBox;
    m_y_dim = new QComboBox;

    m_y_dim->setModel(m_x_dim->model());

    fillDimensions(m_x_dim);

    form->addRow(new QLabel("X:"), m_x_dim);
    form->addRow(new QLabel("Y:"), m_y_dim);

    m_x_dim->setCurrentIndex(0);
    m_y_dim->setCurrentIndex(1);
}

Plot * HeatMapPlotSettings::makePlot(const FutureDataset & dataset)
{
    int x = m_x_dim->currentIndex();
    int y = m_y_dim->currentIndex();
    if (x < 0 || y < 0)
        return nullptr;

    auto plot = new HeatMap;
    plot->setDataSet(dataset, { x, y });
    return plot;
}

ScatterPlot1dSettings::ScatterPlot1dSettings(const DataSetInfo & info, QWidget * parent):
    PlotSettings(info, parent)
{
    auto form = new QFormLayout(this);

    m_orientation = new QComboBox;
    m_orientation->addItem("Horizontal");
    m_orientation->addItem("Vertical");

    m_attribute = new QComboBox;
    fillAttributes(m_attribute);

    form->addRow("Orientation:", m_orientation);
    form->addRow("Attribute:", m_attribute);
}

Plot * ScatterPlot1dSettings::makePlot(const FutureDataset & dataset)
{
    auto orientation = m_orientation->currentIndex() == 0 ? ScatterPlot1d::Horizontal : ScatterPlot1d::Vertical;
    int attribute = m_attribute->currentIndex();
    if (attribute < 0)
        return nullptr;

    auto plot = new ScatterPlot1d;
    plot->setData(dataset, attribute, orientation);
    return plot;
}

ScatterPlot2dSettings::ScatterPlot2dSettings(const DataSetInfo & info, QWidget * parent):
    PlotSettings(info, parent)
{
    auto form = new QFormLayout(this);

    m_x_source = new QComboBox;
    m_y_source = new QComboBox;
    m_y_source->setModel(m_x_source->model());
    m_dots = new QCheckBox;
    m_line = new QCheckBox;

    fillDimensionsAndAttributes(m_x_source);

    form->addRow(new QLabel("X:"), m_x_source);
    form->addRow(new QLabel("Y:"), m_y_source);
    form->addRow(new QLabel("Dots:"), m_dots);
    form->addRow(new QLabel("Line:"), m_line);

    int first_attribute = info.dimensionCount();

    if (info.attributes.size() >= 2)
    {
        m_x_source->setCurrentIndex(first_attribute);
        m_y_source->setCurrentIndex(first_attribute+1);
    }
    else
    {
        m_x_source->setCurrentIndex(0);
        m_y_source->setCurrentIndex(first_attribute);
    }

    m_dots->setChecked(true);
}

Plot * ScatterPlot2dSettings::makePlot(const FutureDataset & dataset)
{
    int x = m_x_source->currentIndex();
    int y = m_y_source->currentIndex();
    if (x < 0 || y < 0)
        return nullptr;

    auto plot = new ScatterPlot2d();
    plot->setData(dataset, x, y);
    plot->setShowDot(m_dots->isChecked());
    plot->setShowLine(m_line->isChecked());
    return plot;
}

}
