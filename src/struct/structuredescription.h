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
 * StructureDescriptopn.h
 *
 *  Created on: Jul 18, 2011
 *      Author: sk
 */

#ifndef STRUCTUREDESCRIPTOPN_H_
#define STRUCTUREDESCRIPTOPN_H_

#include <vector>
#include <string>
#include "StructureElement.h"
#include "PropertyList.h"
#include "Configuration.h"

class StructureVariable;

/**
 * @brief Dynamic description of an C struct
 *
 * Associated with a DBO type, it can hold StructureVariables and other StructureDescriptions. Has to be generated
 * by hand. Would have to be read iteratively (slow), therefore can be converted to a "flat" description (faster).
 */
class StructureDescription: public StructureElement
{
public:
  /// @brief Constructor
  StructureDescription(const std::string &dbo_type, std::string id, std::string description, size_t offset);
  /// @brief Desctructor
  virtual ~StructureDescription();

  /// @brief Adds a StructureVariable, which is returned
  StructureVariable *addStructureVariable (std:: string id, SE_DATA_TYPE type, int number, std::string description, size_t offset);
  /// @brief Adds a StructureDescription, which is returned
  StructureDescription *addStructureDescription (std::string id, std::string description, size_t offset);
  void addPresentStructureVariable (std:: string id, SE_DATA_TYPE type, int number, std::string description, size_t offset);

  /// @brief Returns the number of elements
  unsigned int getSize ();
  void print (std::string prefix);

  void addToFlatStructureDescription (StructureDescription *flatdesc, std::string prefix);
  /// @brief Returns StructureVariable at given index
  StructureVariable *getVariableAt (unsigned int index);

  /// @brief Returns data contents as PropertyList (for Buffer generation)
  PropertyList *convert ();

  virtual bool isVariable() { return false;}

protected:
  /// DBO type
  std::string dbo_type_;
  /// Name identifier
  std::string id_;
  /// Description
  std::string description_;
  /// Offset from base pointer in bytes
  size_t offset_;

  /// Data conversion table, from SE_DATA_TYPES to P_DATA_TYPES
  std::vector <int> data_type_conversion_table_;
  /// Element list
  std::vector <StructureElement*> element_list_;
};

#endif /* STRUCTUREDESCRIPTOPN_H_ */
