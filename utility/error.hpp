#include <stdexcept>
#include <string>
#include <sstream>

namespace datavis {

class Error : public std::exception
{
    using string = std::string;
    string d_reason;

public:
    Error(const string & reason = string()): d_reason(reason) {}

    ostringstream reason() { return ostringstream(d_reason); }

    const char * what() const noexcept
    {
        return d_reason.c_str();
    }
};

}
