#include "trackeraccuracyestimator.h"

TrackerAccuracyEstimator::TrackerAccuracyEstimator(const dbContent::DBDataSource& source)
    : DataSourceAccuracyEstimator(source)
{

}

dbContent::targetReport::PositionAccuracy TrackerAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.position_accuracy_)
        return *tr.position_accuracy_;

    return AccuracyEstimatorBase::PosAccStdFallback;
}

dbContent::targetReport::VelocityAccuracy TrackerAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.velocity_accuracy_)
        return *tr.velocity_accuracy_;

    return AccuracyEstimatorBase::VelAccStdFallback;
}

dbContent::targetReport::AccelerationAccuracy TrackerAccuracyEstimator::accelerationAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::AccAccStdFallback;
}
