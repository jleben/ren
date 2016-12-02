#include "../plot/plot_view.hpp"
#include "../plot/line_plot.hpp"
#include "../plot/heat_map.hpp"
#include "../data/array.hpp"

#include <QApplication>
#include <QDebug>

#include <cmath>

using namespace datavis;

static double pi = std::atan(1.0) * 4.0;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int elem_count = 200;
    datavis::array<double> a({elem_count});

    {
        //cout << "Generating data:" << endl;
        auto region = get_all(a);
        for (auto & i : region)
        {
            auto loc = i.location()[0];
            i.value() = std::sin(loc * 3.0/elem_count * 2 * pi) ;
            //cout << i.value() << endl;
        }
    }

    datavis::array<double> a2({elem_count, elem_count});

    {
        //cout << "Generating data:" << endl;
        auto region = get_all(a2);
        for (auto & i : region)
        {
            //auto loc = i.location()[0];
            double ph = i.location()[0] + i.location()[1];
            i.value() = std::sin(ph * 3.0/elem_count * 2 * pi);
            //i.value() = 0;
            //cout << i.value() << endl;
        }
    }

    auto plot_view = new PlotView;
#if 0
    auto plot = new LinePlot;
    plot->setData(&a);
    plot_view->addPlot(plot);
#endif
    auto heat = new HeatMap;
    heat->setData(&a2);
    plot_view->addPlot(heat);

    plot_view->resize(600,600);
    plot_view->show();

    return app.exec();
}
