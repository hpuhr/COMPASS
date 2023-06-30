#ifndef CALCULATEREFERENCESTASK_H
#define CALCULATEREFERENCESTASK_H

#include "configurable.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/dbcontentcache.h"
#include "task.h"
//#include "global.h"
#include "reconstruction/reconstructor_defs.h"

#include <QObject>

#include <memory>

//#include "boost/date_time/posix_time/posix_time.hpp"

class CalculateReferencesTaskDialog;
class CalculateReferencesStatusDialog;
class CalculateReferencesJob;
class ViewableDataConfig;

// delete from data_reftraj;

struct CalculateReferencesTaskSettings
{
    enum ReconstructorType
    {
        Rec_UMKalman2D = 0,
        Rec_AMKalman2D
    };

    typedef reconstruction::MapProjectionMode MapProjectionMode;

    ReconstructorType rec_type      = ReconstructorType::Rec_UMKalman2D;
    MapProjectionMode map_proj_mode = MapProjectionMode::MapProjectDynamic;

    //default uncertainties
    double        R_std                 = 30.0;     // observation noise (standard)
    double        R_std_high            = 1000.0;   // observation noise (high)
    double        Q_std                 = 30.0;     // process noise
    double        P_std                 = 30.0;     // system noise (standard)
    double        P_std_high            = 1000.0;   // system noise (high)

    //default sensor specific uncertainties
    bool          use_R_std_cat021      = true;     //use adsb specific sensor noise
    double        R_std_pos_cat021      = 30.0;     //position observation noise adsb
    double        R_std_vel_cat021      = 50.0;     //velocity observation noise adsb
    double        R_std_acc_cat021      = 50.0;     //acceleration observation noise adsb

    bool          use_R_std_cat062      = true;     //use systrack specific sensor noise
    double        R_std_pos_cat062      = 30.0;     //position observation noise systracks
    double        R_std_vel_cat062      = 50.0;     //velocity observation noise systracks
    double        R_std_acc_cat062      = 50.0;     //acceleration observation noise systracks

    //chain related
    double        min_dt                = 0.0;      // minimum allowed timestep in seconds
    double        max_dt                = 11.0;     // maximum allowed timestep in seconds
    int           min_chain_size        = 2;        // minimum kalman chain size

    //systrack resampling related
    bool          resample_systracks        = true; // resample system tracks using spline interpolation
    double        resample_systracks_dt     = 1.0;  // resample interval in seconds
    double        resample_systracks_max_dt = 30.0; // maximum timestep to interpolate

    //result resampling related
    bool          resample_result       = true;     // resample (and interpolate) reconstructor result by a fixed time interval
    double        resample_result_dt    = 2.0;      // result resampling time interval in seconds
    double        resample_result_Q_std = 10.0;     // process noise used in result resampling

    //additional options
    bool          use_vel_mm            = true;     // track velocities in measurements
    bool          smooth_rts            = true;     // enable RTS smoother
    bool          verbose               = false;    // reconstruction verbosity
    bool          generate_viewpoints   = false;    // generate viewpoints and add to viewpoints list
    bool          python_compatibility  = false;    // if true settings may be overriden to make rec compatible with python version

    bool use_tracker_data {true};
    nlohmann::json data_sources_tracker;            // map, ds_id str -> active flag, true if not contained

    bool use_adsb_data {true};
    nlohmann::json data_sources_adsb;               // map, ds_id str -> active flag, true if not contained

    bool filter_position_usage {true};

    // tracker position usage
    bool tracker_only_confirmed_positions {true}; // non-tentative
    bool tracker_only_noncoasting_positions {true};
    bool tracker_only_report_detection_positions {false}; // no no detection
    bool tracker_only_report_detection_nonpsronly_positions {true}; // no mono + psr det
    bool tracker_only_high_accuracy_postions {true};
    float tracker_minimum_accuracy {30}; // m

    // adsb position usage
    bool adsb_only_v12_positions {true};

    bool adsb_only_high_nucp_nic_positions {false};
    unsigned int adsb_minimum_nucp_nic {4};

    bool adsb_only_high_nacp_positions {true};
    unsigned int adsb_minimum_nacp {4};

    bool adsb_only_high_sil_positions {false};
    unsigned int adsb_minimum_sil {1};

    // output
    std::string ds_name;
    unsigned int ds_sac {0};
    unsigned int ds_sic {1};
    unsigned int ds_line {0};
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

    bool useTrackerData() const;
    void useTrackerData(bool value);

    std::map<std::string, bool> trackerDataSources();
    void trackerDataSources(std::map<std::string, bool> sources);

    bool useADSBData() const;
    void useADSBData(bool value);

    std::map<std::string, bool> adsbDataSources();
    void adsbDataSources(std::map<std::string, bool> sources);

    bool anySourcesActive(const std::string& ds_type, const nlohmann::json& sources);
    std::string getActiveDataSources(const std::string& ds_type, const nlohmann::json& sources);

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
