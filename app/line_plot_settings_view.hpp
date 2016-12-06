#pragma once

#include <QWidget>
#include <QSpinBox>

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
    void onSourceChanged();
    void onDimXChanged();
    void onDimXEdited();
    void onColorChanged();

    LinePlot * m_plot = nullptr;
    QSpinBox * m_dim_x;
    ColorBox * m_color;
};

}
