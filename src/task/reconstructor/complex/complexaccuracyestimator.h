#pragma once

#include "targetreportdefs.h"
#include "accuracyestimatorbase.h"
#include "adsbaccuracyestimator.h"
#include "mlataccuracyestimator.h"
#include "trackeraccuracyestimator.h"
#include "radaraccuracyestimator.h"

class ComplexReconstructor;

//    • Target report position accuracy
//        ◦ Integrate into current code
//        ◦ "Do not use position" flag
//    • Radar
//        ◦ Model-based accuracy: Pre-defined, re-estimated
//    • MLAT
//        ◦ Model-based pre-defined accuracy
//    • ADS-B
//        ◦ Minimal position quality indicator verification

class ComplexAccuracyEstimator : public AccuracyEstimatorBase
{
  public:
    ComplexAccuracyEstimator(ComplexReconstructor& reconstructor);

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;

  private:
    ComplexReconstructor& reconstructor_;

    ADSBAccuracyEstimator adsb_estimator_;
    MLATAccuracyEstimator mlat_estimator_;
    TrackerAccuracyEstimator tracker_estimator_;
    RadarAccuracyEstimator radar_estimator_;

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
};

