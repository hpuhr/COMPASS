#pragma once

#include "datasourceaccuracyestimator.h"

class ADSBAccuracyEstimator : public DataSourceAccuracyEstimator
{
  public:

    ADSBAccuracyEstimator(const dbContent::DBDataSource& source);
    virtual ~ADSBAccuracyEstimator() {};

    virtual void validate (dbContent::targetReport::ReconstructorInfo& tr, ReconstructorBase& reconstructor) override;

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
};

