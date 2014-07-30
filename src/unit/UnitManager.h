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
 * UnitManager.h
 *
 *  Created on: Oct 24, 2012
 *      Author: sk
 */

#ifndef UNITMANAGER_H_
#define UNITMANAGER_H_

#include "Singleton.h"

#include "Unit.h"

/**
 * @brief Holds and manages all units
 *
 * All units have to call registerUnit.
 */
class UnitManager : public Singleton
{
public:
  /// @brief Destructor
  virtual ~UnitManager();

  /// @brief Registers a unit
  void registerUnit (Unit *unit);
  /// @brief Returns unit with a given name
  Unit *getUnit (std::string name);
  /// @brief Return container with all units
  std::map <std::string, Unit*> &getUnits () { return units_; }

private:
  /// Container with all units (unit name (length, time) -> unit)
  std::map <std::string, Unit*> units_;

  /// @brief Constructor
  UnitManager();

public:
  static UnitManager& getInstance()
  {
    static UnitManager instance;
    return instance;
  }
};

#endif /* UNITMANAGER_H_ */
