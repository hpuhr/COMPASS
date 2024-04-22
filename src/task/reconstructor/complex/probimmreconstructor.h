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

