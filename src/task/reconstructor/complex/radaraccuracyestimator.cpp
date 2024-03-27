#include "radaraccuracyestimator.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "global.h"

#include <osgEarth/GeoMath>

#include <Eigen/Core>

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
        longitude_rad_ = longitude_deg_ * DEG2RAD;
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
    if (tr.position_accuracy_) // could be CAT010
        return *tr.position_accuracy_;

    if (!tr.position_ || !has_position_) // cannot calculate
        return AccuracyEstimatorBase::PosAccStdFallback;

    double x_stddev, y_stddev, xy_cov;
    double plot_latitude_rad = tr.position_->latitude_ * DEG2RAD;
    double plot_longitude_rad = tr.position_->longitude_ * DEG2RAD;

    double distance_m   = osgEarth::GeoMath::distance(latitude_rad_, longitude_rad_,
                                                    plot_latitude_rad, plot_longitude_rad);
    double deg_2_m      = 2 * M_PI * distance_m / 360.0; // circumference / 360
    double bearing_rad  = osgEarth::GeoMath::bearing(latitude_rad_, longitude_rad_,
                                                    plot_latitude_rad, plot_longitude_rad);

    double range_std_m; // stddev along range
    double azm_std_m;   // stddev along azimuth

    if (tr.isPrimaryOnlyDetection()) // 1 Single PSR detection
    {
        range_std_m = primary_range_stddev_;
        azm_std_m   = primary_azimuth_stddev_ * deg_2_m;
    }
    else if (tr.isModeACDetection()) // 2 Single SSR detection
    {
        range_std_m = secondary_range_stddev_;
        azm_std_m   = secondary_azimuth_stddev_ * deg_2_m;
    }
    else if (tr.isModeSDetection()) // 4 Single ModeS All-Call, 5 Single ModeS Roll-Call
    {
        range_std_m = mode_s_range_stddev_;
        azm_std_m   = mode_s_azimuth_stddev_ * deg_2_m;
    }
    else
        assert (false); // no possible

    double rot_angle = bearing_rad; // somehow that's right

    Eigen::Matrix2f cov_polar; // covariance matrix
    cov_polar << pow(range_std_m, 2), 0, 0, pow(azm_std_m, 2);

    Eigen::Matrix2f A; // rotation matrix
    A << sin(rot_angle), cos(rot_angle),
        cos(rot_angle), -sin(rot_angle);

    Eigen::Matrix2f cov_cart = A * cov_polar * A.transpose(); // rotated covariance matrix

    x_stddev = sqrt(cov_cart(0, 0));
    y_stddev = sqrt(cov_cart(1, 1));
    xy_cov   = cov_cart(0, 1);

    return dbContent::targetReport::PositionAccuracy (x_stddev, y_stddev, xy_cov);
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
