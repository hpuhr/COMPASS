#include "managedbobjectstaskwidget.h"
#include "atsdb.h"
#include "dbobjectmanager.h"
#include "dbobjectmanagerwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ManageDBObjectsTaskWidget::ManageDBObjectsTaskWidget(ManageDBObjectsTask& task, QWidget* parent)
    : QWidget(parent), task_(task)
{
    QVBoxLayout* main_layout_ = new QVBoxLayout ();

    //main_layout_->addWidget(new QLabel("DatabaseOpenTaskWidget"));

//    dbinterface_widget_ = ATSDB::instance().interface().widget();
//    //    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), this, SLOT(databaseOpenedSlot()));
//    main_layout_->addWidget(dbinterface_widget_);

    object_manager_widget_ = ATSDB::instance().objectManager().widget();
    //    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), object_manager_widget_, SLOT(databaseOpenedSlot()));
    main_layout_->addWidget(object_manager_widget_);

    setLayout (main_layout_);
}
