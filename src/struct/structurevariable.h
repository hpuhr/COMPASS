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
 * StructureVariable.h
 *
 *  Created on: Jul 18, 2011
 *      Author: sk
 */

#ifndef STRUCTUREVARIABLE_H_
#define STRUCTUREVARIABLE_H_

#include <string>
#include <stdio.h>
#include <iostream>
#include "structureelement.h"

class StructureDescription;

/**
 * @brief Definition of a variable in a C struct
 */
class StructureVariable : public StructureElement
{
public:
  /// @brief Constructor
  StructureVariable(std::string id, StructureElementDataType type, unsigned int number, std::string description, size_t offset);

  void addPresentStructureVariable (std:: string id, StructureElementDataType type, int number, std::string description, size_t offset);

public:
  void print (std::string prefix);

  void addToFlatStructureDescription (StructureDescription *flatdesc, std::string prefix);

  /// @brief Destructor
  virtual ~StructureVariable() {};
  /// @brief Returns description
  std::string getDescription() const
  {
    return description_;
  }
  /// @brief Returns identifier
  std::string getId() const
  {
    return id_;
  }
  /// @brief Returns length in bytes
  int getNumber ()
  {
    return number_;
  }

  unsigned int getSize() const;

  /// @brief Returns data type
  StructureElementDataType getType() const
  {
      return type_;
  }
  /// @brief Returns offset from base pointer
  off_t getOffset() const
  {
      return offset_;
  }

  virtual bool isVariable() { return true;}

protected:
  /// Identifier
  std::string id_;
  /// Length in bytes
  unsigned int number_;
  /// Data type
  StructureElementDataType type_;
  /// Description
  std::string description_;
  /// Offset from base pointer
  size_t offset_;
};

#endif /* STRUCTUREVARIABLE_H_ */
