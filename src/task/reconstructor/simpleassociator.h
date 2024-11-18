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

  private:
    SimpleReconstructor& reconstructor_;

            // distance, target acc, tr acc
    virtual bool canGetPositionOffsetTR(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target, bool use_max_distance=true) override;
    virtual boost::optional<std::tuple<double, double, double>> getPositionOffsetTR(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target, 
        bool do_debug,
        const boost::optional<unsigned int>& thread_id,
        reconstruction::PredictionStats* stats = nullptr) override;

    virtual bool canGetPositionOffsetTargets(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1)  override;
    virtual boost::optional<std::tuple<double, double, double>> getPositionOffsetTargets(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1,
        bool do_debug,
        const boost::optional<unsigned int>& thread_id,
        reconstruction::PredictionStats* stats = nullptr) override;

    virtual boost::optional<bool> checkTrackPositionOffsetAcceptable (
        dbContent::targetReport::ReconstructorInfo& tr, unsigned int utn,
        bool secondary_verified,bool do_debug) override;

    // empty if not possible, else check passed or failed returned
    virtual boost::optional<std::pair<bool, double>> calculatePositionOffsetScore (
        const dbContent::targetReport::ReconstructorInfo& tr, unsigned int other_utn,
        double distance_m, double tgt_est_std_dev, double tr_est_std_dev, bool secondary_verified,
        bool do_debug) override;
    // check passed + score (larger is better) returned
    virtual std::tuple<DistanceClassification, double> checkPositionOffsetScore
        (double distance_m, double sum_stddev_est, bool secondary_verified, bool target_acccuracy_acceptable) override;

    virtual bool isTargetAccuracyAcceptable(
        double tgt_est_std_dev, unsigned int utn, const dbContent::targetReport::ReconstructorInfo& tr, bool do_debug) override;
    //virtual bool isTargetAverageDistanceAcceptable(double distance_score_avg, bool secondary_verified) override;

    virtual ReconstructorBase& reconstructor() override;
};

