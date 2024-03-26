#include "adsbaccuracyestimator.h"

ADSBAccuracyEstimator::ADSBAccuracyEstimator(const dbContent::DBDataSource& source)
    : DataSourceAccuracyEstimator(source)
{
}

dbContent::targetReport::PositionAccuracy ADSBAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.position_accuracy_)
        return *tr.position_accuracy_;

    return AccuracyEstimatorBase::PosAccStdFallback;
}

dbContent::targetReport::VelocityAccuracy ADSBAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.velocity_accuracy_)
        return *tr.velocity_accuracy_;

    return AccuracyEstimatorBase::VelAccStdFallback;
}

dbContent::targetReport::AccelerationAccuracy ADSBAccuracyEstimator::accelerationAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::AccAccStdFallback;
}
