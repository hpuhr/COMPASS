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
 * UnitTime.h
 *
 *  Created on: Dec 3, 2012
 *      Author: sk
 */

#ifndef UNITTIME_H_
#define UNITTIME_H_

#include "Unit.h"

/**
 * @brief Time unit
 *
 * Has units second, minute, hour, millisecond, V7time (seconds/128)
 */
class UnitTime : public Unit
{
public:
  UnitTime ()
   : Unit ("Time")
  {
    registerUnit ("Second", 1.0);
    registerUnit ("Minute", 1.0/60.0);
    registerUnit ("Hour", 1.0/3600.0);
    registerUnit ("MilliSeconds", 1000.0);
    registerUnit ("V7Time", 128.0);
    registerUnit ("V6Time", 4096.0);
  }
  virtual ~UnitTime() {}
};


#endif /* UNITTIME_H_ */
