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

#include "number.h"
#include "logger.h"

#include <cmath>
#include <stdlib.h>

#include <Eigen/Core>

using namespace std;

namespace Utils
{
namespace Number
{
float randomNumber(float min, float max)
{
    return (float)min + rand() / (RAND_MAX / (max - min) + 1);
}

float roundToNearest(float num) { return (num > 0.0) ? floor(num + 0.5) : ceil(num - 0.5); }

double round(float num, unsigned int precision)
{
    return std::round(num * std::pow(10, precision)) / std::pow(10, precision);
}

double calculateAngle(double degrees, double minutes, double seconds)
{
    return degrees + minutes / 60.0 + seconds / 3600.0;
}

double calculateMinAngleDifference(double a_deg, double b_deg)
{
    //double phi = std::fmod(std::fabs(a_deg - b_deg), 360.0);       // This is either the distance or 360 - distance
    //double distance = phi > 180.0 ? 360.0 - phi : phi;

//    if (a_deg < 0)
//        a_deg += 360;

//    if (b_deg < 0)
//        b_deg += 360;

//    assert (a_deg <= 360.0);
//    assert (b_deg <= 360.0);

//    double distance = a_deg - b_deg;

//    while (distance > 180.0)
//        distance -= 360.0;

//    while (distance < -180.0)
//        distance += 360.0;

    // shortest_angle=((((end - start) % 360) + 540) % 360) - 180;

    double distance = fmod(fmod(a_deg - b_deg, 360) + 540, 360) - 180;

    return distance;
}

double deg2rad(double angle)
{
    return angle * M_PI / 180.0;
}

double rad2deg(double angle)
{
    return angle * 180.0 / M_PI;
}

namespace
{
    double normalizeAngle(double angle_deg)
    {
        if (angle_deg > 180.0)
            return angle_deg - 360.0;
        if (angle_deg < -180.0)
            return 360.0 + angle_deg;
        return angle_deg;
    };
}

/**
 * Rotates the given angle to a new 0-basis angle.
 * @param angle_deg Angle in [-180,180]
 * @param new_base_deg New 0-basis in [-180,180] 
 */
double rebaseAngle(double angle_deg, double new_base_deg)
{
    return normalizeAngle(angle_deg - new_base_deg);
}

/**
 * Interpolates the bearings given at the two positions at the given factor.
 * @param x0 First point x
 * @param y0 First point y
 * @param x1 Second point x
 * @param y1 Second point y
 * @param bearing0_deg Bearing at the first point in [-180,180]
 * @param bearing1_deg Bearing at the second point in [-180,180]
 * @param factor Interpolation factor in [0,1]
 */
double interpolateBearing(double x0, double y0, 
                          double x1, double y1, 
                          double bearing0_deg, double bearing1_deg, 
                          double factor)
{
    Eigen::Vector2d t(x1 - x0, y1 - y0);
    double tn = t.norm();

    //numerically instable?
    if (tn < 1e-06)
        return bearing0_deg;

    //normalized tangent estimate
    t /= tn;

    //bearing of tangent
    double angle_t_deg = rad2deg(atan2(t.x(), t.y()));

    //rotate to tangent angle as origin
    double da0 = rebaseAngle(bearing0_deg, angle_t_deg);
    double da1 = rebaseAngle(bearing1_deg, angle_t_deg);

    //which side of tangent are we on?
    bool sign0 = (da0 >= 0);
    bool sign1 = (da1 >= 0);

    if (sign0 != sign1)
    {
        //speed vecs on different sides of tangent => directly interpolate between angles
        double dinterp = da0 + (da1 - da0) * factor;

        //rotate back interpolated angle to bearing 0 as origin and convert back to deg
        return rebaseAngle(dinterp, -angle_t_deg);
    }
    
    //speed vecs approximately on same side of tangent => interpolate with minimum angle
    double diff = Number::calculateMinAngleDifference(bearing1_deg, bearing0_deg);
    return bearing0_deg + diff * factor;
}

unsigned int dsIdFrom (unsigned int sac, unsigned int sic)
{
    return sac * 255 + sic;
}

unsigned int sacFromDsId (unsigned int ds_id)
{
    return ds_id / 255;
}
unsigned int sicFromDsId (unsigned int ds_id)
{
    return ds_id % 255;
}

double knots2MPS(double knots)
{
    return knots * 0.514444;
}

double mps2Knots(double mps)
{
    return mps * 1.94384;
}

std::pair<double, double> bearing2Vec(double bearing_deg)
{
    double bearing_rad = deg2rad(bearing_deg);
    return std::make_pair(std::sin(bearing_rad), std::cos(bearing_rad));
}

std::pair<double, double> speedAngle2SpeedVec(double speed_mps, 
                                              double bearing_deg)
{
    auto vec = bearing2Vec(bearing_deg);
    vec.first  *= speed_mps;
    vec.second *= speed_mps;

    return vec;
}

}  // namespace Number

//void convert(const std::string& conversion_type, NullableVector<unsigned int>& array_list) {}

}  // namespace Utils
