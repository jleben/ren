#pragma once

#include <iostream>
#include <vector>

namespace datavis {

template <typename T>
class coordinate
{
public:
  coordinate();
  coordinate(const vector<T> & e): elements(e) {}

  std::vector<T> elements;

  friend ostream & operator<< (ostream & out, const coordinate<T> & c)
  {
    out << '(';
    int first = true;
    for (auto & e : c.elements)
    {
      if (!first)
        out << ", ";

      first = false;

      out << e;
    }
    out << ')';
    return out;
  }
};

template <typename T> inline
coordinate<T> to_coordinate(const vector<T> & v)
{
  return coordinate<T>(v);
}

}
