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
#include "viewpointgenerator.h"
//#include "util/number.h"

//#include "util/timeconv.h"

//#include "spline.h"

namespace reconstruction
{

const QColor ReconstructorInterp::ColorOk    = QColor(255, 125, 0);
const QColor ReconstructorInterp::ColorFishy = QColor(255, 255, 0);

/**
*/
boost::optional<std::vector<Reference>> ReconstructorInterp::reconstruct_impl(std::vector<Measurement>& measurements, 
                                                                              const std::string& data_info)
{
    size_t n = measurements.size();

    if (n == 0)
        return {};

    SplineInterpolator interp;
    interp.config()                  = config().interp_config;
    interp.config().interpolate_cart = false;

    auto mms_interp = interp.interpolate(measurements);

    if (mms_interp.empty())
        return {};

    size_t ni = mms_interp.size();

    std::vector<Reference> references(ni);

    for (size_t i = 0; i < ni; ++i)
    {
        Measurement& ref = references[ i ];
        ref = mms_interp[ i ];
    }

    if (hasViewPoint())
    {
        std::unique_ptr<ViewPointGenFeatureLineString> lines;
        std::unique_ptr<ViewPointGenFeaturePoints>     points;

        lines.reset(new ViewPointGenFeatureLineString(false));
        lines->setColor(ColorOk);

        points.reset(new ViewPointGenFeaturePoints(ViewPointGenFeaturePoints::Symbol::Square));

        std::vector<Eigen::Vector2d> positions(ni);
        std::vector<QColor>          colors(ni);

        unsigned int num_corr = 0;

        for (size_t i = 0; i < ni; ++i)
        {
            positions[ i ] = references[ i ].position2D(CoordSystem::WGS84);
            colors   [ i ] = mms_interp[ i ].corrected ? ColorFishy : ColorOk;

            if (mms_interp[ i ].corrected)
                ++num_corr;
        }

        lines->addPoints(positions);
        points->addPoints(positions, colors);

        auto anno = viewPoint()->annotations().addAnnotation("Reconstructed Reference");
        anno->addFeature(std::move(lines));
        anno->addFeature(std::move(points));

        viewPoint()->addCustomField("#corrected segments", QVariant(num_corr));
    }
    
    return references;
}

} // namespace reconstruction
