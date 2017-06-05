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
 * StructureDescriptopn.cpp
 *
 *  Created on: Jul 18, 2011
 *      Author: sk
 */

#include <string>
#include <iostream>
#include <typeinfo>
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "dbobjectmanager.h"
#include "structuredescription.h"
#include "structurevariable.h"
#include "logger.h"
#include "property.h"

using namespace std;

StructureDescription::StructureDescription(const std::string &dbo_type, string id, string description, size_t offset)
: dbo_type_(dbo_type), id_(id), description_ (description), offset_(offset)
{
  logdbg  << "StructureDescription: constructor: dbo " << dbo_type << " id " << id;
}

StructureDescription::~StructureDescription()
{
  logdbg  << "StructureDescription: destructor";
  // delete itmes in element list

  //TODO causes exception
  /*
  for (unsigned int cnt=0; cnt < element_list_.size(); cnt ++)
  {
    delete element_list_.at(cnt);
    element_list_.at(cnt)=0;
  }
  element_list_.clear();
  */
}

void StructureDescription::print (std::string prefix)
{
  logdbg  << "StructureDescription: "<< prefix << "SD: Off " << offset_ << " Id '" << id_ << "'";

  for (unsigned int cnt=0; cnt < element_list_.size(); cnt ++)
    element_list_[cnt]->print("  "+prefix+id_+" ");
}

StructureVariable* StructureDescription::addStructureVariable (string id, StructureElementDataType type, int number, string description, size_t offset)
{
  StructureVariable *var = new StructureVariable (id, type, number, description, offset);
  element_list_.push_back (var);

  if (present_variable_)
    var->addPresentStructureVariable(present_variable_->getId(), present_variable_->getType(), present_variable_->getNumber(), present_variable_->getDescription(), present_variable_->getOffset());

  return var;
}

StructureDescription* StructureDescription::addStructureDescription (string id, string description, size_t offset)
{
  StructureDescription *desc = new StructureDescription (dbo_type_, id, description, offset);
  element_list_.push_back (desc);

  if (present_variable_)
    desc->addPresentStructureVariable(present_variable_->getId(), present_variable_->getType(), present_variable_->getNumber(), present_variable_->getDescription(), present_variable_->getOffset());

  return desc;
}

void StructureDescription::addToFlatStructureDescription (StructureDescription *flatdesc, std::string prefix)
{
  if (id_.length() != 0)
    prefix=prefix+id_+"__";

  for (unsigned int cnt=0; cnt < element_list_.size(); cnt ++)
    element_list_[cnt]->addToFlatStructureDescription(flatdesc,prefix);
}

StructureVariable *StructureDescription::getVariableAt (unsigned int index)
{
  if (index >= element_list_.size())
  {
    throw logic_error ("StructureDescription: getVariableAt: index out of bounds");
  }

  StructureElement *elem = element_list_[index];

  if (typeid(*elem) == typeid(StructureVariable))
    return (StructureVariable*) elem;
  else
  {
  	throw logic_error ("StructureDescription: getVariableAt: wrong typeid");
  }
}

unsigned int StructureDescription::getSize ()
{
  return element_list_.size();
}

PropertyList *StructureDescription::convert ()
{
  logdbg  << "StructureDescription: convert: start";

  PropertyList *result = new PropertyList ();
  assert (result);

  for (unsigned int cnt=0; cnt < element_list_.size(); cnt++)
  {
    StructureVariable *var = getVariableAt(cnt);
    std::string id = var->getId();

    StructureElementDataType type = var->getType();
    PropertyDataType proptype = data_type_conversion_table_.at(type);


    result->addProperty(id, proptype);
  }

  logdbg  << "StructureDescription: convert: end";

  return result;
}

