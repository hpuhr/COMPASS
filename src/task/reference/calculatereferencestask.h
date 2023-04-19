#ifndef CALCULATEREFERENCESTASK_H
#define CALCULATEREFERENCESTASK_H

#include "configurable.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/dbcontentcache.h"
#include "task.h"
#include "global.h"

#include <QObject>

#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class CalculateReferencesTaskDialog;
class CalculateReferencesStatusDialog;
class CalculateReferencesJob;

// delete from data_reftraj;

class CalculateReferencesTask : public Task, public Configurable
{
    Q_OBJECT

public slots:
    void dialogRunSlot();
    void dialogCancelSlot();

    void createDoneSlot();
    void createObsoleteSlot();

    void loadedDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

    void calculationStatusSlot(QString status);

    void closeStatusDialogSlot();

public:
    CalculateReferencesTask(const std::string& class_id, const std::string& instance_id,
                            TaskManager& task_manager);
    virtual ~CalculateReferencesTask();

    CalculateReferencesTaskDialog* dialog();

    virtual bool canRun() override;
    virtual void run() override;

protected:

    std::unique_ptr<CalculateReferencesTaskDialog> dialog_;

    std::unique_ptr<CalculateReferencesStatusDialog> status_dialog_;

    std::shared_ptr<dbContent::Cache> cache_;

    std::map<std::string, std::shared_ptr<Buffer>> data_;

    std::shared_ptr<CalculateReferencesJob> create_job_;
    bool create_job_done_{false};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name);
};

#endif // CALCULATEREFERENCESTASK_H
