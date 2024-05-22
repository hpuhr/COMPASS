#include "simpleaccuracyestimator.h"
#include "reconstructorbase.h"
#include "targetreportaccessor.h"
#include "number.h"

using namespace Utils;

const double SimpleAccuracyEstimator::PosAccStdDevDefault = 100.0;
const dbContent::targetReport::PositionAccuracy SimpleAccuracyEstimator::PosAccStdDefault
    { SimpleAccuracyEstimator::PosAccStdDevDefault, SimpleAccuracyEstimator::PosAccStdDevDefault, 0};

const double SimpleAccuracyEstimator::VelAccStdDevDefault = 30.0;
const double SimpleAccuracyEstimator::VelAccStdDevDefaultCAT021 = 10.0;
const double SimpleAccuracyEstimator::VelAccStdDevDefaultCAT062 = 20.0;

const dbContent::targetReport::VelocityAccuracy SimpleAccuracyEstimator::VelAccStdDefault
    { SimpleAccuracyEstimator::VelAccStdDevDefault, SimpleAccuracyEstimator::VelAccStdDevDefault};
const dbContent::targetReport::VelocityAccuracy SimpleAccuracyEstimator::VelAccStdDefaultCAT021
    { SimpleAccuracyEstimator::VelAccStdDevDefaultCAT021, SimpleAccuracyEstimator::VelAccStdDevDefaultCAT021};
const dbContent::targetReport::VelocityAccuracy SimpleAccuracyEstimator::VelAccStdDefaultCAT062
{ SimpleAccuracyEstimator::VelAccStdDevDefaultCAT062, SimpleAccuracyEstimator::VelAccStdDevDefaultCAT062};


const double SimpleAccuracyEstimator::AccAccStdDevDefault = 30.0;
const double SimpleAccuracyEstimator::AccAccStdDevDefaultCAT021 = 10.0;
const double SimpleAccuracyEstimator::AccAccStdDevDefaultCAT062 = 20.0;

const dbContent::targetReport::AccelerationAccuracy SimpleAccuracyEstimator::AccAccStdDefault
    { SimpleAccuracyEstimator::AccAccStdDevDefault, SimpleAccuracyEstimator::AccAccStdDevDefault};
const dbContent::targetReport::AccelerationAccuracy SimpleAccuracyEstimator::AccAccStdDefaultCAT021
    { SimpleAccuracyEstimator::AccAccStdDevDefaultCAT021, SimpleAccuracyEstimator::AccAccStdDevDefaultCAT021};
const dbContent::targetReport::AccelerationAccuracy SimpleAccuracyEstimator::AccAccStdDefaultCAT062
    { SimpleAccuracyEstimator::AccAccStdDevDefaultCAT062, SimpleAccuracyEstimator::AccAccStdDevDefaultCAT062};


SimpleAccuracyEstimator::SimpleAccuracyEstimator()
{

}

void SimpleAccuracyEstimator::validate (
    dbContent::targetReport::ReconstructorInfo& tr, ReconstructorBase& reconstructor)
{
    boost::optional<unsigned char> mops_version = reconstructor.accessor(tr).mopsVersion(tr.buffer_index_);

    if (mops_version && *mops_version == 0)
        tr.do_not_use_position_ = true;

}

dbContent::targetReport::PositionAccuracy SimpleAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.position_accuracy_)
    {
        if (tr.position_accuracy_->maxStdDev() < 10E-6)
            return AccuracyEstimatorBase::PosAccStdMin;

        return *tr.position_accuracy_;
    }

    return PosAccStdDefault;
}

dbContent::targetReport::VelocityAccuracy SimpleAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.velocity_accuracy_)
    {
        if (tr.velocity_accuracy_->maxStdDev() < 10E-6)
            return AccuracyEstimatorBase::VelAccStdMin;

        return *tr.velocity_accuracy_;
    }

    unsigned int dbcont_id = Number::recNumGetDBContId(tr.record_num_);

    if (dbcont_id == 21)
        return VelAccStdDefaultCAT021;
    else if (dbcont_id == 62)
        return VelAccStdDefaultCAT062;
    else
        return VelAccStdDefault;
}

dbContent::targetReport::AccelerationAccuracy SimpleAccuracyEstimator::accelerationAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    unsigned int dbcont_id = Number::recNumGetDBContId(tr.record_num_);

    if (dbcont_id == 21)
        return AccAccStdDefaultCAT021;
    else if (dbcont_id == 62)
        return AccAccStdDefaultCAT062;
    else
        return AccAccStdDefault;
}
