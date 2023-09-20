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

#include "reconstructor_defs.h"

#include <vector>

#include <QColor>
#include <QRectF>

namespace dbContent 
{
    namespace TargetReport 
    {
        class Chain;
    }
}

class ViewPointGenVP;
class ViewPointGenAnnotation;

namespace reconstruction
{

/**
*/
class Reconstructor
{
public:
    typedef std::map<uint32_t, const dbContent::TargetReport::Chain*> SourceMap;

    Reconstructor();
    virtual ~Reconstructor();

    void reset();

    void setSensorUncertainty(const std::string& dbcontent, const Uncertainty& uncert);
    void setSensorInterpolation(const std::string& dbcontent, const InterpOptions& options);

    void addMeasurements(const std::vector<Measurement>& measurements);
    void addChain(const dbContent::TargetReport::Chain* tr_chain, const std::string& dbcontent);

    boost::optional<std::vector<Reference>> reconstruct(const std::string& data_info = "");

    const dbContent::TargetReport::Chain* chainOfReference(const Reference& ref) const;

    void setVerbosity(int v) { verbosity_ = v; }
    int verbosity() const { return verbosity_; }

    void setViewPoint(ViewPointGenVP* vp, int viewpoint_detail = 1);

    virtual bool tracksVelocity() const = 0;
    virtual bool tracksAcceleration() const = 0;

    static std::vector<std::vector<Measurement>> splitMeasurements(const std::vector<Measurement>& measurements,
                                                                   double max_dt);
    static double timestep(const Measurement& mm0, const Measurement& mm1);

    static const QColor ColorResampledMM;

protected:
    virtual boost::optional<std::vector<Reference>> reconstruct_impl(std::vector<Measurement>& measurements, 
                                                                     const std::string& data_info) = 0;
    virtual void reset_impl() {}

    double distance(const Measurement& mm0, const Measurement& mm1, CoordSystem coord_sys) const;
    double distanceSqr(const Measurement& mm0, const Measurement& mm1, CoordSystem coord_sys) const;

    const boost::optional<Uncertainty>& sourceUncertainty(uint32_t source_id) const;

    bool hasViewPoint() const;
    ViewPointGenVP* viewPoint() const;
    int viewPointDetail() const;

    QRectF regionOfInterestWGS84() const;

private:
    struct DBContentInfo
    {
        boost::optional<Uncertainty>   uncert;
        boost::optional<InterpOptions> interp_options;
        unsigned int                   count = 0;
    };

    struct VPTrackData
    {
        std::vector<Eigen::Vector2d> positions;
        std::vector<Eigen::Vector2d> speed_vecs;
        std::vector<QColor>          colors;
        QColor                       color;
    };

    void interpolateMeasurements(std::vector<Measurement>& measurements, 
                                 const InterpOptions& options) const;
    void postprocessMeasurements();
    void postprocessReferences(std::vector<Reference>& references);

    std::vector<Measurement>                     measurements_;
    SourceMap                                    sources_;
    std::map<std::string, DBContentInfo>         dbcontent_infos_;
    std::vector<boost::optional<Uncertainty>>    source_uncerts_;
    boost::optional<double>                      min_height_;
    boost::optional<double>                      max_height_;

    uint32_t source_cnt_ = 0;

    int verbosity_  = 1;

    mutable ViewPointGenVP*          viewpoint_ = nullptr;
    mutable std::vector<VPTrackData> vp_data_interp_;
    int                              viewpoint_detail_ = 0;
};

} // namespace reconstruction
