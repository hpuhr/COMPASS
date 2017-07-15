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

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>

#include "jobmanagerwidget.h"
#include "jobmanager.h"
#include "atsdb.h"
#include "global.h"

JobManagerWidget::JobManagerWidget(JobManager &job_manager)
    : job_manager_(job_manager), info_layout_(nullptr)
{
    unsigned int frame_width = FRAME_SIZE;

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(frame_width);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Jobs");
    main_label->setFont (font_bold);
    main_layout->addWidget (main_label);

    QGridLayout *grid = new QGridLayout ();

    grid->addWidget(new QLabel("DB Jobs"), 0, 0);
    num_dbjobs_label_ = new QLabel ("?");
    num_dbjobs_label_->setAlignment(Qt::AlignRight);
    grid->addWidget(num_dbjobs_label_, 0, 1);

    grid->addWidget(new QLabel("Jobs"), 1, 0);

    num_jobs_label_ = new QLabel ("?");
    num_jobs_label_->setAlignment(Qt::AlignRight);
    grid->addWidget(num_jobs_label_, 1, 1);

    grid->addWidget(new QLabel("Threads"), 2, 0);

    num_threads_label_ = new QLabel ("?");
    num_threads_label_->setAlignment(Qt::AlignRight);
    grid->addWidget(num_threads_label_, 2, 1);

    main_layout->addLayout(grid);

    info_layout_ = new QVBoxLayout ();

    updateSlot();

    main_layout->addLayout(info_layout_);
    main_layout->addStretch();

    setLayout (main_layout);
}

JobManagerWidget::~JobManagerWidget()
{
}


void JobManagerWidget::updateSlot ()
{
    logdbg << "JobManagerWidget: updateSlot";

    assert (num_dbjobs_label_);
    assert (num_jobs_label_);
    assert (num_threads_label_);

    num_dbjobs_label_->setText(QString::number(job_manager_.numDBJobs()));
    num_jobs_label_->setText(QString::number(job_manager_.numJobs()));
    num_threads_label_->setText(QString::number(job_manager_.numThreads()));

//    QLayoutItem* item;
//    while ((item = info_layout_->takeAt(0)) != nullptr)
//    {
//        info_layout_->removeItem(item);
//    }

//    for (auto object : object_manager_.objects())
//    {
//        info_layout_->addWidget(object.second->infoWidget());
//    }
}