//ConfigurableDefinition StructureDescription::createDBObject()
//{
//  logdbg  << "StructureDescription: createDBObject: object " << id_;
//
//  std::string dboname = id_+"0";
//  std::string db_config_name=dboname+"Config0";
//  Configuration &configuration = ConfigurationManager::getInstance().getConfiguration ("DBObject", db_config_name, dboname);
//  configuration.addParameter ("dbo_type", dbo_type_);
//  configuration.addParameter ("name", id_);
//  configuration.addParameter ("is_loadable", !((id_.compare("Sensor") == 0) || (id_.compare("Undefined") == 0)));
//
//  std::string id_variable_id = dboname+"id";
//  std::string id_variable_config_id=id_variable_id+"Config0";
//  logdbg  << "StructureDescription: createDBObject: adding element " << id_variable_id << " with config "  << id_variable_config_id << " to dbo " << dboname << " with config "<< db_config_name;
//  Configuration &id_configuration = ConfigurationManager::getInstance().getConfiguration ("DBOVariable", id_variable_config_id, id_variable_id);
//  id_configuration.addParameter ("id", (std::string) "id");
//  id_configuration.addParameter ("description", (std::string) "Unique id");
//  id_configuration.addParameter ("data_type", P_TYPE_UINT);
//  id_configuration.addParameter ("dbo_type", dbo_type_);
//  id_configuration.addParameter ("representation", R_STANDARD);
//  configuration.addSubConfigurable ("DBOVariable",id_variable_id, id_variable_config_id);
//
//  for (unsigned int cnt=0; cnt < element_list_.size(); cnt++)
//  {
//    StructureElement *element = element_list_.at(cnt);
//    logdbg  << "StructureDescription: createDBObject: element " << cnt << " dbo type " << dbo_type_;
//    if (element->isVariable())
//    {
//      StructureVariable *variable = (StructureVariable *) element;
//      std::string variable_id = dboname+variable->getId();
//      std::string variable_config_id=variable_id+"Config0";
//      Configuration &variable_configuration = ConfigurationManager::getInstance().getConfiguration ("DBOVariable", variable_config_id, variable_id);
////      registerParameter ("id", &id_, "undefined");
//      variable_configuration.addParameter ("id", variable->getId());
////      registerParameter ("description", &description_, "No information");
//      variable_configuration.addParameter ("description", variable->getDescription());
////      registerParameter ("data_type", &data_type_int_, P_TYPE_SENTINEL);
//      variable_configuration.addParameter ("data_type", data_type_conversion_table_.at(variable->getType()));
////      registerParameter ("dbo_type", &dbo_type_int_, DBO_UNDEFINED);
//      variable_configuration.addParameter ("dbo_type", dbo_type_);
////      registerParameter ("representation", &representation_int_, R_STANDARD);
//      variable_configuration.addParameter ("representation", R_STANDARD);
//
//      logdbg  << "StructureDescription: createDBObject: adding element " << variable_id << " with config "  << variable_config_id << " to dbo " << dboname << " with config "<< db_config_name;
//      configuration.addSubConfigurable ("DBOVariable", variable_id, variable_config_id);
//    }
//  }
//  logdbg  << "StructureDescription: createDBObject: element parsed";
////  DBObjectManager::getInstance().generateSubConfigurable ("DBObject", dboname, db_config_name);
//  logdbg  << "StructureDescription: createDBObject: end ";
//
//  ConfigurableDefinition config_definition;
//  config_definition.class_id_ = "DBObject";
//  config_definition.instance_id_ = dboname;
//  config_definition.configuration_id_ = db_config_name;
//
//  return config_definition;
//}

void StructureDescription::addPresentStructureVariable (std:: string id, StructureElementDataType type, int number, std::string description, size_t offset)
{
  if(present_variable_ != 0)
  {
    logerr << "StructureDescription: addPresentStructureVariable: variable " << id_ << " present old " << present_variable_->getId() <<
        " superseded by " << id;
    delete present_variable_;
    present_variable_=0;
  }
  present_variable_ = new StructureVariable (id, type, number, description, offset);

  std::vector <StructureElement*>::iterator it;
  for (it = element_list_.begin(); it != element_list_.end(); it++)
  {
    (*it)->addPresentStructureVariable(id, type, number, description, offset);
  }
}
