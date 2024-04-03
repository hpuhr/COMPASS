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

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional.hpp>

#include <map>
#include <memory>


namespace dbContent 
{

class DBContentAccessor;

namespace targetReport
{


class TargetReportAccessor;

/**
*/
struct Position
{
    Position() {}
    Position(double lat, double lon) : latitude_(lat), longitude_(lon) {}

    double latitude_  {0}; // deg
    double longitude_ {0}; // deg
};

/**
*/
class BarometricAltitude
{
  public:

    enum class Source
    {
        Barometric_ModeC = 0,
        Barometric_CAT062_Trusted,
        Barometric_CAT062_Secondary
    };

    BarometricAltitude() {}
    BarometricAltitude(Source source,
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

    bool hasReliableValue () const
    {
        if (valid_ && *valid_ == false)
            return false;

        if (garbled_ && *garbled_ == true)
            return false;

        return true;
    }
};

/**
*/
struct PositionAccuracy
{
    PositionAccuracy() = default;
    PositionAccuracy(double x_stddev,
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
struct AccuracyTables
{
    static const std::map<int, float> adsb_nucr_nacv_accuracies;
    static const std::map<int, float> adsb_v0_accuracies;
    static const std::map<int, float> adsb_v12_accuracies;
};


/**
*/
struct Velocity
{
    Velocity() {}
    Velocity(double track_angle, double speed) : track_angle_(track_angle), speed_(speed) {}

    double track_angle_ {0}; // true north, deg
    double speed_       {0}; // m/s
};

/**
*/
struct VelocityAccuracy
{
    VelocityAccuracy() = default;
    VelocityAccuracy(double vx_stddev, double vy_stddev) : vx_stddev_(vx_stddev), vy_stddev_(vy_stddev) {}

    double vx_stddev_ {0}; // m/s
    double vy_stddev_ {0}; // m/s
};

struct AccelerationAccuracy
{
    AccelerationAccuracy() = default;
    AccelerationAccuracy(double ax_stddev, double ay_stddev) : ax_stddev_(ax_stddev), ay_stddev_(ay_stddev) {}

    double ax_stddev_ {0}; // m/s^2
    double ay_stddev_ {0}; // m/s^2
};

class ModeACode
{
  public:

    ModeACode() {}
    ModeACode(unsigned int code,
              const boost::optional<bool>& v,
              const boost::optional<bool>& g,
              const boost::optional<bool>& smoothed)
        :   code_  (code)
          ,   valid_   (v)
          ,   garbled_ (g)
          ,   smoothed_(smoothed)
    {}

    unsigned int code_;
    boost::optional<bool> valid_;
    boost::optional<bool> garbled_;
    boost::optional<bool> smoothed_;

    bool hasReliableValue () const
    {
        if (valid_ && *valid_ == false)
            return false;

        if (garbled_ && *garbled_ == true)
            return false;

        return true;
    }

    virtual std::string asStr() const;
};

struct BaseInfo
{
    unsigned int buffer_index_ {0};
    unsigned long record_num_ {0};
    unsigned int ds_id_ {0};
    unsigned int line_id_ {0};
    boost::posix_time::ptime timestamp_;

    virtual std::string asStr() const;
};


struct ReconstructorInfo : public BaseInfo
{
    bool in_current_slice_ {false}; // true in current, false if from old slice (glue data)

    boost::optional<unsigned int> acad_;
    boost::optional<std::string> acid_;
    boost::optional<targetReport::ModeACode> mode_a_code_;

    boost::optional<unsigned int> track_number_;
    boost::optional<bool> track_begin_;
    boost::optional<bool> track_end_;

    boost::optional<targetReport::Position> position_;
    boost::optional<targetReport::PositionAccuracy> position_accuracy_;
    bool do_not_use_position_ {false};

    boost::optional<targetReport::BarometricAltitude> barometric_altitude_;

    boost::optional<targetReport::Velocity> velocity_;
    boost::optional<targetReport::VelocityAccuracy> velocity_accuracy_;

    boost::optional<double> track_angle_;
    boost::optional<bool> ground_bit_;

    TargetReportAccessor* accessor_ {nullptr};

    virtual std::string asStr() const;

    bool isModeSDetection() const;
    bool isModeACDetection() const;
    bool isPrimaryOnlyDetection() const;
};

// tmp list

// flag old/current slice

// secondary stuff: acid, acad, mode a, mode c
// position, corrected position, position acc, corrected pos acc
// velocity, velocity acc
// ground bit

// Kalman update (x,P,F,Q, ...)



//class PerDSChain
//{
//public:
//    PerDSChain(std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name,
//             unsigned int ds_id);
//    virtual ~PerDSChain();

//    void addIndex (boost::posix_time::ptime timestamp, unsigned int index);

//    bool hasData() const;
//    unsigned int size () const;
//};

//class FullChain
//{

//};


} // namespace targetReport

} // namespace dbContent
