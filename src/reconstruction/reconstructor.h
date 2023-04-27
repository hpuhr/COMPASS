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

namespace dbContent 
{
    namespace TargetReport 
    {
        class Chain;
    }
}

class ViewPointGenVP;

class OGRSpatialReference;
class OGRCoordinateTransformation;

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

    void setSensorUncertainty(const std::string& dbcontent, const Uncertainty& uncert);
    void clearSensorUncertainties();

    void addMeasurements(const std::vector<Measurement>& measurements);
    void addChain(const dbContent::TargetReport::Chain* tr_chain, const std::string& dbcontent);

    boost::optional<std::vector<Reference>> reconstruct(const std::string& data_info = "");

    const dbContent::TargetReport::Chain* chainOfReference(const Reference& ref) const;

    void setOverrideCoordSystem(CoordSystem coord_system);
    void setCoordConversion(CoordConversion coord_conv);
    CoordSystem coordSystem() const;

    void setVerbosity(int v) { verbosity_ = v; }
    int verbosity() const { return verbosity_; }

    void setViewPoint(ViewPointGenVP* vp);

    static std::vector<std::vector<Measurement>> splitMeasurements(const std::vector<Measurement>& measurements,
                                                                   double max_dt);

protected:
    virtual boost::optional<std::vector<Reference>> reconstruct_impl(const std::vector<Measurement>& measurements, 
                                                                     const std::string& data_info) = 0;

    static double timestep(const Measurement& mm0, const Measurement& mm1);
    double distance(const Measurement& mm0, const Measurement& mm1) const;
    Eigen::Vector2d position2D(const Measurement& mm) const;
    void position2D(Measurement& mm, const Eigen::Vector2d& pos) const;

    const boost::optional<Uncertainty>& sourceUncertainty(uint32_t source_id) const;

    virtual CoordSystem preferredCoordSystem() const { return CoordSystem::Unknown; } 

    bool hasViewPoint() const;
    ViewPointGenVP* viewPoint() const;

private:
    struct DBContentInfo
    {
        Uncertainty uncert;
    };

    void postprocessMeasurements();
    void postprocessReferences(std::vector<Reference>& references);

    void cacheCoordSystem();

    std::vector<Measurement>                     measurements_;
    SourceMap                                    sources_;
    std::map<std::string, Uncertainty>           dbcontent_uncerts_;
    std::vector<boost::optional<Uncertainty>>    source_uncerts_;
    std::unique_ptr<OGRSpatialReference>         ref_src_;
    std::unique_ptr<OGRSpatialReference>         ref_dst_;
    std::unique_ptr<OGRCoordinateTransformation> trafo_fwd_;
    std::unique_ptr<OGRCoordinateTransformation> trafo_bwd_;
    double                                       x_offs_       = 0.0;
    double                                       y_offs_       = 0.0;

    uint32_t source_cnt_ = 0;

    CoordConversion coord_conv_         = CoordConversion::WGS84ToCart;
    CoordSystem     coord_sys_override_ = CoordSystem::Unknown;
    CoordSystem     coord_sys_          = CoordSystem::Unknown;
    int             verbosity_          = 1;

    mutable ViewPointGenVP* viewpoint_ = nullptr;
};

} // namespace reconstruction
