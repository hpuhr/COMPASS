#pragma once

#include "targetreportdefs.h"
#include "reconstructortarget.h"

class ProbIMMReconstructor;

class ProbabilisticAssociator
{
  public:
    ProbabilisticAssociator(ProbIMMReconstructor& reconstructor);

    void associateNewData();
    void reset();

  private:

    struct EllipseDef
    {
        double rad1;
        double rad2;
        double theta_rad;
    };

    ProbIMMReconstructor& reconstructor_;

    // ds_id -> track num -> utn, last tod
    std::map<unsigned int, std::map<unsigned int, std::pair<unsigned int, boost::posix_time::ptime>>> tn2utn_;

    std::map<unsigned int, unsigned int> getTALookupMap (
        const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);

//    int findUTNForTrackedUpdate (const dbContent::targetReport::ReconstructorInfo& tr,
//                                            const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);
    // tries to find existing utn for tracker update, -1 if failed

    // tries to find existing utn for target report, -1 if failed
//    int findUTNForTargetReport (const dbContent::targetReport::ReconstructorInfo& tr,
//                               const std::vector<unsigned int>& utn_vec,
//                               const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);

    void estimateEllipse(dbContent::targetReport::PositionAccuracy& acc, EllipseDef& def) const;
    double estimateAccuracyAt (EllipseDef& def, double bearing_rad) const;
};

