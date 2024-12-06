#include "simpleaccuracyestimator.h"
#include "reconstructorbase.h"
#include "targetreportaccessor.h"
#include "number.h"

using namespace Utils;

SimpleAccuracyEstimator::SimpleAccuracyEstimator()
{
}

void SimpleAccuracyEstimator::validate (
    dbContent::targetReport::ReconstructorInfo& tr)
{
    assert (reconstructor_);

    boost::optional<unsigned char> mops_version = reconstructor_->accessor(tr).mopsVersion(tr.buffer_index_);

    if (mops_version && *mops_version == 0)
        tr.invalidated_pos_ = true;

}

dbContent::targetReport::PositionAccuracy SimpleAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.position_accuracy_)
    {
        if (tr.position_accuracy_->minStdDev() < reconstructor_->settings().numerical_min_std_dev_)
            return tr.position_accuracy_->getScaledToMinStdDev(reconstructor_->settings().numerical_min_std_dev_);

        return *tr.position_accuracy_;
    }

    if (tr.position_)
        return unspecific_pos_acc_fallback_;
    else
        return no_pos_acc_fallback_;
}

dbContent::targetReport::VelocityAccuracy SimpleAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.velocity_accuracy_)
    {
        if (tr.velocity_accuracy_->minStdDev() < reconstructor_->settings().numerical_min_std_dev_)
            return tr.velocity_accuracy_->getScaledToMinStdDev(reconstructor_->settings().numerical_min_std_dev_);

        return *tr.velocity_accuracy_;
    }

    if (tr.velocity_)
        return unspecifc_vel_acc_fallback_;
    else
        return no_vel_acc_fallback_;
}

dbContent::targetReport::AccelerationAccuracy SimpleAccuracyEstimator::accelerationAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return no_acc_acc_fallback_;
}
