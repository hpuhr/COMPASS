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
    virtual bool canGetPositionOffset(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target) override;
    virtual boost::optional<std::tuple<double, double, double>> getPositionOffset(
        const dbContent::targetReport::ReconstructorInfo& tr,
        const dbContent::ReconstructorTarget& target, 
        bool do_debug,
        const boost::optional<unsigned int>& thread_id,
        reconstruction::PredictionStats* stats = nullptr) override;
    virtual bool canGetPositionOffset(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1)  override;
    virtual boost::optional<std::tuple<double, double, double>> getPositionOffset(
        const boost::posix_time::ptime& ts,
        const dbContent::ReconstructorTarget& target0,
        const dbContent::ReconstructorTarget& target1,
        bool do_debug,
        const boost::optional<unsigned int>& thread_id,
        reconstruction::PredictionStats* stats = nullptr) override;

    virtual boost::optional<bool> checkPositionOffsetAcceptable (
        dbContent::targetReport::ReconstructorInfo& tr, unsigned int utn,
        bool secondary_verified,bool do_debug) override;

    // empty if not possible, else check passed or failed returned
    virtual boost::optional<std::pair<bool, double>> calculatePositionOffsetScore (
        const dbContent::targetReport::ReconstructorInfo& tr, unsigned int other_utn,
        double distance_m, double tgt_est_std_dev, double tr_est_std_dev, bool secondary_verified,
        bool do_debug) override;
    // empty if not possible, else check passed + score (smaller is better) returned
    virtual std::tuple<DistanceClassification, double> checkPositionOffsetScore
        (double distance_m, double sum_stddev_est, bool secondary_verified) override;

    virtual boost::optional<bool> isTargetAccuracyAcceptable(
        double tgt_est_std_dev, unsigned int utn, const boost::posix_time::ptime& ts) override;
    virtual bool isTargetAverageDistanceAcceptable(double distance_score_avg, bool secondary_verified) override;

    virtual ReconstructorBase& reconstructor() override;
};

