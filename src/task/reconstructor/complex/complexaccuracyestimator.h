#pragma once

#include "targetreportdefs.h"
#include "accuracyestimatorbase.h"

#include <memory>

//class ComplexReconstructor;

//    • Target report position accuracy
//        ◦ Integrate into current code
//        ◦ "Do not use position" flag
//    • Radar
//        ◦ Model-based accuracy: Pre-defined, re-estimated
//    • MLAT
//        ◦ Model-based pre-defined accuracy
//    • ADS-B
//        ◦ Minimal position quality indicator verification

#include <QObject>

class DataSourceManager;

namespace dbContent
{
class DBDataSource;
}

class ComplexAccuracyEstimator : public QObject, public AccuracyEstimatorBase
{
    Q_OBJECT

  public slots:
    void updateDataSourcesInfoSlot();

  public:
    ComplexAccuracyEstimator();
    virtual ~ComplexAccuracyEstimator();

    virtual void init(ReconstructorBase* reconstructor) override;

    virtual void validate (dbContent::targetReport::ReconstructorInfo& tr, ReconstructorBase& reconstructor) override;

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;

    virtual void addAssociatedDistance(
        dbContent::targetReport::ReconstructorInfo& tr, const AssociatedDistance& dist) override;
    virtual void analyzeAssociatedDistances() const override;
    virtual void clearAssociatedDistances() override;

    virtual void estimateAccuracies() override;

  private:
    //ComplexReconstructor& reconstructor_;

    bool initialized_ {false};

    std::map<unsigned int, std::unique_ptr<AccuracyEstimatorBase>> ds_acc_estimators_;

    static const double PosAccStdDevDefault;
    static const dbContent::targetReport::PositionAccuracy PosAccStdDefault;

    static const double VelAccStdDevDefault;
    static const double VelAccStdDevDefaultCAT021;
    static const double VelAccStdDevDefaultCAT062;
    static const dbContent::targetReport::VelocityAccuracy VelAccStdDefault;
    static const dbContent::targetReport::VelocityAccuracy VelAccStdDefaultCAT021;
    static const dbContent::targetReport::VelocityAccuracy VelAccStdDefaultCAT062;

    static const double AccAccStdDevDefault;
    static const double AccAccStdDevDefaultCAT021;
    static const double AccAccStdDevDefaultCAT062;
    static const dbContent::targetReport::AccelerationAccuracy AccAccStdDefault;
    static const dbContent::targetReport::AccelerationAccuracy AccAccStdDefaultCAT021;
    static const dbContent::targetReport::AccelerationAccuracy AccAccStdDefaultCAT062;

    std::unique_ptr<AccuracyEstimatorBase> createAccuracyEstimator(dbContent::DBDataSource& ds);
};

