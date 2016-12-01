#include "../plot/plot_view.hpp"
#include "../plot/line_plot.hpp"
#include "../data/array.hpp"

#include <QApplication>
#include <QDebug>

#include <cmath>

using namespace datavis;

static double pi = std::atan(1.0) * 4.0;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    datavis::array<double> a({20});

    {
        cout << "Generating data:" << endl;
        auto region = get_all(a);
        for (auto & i : region)
        {
            auto loc = i.location()[0];
            i.value() = std::sin(loc * 3.0/20 * 2 * pi) ;
            cout << i.value() << endl;
        }
    }

    auto plot = new LinePlot;
    plot->setData(&a);

    auto plot_view = new PlotView;
    plot_view->addPlot(plot);
    plot_view->resize(600,600);
    plot_view->show();

    return app.exec();
}
