#include "../io/hdf5.hpp"

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        cerr << "Required arguments: <input file path> <dataset location>" << endl;
        return 1;
    }

    string file_path(argv[1]);
    string dataset_location(argv[2]);

    auto data = datavis::read_hdf5<double>(file_path, dataset_location);

    cout << "size: ";
    for (auto & s : data.size())
        cout << s << " ";
    cout << endl;

    auto access = datavis::get_all(data);

    int count = 300;
    for(auto & e : access)
    {
        auto loc = e.location();
        for (auto & i : loc)
            cout << i << " ";
        cout << "= ";
        cout << e.value();
        cout << endl;

        if (--count <= 0)
            break;
    }

    return 0;
}
