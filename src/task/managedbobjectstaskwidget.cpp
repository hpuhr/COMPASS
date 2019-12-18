#include "managedbobjectstaskwidget.h"
#include "atsdb.h"
#include "dbobjectmanager.h"
#include "dbobjectmanagerwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ManageDBObjectsTaskWidget::ManageDBObjectsTaskWidget(ManageDBObjectsTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    QVBoxLayout* main_layout_ = new QVBoxLayout ();

    object_manager_widget_ = ATSDB::instance().objectManager().widget();
    main_layout_->addWidget(object_manager_widget_);

    expertModeChangedSlot();

    setLayout (main_layout_);
}

void ManageDBObjectsTaskWidget::expertModeChangedSlot ()
{

}
