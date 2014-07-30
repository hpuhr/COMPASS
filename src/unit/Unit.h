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
 * Unit.h
 *
 *  Created on: Oct 24, 2012
 *      Author: sk
 */

#ifndef UNIT_H_
#define UNIT_H_

#include <string>
#include <map>

/**
 * @brief Definition of a base unit
 *
 * Automatically registers to UnitManager, has a name (length, time), allows registering sub units with scaling factors.
 */
class Unit
{
public:
  /// @brief Constructor with a name
  Unit(std::string name);
  /// @brief Destructor
  virtual ~Unit();

  /// @brief Registers a derived unit (m, cm, km) with factor compared to base unit
  void registerUnit (std::string definition, double factor);
  /// @brief Returns unit name
  std::string getName () { return name_; }

  /// @brief Returns factor from one unit to another
  double getFactor (std::string unit_source, std::string unit_destination);

  /// @brief Returns container with all units
  std::map <std::string, double> &getUnits () { return units_; }

private:
  /// Name
  std::string name_;
  /// Container with all units (unit definition -> double scaling factor)
  std::map <std::string, double> units_;
};

#endif /* UNIT_H_ */
