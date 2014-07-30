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
 * DBSchema.cpp
 *
 *  Created on: Aug 13, 2012
 *      Author: sk
 */

#include "DBSchema.h"
#include "DBTable.h"
#include "MetaDBTable.h"
#include "Logger.h"

DBSchema::DBSchema(std::string class_id, std::string instance_id, Configurable *parent)
: Configurable (class_id, instance_id, parent)
{
  registerParameter ("name", &name_, (std::string) "");
  assert (name_.size() != 0);

  createSubConfigurables();
}

DBSchema::~DBSchema()
{
  std::map <std::string, DBTable*>::iterator it;
  for (it = tables_.begin(); it != tables_.end(); it++)
    delete it->second;
  tables_.clear();

  std::map <std::string, MetaDBTable*>::iterator it2;
  for (it2= meta_tables_.begin(); it2 != meta_tables_.end(); it2++)
    delete it2->second;
  meta_tables_.clear();

}

void DBSchema::generateSubConfigurable (std::string class_id, std::string instance_id)
{
  logdbg  << "DBSchema: generateSubConfigurable: " << class_id_ << " instance " << instance_id_;

  if (class_id.compare("DBTable") == 0)
  {
    logdbg  << "DBSchema '" << name_ << "': generateSubConfigurable: generating DBTable " << instance_id;
    DBTable *table = new DBTable ("DBTable", instance_id, this);
    assert (table->getName().size() != 0);
    assert (tables_.find(table->getName()) == tables_.end());
    tables_[table->getName()] = table;
    logdbg  << "DBSchema '" << name_ << "': generateSubConfigurable: generated DBTable " << table->getName();
  }
  else if (class_id.compare("MetaDBTable") == 0)
  {
    logdbg  << "DBSchema '" << name_ << "': generateSubConfigurable: generating MetaDBTable " << instance_id;
    MetaDBTable *meta_table = new MetaDBTable ("MetaDBTable", instance_id, this, this);
    assert (meta_table->getName().size() != 0);
    assert (meta_tables_.find(meta_table->getName()) == meta_tables_.end());
    meta_tables_[meta_table->getName()] = meta_table;
    logdbg  << "DBSchema '" << name_ << "': generateSubConfigurable: generated MetaDBTable " << meta_table->getName();
  }
  else
    throw std::runtime_error ("DBSchema: generateSubConfigurable: unknown class_id "+class_id);
}

void DBSchema::checkSubConfigurables ()
{
  // nothing to do here
}

DBTable *DBSchema::getTable (std::string name)
{
  assert (tables_.find(name) != tables_.end());
  return tables_[name];
}

bool DBSchema::hasTable (std::string name)
{
  return tables_.find(name) != tables_.end();
}

std::string DBSchema::getTableName (std::string db_table_name)
{
    std::string table_name;

    std::map <std::string, DBTable*>::iterator it;

    for (it = tables_.begin(); it != tables_.end(); it++)
    {
        if (it->second->getDBName() == db_table_name)
            table_name=it->second->getName();
    }

    assert (table_name.size() > 0);
    return table_name;
}

MetaDBTable *DBSchema::getMetaTable (std::string name)
{
  assert (hasMetaTable(name));
  return meta_tables_[name];
}
bool DBSchema::hasMetaTable (std::string name)
{
  return meta_tables_.find(name) != meta_tables_.end();
}

void DBSchema::updateTables ()
{
  logdbg  << "DBSchema: updateTables";
  std::map <std::string, DBTable*> old_tables = tables_;
  tables_.clear();

  std::map <std::string, DBTable*>::iterator it;

  for (it = old_tables.begin(); it != old_tables.end(); it++)
  {
    tables_[it->second->getName()] = it->second;
  }
}

void DBSchema::updateMetaTables ()
{
  logdbg  << "DBSchema: updateMetaTables";
  std::map <std::string, MetaDBTable*> old_meta_tables = meta_tables_;
  meta_tables_.clear();

  std::map <std::string, MetaDBTable*>::iterator it;

  for (it = old_meta_tables.begin(); it != old_meta_tables.end(); it++)
  {
    meta_tables_[it->second->getName()] = it->second;
  }
}

