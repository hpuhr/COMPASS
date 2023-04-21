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
Reference ReconstructorInterp::generateReference(double lat, 
                                                 double lon, 
                                                 const boost::posix_time::ptime& t,
                                                 const Measurement& mm0,
                                                 const Measurement& mm1,
                                                 double interp_factor) const
{
    Reference ref;
    ref.source_id = mm0.source_id;
    ref.t         = t;
    ref.lat       = lat;
    ref.lon       = lon;

    //@TODO: !interpolate existing values!
    
    return ref;
}

/**
*/
Reference ReconstructorInterp::generateReference(const Measurement& mm) const
{
    return generateReference(mm.lat, mm.lon, mm.t, mm, mm, 0.0);
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
            
            double lat = (1.0 - f) * mm0.lat + f * mm1.lat;
            double lon = (1.0 - f) * mm0.lon + f * mm1.lon;

            references.push_back(generateReference(lat, lon, tcur, mm0, mm1, f));

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
boost::optional<std::vector<Reference>> ReconstructorInterp::reconstructPart(const std::vector<Measurement>& measurements, 
                                                                             const std::string& data_info) const
{
    size_t n = measurements.size();

    if (n == 0)
        return {};

    if (n < 4)
        return interpolateLinear(measurements);

    const double offs_lat = measurements[ 0 ].lat;
    const double offs_lon = measurements[ 0 ].lon;

    std::vector<double> x, y, params;
    std::vector<size_t> indices;

    params.reserve(n);
    x.reserve(n);
    y.reserve(n);
    indices.reserve(n);

    auto addMeasurement = [&] (size_t idx, double p)
    {
        const auto& mm = measurements[ idx ];

        x.push_back(mm.lat - offs_lat);
        y.push_back(mm.lon - offs_lon);
        indices.push_back(idx);
        params.push_back(p);
    };

    addMeasurement(0, 0.0);

    double total_len = 0.0;
    
    for (size_t i = 1; i < n; ++i)
    {
        auto idx_last = indices.back();

        double d  = distance(measurements[idx_last], measurements[i], CoordSystem::WGS84);
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

    const auto& mm_first = measurements[indices[0]];

    result.push_back(generateReference(mm_first));

    boost::posix_time::ptime tcur = mm_first.t;

    boost::posix_time::milliseconds time_incr((int)(config().sample_dt * 1000.0));

    for (size_t i = 1; i < np; ++i)
    {
        auto idx0 = indices[i - 1];
        auto idx1 = indices[i    ];

        const auto& mm0 = measurements[idx0];
        const auto& mm1 = measurements[idx1];

        auto t0 = mm0.t;
        auto t1 = mm1.t;

        double dt01 = Utils::Time::partialSeconds(t1 - t0);

        while (tcur >= t0 && tcur < t1)
        {
            double dt = Utils::Time::partialSeconds(tcur - t0);
            double f  = dt / dt01;
            double p  =(1.0 - f) * params[i - 1] + f * params[ i ];

            double lat = sx(p) + offs_lat;
            double lon = sy(p) + offs_lon;

            result.push_back(generateReference(lat, lon, tcur, mm0, mm1, f));

            tcur += time_incr;
        }
    }

    if (result.size() >= 2)
        result.pop_back();

    result.push_back(generateReference(measurements.back()));

    return result;
}

} // namespace reconstruction
