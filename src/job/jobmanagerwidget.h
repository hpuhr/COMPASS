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

#ifndef JOBMANAGERWIDGET_H_
#define JOBMANAGERWIDGET_H_

#include <QFrame>
#include <map>

class Job;
class JobManager;
class QLabel;
class QVBoxLayout;


/**
 * @brief Shows all DBObjects, allows editing and adding new ones
 */
class JobManagerWidget : public QFrame
{
    Q_OBJECT

public slots:
    void updateSlot ();

public:
    /// @brief Constructor
    JobManagerWidget(JobManager &job_manager);
    /// @brief Destructor
    virtual ~JobManagerWidget();

private:
    JobManager &job_manager_;

    QLabel *num_jobs_label_;
    QLabel *num_dbjobs_label_;
    QLabel *num_threads_label_;

    QVBoxLayout *info_layout_;
};

#endif /* DBOBJECTMANAGERINFOWIDGET_H_ */
