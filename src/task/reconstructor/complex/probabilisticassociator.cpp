#include "probabilisticassociator.h"
#include "probimmreconstructor.h"
#include "logger.h"
#include "global.h"
#include "stringconv.h"

#include <osgEarth/GeoMath>

using namespace std;
using namespace dbContent;
using namespace Utils;

ProbabilisticAssociator::ProbabilisticAssociator(ProbIMMReconstructor& reconstructor)
    : reconstructor_(reconstructor)
{

}


void ProbabilisticAssociator::associateNewData()
{
    loginf << "ProbabilisticAssociator: associateNewData";

    unsigned long rec_num;
    int utn;

    std::map<unsigned int, unsigned int> ta_2_utn = getTALookupMap(reconstructor_.targets_);

    reconstruction::Measurement mm;
    bool ret;
    double distance_m;
    dbContent::targetReport::PositionAccuracy pos_acc;
    double max_std_dev;

    for (auto& ts_it : reconstructor_.tr_timestamps_)
    {
        rec_num = ts_it.second;

        assert (reconstructor_.target_reports_.count(rec_num));

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor_.target_reports_.at(rec_num);

        if (!tr.in_current_slice_)
            continue;

        // try by mode-s address
        if (tr.acad_)
        {
            // find utn

            if (ta_2_utn.count(*tr.acad_)) // already exists
            {
                utn = ta_2_utn.at(*tr.acad_);
            }
            else // not yet existing, create & add target
            {
                if (reconstructor_.targets_.size())
                    utn = reconstructor_.targets_.rbegin()->first + 1; // new utn
                else
                    utn = 0;

                reconstructor_.targets_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(utn),   // args for key
                    std::forward_as_tuple(reconstructor_, utn, false));  // args for mapped value tmp_utn, false

                // add to lookup
                ta_2_utn[*tr.acad_] = utn;

            }

            // add associated target reports
            assert (reconstructor_.targets_.count(utn));

            reconstructor_.acc_estimator_->validate(tr, reconstructor_);

            // only if not newly created
            if (!tr.do_not_use_position_ && reconstructor_.targets_.at(utn).canPredict(tr.timestamp_))
            {
                ret = reconstructor_.targets_.at(utn).predict(mm, tr);
                assert (ret);

                distance_m   = osgEarth::GeoMath::distance(tr.position_->latitude_ * DEG2RAD,
                                                         tr.position_->longitude_ * DEG2RAD,
                                                         mm.lat * DEG2RAD, mm.lon * DEG2RAD);

                max_std_dev = 0;

                pos_acc = reconstructor_.acc_estimator_->positionAccuracy(tr);

                max_std_dev += max (pos_acc.x_stddev_, pos_acc.y_stddev_);
                assert (mm.hasStdDevPosition());
                max_std_dev += max (*mm.x_stddev, *mm.y_stddev);

                loginf << " dist " << distance_m << " mahala "
                       << String::doubleToStringPrecision(distance_m / max_std_dev, 2);
            }

            reconstructor_.targets_.at(utn).addTargetReport(rec_num);
            continue; // done
        }
    }
}

void ProbabilisticAssociator::reset()
{
    logdbg << "ProbabilisticAssociator: reset";
}


std::map<unsigned int, unsigned int> ProbabilisticAssociator::getTALookupMap (
    const std::map<unsigned int, ReconstructorTarget>& targets)
{
    logdbg << "ProbabilisticAssociator: getTALookupMap";

    std::map<unsigned int, unsigned int> ta_2_utn;

    for (auto& target_it : targets)
    {
        if (!target_it.second.hasACAD())
            continue;

        assert (target_it.second.acads_.size() == 1);

        assert (!ta_2_utn.count(*target_it.second.acads_.begin()));

        ta_2_utn[*target_it.second.acads_.begin()] = target_it.second.utn_;
    }

    logdbg << "SimpleAssociator: getTALookupMap: done";

    return ta_2_utn;
}

        // tries to find existing utn for target report, -1 if failed
int ProbabilisticAssociator::findUTNForTargetReport (
    const dbContent::targetReport::ReconstructorInfo& tr,
    std::map<unsigned int, unsigned int> ta_2_utn,
    const std::map<unsigned int, dbContent::ReconstructorTarget>& targets)
{

}
