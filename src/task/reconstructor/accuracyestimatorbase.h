#pragma once
#include "targetreportdefs.h"

class AccuracyEstimatorBase
{
  public:
    AccuracyEstimatorBase();

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) = 0;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) = 0;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) = 0;

  protected:
    static const double PosAccStdDevFallback;
    static const dbContent::targetReport::PositionAccuracy PosAccStdFallback;

    static const double VelAccStdDevFallback;
    static const dbContent::targetReport::VelocityAccuracy VelAccStdFallback;

    static const double AccAccStdDevFallback;
    static const dbContent::targetReport::AccelerationAccuracy AccAccStdFallback;
};


