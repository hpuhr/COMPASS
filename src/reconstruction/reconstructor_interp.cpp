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

#include "reconstructor_interp.h"

#include "util/timeconv.h"

#include "spline.h"

namespace reconstruction
{

/**
*/
Reference ReconstructorInterp::generateReference(const Eigen::Vector2d& pos, 
                                                 const boost::posix_time::ptime& t,
                                                 const Measurement& mm0,
                                                 const Measurement& mm1,
                                                 double interp_factor) const
{
    Reference ref;
    ref.source_id = mm0.source_id;
    ref.t         = t;
    
    //set position
    position2D(ref, pos);

    //@TODO: !interpolate existing values!
    
    return ref;
}

/**
*/
Reference ReconstructorInterp::generateReference(const Measurement& mm) const
{
    return generateReference(position2D(mm), mm.t, mm, mm, 0.0);
}

/**
*/
std::vector<Reference> ReconstructorInterp::interpolateLinear(const std::vector<Measurement>& measurements) const
{
    if (measurements.empty())
        return {};

    size_t n = measurements.size();

    std::vector<Reference> references;
    references.push_back(generateReference(measurements[ 0 ]));

    if (n == 1)
        return references;

    auto tcur = measurements[ 0 ].t;

    boost::posix_time::milliseconds time_incr((int)(config().sample_dt * 1000.0));

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
                references.push_back(generateReference(measurements[ i ]));
                tcur += time_incr;
                break;
            }

            //interpolate between measurements
            double dt = Utils::Time::partialSeconds(tcur - t0);
            double f  = dt / dt01;

            auto pos_interp = pos0 * (1.0 - f) + pos1 * f;

            references.push_back(generateReference(pos_interp, tcur, mm0, mm1, f));

            tcur += time_incr;
        }
    }

    if (references.size() >= 2)
        references.pop_back();

    references.push_back(generateReference(measurements.back()));

    return references;
}

/**
*/
boost::optional<std::vector<Reference>> ReconstructorInterp::reconstruct_impl(const std::vector<Measurement>& measurements, 
                                                                              const std::string& data_info)
{
    size_t n = measurements.size();

    if (n == 0)
        return {};

    auto measurements_parts = Reconstructor::splitMeasurements(measurements, config().max_dt);

    std::vector<Reference> references;

    for (const auto& p : measurements_parts)
    {
        auto res = reconstructPart(p, data_info);
        if (res.has_value() && res->size() > 0)
            references.insert(references.begin(), res->begin(), res->end());
    }

    if (references.size() < 1)
        return {};

    return references;
}

/**
*/
bool ReconstructorInterp::isFishySegment(const Measurement& mm0, 
                                         const Measurement& mm1, 
                                         const std::vector<Reference>& refs,
                                         size_t n) const
{
    auto p0 = position2D(mm0);
    auto p1 = position2D(mm1);

    double d_seg = (p0 - p1).norm();
    double d_max = d_seg * config().max_segment_distance_factor;

    auto p_mid = (p0 + p1) / 2;
    
    for (size_t i = 0; i < n; ++i)
    {
        double d = (p_mid - position2D(refs[ i ])).norm();
        if (d > d_max)
            return true;
    }

    return false;
}

/**
*/
boost::optional<std::vector<Reference>> ReconstructorInterp::reconstructPart(const std::vector<Measurement>& measurements, 
                                                                             const std::string& data_info) const
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

        double d  = distance(measurements[idx_last], measurements[i]);
        double dt = timestep(measurements[idx_last], measurements[i]);

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

    std::vector<Reference> result;

    const auto& mm_first = measurements[indices.front()];
    const auto& mm_last  = measurements[indices.back() ];

    result.push_back(generateReference(mm_first));

    boost::posix_time::ptime tcur = mm_first.t;

    boost::posix_time::milliseconds time_incr((int)(config().sample_dt * 1000.0));

    //prereserve samples array
    size_t max_samples = std::ceil(timestep(mm_first, mm_last) / config().sample_dt) + 2;
    std::vector<Reference> tmp_refs(max_samples);

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
            tmp_refs[num_refs_segment++] = generateReference(pos_interp, t_cur_seg, mm0, mm1, f);

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

                tmp_refs[num_refs_segment++] = generateReference(pos_interp, t_cur_seg, mm0, mm1, f);

                t_cur_seg += time_incr;
            }
        }

        //collect temp references
        tcur = t_cur_seg;

        if (num_refs_segment > 0)
            result.insert(result.end(), tmp_refs.begin(), tmp_refs.begin() + num_refs_segment);
    }

    if (result.size() >= 2)
        result.pop_back();

    result.push_back(generateReference(measurements.back()));

    return result;
}

} // namespace reconstruction
