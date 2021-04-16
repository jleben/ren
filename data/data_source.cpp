#include "data_source.hpp"

#include <iostream>

using namespace std;

namespace datavis {

void DataSource::reload()
{
    cerr << "This data source does not support reloading." << endl;
}

}
