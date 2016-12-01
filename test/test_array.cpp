#include "../data/array.hpp"

#include <string>
#include <sstream>
#include <iostream>

using namespace std;
using namespace datavis;

int main()
{
    array<string> a({5,6,7});

    {
        cout << "Writing:" << endl;
        auto region = get_all(a);
        //for(auto i = a.iterator(); i.is_valid(); ++i)
        for (auto & i : region)
        {
            auto loc = i.location();
            ostringstream txt;

            for (auto & l : loc)
            {
                cout << l << " ";
                txt << to_string(l) << " ";
            }
            cout << "(" << i.index() << ")";
            cout << endl;

            i.value() = txt.str();
        }
    }

    {
        cout << "Reading:" << endl;

        //auto region = get_region(a, {1,2,3},{1,3,1});
        auto region = get_region(a, {1,2,3},{2,3,4});

        //for(; i.is_valid(); ++i)
        for(auto & i : region)
        {
            auto loc = i.location();
            for (auto & l : loc)
            {
                cout << l << " ";
            }
            cout << "(" << i.index() << ")";
            cout << " = " << i.value() << endl;
        }
    }
}
