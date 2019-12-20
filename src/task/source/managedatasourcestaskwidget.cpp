#include "managedatasourcestaskwidget.h"
#include "atsdb.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dboeditdatasourceswidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>

ManageDataSourcesTaskWidget::ManageDataSourcesTaskWidget(ManageDataSourcesTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    QVBoxLayout* main_layout_ = new QVBoxLayout ();

    tab_widget_ = new QTabWidget ();
    main_layout_->addWidget(tab_widget_);

    for (auto& dbo_it : ATSDB::instance().objectManager())
    {
        tab_widget_->addTab(dbo_it.second->editDataSourcesWidget(), dbo_it.first.c_str());
    }

    expertModeChangedSlot();

    setLayout (main_layout_);
}

void ManageDataSourcesTaskWidget::expertModeChangedSlot ()
{

}
