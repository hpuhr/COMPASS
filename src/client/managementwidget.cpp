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
#include "dbobject.h"
//#include "FilterConfigWidget.h"
//#include "DBInfoWidget.h"
//#include "ResultSetWidget.h"
//#include "WorkerThreadWidget.h"
//#include "ViewsWidget.h"
#include "logger.h"
#include "global.h"

ManagementWidget::ManagementWidget() : QWidget ()
{
    QVBoxLayout *vlayout = new QVBoxLayout ();
    QHBoxLayout *hlayout = new QHBoxLayout ();

    unsigned int frame_width = FRAME_SIZE;

    vlayout->addLayout (hlayout);

    QVBoxLayout *vlayout2 = new QVBoxLayout ();

    DBInterfaceInfoWidget *interface_widget = ATSDB::instance().interface().infoWidget();
    interface_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    interface_widget->setLineWidth(frame_width);
    vlayout2->addWidget (interface_widget);
    vlayout2->addStretch ();

    QPushButton *button = new QPushButton ("Click Me");
    connect (button, SIGNAL(clicked(bool)), this, SLOT(startSlot()));
    vlayout2->addWidget(button);

    //  result_ = new ResultSetWidget ();
    //  result_->setFrameStyle(QFrame::Panel | QFrame::Raised);
    //  result_->setLineWidth(frame_width);
    //  vlayout2->addWidget (result_);

    hlayout->addLayout (vlayout2);

    //  filter_config_gui_ = new FilterConfigWidget ();
    //  filter_config_gui_->setFrameStyle(QFrame::Panel | QFrame::Raised);
    //  filter_config_gui_->setLineWidth(frame_width);
    //  hlayout->addWidget (filter_config_gui_);

    //  worker_widget_ = new WorkerThreadWidget ();
    //  worker_widget_->setFrameStyle(QFrame::Panel | QFrame::Raised);
    //  worker_widget_->setLineWidth(frame_width);
    //  hlayout->addWidget (worker_widget_);

    //  views_ = new ViewsWidget ();
    //  views_->setFrameStyle(QFrame::Panel | QFrame::Raised);
    //  views_->setLineWidth(frame_width);
    //  hlayout->addWidget (views_);
    //  views_->update();

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

void ManagementWidget::startSlot ()
{
    logdbg  << "ManagementWidget: startSlot";

    ATSDB::instance().objectManager().object("Radar").load();
}
