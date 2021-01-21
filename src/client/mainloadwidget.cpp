/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "mainloadwidget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QVBoxLayout>

#include "compass.h"
#include "dbinterface.h"
#include "dbinterfaceinfowidget.h"
#include "dbobjectmanager.h"
#include "dbobjectmanagerloadwidget.h"
#include "filtermanager.h"
#include "filtermanagerwidget.h"
#include "global.h"
#include "jobmanager.h"
#include "jobmanagerwidget.h"
#include "logger.h"
#include "viewmanager.h"
#include "viewmanagerwidget.h"

MainLoadWidget::MainLoadWidget() : QWidget()
{
    QVBoxLayout* vlayout = new QVBoxLayout();
    QHBoxLayout* hlayout = new QHBoxLayout();

    unsigned int frame_width = FRAME_SIZE;

    vlayout->addLayout(hlayout);

    QVBoxLayout* left_layout = new QVBoxLayout();

    DBInterfaceInfoWidget* interface_widget = COMPASS::instance().interface().infoWidget();
    interface_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    interface_widget->setLineWidth(frame_width);
    left_layout->addWidget(interface_widget);

    DBObjectManagerLoadWidget* objman_widget = COMPASS::instance().objectManager().loadWidget();
    objman_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    objman_widget->setLineWidth(frame_width);
    left_layout->addWidget(objman_widget);

    hlayout->addLayout(left_layout, 1);

    FilterManagerWidget* filman_widget = COMPASS::instance().filterManager().widget();
    filman_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    filman_widget->setLineWidth(frame_width);
    hlayout->addWidget(filman_widget, 2);

    //QVBoxLayout* right_layout = new QVBoxLayout();

//    ViewManagerWidget* viewman_widget = COMPASS::instance().viewManager().widget();
//    viewman_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
//    viewman_widget->setLineWidth(frame_width);
//    right_layout->addWidget(viewman_widget);

//    JobManagerWidget* jobman_widget = JobManager::instance().widget();
//    jobman_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
//    jobman_widget->setLineWidth(frame_width);
//    right_layout->addWidget(jobman_widget);

    //hlayout->addLayout(right_layout, 1);

    setLayout(vlayout);
}

MainLoadWidget::~MainLoadWidget() {}

void MainLoadWidget::resizeEvent(QResizeEvent* event)
{
    logdbg << "ManagementWidget: resizeEvent";
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
