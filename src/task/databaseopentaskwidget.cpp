#include "databaseopentaskwidget.h"
#include "databaseopentask.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbinterfacewidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

DatabaseOpenTaskWidget::DatabaseOpenTaskWidget(DatabaseOpenTask& task, QWidget* parent)
    : QWidget(parent), task_(task)
{
    QVBoxLayout* main_layout_ = new QVBoxLayout ();

    //main_layout_->addWidget(new QLabel("DatabaseOpenTaskWidget"));

    dbinterface_widget_ = ATSDB::instance().interface().widget();
    main_layout_->addWidget(dbinterface_widget_);

    setLayout (main_layout_);
}
