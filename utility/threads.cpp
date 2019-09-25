#include "threads.hpp"

namespace datavis {

QThread * background_thread()
{
    static QThread background_thread;
    return &background_thread;
}

}
