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
class ViewableDataConfig;

// delete from data_reftraj;

struct CalculateReferencesTaskSettings
{
    enum class ReconstructorType
    {
        UMKalman2D = 0,
        AMKalman2D
    };

    double        R_std                 = 30.0;     // observation noise (standard)
    double        R_std_high            = 1000.0;   // observation noise (high)
    double        Q_std                 = 30.0;     // process noise
    double        P_std                 = 30.0;     // system noise (standard)
    double        P_std_high            = 1000.0;   // system noise (high)

    double        min_dt                = 0.0;      // minimum allowed timestep in seconds
    double        max_dt                = 30.0;     // maximum allowed timestep in seconds
    int           min_chain_size        = 2;        // minimum kalman chain size

    bool          use_vel_mm            = true;     // track velocities in measurements
    bool          smooth_rts            = true;     // enable RTS smoother

    bool          resample_systracks    = true;     // resample system tracks using spline interpolation
    double        resample_systracks_dt = 1.0;      // resample interval in seconds

    bool          resample_result       = true;     // resample (and interpolate) reconstructor result by a fixed time interval
    double        resample_result_dt    = 2.0;      // result resampling time interval in seconds

    bool          verbose               = false;    // reconstruction verbosity
    bool          generate_viewpoints   = false;    // generate viewpoints and add to viewpoints list
    bool          python_compatibility  = false;    // if true settings may be overriden to make rec compatible with python version

    ReconstructorType rec_type = ReconstructorType::UMKalman2D;
};

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
    enum class ViewPointMode
    {
        None = 0,            // no viewpoints are generated
        GenerateViewPoints,  // viewpoints are generated for each target and added to the list of viewpoints
        SetFirst             // a viewpoint is generated for the first target and set as the active viewable
    };

    CalculateReferencesTask(const std::string& class_id, 
                            const std::string& instance_id,
                            TaskManager& task_manager);
    virtual ~CalculateReferencesTask();

    CalculateReferencesTaskDialog* dialog();

    virtual bool canRun() override;
    virtual void run() override;

    void runUTN(unsigned int utn);

    CalculateReferencesTaskSettings& settings() { return settings_; }
    const CalculateReferencesTaskSettings& settings() const { return settings_; }

    bool generateViewPoints() const;
    bool writeReferences() const;
    bool closeDialogAfterFinishing() const;

protected:
    std::unique_ptr<CalculateReferencesTaskDialog>   dialog_;
    std::unique_ptr<CalculateReferencesStatusDialog> status_dialog_;
    std::unique_ptr<ViewableDataConfig>              viewable_;

    // calculate only utns list

    std::shared_ptr<dbContent::Cache> cache_;

    std::map<std::string, std::shared_ptr<Buffer>> data_;

    std::shared_ptr<CalculateReferencesJob> create_job_;
    bool create_job_done_{false};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name);

private:
    CalculateReferencesTaskSettings settings_;

    std::vector<unsigned int> utns_; // utns to reconstruct - if empty all utns are reconstructed
};

#endif // CALCULATEREFERENCESTASK_H
