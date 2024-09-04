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

    virtual void init (ReconstructorBase* reconstructor);

    virtual void prepareForNewSlice() {}
    virtual void postProccessNewSlice() {}

    virtual void validate (dbContent::targetReport::ReconstructorInfo& tr) = 0; // can set do not use position flag

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

    bool skip_unused_positions_ {false};
    bool use_min_std_dev_ {true};
    double min_std_dev_ {1.0};

    static const double PosAccStdDevMin;
    static const dbContent::targetReport::PositionAccuracy PosAccStdMin;

    static const double VelAccStdDevMin;
    static const dbContent::targetReport::VelocityAccuracy VelAccStdMin;

    static const double AccAccStdDevMin;
    static const dbContent::targetReport::AccelerationAccuracy AccAccStdMin;

    static const double PosAccStdDevFallback;
    static const dbContent::targetReport::PositionAccuracy PosAccStdFallback;

    static const double PosAccStdDevMax;
    static const dbContent::targetReport::PositionAccuracy PosAccStdMax;

    static const double VelAccStdDevFallback;
    static const dbContent::targetReport::VelocityAccuracy VelAccStdFallback;

    static const double AccAccStdDevFallback;
    static const dbContent::targetReport::AccelerationAccuracy AccAccStdFallback;

    std::string name_;

    ReconstructorBase* reconstructor_ {nullptr};

    std::vector<AssociatedDistance> assoc_distances_;

    void printStatistics (const std::string name, std::function<double(const AssociatedDistance&)>& lambda) const;
};


