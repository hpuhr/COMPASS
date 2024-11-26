#pragma once

#include "targetreportdefs.h"
#include "accuracyestimatorbase.h"

class SimpleReconstructor;

class SimpleAccuracyEstimator : public AccuracyEstimatorBase
{
public:
    SimpleAccuracyEstimator();

    virtual void validate (dbContent::targetReport::ReconstructorInfo& tr) override;

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
};

