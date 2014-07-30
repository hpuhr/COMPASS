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
 * UnitLength.h
 *
 *  Created on: Oct 24, 2012
 *      Author: sk
 */

#ifndef UNITLENGTH_H_
#define UNITLENGTH_H_

#include "Unit.h"

/**
 * @brief Length unit
 *
 * Has units meter, kilometer, mile, nautical mile
 */
class UnitLength : public Unit
{
public:
  UnitLength ()
   : Unit ("Length")
  {
    registerUnit ("Meter", 1.0);
    registerUnit ("Kilometer", 1.0/1000.0);
    registerUnit ("Mile", 1.0/1609.344);
    registerUnit ("NauticalMile", 1.0/1852.0);
  }
  virtual ~UnitLength() {}
};


#endif /* UNITLENGTH_H_ */
