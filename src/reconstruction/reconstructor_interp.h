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

#include "reconstructor.h"
#include "spline_interpolator.h"

#include <QColor>

class ViewPointGenFeatureLineString;
class ViewPointGenFeaturePoints;

namespace reconstruction
{

/**
*/
class ReconstructorInterp : public Reconstructor
{
public:
    struct Config
    {
        SplineInterpolator::Config interp_config;
    };

    ReconstructorInterp() = default;
    virtual ~ReconstructorInterp() = default;

    Config& config() { return config_; }
    const Config& config() const { return config_; }

protected:
    virtual boost::optional<std::vector<Reference>> reconstruct_impl(std::vector<Measurement>& measurements, 
                                                                     const std::string& data_info) override;
    static const QColor ColorOk;
    static const QColor ColorFishy;

private:
    Config config_;
};

} // namespace reconstruction
