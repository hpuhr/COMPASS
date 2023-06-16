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
#include "viewpointgenerator.h"
#include "spline_interpolator.h"
#include "logger.h"

#include <ogr_spatialref.h>

namespace reconstruction
{

const QColor Reconstructor::ColorResampledMM = QColor(123, 123, 123);

/**
*/
Reconstructor::Reconstructor() = default;

/**
*/
Reconstructor::~Reconstructor() = default;

/**
*/
void Reconstructor::reset()
{
    for (auto& info : dbcontent_infos_)
        info.second.count = 0;

    measurements_.clear();
    sources_.clear();
    source_uncerts_.clear();
    ref_src_.reset();
    ref_dst_.reset();
    trafo_fwd_.reset();
    trafo_bwd_.reset();

    x_offs_ = 0.0;
    y_offs_ = 0.0;

    min_height_.reset();
    max_height_.reset();

    source_cnt_ = 0;

    vp_data_interp_.clear();

    reset_impl();
}

/**
*/
void Reconstructor::setViewPoint(ViewPointGenVP* vp, int viewpoint_detail)
{
    viewpoint_        = vp;
    viewpoint_detail_ = viewpoint_detail;
}

/**
*/
bool Reconstructor::hasViewPoint() const
{
    return (viewpoint_ != nullptr);
}

/**
*/
ViewPointGenVP* Reconstructor::viewPoint() const
{
    return viewpoint_;
}

/**
*/
int Reconstructor::viewPointDetail() const
{
    return viewpoint_detail_;
}

/**
*/
void Reconstructor::setCoordConversion(CoordConversion coord_conv)
{
    coord_conv_ = coord_conv;
}

/**
*/
void Reconstructor::setSensorUncertainty(const std::string& dbcontent, const Uncertainty& uncert)
{
    dbcontent_infos_[dbcontent].uncert = uncert;
}

/**
*/
void Reconstructor::setSensorInterpolation(const std::string& dbcontent, const InterpOptions& options)
{
    dbcontent_infos_[dbcontent].interp_options = options;
}

/**
*/
const boost::optional<Uncertainty>& Reconstructor::sourceUncertainty(uint32_t source_id) const
{
    return source_uncerts_.at(source_id);
}

/**
*/
void Reconstructor::addMeasurements(const std::vector<Measurement>& measurements)
{
    if (!measurements.empty())
        measurements_.insert(measurements_.end(), measurements.begin(), measurements.end());
}

namespace
{
    bool mmSortPred(const Measurement& mm0, const Measurement& mm1)
    { 
        //sort by time first
        if (mm0.t != mm1.t)
            return mm0.t < mm1.t;

        //otherwise sort by source at least
        return mm0.source_id < mm1.source_id; 
    }
}

/**
*/
void Reconstructor::interpolateMeasurements(std::vector<Measurement>& measurements, 
                                            const InterpOptions& options) const
{
    //sort measurements by timestamp
    std::sort(measurements.begin(), measurements.end(), mmSortPred);

    SplineInterpolator interp;
    interp.config().check_fishy_segments = true;
    interp.config().interpolate_cart     = false;
    interp.config().sample_dt            = options.sample_dt;
    interp.config().max_dt               = options.max_dt;

    //interpolate using desired timestep
    auto mms_interp = interp.interpolate(measurements);
    
    size_t ni = mms_interp.size();
    if (ni < 1)
        return;

    //assign new measurements
    measurements.resize(ni);

    for (size_t i = 0; i < ni; ++i)
    {
        measurements[ i ] = mms_interp[ i ];
    }

    if (hasViewPoint())
    {
        VPTrackData vp_data;

        std::vector<Eigen::Vector2d> positions(ni);
        for (size_t i = 0; i < ni; ++i)
            positions[ i ] = measurements[ i ].position2D(CoordSystem::WGS84);
  
        vp_data.positions = std::move(positions);
        vp_data.color     = ColorResampledMM;

        vp_data_interp_.push_back(vp_data);
    }
}

/**
*/
void Reconstructor::addChain(const dbContent::TargetReport::Chain* tr_chain, const std::string& dbcontent)
{
    assert(tr_chain);

    uint32_t source_id = source_cnt_++;
    sources_[source_id] = tr_chain;

    source_uncerts_.push_back({}); //add empty uncertainty

    DBContentInfo& info = dbcontent_infos_[dbcontent];

    //if uncertainty is stored for dbcontent => assign
    if (info.uncert.has_value())
        source_uncerts_.back() = info.uncert.value();

    const auto& indices = tr_chain->timestampIndexes();

    info.count += indices.size();

    std::vector<Measurement> mms;
    mms.reserve(indices.size());

    for (const auto& index : indices)
    {
        Measurement mm;
        mm.source_id = source_id;
        mm.t         = index.first;

        boost::optional<dbContent::TargetPosition>         pos;
        boost::optional<dbContent::TargetVelocity>         speed;
        boost::optional<dbContent::TargetPositionAccuracy> accuracy_pos;
        boost::optional<dbContent::TargetVelocityAccuracy> accuracy_vel;

        if (tr_chain->ignorePosition(index))
            continue;

        pos = tr_chain->pos(index);

        mm.lat = pos->latitude_;
        mm.lon = pos->longitude_;
        
        //kepp track of min/max height
        if (pos->has_altitude_ && (!min_height_.has_value() || pos->altitude_ < min_height_.value()))
            min_height_ = pos->altitude_;
        if (pos->has_altitude_ && (!max_height_.has_value() || pos->altitude_ > max_height_.value()))
            max_height_ = pos->altitude_;

        speed        = tr_chain->speed(index);
        accuracy_pos = tr_chain->posAccuracy(index);
        accuracy_vel = tr_chain->speedAccuracy(index);

        if (speed.has_value())
        {
            auto speed_vec = Utils::Number::speedAngle2SpeedVec(speed->speed_, speed->track_angle_);

            mm.vx = speed_vec.first;
            mm.vy = speed_vec.second;
            //@TODO: vz?
        }

        //@TODO: acceleration?

        if (accuracy_pos.has_value())
        {
            mm.x_stddev = accuracy_pos->x_stddev_;
            mm.y_stddev = accuracy_pos->y_stddev_;
            mm.xy_cov   = accuracy_pos->xy_cov_;
        }

        if (accuracy_vel.has_value())
        {
            mm.vx_stddev = accuracy_vel->vx_stddev_;
            mm.vy_stddev = accuracy_vel->vy_stddev_;
        }

        mms.push_back(mm);
    }

    //resample input data?
    if (info.interp_options.has_value())
    {
        //loginf << "Interpolating measurements of dbcontent " << dbcontent;
        interpolateMeasurements(mms, info.interp_options.value());
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
    std::sort(measurements_.begin(), measurements_.end(), mmSortPred);

    //convert coordinates if needed
    if (coord_conv_ == CoordConversion::WGS84ToCart)
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

        double lat0 = (lat_max + lat_min) / 2;
        double lon0 = (lon_max + lon_min) / 2;

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

        x_offs_ = lon0;
        y_offs_ = lat0;
        trafo_fwd_->Transform(1, &x_offs_, &y_offs_);

        for (auto& mm : measurements_)
        {
            mm.x -= x_offs_;
            mm.y -= y_offs_;
        }
    }

    if (hasViewPoint())
    {
        for (const auto& info : dbcontent_infos_)
            viewPoint()->addCustomField("#" + info.first, QVariant(info.second.count));

        if (min_height_.has_value())
            viewPoint()->addCustomField("min_altitude", QVariant(min_height_.value()));
        if (max_height_.has_value())
            viewPoint()->addCustomField("max_altitude", QVariant(max_height_.value()));
    }
}

/**
*/
void Reconstructor::postprocessReferences(std::vector<Reference>& references)
{
    //convert coordinates back if needed
    if (coord_conv_ == CoordConversion::WGS84ToCart)
    {
        assert(trafo_bwd_);

        for (auto& ref : references)
        {
            ref.x += x_offs_;
            ref.y += y_offs_;

            ref.lon = ref.x;
            ref.lat = ref.y;
            trafo_bwd_->Transform(1, &ref.lon, &ref.lat);
        }
    }
}

/**
 * Transforms back a position to the input coordinate system.
 */
Eigen::Vector2d Reconstructor::transformBack(double x, double y) const
{
    if (coord_conv_ == CoordConversion::WGS84ToCart)
    {
        assert(trafo_bwd_);

        double lon = x + x_offs_;
        double lat = y + y_offs_;

        trafo_bwd_->Transform(1, &lon, &lat);

        x = lat;
        y = lon;
    }

    return Eigen::Vector2d(x, y);
}

/**
 * Transforms back a vector to the input coordinate system.
 */
Eigen::Vector2d Reconstructor::transformBack(double x, double y, double vx, double vy) const
{
    return transformBack(x + vx, y + vy) - transformBack(x, y);
}

/**
*/
boost::optional<std::vector<Reference>> Reconstructor::reconstruct(const std::string& data_info)
{
    std::string dinfo = data_info.empty() ? "data" : data_info;

    //loginf << "Reconstructing " << dinfo << " - " << measurements_.size() << " measurement(s)";

    if (measurements_.empty())
        return {};

    boost::optional<std::vector<Reference>> result;

    try
    {
        //postprocess added measurements
        postprocessMeasurements();

        //reconstruct
        result = reconstruct_impl(measurements_, dinfo);

        //postprocess result references
        if (result.has_value())
            postprocessReferences(result.value());
    }
    catch(const std::exception& ex)
    {
        logerr << "Reconstructor: reconstruct: " << ex.what();
        return {};
    }
    catch(...)
    {
        logerr << "Reconstructor: reconstruct: unknown error";
        return {};
    }

    if (hasViewPoint())
    {
        auto anno_interp_pos = viewPoint()->annotations().addAnnotation("interpolated_tracks");
        
        for (const auto& vp_data : vp_data_interp_)
        {
            //add spline-interpolated input tracks
            std::unique_ptr<ViewPointGenFeaturePoints> feat_interp_points;
            feat_interp_points.reset(new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::Square));
            feat_interp_points->setColor(vp_data.color);
            feat_interp_points->addPoints(vp_data.positions);

            std::unique_ptr<ViewPointGenFeatureLineString> feat_interp_lines;
            feat_interp_lines.reset(new ViewPointGenFeatureLineString(false));
            feat_interp_lines->setColor(vp_data.color);
            feat_interp_lines->addPoints(vp_data.positions);

            anno_interp_pos->addFeature(std::move(feat_interp_points));
            anno_interp_pos->addFeature(std::move(feat_interp_lines));
        }

        //std::stringstream sstrm;
        //viewPoint()->print(sstrm);
        //loginf << sstrm.str();
    }

    return result;
}

/**
*/
double Reconstructor::timestep(const Measurement& mm0, const Measurement& mm1)
{
    return Utils::Time::partialSeconds(mm1.t - mm0.t);
}

/**
*/
double Reconstructor::distance(const Measurement& mm0, const Measurement& mm1, CoordSystem coord_sys) const
{
    return mm0.distance(mm1, coord_sys);
}

/**
*/
std::vector<std::vector<Measurement>> Reconstructor::splitMeasurements(const std::vector<Measurement>& measurements,
                                                                       double max_dt)
{
    size_t n = measurements.size();
    if (n == 0)
        return {};

    std::vector<std::vector<Measurement>> split_measurements;
    std::vector<Measurement> current;

    current.push_back(measurements[ 0 ]);

    for (size_t i = 1; i < n; ++i)
    {
        const auto& mm0 = measurements[i - 1];
        const auto& mm1 = measurements[i    ];

        double dt = timestep(mm0, mm1);

        if (dt > max_dt)
        {
            if (current.size() > 0)
            {
                split_measurements.push_back(current);
                current.clear();
            }
        }

        current.push_back(mm1);
    }

    if (current.size() > 0)
        split_measurements.push_back(current);

    return split_measurements;
}

} // namespace reconstruction
