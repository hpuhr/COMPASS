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

#include "structurevariable.h"
#include "structuredescription.h"
#include "logger.h"


StructureVariable::StructureVariable(std::string id, StructureElementDataType type, unsigned int number, std::string description, size_t offset)
{
  id_=id;
  type_=type;
  number_=number;
  description_=description;
  offset_=offset;
}

void StructureVariable::print (std::string prefix)
{
  logdbg << "StructureVariable: " << prefix << "SV: Off " << offset_ << " Id '" << id_ << "' type " << stringFor(type_) << " number " << number_
         << " desc '" << description_ << "'";
}


void StructureVariable::addToFlatStructureDescription (StructureDescription *flatdesc, std::string prefix)
{
  StructureVariable *var = flatdesc->addStructureVariable(prefix+id_, type_, number_, description_, offset_);
  if (present_variable_)
    var->addPresentStructureVariable(present_variable_->getId(), present_variable_->getType(), present_variable_->getNumber(),
                                     present_variable_->getDescription(), present_variable_->getOffset());
}

unsigned int StructureVariable::getSize() const
{
  if (type_  == StructureElementDataType::BOOL)
    return sizeof(bool);
  else
    return number_*sizeFor(type_);
}

void StructureVariable::addPresentStructureVariable (std:: string id, StructureElementDataType type, int number, std::string description, size_t offset)
{
  assert (present_variable_ == 0);
  present_variable_ = new StructureVariable (id, type, number, description, offset);
}
