#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>

#include "dboeditdatasourceswidget.h"
#include "dbobject.h"

DBOEditDataSourcesWidget::DBOEditDataSourcesWidget(DBObject* object, QWidget *parent, Qt::WindowFlags f)
    : QWidget (parent, f), object_(object)
{
    assert (object_);

    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    std::string caption = "Edit DBObject " +object_->name()+ " DataSources";

    QLabel *main_label = new QLabel (caption.c_str());
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    setLayout (main_layout);

    show();
}

void DBOEditDataSourcesWidget::update ()
{

}
