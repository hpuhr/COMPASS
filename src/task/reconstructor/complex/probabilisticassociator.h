#pragma once

#include "reconstructorassociatorbase.h"

class ProbIMMReconstructor;

class ProbabilisticAssociator : public ReconstructorAssociatorBase
{
  public:
    ProbabilisticAssociator(ProbIMMReconstructor& reconstructor);

  protected:

    struct EllipseDef
    {
        double rad1;
        double rad2;
        double theta_rad;
    };

    ProbIMMReconstructor& reconstructor_;

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

    void estimateEllipse(dbContent::targetReport::PositionAccuracy& acc, EllipseDef& def) const;
    double estimateAccuracyAt (EllipseDef& def, double bearing_rad) const;
};

