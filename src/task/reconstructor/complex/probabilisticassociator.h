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

    std::map<unsigned int, unsigned int> getTALookupMap (
        const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);

    // tries to find existing utn for target report, -1 if failed
    int findUTNForTargetReport (const dbContent::targetReport::ReconstructorInfo& tr,
                               std::map<unsigned int, unsigned int> ta_2_utn,
                               const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);

    void estimateEllipse(dbContent::targetReport::PositionAccuracy& acc, EllipseDef& def) const;
    double estimateAccuracyAt (EllipseDef& def, double bearing_rad) const;
};

