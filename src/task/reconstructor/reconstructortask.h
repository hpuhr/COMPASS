#pragma once

#include "configurable.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/dbcontentaccessor.h"
#include "reconstructorbase.h"
#include "task.h"
#include "global.h"
#include "json.hpp"


#include <QObject>

#include <memory>
#include <future>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class ReconstructorTaskDialog;
class DBContent;
class Buffer;
class ReconstructorBase;
class SimpleReconstructor;

#if USE_EXPERIMENTAL_SOURCE == true
class ProbIMMReconstructor;
#endif

namespace dbContent
{
class Variable;
class MetaVariable;
}

class QProgressDialog;

class ReconstructorTask : public Task, public Configurable
{
    Q_OBJECT

  public slots:
    void dialogRunSlot();
    void dialogCancelSlot();

    void deleteCalculatedReferencesDoneSlot();
    void deleteTargetsDoneSlot();
    void deleteAssociationsDoneSlot();

    void loadedDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

    void processingDoneSlot();
    void writeDoneSlot();

    void runCancelSlot();

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

    ReconstructorBase* currentReconstructor() const;
    SimpleReconstructor* simpleReconstructor() const;

#if USE_EXPERIMENTAL_SOURCE == true
    static const std::string ProbImmReconstructorName;

    ProbIMMReconstructor* probIMMReconstructor() const;
#endif

    std::set<unsigned int> disabledDataSources() const;

    const std::set<unsigned int>& debugUTNs() const;
    void debugUTNs(const std::set<unsigned int>& utns);

    const std::set<unsigned long>& debugRecNums() const;
    void debugRecNums(const std::set<unsigned long>& rec_nums);

    bool useDStype(const std::string& ds_type) const;
    void useDSType(const std::string& ds_type, bool value);
    bool useDataSource(unsigned int ds_id) const;
    void useDataSource(unsigned int ds_id, bool value);
    bool useDataSourceLine(unsigned int ds_id, unsigned int line_id) const;
    void useDataSourceLine(unsigned int ds_id, unsigned int line_id, bool value);

    std::set<unsigned int> unusedDSIDs() const;
    std::map<unsigned int, std::set<unsigned int>> unusedDSIDLines() const;

    ReconstructorBase::DataSlice& processingSlice();

  protected:
    std::string current_reconstructor_str_;

    nlohmann::json use_dstypes_; // dstype -> bool
    nlohmann::json use_data_sources_; // ds_id -> bool
    nlohmann::json use_data_sources_lines_; // ds_id -> line_id -> bool

    std::unique_ptr<ReconstructorTaskDialog> dialog_;

    std::unique_ptr<SimpleReconstructor> simple_reconstructor_; // has to be reset after each calculation

#if USE_EXPERIMENTAL_SOURCE == true
    std::unique_ptr<ProbIMMReconstructor> probimm_reconstructor_; // has to be reset after each calculation
#endif

    std::unique_ptr<QProgressDialog> progress_dialog_;
    boost::posix_time::ptime run_start_time_;

    size_t current_slice_idx_ = 0;

    std::unique_ptr<ReconstructorBase::DataSlice> loading_slice_;
    std::unique_ptr<ReconstructorBase::DataSlice> processing_slice_;
    std::unique_ptr<ReconstructorBase::DataSlice> writing_slice_;

    std::set<unsigned int> debug_utns_;
    std::set<unsigned long> debug_rec_nums_;

    std::future<void> delcalcref_future_;
    std::future<void> deltgts_future_;
    std::future<void> delassocs_future_;
    std::future<void> process_future_;
    bool processing_data_slice_ {false};
    bool cancelled_ {false};

    virtual void checkSubConfigurables() override;
    void deleteCalculatedReferences();

    void loadDataSlice();
    void processDataSlice();
    void writeDataSlice();

    void updateProgress(const QString& msg, bool add_slice_progress);
};
