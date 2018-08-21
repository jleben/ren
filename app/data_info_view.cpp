#include "data_info_view.hpp"

#include <QFormLayout>
#include <QLabel>

using namespace std;

namespace datavis {

DataInfoView::DataInfoView(QWidget * parent):
    QWidget(parent)
{
    m_dimensions = new QLabel;
    m_attributes = new QLabel;

    auto form = new QFormLayout(this);

    Qt::Alignment labelAlignment = Qt::AlignRight | Qt::AlignTop;

    {
        auto label = new QLabel("Space:");
        label->setAlignment(labelAlignment);
        form->addRow(label, m_dimensions);
    }
    {
        auto label = new QLabel("Attributes:");
        label->setAlignment(labelAlignment);
        form->addRow(label, m_attributes);
    }
}

void DataInfoView::setInfo(const DataSetInfo & info)
{
    QStringList sizes;
    QStringList dim_infos;

    for (int i = 0; i < info.dimensions.size(); ++i)
    {
        const auto & dim = info.dimensions[i];

        sizes.push_back(QString::number(dim.size));

        QString info;
        if (dim.name.empty())
            info += "(Unnamed)";
        else
            info += QString::fromStdString(dim.name);

        info += ": ";
        info += QString(" [%1, %2]").arg(dim.minimum()).arg(dim.maximum());

        dim_infos.push_back(info);
    }

    QString dimensions_text;
    dimensions_text += sizes.join(" x ");
    dimensions_text += "\n";
    dimensions_text += dim_infos.join("\n");

    m_dimensions->setText(dimensions_text);

    QString attributes_text;

    for (int i = 0; i < info.attributes.size(); ++i)
    {
        const auto & attribute = info.attributes[i];

        QString text;
        if (attribute.name.empty())
            text += "(Unnamed)";
        else
            text += QString::fromStdString(attribute.name);

        if (!attributes_text.isEmpty())
            attributes_text += "\n";
        attributes_text += text;
    }

    m_attributes->setText(attributes_text);
}

}
