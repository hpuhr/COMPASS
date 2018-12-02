#include "jsonobjectparserwidget.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>


JSONObjectParserWidget::JSONObjectParserWidget(JSONObjectParser& parser, QWidget *parent)
    : QWidget(parent), parser_ (&parser)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("JSON Parser ");
    main_label->setFont (font_bold);
    main_layout->addWidget (main_label);

    setLayout (main_layout);

    show();
}

void JSONObjectParserWidget::setParser (JSONObjectParser& parser)
{
    parser_ = &parser;
}
