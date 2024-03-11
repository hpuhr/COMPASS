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
class ReconstructorBase;

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

    void loadedDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

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

    std::unique_ptr<ReconstructorBase> reconstructor_; // has to be reset after each calculation

    bool job_done_{false};

    void deleteCalculatedReferences();

    void loadDataSlice();

};

