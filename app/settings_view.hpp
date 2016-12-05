#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace datavis {

class SettingsView : public QWidget
{
public:
    SettingsView(QWidget * parent = 0);
    void setPlotSettingsView(QWidget *);
private:
    QVBoxLayout * m_layout = nullptr;
    QWidget * m_plot_settings_view = nullptr;
};

}
