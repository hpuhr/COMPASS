#pragma once

#include <QObject>

#include "reconstructorbase.h"
//#include "targetreportdefs.h"
//#include "global.h"
#include "probabilisticassociator.h"
#include "accuracyestimatorbase.h"
#include "simplereferencecalculator.h"

class ProbIMMReconstructorSettings
{
  public:
    ProbIMMReconstructorSettings() {};

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

    void updateWidgets();

  protected:

    ProbIMMReconstructorSettings settings_;
    ProbabilisticAssociator associatior_;
    SimpleReferenceCalculator ref_calculator_;

    virtual bool processSlice_impl() override;

};

