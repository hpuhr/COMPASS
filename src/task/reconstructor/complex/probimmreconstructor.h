#pragma once

#include <QObject>

#include "reconstructorbase.h"
#include "probabilisticassociator.h"
#include "accuracyestimatorbase.h"
#include "simplereferencecalculator.h"

class ProbIMMReconstructorSettings
{
  public:
    ProbIMMReconstructorSettings() {};

    float max_time_diff_ {15}; // sec
    float track_max_time_diff_ {300.0}; // sec

    float max_altitude_diff_ {300.0};

    float max_mahalanobis_sec_verified_dist_ {10.0};
    float max_mahalanobis_sec_unknown_dist_ {5.0};
    float max_tgt_est_std_dev_  {1000.0};

    float max_sum_est_std_dev_ {2000.0};
    float min_sum_est_std_dev_ {50.0};

    // compare targets related
    double       prob_min_time_overlap_tracker_       {0.1};
    unsigned int min_updates_tracker_                   {5};
    const double max_positions_dubious_verified_rate_ {0.5};
    const double max_positions_dubious_unknown_rate_  {0.3};

    float max_mahalanobis_quit_verified_       {12};
    float max_mahalanobis_dubious_verified_     {5};
    float max_mahalanobis_acceptable_verified_  {3};

    float max_mahalanobis_quit_unverified_       {8};
    float max_mahalanobis_dubious_unverified_    {4};
    float max_mahalanobis_acceptable_unverified_ {2};

    //ref calculation
    SimpleReferenceCalculator::Settings ref_calc_settings_;
};


class ProbIMMReconstructorWidget;

class ProbIMMReconstructor : public QObject, public ReconstructorBase
{
    Q_OBJECT

  signals:
    void updateWidgetsSignal();

  public:
    ProbIMMReconstructor(const std::string& class_id, const std::string& instance_id,
                         ReconstructorTask& task, std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator);
    virtual ~ProbIMMReconstructor();

    virtual dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name) const override;

    virtual void reset() override;

    ProbIMMReconstructorSettings& settings();

    ProbIMMReconstructorWidget* widget(); // ownage by caller

    void updateWidgets() override;

  protected:

    ProbIMMReconstructorSettings settings_;
    ProbabilisticAssociator associatior_;
    SimpleReferenceCalculator ref_calculator_;

    virtual void processSlice_impl() override;

};

