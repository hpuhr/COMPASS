/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <map>

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional.hpp>

namespace dbContent 
{
namespace TargetReport 
{

/**
*/
struct AccuracyTables
{
    static const std::map<int, float> adsb_nucr_nacv_accuracies;
    static const std::map<int, float> adsb_v0_accuracies;
    static const std::map<int, float> adsb_v12_accuracies;
};

/**
*/
struct Index
{
    Index() = default;
    Index(unsigned int idx_ext,
          unsigned int idx_int) : idx_external(idx_ext), idx_internal(idx_int) {}

    unsigned int idx_external; //external index (usually index into buffer)
    unsigned int idx_internal; //internal index (index into internal data structures)
};

/**
*/
class DataID
{
public:
    typedef std::pair<const boost::posix_time::ptime, Index> IndexPair;

    DataID() = default;
    DataID(const boost::posix_time::ptime& timestamp) : timestamp_(timestamp), valid_(true) {}
    DataID(const boost::posix_time::ptime& timestamp, const Index& index) : timestamp_(timestamp), index_(index), valid_(true) {}
    DataID(const IndexPair& ipair) : timestamp_(ipair.first), index_(ipair.second), valid_(true) {}
    virtual ~DataID() = default;

    bool valid() const { return valid_; }
    const boost::posix_time::ptime& timestamp() const { return timestamp_; }
    bool hasIndex() const { return index_.has_value(); }

    DataID& addIndex(const Index& index)
    {
        index_ = index;
        return *this;
    }

    const Index& index() const
    {
        if (!hasIndex())
            throw std::runtime_error("DataID: index: No index stored");
        return index_.value();
    }

private:
    boost::posix_time::ptime timestamp_;
    boost::optional<Index>   index_;
    bool                     valid_ = false;
};

/**
*/
struct TargetReportPos
{
    TargetReportPos() {}
    TargetReportPos(double lat, double lon) : latitude_(lat), longitude_(lon) {}

    double latitude_  {0}; // deg
    double longitude_ {0}; // deg
};

/**
*/
struct TargetReportAlt
{
    enum class Source
    {
        Barometric_ModeC = 0,
        Barometric_CAT062_Trusted,
        Barometric_CAT062_Secondary
    };

    TargetReportAlt() {}
    TargetReportAlt(Source source, 
                    float alt, 
                    const boost::optional<bool>& v,
                    const boost::optional<bool>& g)
    :   source_  (source)
    ,   altitude_(alt   )
    ,   valid_   (v     )
    ,   garbled_ (g     ) {}

    Source                source_   = Source::Barometric_ModeC;
    float                 altitude_ = 0.0f;
    boost::optional<bool> valid_;
    boost::optional<bool> garbled_;   
};

/**
*/
struct TargetReportPosAcc
{
    TargetReportPosAcc() = default;
    TargetReportPosAcc(double x_stddev, 
                       double y_stddev, 
                       double xy_cov)
    :   x_stddev_(x_stddev), 
        y_stddev_(y_stddev), 
        xy_cov_  (xy_cov  ) {}

    double x_stddev_ {0}; // m
    double y_stddev_ {0}; // m
    double xy_cov_   {0}; // m^2
};

/**
*/
struct TargetReportVel
{
    TargetReportVel() {}
    TargetReportVel(double track_angle, double speed) : track_angle_(track_angle), speed_(speed) {}

    double track_angle_ {0}; // true north, deg
    double speed_       {0}; // m/s
};

/**
*/
struct TargetReportVelAcc
{
    TargetReportVelAcc() = default;
    TargetReportVelAcc(double vx_stddev, double vy_stddev) : vx_stddev_(vx_stddev), vy_stddev_(vy_stddev) {}

    double vx_stddev_ {0}; // m/s
    double vy_stddev_ {0}; // m/s
};

} // namespace TargetReport

} // namespace dbContent
