#ifndef DBCONTENT_TARGETPOSITIONACCURACY_H
#define DBCONTENT_TARGETPOSITIONACCURACY_H

#include <boost/optional.hpp>

#include <memory>

namespace dbContent {

class TargetPositionAccuracy
{
public:
    TargetPositionAccuracy() = default;
    TargetPositionAccuracy(double x_stddev, double y_stddev, double xy_cov)
        : x_stddev_(x_stddev), y_stddev_(y_stddev), xy_cov_(xy_cov)
    {}

    double x_stddev_ {0}; // m
    double y_stddev_ {0}; // m
    double xy_cov_ {0}; // m^2
};

class DBContentAccessor;
class BufferAccessor;

boost::optional<TargetPositionAccuracy> getPositionAccuracy(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index);

boost::optional<TargetPositionAccuracy> getRadarPositionAccuracy(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index);

boost::optional<TargetPositionAccuracy> getADSBPositionAccuracy(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index);

// cat010, cat020, cat062, reftraj
boost::optional<TargetPositionAccuracy> getXYPositionAccuracy(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index);

} // namespace dbContent

#endif // DBCONTENT_TARGETPOSITIONACCURACY_H
