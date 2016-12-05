#pragma once

#include <QWidget>
#include <QSpinBox>

namespace datavis {

class LinePlotSettings;
class ColorBox;

class LinePlotSettingsView : public QWidget
{
public:
    LinePlotSettingsView(QWidget * parent = 0);
    void setSettings(LinePlotSettings *);

private:
    void updateAll();
    void onSourceChanged();
    void onDimXChanged();
    void onDimXEdited();
    void onColorChanged();

    LinePlotSettings * m_settings = nullptr;
    QSpinBox * m_dim_x;
    ColorBox * m_color;
};

}
