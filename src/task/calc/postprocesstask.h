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

#ifndef POSTPROCESSTASK_H
#define POSTPROCESSTASK_H

#include <QObject>
#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "configurable.h"
#include "job.h"
#include "task.h"

class TaskManager;
class PostProcessTaskWidget;
class QProgressDialog;

class PostProcessTask : public Task, public Configurable
{
    Q_OBJECT

  public slots:
    void postProcessingJobDoneSlot();

  public:
    PostProcessTask(const std::string& class_id, const std::string& instance_id,
                    TaskManager& task_manager);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual TaskWidget* widget();
    virtual void deleteWidget();

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired() { return true; }

    virtual bool canRun();
    void run();

    static const std::string DONE_PROPERTY_NAME;

  protected:
    std::unique_ptr<PostProcessTaskWidget> widget_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    std::vector<std::shared_ptr<Job>> postprocess_jobs_;
    QProgressDialog* postprocess_dialog_{nullptr};
    size_t postprocess_job_num_{0};

    virtual void checkSubConfigurables() {}
};

#endif  // POSTPROCESSTASK_H
