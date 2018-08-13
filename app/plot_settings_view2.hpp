#pragma once

#include "../data/data_source.hpp"

#include <QWidget>
#include <QListWidget>
#include <QComboBox>
#include <QStackedLayout>

namespace datavis {

class Plot;
class PlotSettings;

class PlotSettingsView : public QWidget
{
public:
    PlotSettingsView(const DataSetInfo & info, QWidget * parent = 0);
    Plot * makePlot(const DataSetPtr &);
private:
    DataSetInfo m_info;
    QComboBox * m_type = nullptr;
    QStackedLayout * m_settings_stack = nullptr;
    vector<PlotSettings*> m_settings;
};

class PlotSettings : public QWidget
{
public:
    PlotSettings(const DataSetInfo & info, QWidget * parent = nullptr);
    virtual Plot * makePlot(const DataSetPtr &) = 0;
protected:
    void fillDimensionsAndAttributes(QComboBox *);
    void fillDimensions(QComboBox *);
    void fillAttributes(QComboBox *);
    QComboBox * addDimensionsAndAttributesBox(const QString & name);

    const DataSetInfo & m_info;
};

class LinePlotSettings : public PlotSettings
{
public:
    LinePlotSettings(const DataSetInfo & info, QWidget * parent = nullptr);
    Plot * makePlot(const DataSetPtr &) override;
private:
    QComboBox * m_dimension = nullptr;
};

class HeatMapPlotSettings : public PlotSettings
{
public:
    HeatMapPlotSettings(const DataSetInfo & info, QWidget * parent = nullptr);
    Plot * makePlot(const DataSetPtr &) override;
private:
    QComboBox * m_x_dim = nullptr;
    QComboBox * m_y_dim = nullptr;
};

class ScatterPlot1dSettings : public PlotSettings
{
public:
    ScatterPlot1dSettings(const DataSetInfo & info, QWidget * parent = nullptr);
    Plot * makePlot(const DataSetPtr &) override;
private:
    QComboBox * m_orientation = nullptr;
    QComboBox * m_attribute = nullptr;
};

class ScatterPlot2dSettings : public PlotSettings
{
public:
    ScatterPlot2dSettings(const DataSetInfo & info, QWidget * parent = nullptr);
    Plot * makePlot(const DataSetPtr &) override;

private:
    QComboBox * m_x_source = nullptr;
    QComboBox * m_y_source = nullptr;
};

}

