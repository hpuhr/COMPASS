/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * WorkerThreadWidget.cpp
 *
 *  Created on: Apr 12, 2012
 *      Author: sk
 */

#include <QLabel>
#include <QVBoxLayout>
#include <sstream>
#include "WorkerThreadWidget.h"
#include "WorkerThreadManager.h"
#include "WorkerThread.h"
#include "Logger.h"
#include "String.h"

using namespace Utils::String;

WorkerThreadWidget::WorkerThreadWidget()  : QFrame (), manager_ (WorkerThreadManager::getInstance()),
 timer_id_ (0)
{
  logdbg  << "WorkerThreadWidget: constructor: start";
  //manager_.setWidget(this);

  num_workers_ = manager_.getNumWorkers();

  createWidgets ();
  logdbg  << "WorkerThreadWidget: constructor: end";

  timer_id_=startTimer(250);
}

WorkerThreadWidget::~WorkerThreadWidget()
{
  if (timer_id_ != 0)
    killTimer (timer_id_);
  timer_id_=0;
}

void WorkerThreadWidget::createWidgets ()
{
  QFont font_bold;
  font_bold.setBold(true);

  QFont font_big;
  font_big.setPointSize(18);

  QVBoxLayout *layout = new QVBoxLayout();

  QLabel *head = new QLabel (tr("Transformation"));
  head->setFont (font_big);
  layout->addWidget(head);

  std::string threads_info = "Number of workers: <b>" + intToString(num_workers_)+"</b>";
  QLabel *number= new QLabel (tr(threads_info.c_str()));
  layout->addWidget(number);

  std::string jobs_info = "Number of jobs: <b>0</b>";
  jobs_= new QLabel (tr(jobs_info.c_str()));
  layout->addWidget(jobs_);

  QGridLayout *threads_layout = new QGridLayout ();

  unsigned int row=0;
  for (unsigned int cnt=0; cnt < num_workers_; cnt ++)
  {
    QLabel *label_space = new QLabel (tr(""));
    threads_layout->addWidget(label_space, row, 0);
    row++;

    std::string name;
    if (cnt == 0)
      name = "<b>DB Thread</b>";
    else
      name = "<b>Thread " + intToString(cnt)+"</b>";

    QLabel *label_name = new QLabel (tr(name.c_str()));
    threads_layout->addWidget(label_name, row, 0);
    row++;

//    QLabel *state1 = new QLabel (tr("State"));
//    threads_layout->addWidget(state1, row, 0);
//
//    QLabel *state2 = new QLabel (tr("IDLE"));
//    threads_layout->addWidget(state2, row, 1);
//    thread_states_.push_back(state2);
//    row++;

    QLabel *working1 = new QLabel (tr("Working"));
    threads_layout->addWidget(working1, row, 0);

    QLabel *working2 = new QLabel (tr("0%"));
    threads_layout->addWidget(working2, row, 1);
    thread_working_.push_back(working2);
    row++;

    QLabel *idle1 = new QLabel (tr("Idle"));
    threads_layout->addWidget(idle1, row, 0);

    QLabel *idle2 = new QLabel (tr("100.00%"));
    threads_layout->addWidget(idle2, row, 1);
    thread_idle_.push_back(idle2);
    row++;

    QLabel *jobs1 = new QLabel (tr("Jobs"));
    threads_layout->addWidget(jobs1, row, 0);

    QLabel *jobs2 = new QLabel (tr("0"));
    threads_layout->addWidget(jobs2, row, 1);
    thread_jobs_.push_back(jobs2);
    row++;

  }

  layout->addLayout (threads_layout);

  layout->addStretch();
  setLayout (layout);
}

void WorkerThreadWidget::update ()
{
  std::string jobs_info = "Number of jobs: <b>" + intToString(manager_.getJobNumber())+"</b>";
  jobs_->setText (tr(jobs_info.c_str()));

  for (unsigned int cnt=0; cnt < num_workers_; cnt ++)
  {
    WorkerThread *worker = manager_.getWorker(cnt);
    std::pair <double, double> usage = worker->getUsage();
    //bool state = worker->getWorking();

    double sum = usage.first + usage.second;

//    if (state)
//      thread_states_.at(cnt)->setText("Working");
//    else
//      thread_states_.at(cnt)->setText("Idle");

    std::string working = percentToString(usage.first/sum * 100.0)+"%";
    thread_working_.at(cnt)->setText(tr(working.c_str()));
    std::string idle = percentToString(usage.second/sum * 100.0)+"%";
    thread_idle_.at(cnt)->setText(tr(idle.c_str()));
    thread_jobs_.at(cnt)->setText(tr(intToString(worker->getJobNumber()).c_str()));
  }
}

void WorkerThreadWidget::timerEvent (QTimerEvent *e)
{
  logdbg  << "WorkerThreadWidget: timerEvent";
  update();
}
