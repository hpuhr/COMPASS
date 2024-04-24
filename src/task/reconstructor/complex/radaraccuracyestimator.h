#pragma once

#include "datasourceaccuracyestimator.h"

class RadarAccuracyEstimator : public DataSourceAccuracyEstimator
{
  public:
    RadarAccuracyEstimator(const dbContent::DBDataSource& source);
    virtual ~RadarAccuracyEstimator() {};

    virtual void validate (dbContent::targetReport::ReconstructorInfo& tr, ReconstructorBase& reconstructor) override;

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;

  protected:
    virtual void estimateAccuracyUsing(
        dbContent::ReconstructorTarget& target, unsigned int dbcont_id,
        const std::map<unsigned int, std::multimap<boost::posix_time::ptime, unsigned long>>& tr_ds_timestamps_)
        override;
    virtual void finalizeEstimateAccuracy() override;



  private:

    bool has_position_ {false};
    double latitude_deg_{0}, latitude_rad_{0};
    double longitude_deg_{0}, longitude_rad_{0};

    double primary_azimuth_stddev_{0};    // degrees
    double primary_range_stddev_{0};      // meters
    double secondary_azimuth_stddev_{0};  // degrees
    double secondary_range_stddev_{0};    // meters
    double mode_s_azimuth_stddev_{0};     // degrees
    double mode_s_range_stddev_{0};       // meters
};

