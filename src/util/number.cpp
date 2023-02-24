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

}  // namespace Number

//void convert(const std::string& conversion_type, NullableVector<unsigned int>& array_list) {}

}  // namespace Utils
