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

class ViewPointGenVP;
class ViewPointGenAnnotation;

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

public:
    struct DebugSettings
    {
        bool debug_ {false};
        std::set<unsigned int> debug_utns_;
        std::set<unsigned long> debug_rec_nums_;

        boost::posix_time::ptime debug_timestamp_min_;
        boost::posix_time::ptime debug_timestamp_max_;

        bool debug_accuracy_estimation_ {false};

        bool debugUTN(unsigned int utn) { return debug_utns_.count(utn); }
        bool debugRecNum(unsigned long rec_num) { return debug_rec_nums_.count(rec_num); }
    };

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

    void runCancelledSlot();

    void updateProgressSlot(const QString& msg, bool add_slice_progress);

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

    bool useDStype(const std::string& ds_type) const;
    void useDSType(const std::string& ds_type, bool value);
    bool useDataSource(unsigned int ds_id) const;
    void useDataSource(unsigned int ds_id, bool value);
    bool useDataSourceLine(unsigned int ds_id, unsigned int line_id) const;
    void useDataSourceLine(unsigned int ds_id, unsigned int line_id, bool value);

    std::set<unsigned int> unusedDSIDs() const;
    std::map<unsigned int, std::set<unsigned int>> unusedDSIDLines() const;

    ReconstructorBase::DataSlice& processingSlice();

    ViewPointGenVP* getDebugViewpoint(const std::string& name, const std::string& type, bool* created = nullptr) const;
    ViewPointGenVP* getDebugViewpointNoData(const std::string& name, const std::string& type); // w/o sur data
    ViewPointGenVP* getDebugViewpointForUTN(unsigned long utn, const std::string& name_prefix="") const;
    ViewPointGenAnnotation* getDebugAnnotationForUTNSlice(unsigned long utn, size_t slice_idx) const;
    void saveDebugViewPoints();

    bool skipReferenceDataWriting() const;
    void skipReferenceDataWriting(bool newSkip_reference_data_writing);

    DebugSettings& debugSettings() { return debug_settings_; }

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
    boost::posix_time::ptime run_start_time_after_del_;

    size_t current_slice_idx_ = 0;

    std::unique_ptr<ReconstructorBase::DataSlice> loading_slice_;
    bool loading_data_ {false};
    std::unique_ptr<ReconstructorBase::DataSlice> processing_slice_;
    std::unique_ptr<ReconstructorBase::DataSlice> writing_slice_;

    DebugSettings debug_settings_;

    std::future<void> delcalcref_future_;
    std::future<void> deltgts_future_;
    std::future<void> delassocs_future_;
    std::future<void> process_future_;
    bool processing_data_slice_ {false};
    bool cancelled_ {false};

    bool skip_reference_data_writing_ {false};

    mutable std::map<std::pair<std::string,std::string>, std::unique_ptr<ViewPointGenVP>> debug_viewpoints_;

    virtual void checkSubConfigurables() override;
    void deleteCalculatedReferences();

    void loadDataSlice();
    void processDataSlice();
    void writeDataSlice();

};
