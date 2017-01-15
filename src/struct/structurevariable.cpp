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
 * StructureVariable.cpp
 *
 *  Created on: Nov 19, 2012
 *      Author: sk
 */

#include "StructureVariable.h"
#include "StructureDescription.h"
#include "Logger.h"

StructureVariable::StructureVariable(std::string id, SE_DATA_TYPE type, unsigned int number, std::string description, size_t offset)
{
  id_=id;
  type_=type;
  number_=number;
  description_=description;
  offset_=offset;
}

void StructureVariable::print (std::string prefix)
{
  std::string type;

  if (type_  == SE_TYPE_BOOL)
    type = "bool";
  if (type_  == SE_TYPE_TINYINT)
    type = "tinyint";
  if (type_  == SE_TYPE_UTINYINT)
    type = "utinyint";
  if (type_  == SE_TYPE_SMALLINT)
    type = "smallint";
  if (type_  == SE_TYPE_USMALLINT)
    type = "usmallint";
  if (type_  == SE_TYPE_INT)
    type = "int";
  if (type_  == SE_TYPE_UINT)
    type = "uint";
  if (type_  == SE_TYPE_VARCHAR)
    type = "varchar";
  if (type_  == SE_TYPE_FLOAT)
    type = "float";
  if (type_  == SE_TYPE_DOUBLE)
    type = "double";

  logdbg << "StructureVariable: " << prefix << "SV: Off " << offset_ << " Id '" << id_ << "' type " << type << " number " << number_ << " desc '" << description_ << "'";
}


void StructureVariable::addToFlatStructureDescription (StructureDescription *flatdesc, std::string prefix)
{
  StructureVariable *var = flatdesc->addStructureVariable(prefix+id_, type_, number_, description_, offset_);
  if (present_variable_)
    var->addPresentStructureVariable(present_variable_->getId(), present_variable_->getType(), present_variable_->getNumber(), present_variable_->getDescription(), present_variable_->getOffset());
}

unsigned int StructureVariable::getSize() const
{
  if (type_  == SE_TYPE_BOOL)
    return 1;
  if (type_  == SE_TYPE_TINYINT || type_  == SE_TYPE_UTINYINT)
    return number_;
  if (type_  == SE_TYPE_SMALLINT || type_  == SE_TYPE_USMALLINT)
    return 2*number_;
  if (type_  == SE_TYPE_INT || type_  == SE_TYPE_UINT)
    return 4*number_;
  if (type_  == SE_TYPE_VARCHAR || type_ == SE_TYPE_VARCHAR_ARRAY)
    return number_;
  if (type_  == SE_TYPE_FLOAT)
    return 4*number_;
  if (type_  == SE_TYPE_DOUBLE)
    return 8*number_;

  logerr  << "StructureVariable: getSize: unknow type " << type_;
  return 0;
}

void StructureVariable::addPresentStructureVariable (std:: string id, SE_DATA_TYPE type, int number, std::string description, size_t offset)
{
  assert (present_variable_ == 0);
  present_variable_ = new StructureVariable (id, type, number, description, offset);
}
