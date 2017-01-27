#include "../io/hdf5.hpp"

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Required arguments: <input file path> <dataset location>" << endl;
        return 1;
    }

    string file_path(argv[1]);

    string dataset_location;

    int printed_data_count = 20;

    if (argc > 2)
        dataset_location = argv[2];

    if (argc > 3)
        printed_data_count = atoi(argv[3]);

    datavis::Hdf5Source source(file_path);

    int selected_object_index = -1;

    for (int i = 0; i < source.count(); ++i)
    {
        auto info = source.info(i);
        cout << "* Object: " << info.id << endl;
        cout << "  - size: ";
        for (auto & s : info.size)
            cout << s << " ";
        cout << endl;

        if (info.id == dataset_location)
            selected_object_index = i;
    }

    if (dataset_location.empty())
        return 0;

    if (selected_object_index < 0)
    {
        cerr << "No object for location " << dataset_location << endl;
        return 1;
    }

    auto object = source.dataset(selected_object_index);

    cout << "Open data size: ";
    for (auto & s : object->data()->size())
        cout << s << " ";
    cout << endl;

    auto access = datavis::get_all(*object->data());

    int count = printed_data_count;

    cout << "First " << count << " or less elements:" << endl;

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
