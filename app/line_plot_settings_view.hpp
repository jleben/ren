#pragma once

#include <QWidget>
#include <QSpinBox>
#include <QComboBox>

namespace datavis {

class LinePlot;
class ColorBox;

class LinePlotSettingsView : public QWidget
{
public:
    LinePlotSettingsView(QWidget * parent = 0);
    void setPlot(LinePlot *);

private:
    void updateAll();
    void updateDimensionList();
    void onSourceChanged();
    void onXDimChanged();
    void onXDimEdited();
    void onSelectorDimChanged();
    void onSelectorDimEdited();
    void onColorChanged();

    LinePlot * m_plot = nullptr;
    QComboBox * m_x_dim;
    QComboBox * m_selector_dim;
    ColorBox * m_color;
};

}
