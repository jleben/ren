#include "../io/hdf5.hpp"
#include "../plot/plot_view.hpp"
#include "../plot/line_plot.hpp"
#include "../plot/heat_map.hpp"

#include <iostream>

#include <QApplication>

using namespace std;
using namespace datavis;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cerr << "Required arguments: <input file path> <dataset location>" << endl;
        return 1;
    }

    string file_path(argv[1]);
    string dataset_location(argv[2]);

    QApplication app(argc, argv);

    Hdf5Source source(file_path);
    auto object_index = source.index(dataset_location);
    if (object_index < 0)
    {
        cerr << "No object named " << dataset_location << endl;
        return 1;
    }

    auto object = source.dataset(object_index);

    auto plot_view = new PlotView;

    auto heat = new HeatMap;
    heat->setDataSet(object);
    plot_view->addPlot(heat);

    plot_view->resize(600,600);
    plot_view->show();

    int result = app.exec();

    delete object;

    return result;
}
