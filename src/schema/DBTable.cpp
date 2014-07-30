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
 * DBTable.cpp
 *
 *  Created on: Aug 13, 2012
 *      Author: sk
 */

#include "DBTable.h"
#include "DBTableColumn.h"
#include "Logger.h"

DBTable::DBTable(std::string class_id, std::string instance_id, Configurable *parent)
 : Configurable (class_id, instance_id, parent)
{
  registerParameter ("db_name", &db_name_, (std::string) "");
  registerParameter ("name", &name_, (std::string) "");
  registerParameter ("info", &info_, (std::string) "");
  registerParameter ("key_name", &key_name_, (std::string) "");

  createSubConfigurables();
}

DBTable::~DBTable()
{
  std::map <std::string, DBTableColumn *>::iterator it;
  for (it =  columns_.begin(); it !=  columns_.end(); it++)
    delete it->second;
  columns_.clear();
}

void DBTable::generateSubConfigurable (std::string class_id, std::string instance_id)
{
  logdbg  << "DBTable: generateSubConfigurable: " << class_id_ << " instance " << instance_id_;

  if (class_id.compare("DBTableColumn") == 0)
  {
    DBTableColumn *column = new DBTableColumn ("DBTableColumn", instance_id, this, db_name_);
    assert (column->getName().size() != 0);
    assert (columns_.find(column->getName()) == columns_.end());
    columns_ [column->getName()] = column;

    if (column->isKey())
      key_name_ = column->getName();
  }
  else
    throw std::runtime_error ("DBTable: generateSubConfigurable: unknown class_id "+class_id);
}
void DBTable::checkSubConfigurables ()
{
  // move along, sir.
}

bool DBTable::hasTableColumn (std::string name)
{
    return columns_.find(name) != columns_.end();
}

DBTableColumn *DBTable::getTableColumn (std::string name)
{
  assert (columns_.find(name) != columns_.end());
  return columns_[name];
}

void DBTable::deleteColumn (std::string name)
{
  std::map <std::string, DBTableColumn *>::iterator it;
  it =  columns_.find(name);
  assert (it !=  columns_.end());
  delete it->second;
  columns_.erase(it);
}
