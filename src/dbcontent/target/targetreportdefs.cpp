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

#include "targetreportdefs.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "util/number.h"
#include "compass.h"
#include "dbcontentmanager.h"

#include <boost/optional/optional_io.hpp>

#include <cassert>
#include <algorithm>
#include <sstream>

using namespace std;
using namespace Utils;

namespace dbContent 
{
namespace targetReport
{

// NUCr
//0 	N/A 	N/A
//1 	<10 m/s 	<15.2 m/s (50 fps)
//2 	<3 m/s 	<4.5 m/s (15 fps)
//3 	<1 m/s 	<1.5 m/s (5 fps)
//4 	<0.3 m/s 	<0.46 m/s (1.5 fps)

// NACv
//0 	N/A 	N/A
//1 	<10 m/s 	<15.2 m/s (50 fps)
//2 	<3 m/s 	<4.5 m/s (15 fps)
//3 	<1 m/s 	<1.5 m/s (5 fps)
//4 	<0.3 m/s 	<0.46 m/s (1.5 fps)

const std::map<int, float> AccuracyTables::adsb_nucr_nacv_accuracies =
    {
        {1, 5   },
        {2, 1.5 },
        {3, 0.5 },
        {4, 0.15}
};

// NUCp | HPL                | RCu                 |
// 9   & < 7.5 m            & < 3 m             & 1.5  \\ \hline
// 8   & < 25 m             & < 10 m            & 5  \\ \hline
// 7   & < 0.1 NM (185 m)   & < 0.05 NM (93 m)  & 46.5  \\ \hline
// 6   & < 0.2 NM (370 m)   & < 0.1 NM (185 m)  & 92.5  \\ \hline
// 5   & < 0.5 NM (926 m)   & < 0.25 NM (463 m) & 231.5  \\ \hline
// 4   & < 1 NM (1852 m)    & < 0.5 NM (926 m)  & 463  \\ \hline
// 3   & < 2 NM (3704 m)    & < 1 NM (1852 m)   & 926  \\ \hline
// 2   & < 10 NM (18520 m)  & < 5 NM (9260 m)   & 4630  \\ \hline
// 1   & < 20 NM (37040 m)  & < 10 NM (18520 m) & 9260  \\ \hline
// 0   & > 20 NM (37040 m)  & > 10 NM (18520 m) & -  \\ \hline

const std::map<int, float> AccuracyTables::adsb_v0_accuracies =
    {
        {0, 92600},
        {1, 9260 },
        {2, 4630 },
        {3, 926  },
        {4, 463  },
        {5, 231.5},
        {6, 92.5 },
        {7, 46.5 },
        {8, 5    },
        {9, 1.5  }
};

//    NACp | EPU (HFOM)         | VEPU (VFOM) |
//    11  & < 3 m              & < 4 m  & 1.5 \\ \hline
//    10  & < 10 m             & < 15 m & 5 \\ \hline
//    9   & < 30 m             & < 45 m & 15 \\ \hline
//    8   & < 0.05 NM (93 m)   & & 46.5 \\ \hline
//    7   & < 0.1 NM (185 m)   & & 92.5 \\ \hline
//    6   & < 0.3 NM (556 m)   & & 278 \\ \hline
//    5   & < 0.5 NM (926 m)   & & 463 \\ \hline
//    4   & < 1.0 NM (1852 m)  & & 926 \\ \hline
//    3   & < 2 NM (3704 m)    & & 1852 \\ \hline
//    2   & < 4 NM (7408 m)    & & 3704 \\ \hline
//    1   & < 10 NM (18520 km) & & 9260 \\ \hline
//    0   & > 10 NM or Unknown & & - \\ \hline|

const std::map<int, float> AccuracyTables::adsb_v12_accuracies =
    {
        { 0, 92600},
        { 1, 9260 },
        { 2, 3704 },
        { 3, 1852 },
        { 4, 926  },
        { 5, 463  },
        { 6, 278  },
        { 7, 92.5 },
        { 8, 46.5 },
        { 9, 15   },
        {10, 5    },
        {11, 1.5  }
};

std::string BaseInfo::asStr() const
{
    stringstream ss;

    ss << "dbcont " << COMPASS::instance().dbContentManager().dbContentWithId(Number::recNumGetDBContId(record_num_))
       << " ds_id " << ds_id_  << " line_id " << line_id_ << " rec_num " << record_num_
       << " ts " << Time::toString(timestamp_);

    return ss.str();
}

PositionAccuracy PositionAccuracy::getScaledToMinStdDev (double min_std_dev) const
{
    PositionAccuracy ret = *this;

    ret.scaleToMinStdDev(min_std_dev);

    return ret;
}

void PositionAccuracy::scaleToMinStdDev(double min_stddev)
{
    // Check if either standard deviation is below the minimum threshold
    if (x_stddev_ < min_stddev || y_stddev_ < min_stddev) {
        // Calculate the scaling factor to bring the smallest sigma up to min_stddev
        double scale_factor_x = (x_stddev_ < min_stddev) ? min_stddev / x_stddev_ : 1.0;
        double scale_factor_y = (y_stddev_ < min_stddev) ? min_stddev / y_stddev_ : 1.0;

        // Use the larger scaling factor to maintain the relative covariance structure
        double scale_factor = std::max(scale_factor_x, scale_factor_y);

        // Apply scaling
        if (std::isfinite(scale_factor))
        {
            x_stddev_ *= scale_factor;
            y_stddev_ *= scale_factor;
            xy_cov_ *= scale_factor * scale_factor; // Covariance scales with the square of the factor
        }
        else
        {
            if (x_stddev_ < min_stddev)
                x_stddev_ = min_stddev;

            if (y_stddev_ < min_stddev)
                y_stddev_ = min_stddev;
        }
    }
}

VelocityAccuracy VelocityAccuracy::getScaledToMinStdDev (double min_std_dev) const
{
    VelocityAccuracy ret = *this;

    ret.scaleToMinStdDev(min_std_dev);

    return ret;
}

void VelocityAccuracy::scaleToMinStdDev(double min_stddev)
{
    // Check if either standard deviation is below the minimum threshold
    if (vx_stddev_ < min_stddev || vy_stddev_ < min_stddev) {
        // Calculate the scaling factor to bring the smallest sigma up to min_stddev
        double scale_factor_x = (vx_stddev_ < min_stddev) ? min_stddev / vx_stddev_ : 1.0;
        double scale_factor_y = (vy_stddev_ < min_stddev) ? min_stddev / vy_stddev_ : 1.0;

        // Use the larger scaling factor to maintain the relative covariance structure
        double scale_factor = std::max(scale_factor_x, scale_factor_y);

        // Apply scaling
        if (std::isfinite(scale_factor))
        {
            vx_stddev_ *= scale_factor;
            vy_stddev_ *= scale_factor;
        }
        else
        {
            if (vx_stddev_ < min_stddev)
                vx_stddev_ = min_stddev;

            if (vy_stddev_ < min_stddev)
                vy_stddev_ = min_stddev;
        }
    }
}



boost::optional<targetReport::Position>& ReconstructorInfo::position()
{
    if (position_corrected_)
        return position_corrected_;

    return position_;
}

const boost::optional<targetReport::Position>& ReconstructorInfo::position() const
{
    if (position_corrected_)
        return position_corrected_;

    return position_;
}

std::string ReconstructorInfo::asStr() const
{
    stringstream ss;

    ss << BaseInfo::asStr();

    ss << " acad " << (acad_ ? String::hexStringFromInt(*acad_, 6, '0') : "''")
       << " acid '" << (acid_ ? *acid_ : "")  << "'"
       << " m3a " << (mode_a_code_ ? mode_a_code_->asStr() : "");

    if (track_number_)
        ss << " tn " << *track_number_;
    if (track_begin_)
        ss << " tbeg " << *track_begin_;
    if (track_end_)
        ss << " tend " << *track_end_;

    ss << " curslc " << in_current_slice_;

    return ss.str();
}

bool ReconstructorInfo::isModeSDetection() const
{
    return acad_ || acid_;
}

bool ReconstructorInfo::isModeACDetection() const
{
    return !isModeSDetection() && (mode_a_code_ || barometric_altitude_);
}

bool ReconstructorInfo::isPrimaryOnlyDetection() const
{
    return !isModeSDetection() && !isModeACDetection();
}

bool ReconstructorInfo::isOnGround() const
{
    if (ground_bit_)
        return *ground_bit_;

    if (barometric_altitude_ && barometric_altitude_->altitude_ < 1000.0)
        return true;

    return false;
}

bool ReconstructorInfo::doNotUsePosition() const
{
    return unsused_ds_pos_ || invalidated_pos_ || is_pos_outlier_;
}

std::string ModeACode::asStr() const
{
    stringstream ss;

    ss << String::octStringFromInt(code_, 4, '0')
       << (valid_ ? (*valid_ ? "V" : "I") : "")
       << (garbled_ ? (*garbled_ ? "G" : "") : "")
       << (smoothed_ ? (*smoothed_ ? "S" : "") : "");

    return ss.str();
}

std::string PositionAccuracy::asStr() const
{
    return "x " + String::doubleToStringPrecision(x_stddev_, 2)
           +" y " + String::doubleToStringPrecision(y_stddev_, 2)
           +" cov " + String::doubleToStringPrecision(xy_cov_, 2);
}

} // namespace TargetReport

} // namespace dbContent
