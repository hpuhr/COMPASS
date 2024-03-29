#pragma once

#include "datasourceaccuracyestimator.h"

class TrackerAccuracyEstimator : public DataSourceAccuracyEstimator
{
  public:
    TrackerAccuracyEstimator(const dbContent::DBDataSource& source);
    virtual ~TrackerAccuracyEstimator() {};

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
};

