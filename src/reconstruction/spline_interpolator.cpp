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

#include "spline_interpolator.h"
#include "reconstructor.h"

#include "util/number.h"
#include "util/timeconv.h"

#include "spline.h"

namespace reconstruction
{

namespace
{
    double interp(double v0, double v1, double f)
    {
        return (1.0 - f) * v0 + f * v1;
    }

    void interpolateVector2D(boost::optional<double>& vx_interp,
                             boost::optional<double>& vy_interp,
                             const boost::optional<double>& vx0,
                             const boost::optional<double>& vy0,
                             const boost::optional<double>& vx1,
                             const boost::optional<double>& vy1,
                             double interp_factor)
    {
        //convert to polar
        auto speed_angle0 = Utils::Number::speedVec2SpeedAngle(vx0.value(), vy0.value());
        auto speed_angle1 = Utils::Number::speedVec2SpeedAngle(vx1.value(), vy1.value());

        if (speed_angle0.first < 1e-07 || speed_angle1.first < 1e-07)
        {
            //speed value very small => interpolate speed vectors linear instead
            vx_interp = interp(vx0.value(), vx1.value(), interp_factor);
            vy_interp = interp(vy0.value(), vy1.value(), interp_factor);
        }
        else
        {
            //interpolate speed and bearing individually
            double speed_interp = interp(speed_angle0.first, speed_angle1.first, interp_factor);
#if 0
            auto pos0 = position2D(mm0);
            auto pos1 = position2D(mm1);
            double angle_interp = Utils::Number::interpolateBearing(pos0.x(), pos0.y(), pos1.x(), pos1.y(), speed_angle0.second, speed_angle1.second, interp_factor);
#else
            double da           = Utils::Number::calculateMinAngleDifference(speed_angle1.second, speed_angle0.second);
            double angle_interp = speed_angle0.second + interp_factor * da;
#endif
            auto speedvec_interp = Utils::Number::speedAngle2SpeedVec(speed_interp, angle_interp);
            vx_interp = speedvec_interp.first;
            vy_interp = speedvec_interp.second;
        }
    }
}

/**
*/
Eigen::Vector2d SplineInterpolator::position2D(const Measurement& mm) const
{
    return mm.position2D(config_.interpolate_cart ? CoordSystem::Cart :
                                                    CoordSystem::WGS84);
}

/**
*/
void SplineInterpolator::position2D(Measurement& mm, const Eigen::Vector2d& pos) const
{
    return mm.position2D(pos, config_.interpolate_cart ? CoordSystem::Cart :
                                                         CoordSystem::WGS84);
}

/**
*/
MeasurementInterp SplineInterpolator::generateMeasurement(const Eigen::Vector2d& pos, 
                                                          const boost::posix_time::ptime& t,
                                                          const Measurement& mm0,
                                                          const Measurement& mm1,
                                                          double interp_factor,
                                                          bool corrected) const
{
    if (interp_factor == 0.0)
        return MeasurementInterp(mm0, false, corrected);
    if (interp_factor == 1.0)
        return MeasurementInterp(mm1, false, corrected);

    MeasurementInterp mm_interp;
    mm_interp.source_id    = mm0.source_id;
    mm_interp.t            = t;
    mm_interp.interpolated = true;
    mm_interp.corrected    = corrected;

    assert(interp_factor >= 0.0 && interp_factor <= 1.0);
    
    //set position
    position2D(mm_interp, pos);

    if (mm0.hasVelocity() && mm1.hasVelocity())
    {
        interpolateVector2D(mm_interp.vx, mm_interp.vy, mm0.vx, mm0.vy, mm1.vx, mm1.vy, interp_factor);
    }
    if (mm0.hasAcceleration() && mm1.hasAcceleration())
    {
        interpolateVector2D(mm_interp.ax, mm_interp.ay, mm0.ax, mm0.ay, mm1.ax, mm1.ay, interp_factor);
    }
    if (mm0.hasStdDevPosition() && mm1.hasStdDevPosition())
    {
        mm_interp.x_stddev = interp(mm0.x_stddev.value(), mm1.x_stddev.value(), interp_factor);
        mm_interp.y_stddev = interp(mm0.y_stddev.value(), mm1.y_stddev.value(), interp_factor);
    }
    if (mm0.xy_cov.has_value() && mm1.xy_cov.has_value())
    {
        mm_interp.xy_cov = interp(mm0.xy_cov.value(), mm1.xy_cov.value(), interp_factor);
    }
    if (mm0.hasStdDevVelocity() && mm1.hasStdDevVelocity())
    {
        mm_interp.vx_stddev = interp(mm0.vx_stddev.value(), mm1.vx_stddev.value(), interp_factor);
        mm_interp.vy_stddev = interp(mm0.vy_stddev.value(), mm1.vy_stddev.value(), interp_factor);
    }

    return mm_interp;
}

/**
*/
MeasurementInterp SplineInterpolator::generateMeasurement(const Measurement& mm) const
{
    return generateMeasurement(position2D(mm), mm.t, mm, mm, 0.0, false);
}

/**
*/
size_t SplineInterpolator::estimatedSamples(const Measurement& mm0, 
                                            const Measurement& mm1,
                                            double dt) const
{
    return (size_t)(std::ceil(Reconstructor::timestep(mm0, mm1) / dt) + 2);
}

/**
*/
void SplineInterpolator::finalizeInterp(std::vector<MeasurementInterp>& samples, const Measurement& mm_last) const
{
    //remove last sample
    if (samples.size() >= 2)
        samples.pop_back();

    //replace with last input measurement
    samples.push_back(generateMeasurement(mm_last));
}

/**
*/
std::vector<MeasurementInterp> SplineInterpolator::interpolateLinear(const std::vector<Measurement>& measurements) const
{
    if (measurements.empty())
        return {};

    size_t n = measurements.size();

    std::vector<MeasurementInterp> interpolation;
    interpolation.push_back(generateMeasurement(measurements[ 0 ]));

    if (n == 1)
        return interpolation;

    auto tcur = measurements[ 0 ].t;

    boost::posix_time::milliseconds time_incr((int)(config().sample_dt * 1000.0));

    interpolation.reserve(estimatedSamples(measurements.front(), measurements.back(), config_.sample_dt));

    for (size_t i = 1; i < n; ++i)
    {
        const auto& mm0 = measurements[i - 1];
        const auto& mm1 = measurements[i    ];

        auto pos0 = position2D(mm0);
        auto pos1 = position2D(mm1);

        auto t0 = mm0.t;
        auto t1 = mm1.t;

        double dt01 = Utils::Time::partialSeconds(t1 - t0);

        while (tcur >= t0 && tcur < t1)
        {
            if (dt01 < config().min_dt)
            {
                //very small time interval => store second measurement and leave interval
                interpolation.push_back(generateMeasurement(measurements[ i ]));
                tcur += time_incr;
                break;
            }

            //interpolate between measurements
            double dt = Utils::Time::partialSeconds(tcur - t0);
            double f  = dt / dt01;

            auto pos_interp = pos0 * (1.0 - f) + pos1 * f;

            interpolation.push_back(generateMeasurement(pos_interp, tcur, mm0, mm1, f, false));

            tcur += time_incr;
        }
    }

    finalizeInterp(interpolation, measurements.back());

    interpolation.shrink_to_fit();

    return interpolation;
}

/**
*/
std::vector<MeasurementInterp> SplineInterpolator::interpolate(const std::vector<Measurement>& measurements)
{
    size_t n = measurements.size();

    if (n == 0)
        return {};

    //split up measurements time-based
    auto measurements_parts = Reconstructor::splitMeasurements(measurements, config().max_dt);

    std::vector<MeasurementInterp> interpolation;

    //interpolate individual parts
    for (const auto& p : measurements_parts)
    {
        auto res = interpolatePart(p);

        if (res.size() > 0)
            interpolation.insert(interpolation.begin(), res.begin(), res.end());
    }
    
    return interpolation;
}

/**
*/
bool SplineInterpolator::isFishySegment(const Measurement& mm0, 
                                        const Measurement& mm1, 
                                        const std::vector<MeasurementInterp>& segment,
                                        size_t n) const
{
    auto p0 = position2D(mm0);
    auto p1 = position2D(mm1);

    double d_seg = (p0 - p1).norm();
    double d_max = d_seg * config().max_segment_distance_factor;

    auto p_mid = (p0 + p1) / 2;
    
    for (size_t i = 0; i < n; ++i)
    {
        double d = (p_mid - position2D(segment[ i ])).norm();
        if (d > d_max)
            return true;
    }

    return false;
}

/**
*/
std::vector<MeasurementInterp> SplineInterpolator::interpolatePart(const std::vector<Measurement>& measurements) const
{
    size_t n = measurements.size();

    if (n == 0)
        return {};

    if (n < 4)
        return interpolateLinear(measurements);

    auto offs = position2D(measurements[ 0 ]);

    std::vector<double> x, y, params;
    std::vector<size_t> indices;

    params.reserve(n);
    x.reserve(n);
    y.reserve(n);
    indices.reserve(n);

    auto addMeasurement = [&] (size_t idx, double p)
    {
        const auto& mm = measurements[ idx ];

        Eigen::Vector2d pos = position2D(mm) - offs;

        x.push_back(pos[ 0 ]);
        y.push_back(pos[ 1 ]);
        indices.push_back(idx);
        params.push_back(p);
    };

    addMeasurement(0, 0.0);

    double total_len = 0.0;
    
    for (size_t i = 1; i < n; ++i)
    {
        auto idx_last = indices.back();

        double d  = (position2D(measurements[idx_last])- position2D(measurements[i])).norm();
        double dt = Reconstructor::timestep(measurements[idx_last], measurements[i]);

        if (d < config().min_len || dt < config().min_dt)
            continue;

        total_len += d;

        addMeasurement(i, total_len);
    }

    size_t np = params.size();

    if (np < 4)
        return interpolateLinear(measurements);

    for (double& p : params)
        p /= total_len;

    tk::spline sx(params, x);
    tk::spline sy(params, y);

    std::vector<MeasurementInterp> result;

    const auto& mm_first = measurements[indices.front()];
    const auto& mm_last  = measurements[indices.back() ];

    result.push_back(generateMeasurement(mm_first));

    boost::posix_time::ptime tcur = mm_first.t;

    boost::posix_time::milliseconds time_incr((int)(config_.sample_dt * 1000.0));

    //prereserve samples array
    size_t max_samples = estimatedSamples(mm_first, mm_last, config_.sample_dt);
    std::vector<MeasurementInterp> tmp_refs(max_samples);

    result.reserve(max_samples);

    for (size_t i = 1; i < np; ++i)
    {
        auto idx0 = indices[i - 1];
        auto idx1 = indices[i    ];

        const auto& mm0 = measurements[idx0];
        const auto& mm1 = measurements[idx1];

        auto t0 = mm0.t;
        auto t1 = mm1.t;

        double dt01 = Utils::Time::partialSeconds(t1 - t0);

        size_t num_refs_segment = 0;

        auto t_cur_seg = tcur;

        //interpolate segment with spline
        while (t_cur_seg >= t0 && t_cur_seg < t1)
        {
            double dt = Utils::Time::partialSeconds(t_cur_seg - t0);
            double f  = dt / dt01;
            double p  =(1.0 - f) * params[i - 1] + f * params[ i ];

            Eigen::Vector2d pos_interp(sx(p), sy(p));
            pos_interp += offs;

            assert(num_refs_segment < tmp_refs.size());

            //collect temporary references
            tmp_refs[num_refs_segment++] = generateMeasurement(pos_interp, t_cur_seg, mm0, mm1, f, false);

            t_cur_seg += time_incr;
        }

        //check segment references => segment fishy?
        if (config().check_fishy_segments && isFishySegment(mm0, mm1, tmp_refs, num_refs_segment))
        {
            //reset and interpolate segment linear instead
            t_cur_seg        = tcur;
            num_refs_segment = 0;

            auto pos0 = position2D(mm0);
            auto pos1 = position2D(mm1);

            while (t_cur_seg >= t0 && t_cur_seg < t1)
            {
                double dt = Utils::Time::partialSeconds(t_cur_seg - t0);
                double f  = dt / dt01;

                Eigen::Vector2d pos_interp = pos0 * (1.0 - f) + pos1 * f;
            
                assert(num_refs_segment < tmp_refs.size());

                tmp_refs[num_refs_segment++] = generateMeasurement(pos_interp, t_cur_seg, mm0, mm1, f, true);

                t_cur_seg += time_incr;
            }
        }

        //collect temp references
        tcur = t_cur_seg;

        if (num_refs_segment > 0)
            result.insert(result.end(), tmp_refs.begin(), tmp_refs.begin() + num_refs_segment);
    }

    finalizeInterp(result, measurements.back());

    result.shrink_to_fit();

    return result;
}

} // namespace reconstruction
