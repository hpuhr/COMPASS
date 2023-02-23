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

#ifndef NUMBER_H_
#define NUMBER_H_

#include <string>

#include "nullablevector.h"

namespace Utils
{
namespace Number
{
/// @brief Returns random number between min and max
extern float randomNumber(float min, float max);
/// @brief Returns rounded number to nearest integer
extern float roundToNearest(float num);
extern double round(float num, unsigned int precision);
/// @brief Returns angle (degrees) calculated from given values
extern double calculateAngle(double degrees, double minutes, double seconds);

extern double calculateAngleDifference(double a_deg, double b_deg);

//extern void convert(const std::string& conversion_type, NullableVector<unsigned int>& array_list);

extern unsigned int dsIdFrom (unsigned int sac, unsigned int sic);
extern unsigned int sacFromDsId (unsigned int ds_id);
extern unsigned int sicFromDsId (unsigned int ds_id);

}  // namespace Number

}  // namespace Utils

#endif /* NUMBER_H_ */
