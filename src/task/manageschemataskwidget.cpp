#include "manageschemataskwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ManageSchemaTaskWidget::ManageSchemaTaskWidget(ManageSchemaTask& task, QWidget* parent)
    : QWidget(parent), task_(task)
{
    QVBoxLayout* main_layout_ = new QVBoxLayout ();

    main_layout_->addWidget(new QLabel("ManageSchemaTaskWidget"));

//    dbinterface_widget_ = ATSDB::instance().interface().widget();
//    main_layout_->addWidget(dbinterface_widget_);

    setLayout (main_layout_);
}
