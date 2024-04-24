#include "trackeraccuracyestimator.h"
#include "reconstructorbase.h"
#include "timeconv.h"
#include "stringconv.h"
#include "number.h"

using namespace std;
using namespace Utils;

TrackerAccuracyEstimator::TrackerAccuracyEstimator(const dbContent::DBDataSource& source)
    : DataSourceAccuracyEstimator(source)
{

}

void TrackerAccuracyEstimator::validate (
    dbContent::targetReport::ReconstructorInfo& tr, ReconstructorBase& reconstructor)
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

void TrackerAccuracyEstimator::estimateAccuracyUsing(
    dbContent::ReconstructorTarget& target, unsigned int dbcont_id,
    const std::map<unsigned int, std::multimap<boost::posix_time::ptime, unsigned long>>& tr_ds_timestamps_)
{
    logdbg << "TrackerAccuracyEstimator " << name_ << ": estimateAccuracyUsing";

    const boost::posix_time::time_duration max_time_diff = Time::partialSeconds(5);

    double distance_m, bearing_rad;

    for (auto& line_it : tr_ds_timestamps_) // line id -> record_num, sorted by ts
    {
        for (auto& ts_it : line_it.second)
        {
            assert (reconstructor_->target_reports_.count(ts_it.second));

            dbContent::targetReport::ReconstructorInfo& tr = reconstructor_->target_reports_.at(ts_it.second);

            if (!tr.position_)
                continue;

            boost::optional<dbContent::targetReport::Position> ref_pos =
                target.interpolatedRefPosForTimeFast(ts_it.first, max_time_diff);

            if (!ref_pos)
                continue;

            std::tie(distance_m, bearing_rad) = getOffset(tr, *ref_pos);

//            loginf << "dist " << String::doubleToStringPrecision(distance_m, 2)
//                   << " stddev " << String::doubleToStringPrecision(positionAccuracy(tr).maxStdDev(), 2);

            distances_.push_back(distance_m);
            std_devs_.push_back(positionAccuracy(tr).maxStdDev());
        }
    }
}

void TrackerAccuracyEstimator::finalizeEstimateAccuracy()
{
    if (!distances_.size())
        return;

    std::tuple<double,double,double,double> dist_stat = Number::getStatistics (distances_);

    loginf << "TrackerAccuracyEstimator SRC " << name_ << ": dist "
           << " avg " << String::doubleToStringPrecision(get<0>(dist_stat), 2)
           << " stddev " << String::doubleToStringPrecision(get<1>(dist_stat), 2)
           << " min " << String::doubleToStringPrecision(get<2>(dist_stat), 2)
           << " max " << String::doubleToStringPrecision(get<3>(dist_stat), 2);

    std::tuple<double,double,double,double> stddev_stat = Number::getStatistics (std_devs_);

    loginf << "TrackerAccuracyEstimator SRC " << name_ << ": stddev "
           << " avg " << String::doubleToStringPrecision(get<0>(stddev_stat), 2)
           << " stddev " << String::doubleToStringPrecision(get<1>(stddev_stat), 2)
           << " min " << String::doubleToStringPrecision(get<2>(stddev_stat), 2)
           << " max " << String::doubleToStringPrecision(get<3>(stddev_stat), 2);
}
