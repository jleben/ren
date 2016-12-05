#pragma once

#include <QObject>
#include <QColor>

namespace datavis {

class DataSource;

class LinePlotSettings : public QObject
{
    Q_OBJECT

public:
    LinePlotSettings(QObject * parent = 0);

    void setDataSource(DataSource *);
    DataSource * source() const { return m_source; }

    void setDimensionX(int dim);
    void setColor(const QColor &);
    void setDefault();

    int dimensionX() const { return m_dim_x; }
    QColor color() const { return m_color; }

signals:
    void sourceChanged();
    void dimensionXChanged(int dim);
    void colorChanged();

private:
    void justSetDefault();

    DataSource * m_source = nullptr;
    int m_dim_x = 0;
    QColor m_color { Qt::black };
};

}
