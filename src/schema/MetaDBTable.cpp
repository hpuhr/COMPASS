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
 * MetaDBTable.cpp
 *
 *  Created on: Jul 28, 2012
 *      Author: sk
 */

#include <algorithm>
#include "DBSchema.h"
#include "ATSDB.h"
#include "DBTableColumn.h"
#include "DBTable.h"
#include "MetaDBTable.h"
#include "Logger.h"

MetaDBTable::MetaDBTable(std::string class_id, std::string instance_id, Configurable *parent, DBSchema *schema)
 : Configurable (class_id, instance_id, parent), schema_(schema), table_ (0)
{
  assert (schema_);

  registerParameter ("name", &name_, (std::string)"");
  registerParameter ("info", &info_, (std::string)"");
  registerParameter ("table", &table_name_, (std::string)"");

  logdbg  << "MetaDBTable: constructor: name " << name_ << " table " << table_name_;

  assert (schema_->hasTable (table_name_));
  table_ = schema_->getTable (table_name_);
  assert (table_);

  std::map <std::string, DBTableColumn *> columns =  table_->getColumns ();
  std::map <std::string, DBTableColumn *>::iterator it;

  for (it = columns.begin(); it != columns.end(); it++)
  {
    assert (columns_.find(it->first) == columns_.end());
    columns_[it->first] = it->second;
  }

  createSubConfigurables ();
}

MetaDBTable::~MetaDBTable()
{
//  std::map <std::pair<std::string, std::string>, MetaDBTable*>::iterator it;
//
//  for (it=sub_tables_.begin(); it != sub_tables_.end(); it++)
//    delete it->second;

  std::vector <SubTableDefinition *>::iterator it;
  for (it=sub_table_definitions_.begin(); it != sub_table_definitions_.end(); it++)
    delete (*it);

  sub_tables_.clear(); // are only pointers to tables from schema
}

std::string MetaDBTable::getTableDBName ()
{
  assert (table_);
  return table_->getDBName();
}

std::string MetaDBTable::getTableDBNameForVariable (std::string variable_name)
{
  setSubTablesIfRequired ();

  if (columns_.find(variable_name) == columns_.end())
      logerr << "MetaDBTable: getTableDBNameForVariable: column " << variable_name << " undefined in " << name_;

  assert (columns_.find(variable_name) != columns_.end());
  return (columns_ [variable_name])->getDBTableName();
}


const DBTableColumn &MetaDBTable::getTableColumn (std::string column)
{
  setSubTablesIfRequired();
  assert (hasTableColumn (column));
  return *columns_[column];
}

void MetaDBTable::generateSubConfigurable (std::string class_id, std::string instance_id)
{
  logdbg  << "MetaDBTable: generateSubConfigurable: " << class_id_ << " instance " << instance_id_;

  if (class_id.compare("SubTableDefinition") == 0) // subtable
  {
    logdbg  << "MetaDBTable: generateSubConfigurable: generating sub table";

      SubTableDefinition *def = new SubTableDefinition (class_id, instance_id, this);
      sub_table_definitions_.push_back (def);
  }
  else
    throw std::runtime_error ("MetaDBTable: generateSubConfigurable: unknown class_id "+class_id);
}


void MetaDBTable::checkSubConfigurables ()
{
  // nothing to see here.
}

bool MetaDBTable::hasTableColumn (std::string column)
{
  setSubTablesIfRequired();
  return columns_.find (column) != columns_.end();
}

std::string MetaDBTable::getSubTableNames ()
{
  setSubTablesIfRequired();

  if (sub_tables_.size() == 0)
    return "None";
  else
  {
    std::stringstream ss;
    std::map <SubTableDefinition*, MetaDBTable*>::iterator it2;

    for (it2=sub_tables_.begin(); it2 != sub_tables_.end(); it2++)
    {
      if (it2 != sub_tables_.begin())
        ss << ", ";
      ss << it2->second->getName();
    }
    return ss.str();
  }
}

std::string MetaDBTable::getAllTableNames ()
{
  setSubTablesIfRequired();

  std::stringstream ss;

  ss << getTableDBName();

  if (sub_tables_.size() != 0)
  {

    std::map <SubTableDefinition*, MetaDBTable*>::iterator it2;

    for (it2=sub_tables_.begin(); it2 != sub_tables_.end(); it2++)
    {
      ss << ", ";
      ss << it2->second->getTableDBName();
    }
  }
  return ss.str();
}

