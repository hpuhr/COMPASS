#pragma once

#include "targetreportdefs.h"
#include "reconstructortarget.h"

class SimpleReconstructor;

class SimpleAccuracyEstimator
{
  public:
    SimpleAccuracyEstimator(SimpleReconstructor& reconstructor);

    dbContent::targetReport::PositionAccuracy positionAccuracy (const dbContent::targetReport::ReconstructorInfo& tr);
    dbContent::targetReport::VelocityAccuracy velocityAccuracy (const dbContent::targetReport::ReconstructorInfo& tr);
    dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr);

  private:
    SimpleReconstructor& reconstructor_;

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

