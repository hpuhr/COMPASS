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
 * DBObject.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: sk
 */

#include <algorithm>

#include "DBSchema.h"
#include "DBSchemaManager.h"
#include "DBObject.h"
#include "DBObjectManager.h"
#include "DBOVariable.h"
#include "StructureDescriptionManager.h"
#include "PropertyList.h"
#include "MetaDBTable.h"
//#include "ActiveSourcesObserver.h"
//#include "ATSDB.h"

/**
 * Registers parameters, creates sub configurables
 */
DBObject::DBObject(std::string class_id, std::string instance_id, Configurable *parent)
 : Configurable (class_id, instance_id, parent), current_meta_table_(0), variables_checked_(false),
   has_active_data_sources_info_(false)
{
  registerParameter ("name" , &name_, "Undefined");
  registerParameter ("info" , &info_, "");
  registerParameter ("dbo_type" , &dbo_type_, "");
  registerParameter ("is_loadable" , &is_loadable_, false);
  registerParameter ("is_meta" , &is_meta_, false);

  createSubConfigurables ();

  logdbg  << "DBObject: constructor: created with instance_id " << instance_id_ << " name " << name_;
}

/**
 * Deletes data source definitions, schema & meta table definitions and variables
 */
DBObject::~DBObject()
{
  std::map <std::string, DBODataSourceDefinition *>::iterator it;
  for (it = data_source_definitions_.begin(); it != data_source_definitions_.end(); it++)
    delete it->second;
  data_source_definitions_.clear();

  std::vector <DBOSchemaMetaTableDefinition *>::iterator it2;
  for (it2 = meta_table_definitions_.begin(); it2 != meta_table_definitions_.end(); it2++)
    delete *it2;
  meta_table_definitions_.clear();

  std::map<std::string, DBOVariable*>::iterator it3;
  for (it3 = variables_.begin(); it3 !=  variables_.end(); it3++)
    delete it3->second;
  variables_.clear();

}

bool DBObject::hasVariable (std::string id)
{
//  if (!variables_checked_)
//    checkVariables ();

  return (variables_.find (id) != variables_.end());
}

DBOVariable *DBObject::getVariable (std::string id)
{
//  if (!variables_checked_)
//    checkVariables ();

  assert (hasVariable (id));
  return variables_[id];
}

void DBObject::deleteVariable (std::string id)
{
//  if (!variables_checked_)
//    checkVariables ();

  assert (hasVariable (id));
  std::map<std::string, DBOVariable*>::iterator it;
  it = variables_.find (id);
  DBOVariable *variable = it->second;
  variables_.erase(it);
  delete variable;
}

std::map<std::string, DBOVariable*> &DBObject::getVariables ()
{
//  if (!variables_checked_)
//    checkVariables ();

  return variables_;
}

/**
 * Iterates through all variables, and checks if the current meta table has a variable with the same name, if so it adds
 * the variable to the returned list.
 */
std::vector <DBOVariable*> DBObject::getVariablesForTable (std::string table)
{
//  if (!variables_checked_)
//    checkVariables ();

  std::vector <DBOVariable*> variables;

  std::map<std::string, DBOVariable*>::iterator it;

  for (it = variables_.begin(); it != variables_.end(); it++)
  {
    if (getCurrentMetaTable()->getTableDBNameForVariable(it->second->getCurrentVariableName()).compare (table) != 0)
      continue;
    variables.push_back (it->second);
  }

  return variables;
}

std::string DBObject::getMetaTable (std::string schema)
{
  assert (meta_tables_.find(schema) != meta_tables_.end());
  return meta_tables_[schema];
}

/**
 * Returns if the current schema name exists in the data source definitions
 */
bool DBObject::hasCurrentDataSource ()
{
  return (data_source_definitions_.find(DBSchemaManager::getInstance().getCurrentSchema()->getName()) != data_source_definitions_.end());
}

DBODataSourceDefinition *DBObject::getCurrentDataSource ()
{
  assert (hasCurrentDataSource());
  return data_source_definitions_[DBSchemaManager::getInstance().getCurrentSchema()->getName()];
}

/**
 * Returns true if current_meta_table_ is not null, else gets the current schema, checks and get the meta_table_ entry for
 * the current schema, and returns if the meta table for the current schema exists in the current schema.
 */
bool DBObject::hasCurrentMetaTable ()
{
  if (current_meta_table_ != 0)
    return true;
  else
  {
    DBSchema *schema = DBSchemaManager::getInstance().getCurrentSchema();
    logdbg  << "DBObject "<< getName() << ": hasCurrentMetaTable: got current schema " << schema->getName();
    assert (meta_tables_.find(schema->getName()) != meta_tables_.end());
    std::string meta_table_name = meta_tables_ [schema->getName()];
    return schema->hasMetaTable (meta_table_name);
  }

}

/**
 * If current_meta_table_ is not set, it is set be getting the current schema, and getting the current meta table from
 * the schema by its identifier. Then current_meta_table_ is returned.
 */
