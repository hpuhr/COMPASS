#include "radaraccuracyestimator.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "global.h"

using namespace dbContent;

RadarAccuracyEstimator::RadarAccuracyEstimator(const dbContent::DBDataSource& ds)
    : DataSourceAccuracyEstimator(ds)
{
    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    primary_azimuth_stddev_ = ds_man.config().primary_azimuth_stddev_;    // degrees
    primary_range_stddev_ = ds_man.config().primary_range_stddev_;      // meters
    secondary_azimuth_stddev_ = ds_man.config().secondary_azimuth_stddev_;  // degrees
    secondary_range_stddev_ = ds_man.config().secondary_range_stddev_;    // meters
    mode_s_azimuth_stddev_ = ds_man.config().mode_s_azimuth_stddev_;      // degrees
    mode_s_range_stddev_ = ds_man.config().mode_s_range_stddev_;        // meters

    has_position_ = ds.hasFullPosition();

    if (has_position_)
    {
        latitude_deg_ = ds.latitude();
        longitude_deg_ = ds.longitude();

        latitude_rad_ = latitude_deg_ * DEG2RAD;
        latitude_rad_ = latitude_deg_ * DEG2RAD;
    }

    if (ds.hasRadarAccuracies())
    {
        std::map<std::string, double> acc = ds.radarAccuracies();

                // set variances with data source values
        if (acc.count(DataSourceBase::PSRAzmSDKey))
            primary_azimuth_stddev_ = acc.at(DataSourceBase::PSRAzmSDKey);

        if (acc.count(DataSourceBase::PSRRngSDKey))
            primary_range_stddev_ = acc.at(DataSourceBase::PSRRngSDKey);

        if (acc.count(DataSourceBase::SSRAzmSDKey))
            secondary_range_stddev_ = acc.at(DataSourceBase::SSRAzmSDKey);

        if (acc.count(DataSourceBase::SSRRngSDKey))
            secondary_range_stddev_ = acc.at(DataSourceBase::SSRRngSDKey);

        if (acc.count(DataSourceBase::ModeSAzmSDKey))
            mode_s_azimuth_stddev_ = acc.at(DataSourceBase::ModeSAzmSDKey);

        if (acc.count(DataSourceBase::ModeSRngSDKey))
            mode_s_range_stddev_ = acc.at(DataSourceBase::ModeSRngSDKey);
    }
}

dbContent::targetReport::PositionAccuracy RadarAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::PosAccStdFallback;
}

dbContent::targetReport::VelocityAccuracy RadarAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::VelAccStdFallback;
}

dbContent::targetReport::AccelerationAccuracy RadarAccuracyEstimator::accelerationAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    return AccuracyEstimatorBase::AccAccStdFallback;
}
