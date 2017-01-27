#include "../plot/plot_view.hpp"
#include "../plot/line_plot.hpp"
#include "../plot/heat_map.hpp"
#include "../data/array.hpp"
#include "../data/data_set.hpp"

#include <QApplication>
#include <QDebug>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

#include <cmath>

using namespace datavis;

static double pi = std::atan(1.0) * 4.0;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int data_size = 200;

    double base_freq = 1.0/200.0;

    auto source1 = make_shared<DataSet>(vector<int>({data_size}));

    {
        //cout << "Generating data:" << endl;
        auto region = get_all(*source1->data());
        for (auto & i : region)
        {
            auto loc = i.location()[0];
            i.value() = std::sin(2.0 * loc / data_size * 2 * pi) ;
            //cout << i.value() << endl;
        }
    }

    auto source2 = make_shared<DataSet>(vector<int>({data_size}));

    Dimension dim;
    dim.map.offset = 100;
    dim.map.scale = 0.5;
    source2->setDimension(0, dim);

    {
        //cout << "Generating data:" << endl;
        auto region = get_all(*source2->data());
        for (auto & i : region)
        {
            auto loc = i.location()[0];
            i.value() = 0.5 * std::sin((0.5 + 2.0 * loc / data_size) * 2 * pi) ;
            //cout << i.value() << endl;
        }
    }

    auto plot_view = new PlotView;

    {
        auto plot = new LinePlot;
        plot->setDataSet(source1);
        plot_view->addPlot(plot);
    }
    {
        auto plot = new LinePlot;
        plot->setDataSet(source2);
        plot_view->addPlot(plot);
    }

    auto win = new QWidget;

    auto stacked_option = new QCheckBox("Stacked");
    stacked_option->setChecked(plot_view->isStacked());

    auto common_x_option = new QCheckBox("Common X");
    common_x_option->setChecked(plot_view->hasCommonX());

    auto common_y_option = new QCheckBox("Common Y");
    common_y_option->setChecked(plot_view->hasCommonY());

    auto layout = new QVBoxLayout(win);
    layout->addWidget(stacked_option);
    layout->addWidget(common_x_option);
    layout->addWidget(common_y_option);
    layout->addWidget(plot_view);

    QObject::connect(stacked_option, &QCheckBox::clicked,
                     plot_view, &PlotView::setStacked);
    QObject::connect(common_x_option, &QCheckBox::clicked,
                     plot_view, &PlotView::setCommonX);
    QObject::connect(common_y_option, &QCheckBox::clicked,
                     plot_view, &PlotView::setCommonY);

    win->resize(600,600);
    win->show();

    return app.exec();
}


