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

#include "kalman_defs.h"
#include "measurement.h"

#include <ostream>

#include <boost/optional.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace reconstruction
{

enum MapProjectionMode
{
    MapProjectNone = 0, //no projection (cartesian coords pre-stored in the measurements are used,
                        //unprojection to geodetic should happen outside of reconstructor)

    MapProjectStatic,   //a single map projection is used for the whole track (origin of projection is set to
                        //center of data bounds, might result in inaccuracies in locations far away from the projection center)
    
    MapProjectDynamic   //the map projection varies dynamically based on a maximum distance threshold (most accurate,
                        //but results in some computation overhead)
};

enum class StateInterpMode
{
    BlendHalf,      // time-projected kalman states will be blended halfways
    BlendLinear,    // time-projected kalman states will be blended linearly with time
    BlendStdDev,    // states will be weighted by time-projected stddevs
    BlendVar        // states will be weighted by time-projected variances
};

/**
*/
struct Reference : public Measurement
{
    bool reset_pos      = false; // is this a position where kalman filter was resetted (e.g. due to long time offset)
    bool nospeed_pos    = false; // did this position not obtain speed information
    bool noaccel_pos    = false; // did this position not obtain acceleration information
    bool nostddev_pos   = false; // did this position not obtain stddev information
    bool projchange_pos = false; // position where a change of map projection happened 

    bool ref_interp     = false; // reference has been interpolated (e.g. from kalman samples)

    Eigen::MatrixXd cov; // covariance matrix
    double          dt;  // last timestep (only set for non-interpolated)
};

/**
*/
struct MeasurementInterp : public Measurement
{
    MeasurementInterp() = default;
    MeasurementInterp(const Measurement& mm, bool is_interp, bool is_corrected) : Measurement(mm), corrected(is_corrected) 
    {
        mm_interp = is_interp;
    }

    bool corrected = false; //interpolation failed, so the measurement was interpolated linearly
};

/**
*/
struct Uncertainty
{
    double pos_var   = 100.0;
    double speed_var = 100.0;
    double acc_var   = 100.0;
};

/**
*/
struct InterpOptions
{
    double sample_dt = 1.0;
    double max_dt    = 30.0;
};

/**
*/
struct UpdateStats
{
    bool   set         = false;
    size_t num_fresh   = 0;
    size_t num_updated = 0;
    size_t num_valid   = 0;
    size_t num_failed  = 0;
    size_t num_skipped = 0;
};

/**
*/
struct PredictionStats
{
    size_t num_predictions = 0;
    size_t num_failed      = 0;
    size_t num_fixed       = 0;
};

} // namespace reconstruction
