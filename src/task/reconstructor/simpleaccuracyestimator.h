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

private:
    dbContent::targetReport::PositionAccuracy unspecific_pos_acc_fallback_;
    dbContent::targetReport::PositionAccuracy no_pos_acc_fallback_;

    dbContent::targetReport::VelocityAccuracy unspecifc_vel_acc_fallback_;
    dbContent::targetReport::VelocityAccuracy no_vel_acc_fallback_;

    dbContent::targetReport::AccelerationAccuracy no_acc_acc_fallback_;
};

