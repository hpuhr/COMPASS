#include "manageschemataskwidget.h"
#include "manageschematask.h"
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

    DBSchemaManagerWidget* dbschema_manager_widget = ATSDB::instance().schemaManager().widget();
    connect(dbschema_manager_widget, &DBSchemaManagerWidget::schemaLockedSignal,
            this, &ManageSchemaTaskWidget::schemaLockedSlot);
    main_layout->addWidget(dbschema_manager_widget);

    setLayout (main_layout);
}


void ManageSchemaTaskWidget::schemaLockedSlot ()
{
    loginf << "ManageSchemaTaskWidget: schemaLockedSlot";
    emit task_.statusChangedSignal(task_.name());
}
