#include "adsbaccuracyestimator.h"

ADSBAccuracyEstimator::ADSBAccuracyEstimator()
{

}

dbContent::targetReport::PositionAccuracy ADSBAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::PosAccStdFallback;
}

dbContent::targetReport::VelocityAccuracy ADSBAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::VelAccStdFallback;
}

dbContent::targetReport::AccelerationAccuracy ADSBAccuracyEstimator::accelerationAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::AccAccStdFallback;
}
