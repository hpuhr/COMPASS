#include "complexaccuracyestimator.h"
#include "number.h"

using namespace Utils;

const double ComplexAccuracyEstimator::PosAccStdDevDefault = 100.0;
const dbContent::targetReport::PositionAccuracy ComplexAccuracyEstimator::PosAccStdDefault
    { ComplexAccuracyEstimator::PosAccStdDevDefault, ComplexAccuracyEstimator::PosAccStdDevDefault, 0};

const double ComplexAccuracyEstimator::VelAccStdDevDefault = 30.0;
const double ComplexAccuracyEstimator::VelAccStdDevDefaultCAT021 = 10.0;
const double ComplexAccuracyEstimator::VelAccStdDevDefaultCAT062 = 20.0;

const dbContent::targetReport::VelocityAccuracy ComplexAccuracyEstimator::VelAccStdDefault
    { ComplexAccuracyEstimator::VelAccStdDevDefault, ComplexAccuracyEstimator::VelAccStdDevDefault};
const dbContent::targetReport::VelocityAccuracy ComplexAccuracyEstimator::VelAccStdDefaultCAT021
    { ComplexAccuracyEstimator::VelAccStdDevDefaultCAT021, ComplexAccuracyEstimator::VelAccStdDevDefaultCAT021};
const dbContent::targetReport::VelocityAccuracy ComplexAccuracyEstimator::VelAccStdDefaultCAT062
{ ComplexAccuracyEstimator::VelAccStdDevDefaultCAT062, ComplexAccuracyEstimator::VelAccStdDevDefaultCAT062};


const double ComplexAccuracyEstimator::AccAccStdDevDefault = 30.0;
const double ComplexAccuracyEstimator::AccAccStdDevDefaultCAT021 = 10.0;
const double ComplexAccuracyEstimator::AccAccStdDevDefaultCAT062 = 20.0;

const dbContent::targetReport::AccelerationAccuracy ComplexAccuracyEstimator::AccAccStdDefault
    { ComplexAccuracyEstimator::AccAccStdDevDefault, ComplexAccuracyEstimator::AccAccStdDevDefault};
const dbContent::targetReport::AccelerationAccuracy ComplexAccuracyEstimator::AccAccStdDefaultCAT021
    { ComplexAccuracyEstimator::AccAccStdDevDefaultCAT021, ComplexAccuracyEstimator::AccAccStdDevDefaultCAT021};
const dbContent::targetReport::AccelerationAccuracy ComplexAccuracyEstimator::AccAccStdDefaultCAT062
    { ComplexAccuracyEstimator::AccAccStdDevDefaultCAT062, ComplexAccuracyEstimator::AccAccStdDevDefaultCAT062};


ComplexAccuracyEstimator::ComplexAccuracyEstimator(ComplexReconstructor& reconstructor)
    : reconstructor_(reconstructor)
{

}

dbContent::targetReport::PositionAccuracy ComplexAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.position_accuracy_)
        return *tr.position_accuracy_;

    return PosAccStdDefault;
}

dbContent::targetReport::VelocityAccuracy ComplexAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    if (tr.velocity_accuracy_)
        return *tr.velocity_accuracy_;

    unsigned int dbcont_id = Number::recNumGetDBContId(tr.record_num_);

    if (dbcont_id == 21)
        return VelAccStdDefaultCAT021;
    else if (dbcont_id == 62)
        return VelAccStdDefaultCAT062;
    else
        return VelAccStdDefault;
}

dbContent::targetReport::AccelerationAccuracy ComplexAccuracyEstimator::accelerationAccuracy (
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