std::vector<std::string> MetaDBTable::getAllTableNamesAsVector ()
{
  setSubTablesIfRequired();

  std::vector <std::string> table_names;

  table_names.push_back (getTableName());

  if (sub_tables_.size() != 0)
  {
    std::map <SubTableDefinition*, MetaDBTable*>::iterator it2;

    for (it2=sub_tables_.begin(); it2 != sub_tables_.end(); it2++)
    {
      std::vector <std::string> sub_table_names = it2->second->getAllTableNamesAsVector();
      table_names.insert ( table_names.end(), sub_table_names.begin(), sub_table_names.end());
    }
  }
  return table_names;
}

std::string MetaDBTable::getSubTablesWhereClause(std::vector <std::string> &used_tables)
{
  setSubTablesIfRequired();

  std::stringstream ss;

  std::map <SubTableDefinition*, MetaDBTable*>::iterator it;

  bool first=true;

  for (it=sub_tables_.begin(); it != sub_tables_.end(); it++)
  {
    if (find (used_tables.begin(), used_tables.end(), it->second->getTableDBName()) == used_tables.end())
      continue;


    if (!first)
      ss << " AND ";

    ss << getTableDBName() << "." << it->first->getLocalKey() << "=" << it->second->getTableDBName() << "." << it->first->getSubTableKey();
    first=false;
  }

  return ss.str();
}

std::string MetaDBTable::getSubTableKeyClause (std::string sub_table_name)
{
  setSubTablesIfRequired();

  std::stringstream ss;

  std::map <SubTableDefinition*, MetaDBTable*>::iterator it;

  for (it=sub_tables_.begin(); it != sub_tables_.end(); it++)
  {
    if (it->second->getTableDBName().compare (sub_table_name) != 0)
      continue;

    // found subtable

    ss << getTableDBName() << "." << it->first->getLocalKey() << "=" << it->second->getTableDBName() << "." << it->first->getSubTableKey();
    return ss.str();
  }

  throw std::runtime_error ("MetaDBTable: getSubTableKeyClause: sub_table_name "+sub_table_name+" not found");
}

unsigned int MetaDBTable::getNumColumns ()
{
  setSubTablesIfRequired();
  return columns_.size();
}

std::map <std::string, DBTableColumn*>& MetaDBTable::getColumns ()
{
  setSubTablesIfRequired();
  return columns_;
}

std::map <SubTableDefinition*, MetaDBTable*> &MetaDBTable::getSubTables ()
{
  setSubTablesIfRequired();
  return sub_tables_;
}

void MetaDBTable::setSubTablesIfRequired ()
{
  if (sub_table_definitions_.size() == 0)
    return;

  if (sub_table_definitions_.size() == sub_tables_.size())
    return;

  logdbg  << "MetaDBTable: setSubTablesIfRequired: for name " << name_ << " table "  << table_name_;

  std::vector <SubTableDefinition *>::iterator it;

  DBSchema *current_schema = ATSDB::getInstance().getCurrentSchema ();

  for (it = sub_table_definitions_.begin(); it != sub_table_definitions_.end(); it++)
  {
    SubTableDefinition *def = *it;
    logdbg  << "MetaDBTable: setSubTablesIfRequired: adding sub table " << def->getSubTableName();
    assert (columns_.find (def->getLocalKey()) != columns_.end());
    assert (current_schema->hasMetaTable(def->getSubTableName()));

    MetaDBTable *sub_table = current_schema->getMetaTable(def->getSubTableName());
    if (sub_tables_.find(def) != sub_tables_.end())
    {
      // already exists
      continue;
    }
    sub_tables_[def] = sub_table;

    std::map <std::string, DBTableColumn *> columns =  sub_table->getColumns ();
    std::map <std::string, DBTableColumn *>::iterator it;

    for (it = columns.begin(); it != columns.end(); it++)
    {
      if (columns_.find(it->first) == columns_.end())
        columns_[it->first] = it->second;
      else
      {
        std::string othername = sub_table->getName()+"."+it->first;
        if (columns_.find(othername) != columns_.end())
          logwrn  << "MetaDBTable: setSubTablesIfRequired: " << class_id_ << " instance " << instance_id_ << " already got column name " << othername;
        else
        {
          columns_[othername] = it->second;
        }
      }
    }
  }
  assert ((sub_table_definitions_.size() == sub_tables_.size()));
}
