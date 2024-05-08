#pragma once

#include "targetreportdefs.h"
#include "reconstructortarget.h"
#include "reconstructorassociatorbase.h"

class SimpleReconstructor;

class SimpleAssociator : public ReconstructorAssociatorBase
{
  public:
    SimpleAssociator(SimpleReconstructor& reconstructor);

    virtual void associateNewData() override;
    //virtual void reset() override;

  private:
    SimpleReconstructor& reconstructor_;

    boost::posix_time::time_duration max_time_diff_;

//    void createReferenceUTNs();

//    void createTrackerUTNs();
//    void createNonTrackerUTNS();

//    // creates tmp tracked targets to be added
//    std::map<unsigned int, dbContent::ReconstructorTarget> createTrackedTargets(
//        unsigned int dbcont_id, unsigned int ds_id);
//    void selfAssociateTrackerUTNs();

//    void addTrackerUTNs(const std::string& ds_name, std::map<unsigned int, dbContent::ReconstructorTarget> from_targets,
//                        std::map<unsigned int, dbContent::ReconstructorTarget>& to_targets);

//    int findContinuationUTNForTrackerUpdate (const dbContent::targetReport::ReconstructorInfo& tr,
//                                            const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);
//    // tries to find existing utn for tracker update, -1 if failed
//    int findUTNForTrackerTarget (const dbContent::ReconstructorTarget& target,
//                                const std::map<unsigned int, dbContent::ReconstructorTarget>& targets,
//                                int max_utn=-100);
//    // tries to find existing utn for target, -1 if failed
//    int findUTNForTargetByTA (const dbContent::ReconstructorTarget& target,
//                             const std::map<unsigned int, dbContent::ReconstructorTarget>& targets,
//                             int max_utn);
//    // tries to find existing utn for target by target address, -1 if failed

//    std::map<unsigned int, unsigned int> getTALookupMap (
//        const std::map<unsigned int, dbContent::ReconstructorTarget>& targets);

            // distance, target acc, tr acc
    virtual bool canGetPositionOffset(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target) override;
    virtual std::tuple<double, double, double> getPositionOffset(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target, bool do_debug) override;
    virtual bool canGetPositionOffset(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1)  override;
    virtual std::tuple<double, double, double> getPositionOffset(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1,
        int thread_id, bool do_debug) override;


    virtual boost::optional<bool> checkPositionOffsetAcceptable (
        const dbContent::targetReport::ReconstructorInfo& tr,
        double distance_m, double tgt_est_std_dev, double tr_est_std_dev, bool secondary_verified,
        bool do_debug) override;
    // empty if not possible, else check passed or failed returned
    virtual boost::optional<std::pair<bool, double>> calculatePositionOffsetScore (
        const dbContent::targetReport::ReconstructorInfo& tr, unsigned int other_utn,
        double distance_m, double tgt_est_std_dev, double tr_est_std_dev, bool secondary_verified,
        bool do_debug) override;
    // empty if not possible, else check passed + score (smaller is better) returned
    virtual std::tuple<DistanceClassification, double> checkPositionOffsetScore
        (double distance_m, double sum_stddev_est, bool secondary_verified) override;

    virtual bool isTargetAccuracyAcceptable(double tgt_est_std_dev) override;
    virtual bool isTargetAverageDistanceAcceptable(double distance_score_avg, bool secondary_verified) override;

    virtual ReconstructorBase& reconstructor() override;
};

