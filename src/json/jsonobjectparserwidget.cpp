#include "jsonobjectparserwidget.h"
#include "jsonobjectparser.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>


JSONObjectParserWidget::JSONObjectParserWidget(JSONObjectParser& parser, QWidget *parent)
    : QWidget(parent), parser_ (&parser)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    std::string tmp ="JSON Object Parser " + parser_->dbObjectName();
    QLabel *main_label = new QLabel (tmp.c_str());
    main_label->setFont (font_bold);
    main_layout->addWidget (main_label);

    setLayout (main_layout);

    show();
}

void JSONObjectParserWidget::setParser (JSONObjectParser& parser)
{
    parser_ = &parser;
}
