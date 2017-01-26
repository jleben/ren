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

    DataObject data("", datavis::read_hdf5<double>(file_path, dataset_location));

    auto plot_view = new PlotView;

    auto heat = new HeatMap;
    heat->setDataObject(&data);
    plot_view->addPlot(heat);

    plot_view->resize(600,600);
    plot_view->show();

    return app.exec();
}
