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
 * Unit.cpp
 *
 *  Created on: Oct 24, 2012
 *      Author: sk
 */

#include "Unit.h"
#include "UnitManager.h"

#include "Logger.h"

Unit::Unit(std::string name)
 : name_(name)
{
  assert (name_.size() != 0);
}

Unit::~Unit()
{

}

void Unit::registerUnit (std::string unit_definition, double factor)
{
  assert (units_.find(unit_definition) == units_.end());
  units_[unit_definition] = factor;
}

double Unit::getFactor (std::string unit_source, std::string unit_destination)
{
  assert (units_.find(unit_source) != units_.end());
  assert (units_.find(unit_destination) != units_.end());
  double factor = 1.0;
  factor /= units_[unit_source];
  factor *= units_[unit_destination];
  return factor;
}
