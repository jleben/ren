#pragma once

#include <QObject>

namespace datavis {

class Selector : public QObject
{
    Q_OBJECT

public:
    double value() const { return m_value; }

    void setValue(double value)
    {
        if (m_value != value)
        {
            m_value = value;
            emit valueChanged(m_value);
        }
    }

signals:
    void valueChanged(double value);

private:
    double m_value = 0;
};

}
