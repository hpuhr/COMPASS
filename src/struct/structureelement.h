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

#ifndef STRUCTUREELEMENT_H_
#define STRUCTUREELEMENT_H_

#include <cassert>
#include <map>
#include <string>

#include "global.h"
#include "property.h"

class StructureDescription;
class StructureVariable;

/// C struct data type, if any new data types are added also add them to the conversion mechanism in
/// StructureConverter.cpp
enum class StructureElementDataType
{
    BOOL,
    TINYINT,
    SMALLINT,
    INT,
    UTINYINT,
    USMALLINT,
    UINT,
    VARCHAR,
    VARCHAR_ARRAY,
    FLOAT,
    DOUBLE
};

/**
 * @brief Interface for a StructureDescription or a StructureVariable
 *
 * Provides generic encapsulation for storage
 */
class StructureElement
{
  public:
    StructureElement();
    virtual ~StructureElement(){};
    /// @brief Prints element contents (for debugging)
    virtual void print(std::string prefix) = 0;
    /// @brief Adds element contents iteratively to the supplied (flat) StructureDescription
    virtual void addToFlatStructureDescription(StructureDescription* flatdesc,
                                               std::string prefix) = 0;
    /// @brief Returns if element is a StructureVariable
    virtual bool isVariable() = 0;
    /// @brief Sets the present_variable_
    virtual void addPresentStructureVariable(std::string id, StructureElementDataType type,
                                             int number, std::string description,
                                             size_t offset) = 0;

    /// @brief Returns if present_variable_ was set
    bool hasPresentVariable() { return present_variable_ != 0; }
    /// @brief Returns present_variable_
    StructureVariable* getPresentVariable()
    {
        assert(present_variable_);
        return present_variable_;
    }

    const std::string& stringFor(StructureElementDataType type) const
    {
        return data_type_strings_.at(type);
    }
    size_t sizeFor(StructureElementDataType type) const { return data_type_sizes_.at(type); }
    PropertyDataType propertyDataTypeFor(StructureElementDataType type) const
    {
        return data_type_conversion_table_.at(type);
    }

  protected:
    /// Present variable, which indicates if data in struct is valid
    StructureVariable* present_variable_;
    static std::map<StructureElementDataType, std::string> data_type_strings_;
    static std::map<StructureElementDataType, size_t> data_type_sizes_;
    static std::map<StructureElementDataType, PropertyDataType> data_type_conversion_table_;
};

#endif /* STRUCTUREELEMENT_H_ */
