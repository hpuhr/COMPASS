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

#include "reconstructor.h"

#include "util/timeconv.h"
#include "util/number.h"
#include "targetreportchain.h"
#include "logger.h"

#include <ogr_spatialref.h>

namespace reconstruction
{

/**
*/
Reconstructor::Reconstructor() = default;

/**
*/
Reconstructor::~Reconstructor() = default;

/**
*/
void Reconstructor::addMeasurements(const std::vector<Measurement>& measurements)
{
    if (!measurements.empty())
        measurements_.insert(measurements_.end(), measurements.begin(), measurements.end());
}

/**
*/
void Reconstructor::addChain(const dbContent::TargetReport::Chain* tr_chain)
{
    assert(tr_chain);

    uint32_t source_id = source_cnt_++;
    sources_[source_id] = tr_chain;

    const auto& indices = tr_chain->timestampIndexes();

    std::vector<Measurement> mms;
    mms.reserve(indices.size());

    for (const auto& index : indices)
    {
        Measurement mm;
        mm.source_id = source_id;
        mm.t         = index.first;

        boost::optional<dbContent::TargetPosition>         pos;
        boost::optional<dbContent::TargetVelocity>         speed;
        boost::optional<dbContent::TargetPositionAccuracy> accuracy;

        pos = tr_chain->pos(index);

        mm.lat = pos->latitude_;
        mm.lon = pos->longitude_;
        //@TODO: altitude?

        speed    = tr_chain->speed(index);
        accuracy = tr_chain->posAccuracy(index);

        if (speed.has_value())
        {
            auto speed_vec = Utils::Number::speedAngle2SpeedVec(speed->speed_, speed->track_angle_);

            mm.vx = speed_vec.first;
            mm.vy = speed_vec.second;
            //@TODO: vz?
        }

        //@TODO: acceleration?

        if (accuracy.has_value())
        {
            mm.stddev_x = accuracy->x_stddev_;
            mm.stddev_y = accuracy->y_stddev_;
            mm.cov_xy   = accuracy->xy_cov_;
        }

        mms.push_back(mm);
    }

    if (!mms.empty())
        measurements_.insert(measurements_.end(), mms.begin(), mms.end());
}

/**
*/
const dbContent::TargetReport::Chain* Reconstructor::chainOfReference(const Reference& ref) const
{
    auto it = sources_.find(ref.source_id);
    if (it == sources_.end())
        return nullptr;

    return it->second;
}

/**
*/
void Reconstructor::postprocessMeasurements()
{
    if (measurements_.empty())
        return;
    
    //sort measurements by timestamp
    std::sort(measurements_.begin(), measurements_.end(), [&] (const Measurement& mm0, const Measurement& mm1) { return mm0.t < mm1.t; });

    //compute cartesian positions from lat lon
    if (coord_sys_ == CoordSystem::WGS84)
    {
        double lat_min = measurements_[ 0 ].lat;
        double lon_min = measurements_[ 0 ].lon;
        double lat_max = measurements_[ 0 ].lat;
        double lon_max = measurements_[ 0 ].lon;
        
        for (size_t i = 1; i < measurements_.size(); ++i)
        {
            const auto& mm  = measurements_[ i ];

            if (mm.lat < lat_min) lat_min = mm.lat;
            if (mm.lon < lon_min) lon_min = mm.lon;
            if (mm.lat > lat_max) lat_max = mm.lat;
            if (mm.lon > lon_max) lon_max = mm.lon;
        }

        double lat0 = (lat_max - lat_min) / 2;
        double lon0 = (lon_max - lon_min) / 2;

        ref_src_.reset(new OGRSpatialReference);
        ref_src_->SetWellKnownGeogCS("WGS84");

        ref_dst_.reset(new OGRSpatialReference);
        ref_dst_->SetStereographic(lat0, lon0, 1.0, 0.0, 0.0);

        trafo_fwd_.reset(OGRCreateCoordinateTransformation(ref_src_.get(), ref_dst_.get()));
        trafo_bwd_.reset(OGRCreateCoordinateTransformation(ref_dst_.get(), ref_src_.get()));

        for (auto& mm : measurements_)
        {
            mm.x = mm.lon;
            mm.y = mm.lat;
            trafo_fwd_->Transform(1, &mm.x, &mm.y);
        }
    }
}

/**
*/
void Reconstructor::postprocessReferences(std::vector<Reference>& references)
{
    if (coord_sys_ == CoordSystem::WGS84 && trafo_bwd_)
    {
        //compute lat lon from cartesian positions
        for (auto& ref : references)
        {
            ref.lon = ref.x;
            ref.lat = ref.y;
            trafo_bwd_->Transform(1, &ref.lon, &ref.lat);
        }
    }
}

/**
*/
boost::optional<std::vector<Reference>> Reconstructor::reconstruct(const std::string& data_info)
{
    std::string dinfo = data_info.empty() ? "data" : data_info;

    loginf << "Reconstructing " << dinfo << " - " << measurements_.size() << " measurement(s)";

    if (measurements_.empty())
        return {};

    try
    {
        //postprocess added measurements
        postprocessMeasurements();

        //reconstruct
        auto result = reconstruct_impl(measurements_, data_info);

        //postprocess result references
        if (result.has_value())
            postprocessReferences(result.value());

        return result;
    }
    catch(const std::exception& ex)
    {
        logerr << "Reconstructor::reconstruct(): " << ex.what();
    }
    catch(...)
    {
        logerr << "Reconstructor::reconstruct(): unknown error";
    }

    return {};
}

/**
*/
double Reconstructor::timestep(const Measurement& mm0, const Measurement& mm1)
{
    return Utils::Time::partialSeconds(mm1.t - mm0.t);
}

/**
*/
double Reconstructor::distance(const Measurement& mm0, const Measurement& mm1)
{
    if (mm0.z.has_value() && mm1.z.has_value())
        return (Eigen::Vector3d(mm0.x, mm0.y, mm0.z.value()) - Eigen::Vector3d(mm1.x, mm1.y, mm1.z.value())).norm();
    
    return (Eigen::Vector2d(mm0.x, mm0.y) - Eigen::Vector2d(mm1.x, mm1.y)).norm();
}

}
