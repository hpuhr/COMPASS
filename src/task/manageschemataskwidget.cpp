#include "manageschemataskwidget.h"
#include "atsdb.h"
#include "dbschemamanager.h"
#include "dbschemamanagerwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ManageSchemaTaskWidget::ManageSchemaTaskWidget(ManageSchemaTask& task, QWidget* parent)
    : QWidget(parent), task_(task)
{
    QVBoxLayout* main_layout = new QVBoxLayout ();

    //main_layout_->addWidget(new QLabel("ManageSchemaTaskWidget"));

    dbschema_manager_widget_ = ATSDB::instance().schemaManager().widget();
    //    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), dbschema_manager_widget_, SLOT(databaseOpenedSlot()));
    main_layout->addWidget(dbschema_manager_widget_);

    setLayout (main_layout);
}
