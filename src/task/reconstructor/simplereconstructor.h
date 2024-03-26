#pragma once

#include <QObject>

#include "reconstructorbase.h"
#include "targetreportdefs.h"
#include "global.h"
#include "reconstructortarget.h"
#include "simpleassociator.h"
#include "simpleaccuracyestimator.h"
#include "simplereferencecalculator.h"

class SimpleReconstructorSettings
{
  public:
    SimpleReconstructorSettings() {};

    bool associate_non_mode_s_ {true};
    //bool clean_dubious_utns_ {true};
    //bool mark_dubious_utns_unused_ {false};
    //bool comment_dubious_utns_ {true};

    // tracker stuff
    double max_time_diff_tracker_ {15.0};

    double max_distance_quit_tracker_ {10*NM2M}; //10nm in meters // kb 5
    double max_distance_dubious_tracker_ {3*NM2M}; //kb 2.5? 2.5 lowest
    unsigned int max_positions_dubious_tracker_ {5};

    double max_distance_acceptable_tracker_ {NM2M/2.0};
    double max_altitude_diff_tracker_ {300.0};

    unsigned int min_updates_tracker_ {2}; // kb 3!!!

    double prob_min_time_overlap_tracker_ {0.5}; //kb 0.7

    //double max_speed_tracker_kts_ {100000};

    double cont_max_time_diff_tracker_ {30.0};
    double cont_max_distance_acceptable_tracker_ {1852.0};

    // sensor
    double max_time_diff_sensor_ {15.0};
    double max_distance_acceptable_sensor_ {2*NM2M};
    double max_altitude_diff_sensor_ {300.0};

    // other, not registered
    std::set<unsigned int> mode_a_conspicuity_codes_ {512, 1024}; // decimal, oct 1000, 2000
};

class SimpleReconstructorWidget;

class SimpleReconstructor : public QObject, public ReconstructorBase
{
    Q_OBJECT

  signals:
    void updateWidgetsSignal();

  public:
    SimpleReconstructor(const std::string& class_id, const std::string& instance_id,
                        ReconstructorTask& task);
    virtual ~SimpleReconstructor();

    virtual dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name) const override;

    virtual void reset() override;

    SimpleReconstructorSettings& settings();

    SimpleReconstructorWidget* widget(); // ownage by caller

    void updateWidgets();

  protected:
  
    friend class dbContent::ReconstructorTarget;
    friend class SimpleAssociator;
    friend class SimpleReferenceCalculator;

    SimpleReconstructorSettings settings_;
    SimpleAssociator associatior_;
    SimpleAccuracyEstimator acc_estimator_;
    SimpleReferenceCalculator ref_calculator_;

    std::map<unsigned long, dbContent::targetReport::ReconstructorInfo> target_reports_;
    // all sources, record_num -> base info
    std::multimap<boost::posix_time::ptime, unsigned long> tr_timestamps_;
    // all sources sorted by time, ts -> record_num
    std::map<unsigned int, std::map<unsigned int, std::map<unsigned int, std::vector<unsigned long>>>> tr_ds_;
    // dbcontent id -> ds_id -> line id -> record_num, sorted by ts

    std::map<unsigned int, dbContent::ReconstructorTarget> targets_; // utn -> tgt

    //std::map<unsigned int, std::map<unsigned long, std::tuple<unsigned int>>> associations_;
    // dbcontent if -> rec_num -> utn, no src rec_nums (dbcontent, rec_num)

    virtual bool processSlice_impl() override;

    void clearOldTargetReports();
    void createTargetReports();

    std::map<unsigned int, std::map<unsigned long, unsigned int>> createAssociations();
    void saveAssociations(std::map<unsigned int, std::map<unsigned long, unsigned int>> associations);
    void saveTargets();
};

