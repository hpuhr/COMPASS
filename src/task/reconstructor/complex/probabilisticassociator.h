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

    std::vector<unsigned int> utn_vec_;
    std::map<unsigned int, unsigned int> acad_2_utn_;

            // ds_id -> track num -> utn, last tod
    std::map<unsigned int, std::map<unsigned int, std::pair<unsigned int, boost::posix_time::ptime>>> tn2utn_;

    void checkACADLookup();

            // tries to find existing utn for target report, based on mode a/c and position, -1 if failed
    int findUTNForTargetReport (const dbContent::targetReport::ReconstructorInfo& tr,
                               const std::vector<unsigned int>& utn_vec,
                               const std::set<unsigned long>& debug_rec_nums,
                               const std::set<unsigned int>& debug_utns);

    unsigned int createNewTarget(const dbContent::targetReport::ReconstructorInfo& tr);

    void estimateEllipse(dbContent::targetReport::PositionAccuracy& acc, EllipseDef& def) const;
    double estimateAccuracyAt (EllipseDef& def, double bearing_rad) const;
};

