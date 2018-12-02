
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include "jsondatamappingwidget.h"

JSONDataMappingWidget::JSONDataMappingWidget(JSONDataMapping& mapping, QWidget *parent)
    : QWidget(parent), mapping_(&mapping)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("JSON Mapping ");
    main_label->setFont (font_bold);
    main_layout->addWidget (main_label);

    setLayout (main_layout);

    show();
}

void JSONDataMappingWidget::setMapping (JSONDataMapping& mapping)
{
    mapping_ = &mapping;
}
