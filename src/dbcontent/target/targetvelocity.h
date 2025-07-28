#pragma once

#include <boost/optional.hpp>

#include <memory>

namespace dbContent {

class TargetVelocity
{
public:
    TargetVelocity() {}

    double track_angle_ {0}; // true north, deg
    double speed_ {0}; // m/s
};

class TargetVelocityAccuracy
{
public:
    TargetVelocityAccuracy() = default;
    TargetVelocityAccuracy(double vx_stddev, double vy_stddev)
        : vx_stddev_(vx_stddev), vy_stddev_(vy_stddev)
    {}

    double vx_stddev_ {0}; // m/s
    double vy_stddev_ {0}; // m/s
};

class DBContentAccessor;

boost::optional<TargetVelocityAccuracy> getVelocityAccuracy(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index);

boost::optional<TargetVelocityAccuracy> getVelocityAccuracyADSB(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index);

boost::optional<TargetVelocityAccuracy> getVelocityAccuracyTracker(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index);

}
