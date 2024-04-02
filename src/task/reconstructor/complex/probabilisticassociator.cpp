#include "probabilisticassociator.h"
#include "probimmreconstructor.h"
#include "logger.h"

using namespace std;
using namespace dbContent;

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
