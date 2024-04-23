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
    std::map<unsigned int, unsigned int> acad_2_utn_; // acad dec -> utn

            // ds_id -> line id -> track num -> utn, last tod
    std::map<unsigned int, std::map<unsigned int,
                                    std::map<unsigned int,
                                             std::pair<unsigned int, boost::posix_time::ptime>>>> tn2utn_;

    unsigned int num_merges_ {0};

    void selfAccociateNewUTNs();
    void checkACADLookup();

            // tries to find existing utn for target report, based on mode a/c and position, -1 if failed
    int findUTNForTargetReport (const dbContent::targetReport::ReconstructorInfo& tr,
                               const std::vector<unsigned int>& utn_vec,
                               const std::set<unsigned long>& debug_rec_nums,
                               const std::set<unsigned int>& debug_utns);

    int findUTNForTarget (unsigned int utn,
                         const std::set<unsigned long>& debug_rec_nums,
                         const std::set<unsigned int>& debug_utns);

    unsigned int createNewTarget(const dbContent::targetReport::ReconstructorInfo& tr);

    // distance, target acc, tr acc
    bool canGetPositionOffset(const dbContent::targetReport::ReconstructorInfo& tr,
                               const dbContent::ReconstructorTarget& target);
    std::tuple<double, double, double> getPositionOffset(const dbContent::targetReport::ReconstructorInfo& tr,
                                                         const dbContent::ReconstructorTarget& target, bool do_debug);

    void estimateEllipse(dbContent::targetReport::PositionAccuracy& acc, EllipseDef& def) const;
    double estimateAccuracyAt (EllipseDef& def, double bearing_rad) const;
};

