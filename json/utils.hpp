#include "json.hpp"
#include <QRect>

namespace datavis {

using nlohmann::json;

inline json & operator<<(json & j, const QRect & r)
{
    j["x"] = r.x();
    j["y"] = r.y();
    j["width"] = r.width();
    j["height"] = r.height();
    return j;
}

inline json & operator>>(json & j, QRect & r)
{
    r.setX(j.at("x"));
    r.setY(j.at("y"));
    r.setWidth(j.at("width"));
    r.setHeight(j.at("height"));
    return j;
}

}
