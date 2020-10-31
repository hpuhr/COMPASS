/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "number.h"

#include <math.h>
#include <stdlib.h>

#include "logger.h"

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

}  // namespace Number

void convert(const std::string& conversion_type, NullableVector<unsigned int>& array_list) {}
}  // namespace Utils
