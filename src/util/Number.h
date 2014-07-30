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

/*
 * Number.h
 *
 *  Created on: Mar 3, 2014
 *      Author: sk
 */

#ifndef NUMBER_H_
#define NUMBER_H_

namespace Utils
{

namespace Number
{
/// @brief Returns random number between min and max
extern float randomNumber (float min, float max);
/// @brief Returns rounded number to nearest integer
extern float roundToNearest(float num);
/// @brief Returns angle (degrees) calculated from given values
extern double calculateAngle( double degrees, double minutes, double seconds );
}

}

#endif /* NUMBER_H_ */
