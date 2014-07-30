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
 * StructureElement.h
 *
 *  Created on: Jul 18, 2011
 *      Author: sk
 */

#ifndef STRUCTUREELEMENT_H_
#define STRUCTUREELEMENT_H_

#include <string>
#include "Global.h"

class StructureDescription;
class StructureVariable;

/**
 * @brief Interface for a StructureDescription or a StructureVariable
 *
 * Provides generic encapsulation for storage
 */
class StructureElement
{
public:
  StructureElement() : present_variable_ (0) {};
  virtual ~StructureElement() {};
  /// @brief Prints element contents (for debugging)
  virtual void print (std::string prefix)=0;
  /// @brief Adds element contents iteratively to the supplied (flat) StructureDescription
  virtual void addToFlatStructureDescription (StructureDescription *flatdesc, std::string prefix)=0;
  /// @brief Returns if element is a StructureVariable
  virtual bool isVariable()=0;
  /// @brief Sets the present_variable_
  virtual void addPresentStructureVariable (std:: string id, SE_DATA_TYPE type, int number, std::string description, size_t offset)=0;

  /// @brief Returns if present_variable_ was set
  bool hasPresentVariable () { return present_variable_ != 0;}
  /// @brief Returns present_variable_
  StructureVariable *getPresentVariable () { assert (present_variable_); return present_variable_; }

protected:
  /// Present variable, which indicates if data in struct is valid
  StructureVariable *present_variable_;
};

#endif /* STRUCTUREELEMENT_H_ */
