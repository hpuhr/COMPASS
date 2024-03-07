#pragma once

#include "configurable.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/dbcontentaccessor.h"
#include "task.h"
#include "global.h"


#include <QObject>

#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class ReconstructorTaskDialog;
class DBContent;
class Buffer;
class ReconstructorTaskJob;

namespace dbContent
{
class Variable;
class MetaVariable;
}


class ReconstructorTask : public Task, public Configurable
{
    Q_OBJECT

  public slots:
    void dialogRunSlot();
    void dialogCancelSlot();

    void closeStatusDialogSlot();

  public:
    ReconstructorTask(const std::string& class_id, const std::string& instance_id,
                      TaskManager& task_manager);
    virtual ~ReconstructorTask();


    ReconstructorTaskDialog* dialog();

    virtual bool canRun() override;
    virtual void run() override;

  protected:

    std::unique_ptr<ReconstructorTaskDialog> dialog_;

    //std::unique_ptr<CreateAssociationsStatusDialog> status_dialog_;

    std::shared_ptr<dbContent::DBContentAccessor> accessor_;

    std::map<std::string, std::shared_ptr<Buffer>> data_;

    std::shared_ptr<ReconstructorTaskJob> job_;
    bool job_done_{false};

};