MetaDBTable *DBObject::getCurrentMetaTable ()
{
  if (!current_meta_table_)
  {
    DBSchema *schema = DBSchemaManager::getInstance().getCurrentSchema();
    assert (meta_tables_.find(schema->getName()) != meta_tables_.end());
    std::string meta_table_name = meta_tables_ [schema->getName()];
    assert (schema->hasMetaTable (meta_table_name));
    current_meta_table_ = schema->getMetaTable (meta_table_name);
  }
  assert (current_meta_table_);
  return current_meta_table_;
}

/**
 * Can generate DBOVariables, DBOSchemaMetaTableDefinitions and DBODataSourceDefinitions.
 */
void DBObject::generateSubConfigurable (std::string class_id, std::string instance_id)
{
  logdbg  << "DBObject: generateSubConfigurable: generating variable " << instance_id;
  if (class_id.compare ("DBOVariable") == 0)
  {
    DBOVariable *variable = new DBOVariable (class_id, instance_id, this);
    assert (variables_.find (variable->getId()) == variables_.end());
    variables_[variable->getId()]=variable;
  }
  else if (class_id.compare ("DBOSchemaMetaTableDefinition") == 0)
  {
    DBOSchemaMetaTableDefinition *def = new DBOSchemaMetaTableDefinition (class_id, instance_id, this);
    meta_table_definitions_.push_back (def);

    logdbg  << "DBObject "<< getName() << ": generateSubConfigurable: schema " << def->getSchema() << " meta " << def->getMetaTable();

    assert (meta_tables_.find (def->getSchema()) == meta_tables_.end());
    meta_tables_[def->getSchema()] = def->getMetaTable();
  }
  else if (class_id.compare ("DBODataSourceDefinition") == 0)
  {
    DBODataSourceDefinition *def = new DBODataSourceDefinition (class_id, instance_id, this);
    assert (data_source_definitions_.find(def->getSchema()) == data_source_definitions_.end());
    data_source_definitions_ [def->getSchema()] = def;
  }
  else
    throw std::runtime_error ("DBObject: generateSubConfigurable: unknown class_id "+class_id );
}

void DBObject::checkSubConfigurables ()
{
  //nothing to see here
}

void DBObject::checkVariables ()
{
  assert (hasCurrentMetaTable());
  std::map<std::string, DBOVariable*>::iterator it;

  for (it =  variables_.begin(); it !=  variables_.end(); it++)
  {
    bool found = current_meta_table_->hasTableColumn(it->first);

    if (!found)
    {
      logwrn  << "DBObject: checkVariables: erasing non-existing variable '"<<it->first<<"'";
      delete it->second;
      variables_.erase(it);
    }
  }
  variables_checked_=true;
}

//void DBObject::addActiveSourcesObserver (ActiveSourcesObserver *observer)
//{
//    assert (find (active_sources_observers_.begin(), active_sources_observers_.end(), observer) ==
//            active_sources_observers_.end());
//    active_sources_observers_.push_back (observer);
//}

//void DBObject::removeActiveSourcesObserver (ActiveSourcesObserver *observer)
//{
//    assert (find (active_sources_observers_.begin(), active_sources_observers_.end(), observer) !=
//            active_sources_observers_.end());
//    active_sources_observers_.erase (find (active_sources_observers_.begin(), active_sources_observers_.end(), observer));
//}

//void DBObject::notifyActiveDataSourcesObservers ()
//{
//    std::vector <ActiveSourcesObserver *>::iterator it;
//    for (it=active_sources_observers_.begin(); it != active_sources_observers_.end(); it++)
//        (*it)->notifyActiveSources ();
//}

//bool DBObject::hasActiveDataSourcesInfo ()
//{
//    return ATSDB::getInstance().hasActiveDataSourcesInfo(dbo_type_);
//}

//void DBObject::buildActiveDataSourcesInfo ()
//{
//    assert (hasCurrentDataSource());
//    assert (!hasActiveDataSourcesInfo());
//    ATSDB::getInstance().buildActiveDataSourcesInfo(dbo_type_);
//}

//void DBObject::setActiveDataSources (std::set<int> active_data_sources)
//{
//    assert (hasCurrentDataSource());
//    active_data_sources_= active_data_sources;
//    notifyActiveDataSourcesObservers();
//}

/**
 * Bit of a hack, explanation at DBOVariable::registerAsParent ().
 */
void DBObject::registerParentVariables ()
{
    logdbg << "DBObject: registerParentVariables";
    assert (isMeta());
    std::map<std::string, DBOVariable*>::iterator it;

    for (it = variables_.begin(); it != variables_.end(); it++)
    {
        assert (it->second->isMetaVariable());
        it->second->registerAsParent();
    }
}

/**
 * Bit of a hack, explanation at DBOVariable::registerAsParent ().
 */
void DBObject::unregisterParentVariables ()
{
    logdbg << "DBObject: unregisterParentVariables";
    assert (isMeta());
    std::map<std::string, DBOVariable*>::iterator it;

    for (it = variables_.begin(); it != variables_.end(); it++)
    {
        assert (it->second->isMetaVariable());
        it->second->unregisterAsParent();
    }
}
