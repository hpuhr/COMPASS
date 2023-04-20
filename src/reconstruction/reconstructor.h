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

    Reconstructor() = default;
    virtual ~Reconstructor() = default;

    void addChain(const dbContent::TargetReport::Chain* tr_chain);

    boost::optional<std::vector<Reference>> reconstruct(const std::string& data_info = "");

    const dbContent::TargetReport::Chain* chainOfReference(const Reference& ref) const;

protected:
    virtual boost::optional<std::vector<Reference>> reconstruct_impl(const std::vector<Measurement>& measurements, 
                                                                     const std::string& data_info = "") = 0;

    static double timestep(const Measurement& mm0, const Measurement& mm1);
    static double distance(const Measurement& mm0, const Measurement& mm1);

private:
    void postprocessMeasurements();
    void postprocessReferences(std::vector<Reference>& references);

    std::vector<Measurement>                     measurements_;
    SourceMap                                    sources_;
    std::unique_ptr<OGRSpatialReference>         ref_src_;
    std::unique_ptr<OGRSpatialReference>         ref_dst_;
    std::unique_ptr<OGRCoordinateTransformation> trafo_fwd_;
    std::unique_ptr<OGRCoordinateTransformation> trafo_bwd_;

    uint32_t source_cnt_ = 0;
};

} // namespace reconstruction
