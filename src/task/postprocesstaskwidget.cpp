#include "postprocesstaskwidget.h"
#include "postprocesstask.h"
#include "atsdb.h"
#include "dbschemamanager.h"
#include "dbschemamanagerwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

PostProcessTaskWidget::PostProcessTaskWidget(PostProcessTask& task, QWidget* parent)
    : QWidget(parent), task_(task)
{
    QVBoxLayout* main_layout = new QVBoxLayout ();

    //main_layout->addWidget(new QLabel(""));

    QPushButton* run_button = new QPushButton ("Run");
    connect (run_button, &QPushButton::clicked, this, &PostProcessTaskWidget::runSlot);
    main_layout->addWidget(run_button);

//    dbschema_manager_widget_ = ATSDB::instance().schemaManager().widget();
//    //    QObject::connect(dbinterface_widget_, SIGNAL(databaseOpenedSignal()), dbschema_manager_widget_, SLOT(databaseOpenedSlot()));
//    main_layout->addWidget(dbschema_manager_widget_);

    setLayout (main_layout);
}

void PostProcessTaskWidget::runSlot ()
{
    loginf << "PostProcessTaskWidget: runSlot";

    task_.run();
}
