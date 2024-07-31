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

#include <string>
#include <vector>
#include <tuple>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace Utils
{
namespace Number
{

/// @brief Returns random number between min and max
extern float randomNumber(float min, float max);
/// @brief Returns rounded number to nearest integer
extern float roundToNearest(float num);
extern double round(float num, unsigned int precision);

extern unsigned int numDecimals(double v, unsigned int dec_max = 9);

/// @brief Returns angle (degrees) calculated from given values
extern double calculateAngle(double degrees, double minutes, double seconds);

extern double calculateMinAngleDifference(double a_deg, double b_deg);

extern double deg2rad(double angle);
extern double rad2deg(double angle);
extern double rebaseAngle(double angle_deg, double new_base_deg);

extern double knots2MPS(double knots);
extern double mps2Knots(double mps);

extern std::pair<double, double> bearing2Vec(double bearing_deg);
extern double vec2Bearing(double dx, double dy);
extern std::pair<double, double> speedAngle2SpeedVec(double speed_mps, double bearing_deg);
extern std::pair<double, double> speedVec2SpeedAngle(double vx_mps, double vy_mps);

extern double interpolateBearing(double x0, double y0, 
                                 double x1, double y1, 
                                 double bearing0_deg, double bearing1_deg, 
                                 double factor);

//extern void convert(const std::string& conversion_type, NullableVector<unsigned int>& array_list);

extern unsigned int dsIdFrom (unsigned int sac, unsigned int sic);
extern unsigned int sacFromDsId (unsigned int ds_id);
extern unsigned int sicFromDsId (unsigned int ds_id);

extern unsigned long recNumAddDBContId (unsigned long rec_num_wo_dbcont_id, unsigned int dbcont_id);
extern unsigned long recNumGetWithoutDBContId (unsigned long rec_num);
extern unsigned int recNumGetDBContId (unsigned long rec_num);

template <typename T>
std::tuple<double,double,double,double> getStatistics (const T& values)
{
    double mean=0, stddev=0, min=0, max=0;

    double sum = std::accumulate(values.begin(), values.end(), 0.0);

    mean = sum / values.size();

    std::vector<double> diff(values.size());
    std::transform(values.begin(), values.end(), diff.begin(),
                   [mean](const double val) { return val - mean; });
    double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    stddev = std::sqrt(sq_sum / values.size());

    min = *std::min_element(values.begin(), values.end());
    max = *std::max_element(values.begin(), values.end());

    return std::tuple<double,double,double,double>(mean, stddev, min, max);
}

extern std::tuple<double,double,double,double> getStatistics (const std::vector<double>& values);

//template <typename T>
//double getStatistics(const std::vector<>);

}  // namespace Number

}  // namespace Utils

