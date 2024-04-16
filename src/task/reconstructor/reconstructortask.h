#pragma once

#include "configurable.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/dbcontentaccessor.h"
#include "task.h"
#include "global.h"
#include "json.hpp"


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

class QProgressDialog;

class ReconstructorTask : public Task, public Configurable
{
    Q_OBJECT

  public slots:
    void dialogRunSlot();
    void dialogCancelSlot();

    void loadedDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

    bool useDStype(const std::string& ds_type) const;
    void useDSType(const std::string& ds_type, bool value);
    bool useDataSource(unsigned int ds_id) const;
    void useDataSource(unsigned int ds_id, bool value);
    bool useDataSourceLine(unsigned int ds_id, unsigned int line_id) const;
    void useDataSourceLine(unsigned int ds_id, unsigned int line_id, bool value);

    std::set<unsigned int> unusedDSIDs() const;
    std::map<unsigned int, std::set<unsigned int>> unusedDSIDLines() const;

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

    std::set<unsigned int> disabledDataSources() const;

    const std::set<unsigned int>& debugUTNs() const;
    void debugUTNs(const std::set<unsigned int>& utns);

    std::set<unsigned long> debugRecNums() const;
    void debugRecNums(const std::set<unsigned long>& rec_nums);

  protected:
    std::string current_reconstructor_str_;

    nlohmann::json use_dstypes_; // dstype -> bool
    nlohmann::json use_data_sources_; // ds_id -> bool
    nlohmann::json use_data_sources_lines_; // ds_id -> line_id -> bool

    std::unique_ptr<ReconstructorTaskDialog> dialog_;

    std::shared_ptr<dbContent::DBContentAccessor> accessor_;

    std::map<std::string, std::shared_ptr<Buffer>> data_;

    std::unique_ptr<SimpleReconstructor> simple_reconstructor_; // has to be reset after each calculation
    std::unique_ptr<ProbIMMReconstructor> probimm_reconstructor_; // has to be reset after each calculation

    std::unique_ptr<QProgressDialog> progress_dialog_;
    boost::posix_time::ptime run_start_time_;

    size_t current_slice_idx_ = 0;

    std::set<unsigned int> debug_utns_;
    std::set<unsigned long> debug_rec_nums_;

    virtual void checkSubConfigurables() override;
    void deleteCalculatedReferences();

    void loadDataSlice();

    void updateProgress(const QString& msg, bool add_slice_progress);
};
