#pragma once

#include <QObject>
#include <string>
#include <memory>

#include "math.hpp"

namespace datavis {

using std::string;
using std::shared_ptr;

// When to remove dimensions?
// How to track data range?

// When a data source is created,
// add it to a collection, which updates the dimension, ranges, etc.
// When a data source is destroyed, remove it from the collection,
// which then updates dimensions...

class Dimension : public QObject
{
    Q_OBJECT

public:
    Dimension();

    double focus() const { return d_focus; }

    void setFocus(double value)
    {
        d_focus = value;
        emit focusChanged(value);
    }

    const Range & range() const { return d_range; }
    Range & range() { return d_range; }

signals:
    void focusChanged(double value);

private:
    double d_focus = 0;
    Range d_range = { 0, 0 };
};

using DimensionPtr = std::shared_ptr<Dimension>;


}
