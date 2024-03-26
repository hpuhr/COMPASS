#include "radaraccuracyestimator.h"

RadarAccuracyEstimator::RadarAccuracyEstimator()
{

}

dbContent::targetReport::PositionAccuracy RadarAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::PosAccStdFallback;
}

dbContent::targetReport::VelocityAccuracy RadarAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::VelAccStdFallback;
}

dbContent::targetReport::AccelerationAccuracy RadarAccuracyEstimator::accelerationAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::AccAccStdFallback;
}
