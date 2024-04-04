#include "adsbaccuracyestimator.h"
#include "reconstructorbase.h"
#include "targetreportaccessor.h"

ADSBAccuracyEstimator::ADSBAccuracyEstimator(const dbContent::DBDataSource& source)
    : DataSourceAccuracyEstimator(source)
{
}

void ADSBAccuracyEstimator::validate (
    dbContent::targetReport::ReconstructorInfo& tr, ReconstructorBase& reconstructor)
{
    boost::optional<unsigned char> mops_version = reconstructor.accessor(tr).mopsVersion(tr.buffer_index_);

    if (mops_version && *mops_version == 0)
        tr.do_not_use_position_ = true;
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
