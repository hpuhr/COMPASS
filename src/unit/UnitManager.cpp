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
 * UnitManager.cpp
 *
 *  Created on: Oct 24, 2012
 *      Author: sk
 */

#include "UnitManager.h"
#include "UnitLength.h"
#include "UnitTime.h"
#include "Logger.h"

UnitManager::UnitManager()
{
  registerUnit (new UnitLength());
  registerUnit (new UnitTime());
}

UnitManager::~UnitManager()
{
  std::map <std::string, Unit*>::iterator it;

  for (it = units_.begin(); it != units_.end(); it++)
    delete it->second;
  units_.clear();
}

void UnitManager::registerUnit (Unit *unit)
{
  assert (unit);
  std::string unit_def = unit->getName();
  assert (units_.find (unit_def) == units_.end());
  units_[unit_def]=unit;
}
Unit *UnitManager::getUnit (std::string name)
{
  assert (units_.find (name) != units_.end());
  return units_[name];
}


