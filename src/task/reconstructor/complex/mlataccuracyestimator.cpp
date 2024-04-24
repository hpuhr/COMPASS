#include "mlataccuracyestimator.h"

MLATAccuracyEstimator::MLATAccuracyEstimator(const dbContent::DBDataSource& source)
    : DataSourceAccuracyEstimator(source)
{

}

void MLATAccuracyEstimator::validate (
    dbContent::targetReport::ReconstructorInfo& tr, ReconstructorBase& reconstructor)
{

}

dbContent::targetReport::PositionAccuracy MLATAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.position_accuracy_)
        return *tr.position_accuracy_;

    return AccuracyEstimatorBase::PosAccStdFallback;
}

dbContent::targetReport::VelocityAccuracy MLATAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.velocity_accuracy_)
        return *tr.velocity_accuracy_;

    return AccuracyEstimatorBase::VelAccStdFallback;
}

dbContent::targetReport::AccelerationAccuracy MLATAccuracyEstimator::accelerationAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::AccAccStdFallback;
}

void MLATAccuracyEstimator::estimateAccuracyUsing(
    dbContent::ReconstructorTarget& target, unsigned int dbcont_id,
    const std::map<unsigned int, std::multimap<boost::posix_time::ptime, unsigned long>>& tr_ds_timestamps_)
{

}
void MLATAccuracyEstimator::finalizeEstimateAccuracy()
{

}
