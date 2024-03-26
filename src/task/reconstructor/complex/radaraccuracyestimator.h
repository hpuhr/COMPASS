#pragma once

#include "datasourceaccuracyestimator.h"

class RadarAccuracyEstimator : public DataSourceAccuracyEstimator
{
  public:
    RadarAccuracyEstimator(const dbContent::DBDataSource& source);

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
};

