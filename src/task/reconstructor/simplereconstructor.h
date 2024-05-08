#pragma once

#include <QObject>

#include "reconstructorbase.h"
#include "targetreportdefs.h"
#include "global.h"
#include "reconstructortarget.h"
#include "simpleassociator.h"
#include "simpleaccuracyestimator.h"
#include "simplereferencecalculator.h"

class SimpleReconstructorSettings : public ReconstructorBaseSettings
{
  public:
    SimpleReconstructorSettings() {};

    //bool associate_non_mode_s_ {true};

    double max_distance_quit_ {10*NM2M}; //10nm in meters // kb 5
    double max_distance_dubious_ {3*NM2M}; //kb 2.5? 2.5 lowest
    double max_distance_acceptable_ {1*NM2M};

    //unsigned int max_positions_dubious_ {5};

    //double max_altitude_diff_ {300.0};

    //unsigned int min_updates_ {3}; // kb 3!!!

    //double prob_min_time_overlap_ {0.5}; //kb 0.7

//    double cont_max_time_diff_ {30.0};
//    double cont_max_distance_acceptable_ {1*NM2M};

    // sensor
//    double max_time_diff_sensor_ {15.0};
//    double max_distance_acceptable_sensor_ {2*NM2M};
//    double max_altitude_diff_sensor_ {300.0};

    // other, not registered
   // std::set<unsigned int> mode_a_conspicuity_codes_ {512, 1024}; // decimal, oct 1000, 2000
};

class SimpleReconstructorWidget;

class SimpleReconstructor : public QObject, public ReconstructorBase
{
    Q_OBJECT

  signals:
    void updateWidgetsSignal();

  public:
    SimpleReconstructor(const std::string& class_id, const std::string& instance_id,
                        ReconstructorTask& task, std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator);
    virtual ~SimpleReconstructor();

    virtual dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name) const override;

    virtual void reset() override;

    virtual SimpleReconstructorSettings& settings() override;

    SimpleReconstructorWidget* widget(); // ownage by caller

    virtual void updateWidgets() override;

  protected:
  
    friend class dbContent::ReconstructorTarget;
    friend class SimpleAssociator;
    friend class SimpleReferenceCalculator;

    SimpleReconstructorSettings settings_;
    SimpleAssociator associatior_;
    SimpleReferenceCalculator ref_calculator_;

    virtual void processSlice_impl() override;
};

