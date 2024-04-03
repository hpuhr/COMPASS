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
class SimpleReconstructor;
class ProbIMMReconstructor;

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

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    ReconstructorTaskDialog* dialog();

    virtual bool canRun() override;
    virtual void run() override;

    std::string currentReconstructorStr() const;
    void currentReconstructorStr(const std::string& value);

    static const std::string ScoringUMReconstructorName;
    static const std::string ProbImmReconstructorName;

    ReconstructorBase* currentReconstructor() const;
    SimpleReconstructor* simpleReconstructor() const;
    ProbIMMReconstructor* probIMMReconstructor() const;

  protected:

    std::string current_reconstructor_str_;

    std::unique_ptr<ReconstructorTaskDialog> dialog_;

    //std::unique_ptr<CreateAssociationsStatusDialog> status_dialog_;

    std::shared_ptr<dbContent::DBContentAccessor> accessor_;

    std::map<std::string, std::shared_ptr<Buffer>> data_;

    std::unique_ptr<SimpleReconstructor> simple_reconstructor_; // has to be reset after each calculation
    std::unique_ptr<ProbIMMReconstructor> probimm_reconstructor_; // has to be reset after each calculation

    //bool job_done_{false};

    virtual void checkSubConfigurables() override;
    void deleteCalculatedReferences();

    void loadDataSlice();

};

