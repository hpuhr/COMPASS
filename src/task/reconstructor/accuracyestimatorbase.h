#pragma once

#include "targetreportdefs.h"

#include <vector>

class ReconstructorBase;

class AccuracyEstimatorBase
{
  public:
    struct AssociatedDistance
    {
        AssociatedDistance() = default;
        AssociatedDistance(double latitude_deg, double longitude_deg,
                           double distance_m, double est_std_dev,
                           double mahalanobis_distance)
            : latitude_deg_(latitude_deg), longitude_deg_(longitude_deg), distance_m_(distance_m),
              est_std_dev_(est_std_dev), mahalanobis_distance_(mahalanobis_distance)
        {}

        double latitude_deg_, longitude_deg_;
        double distance_m_;
        double est_std_dev_;
        double mahalanobis_distance_;
    };

    AccuracyEstimatorBase();
    virtual ~AccuracyEstimatorBase() {};

    virtual void init (ReconstructorBase* reconstructor_ptr);

    virtual void prepareForNewSlice() {}
    virtual void postProccessNewSlice() {}

    virtual void validate (dbContent::targetReport::ReconstructorInfo& tr) = 0; // can set do not use position flag

    virtual void doOutlierDetection (
        dbContent::targetReport::ReconstructorInfo& tr,
        unsigned int utn, bool do_debug) {};

    virtual bool canCorrectPosition(const dbContent::targetReport::ReconstructorInfo& tr) { return false; }
    virtual void correctPosition(dbContent::targetReport::ReconstructorInfo& tr) {} // save in position_corrected_

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) = 0;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) = 0;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) = 0;

    virtual void addAssociatedDistance(dbContent::targetReport::ReconstructorInfo& tr, const AssociatedDistance& dist);
    virtual void analyzeAssociatedDistances() const;
    virtual void clearAssociatedDistances();

  protected:
    std::string name_;

    ReconstructorBase* reconstructor_ {nullptr};

    dbContent::targetReport::PositionAccuracy unspecific_pos_acc_fallback_;
    dbContent::targetReport::PositionAccuracy no_pos_acc_fallback_;

    dbContent::targetReport::VelocityAccuracy unspecifc_vel_acc_fallback_;
    dbContent::targetReport::VelocityAccuracy no_vel_acc_fallback_;

    dbContent::targetReport::AccelerationAccuracy no_acc_acc_fallback_;

    std::vector<AssociatedDistance> assoc_distances_;

    void printStatistics (const std::string name, std::function<double(const AssociatedDistance&)>& lambda) const;
};


