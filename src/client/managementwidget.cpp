/*
 * ManagementWidget.cpp
 *
 *  Created on: Mar 31, 2012
 *      Author: sk
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QPushButton>

#include "managementwidget.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "dbinterfaceinfowidget.h"
#include "dbobjectmanager.h"
#include "dbobjectmanagerinfowidget.h"
#include "dbobject.h"
//#include "FilterConfigWidget.h"
//#include "DBInfoWidget.h"
//#include "ResultSetWidget.h"
//#include "WorkerThreadWidget.h"
#include "viewmanager.h"
#include "viewmanagerwidget.h"
#include "jobmanager.h"
#include "jobmanagerwidget.h"
#include "logger.h"
#include "global.h"

ManagementWidget::ManagementWidget() : QWidget ()
{
    QVBoxLayout *vlayout = new QVBoxLayout ();
    QHBoxLayout *hlayout = new QHBoxLayout ();

    unsigned int frame_width = FRAME_SIZE;

    vlayout->addLayout (hlayout);

    QVBoxLayout *left_layout = new QVBoxLayout ();

    DBInterfaceInfoWidget *interface_widget = ATSDB::instance().interface().infoWidget();
    interface_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    interface_widget->setLineWidth(frame_width);
    left_layout->addWidget (interface_widget);
    left_layout->addStretch ();

    DBObjectManagerInfoWidget *objman_widget = ATSDB::instance().objectManager().infoWidget();
    objman_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    objman_widget->setLineWidth(frame_width);
    left_layout->addWidget (objman_widget);

    //  result_ = new ResultSetWidget ();
    //  result_->setFrameStyle(QFrame::Panel | QFrame::Raised);
    //  result_->setLineWidth(frame_width);
    //  vlayout2->addWidget (result_);

    hlayout->addLayout (left_layout);
    hlayout->addSpacing(400);

    //  filter_config_gui_ = new FilterConfigWidget ();
    //  filter_config_gui_->setFrameStyle(QFrame::Panel | QFrame::Raised);
    //  filter_config_gui_->setLineWidth(frame_width);
    //  hlayout->addWidget (filter_config_gui_);

    //  worker_widget_ = new WorkerThreadWidget ();
    //  worker_widget_->setFrameStyle(QFrame::Panel | QFrame::Raised);
    //  worker_widget_->setLineWidth(frame_width);
    //  hlayout->addWidget (worker_widget_);

    QVBoxLayout *right_layout = new QVBoxLayout ();

    ViewManagerWidget *viewman_widget = ATSDB::instance().viewManager().widget();
    viewman_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    viewman_widget->setLineWidth(frame_width);
    right_layout->addWidget (viewman_widget);

    right_layout->addStretch();

    JobManagerWidget *jobman_widget = JobManager::instance().widget();
    jobman_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    jobman_widget->setLineWidth(frame_width);
    right_layout->addWidget (jobman_widget);

    hlayout->addLayout (right_layout);

    setLayout(vlayout);
}

ManagementWidget::~ManagementWidget()
{
}

void ManagementWidget::resizeEvent (QResizeEvent *event)
{
    logdbg  << "ManagementWidget: resizeEvent";
    //  int tmp_width = event->size().width();

    //  if (tmp_width > 1000)
    //    tmp_width=1000;

    //  unsigned int tabs=4;
    //  unsigned int min_width=(float)(tmp_width)/(float)tabs-5;

    //  db_info_->setMinimumWidth (min_width);
    //  db_info_->setMaximumWidth (min_width);

    //  result_->setMinimumWidth (min_width);
    //  result_->setMaximumWidth (min_width);

    //  filter_config_gui_->setMinimumWidth (min_width);

    //  worker_widget_->setMinimumWidth (min_width);
    //  worker_widget_->setMaximumWidth (min_width);

    //  views_->setMinimumWidth (min_width);
    //  views_->setMaximumWidth (min_width);
}

