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
 * DBTableColumn.cpp
 *
 *  Created on: Aug 13, 2012
 *      Author: sk
 */

#include "DBTableColumn.h"
#include "Logger.h"

DBTableColumn::DBTableColumn(const std::string &class_id, const std::string &instance_id, Configurable *parent, const std::string &db_table_name)
 : Configurable (class_id, instance_id, parent), db_table_name_ (db_table_name)
{
  registerParameter ("name", &name_, (std::string) "");
  registerParameter ("type", &type_, (std::string) "");
  registerParameter ("is_key", &is_key_, false);
  registerParameter ("unit_dimension", &unit_dimension_, (std::string) "");
  registerParameter ("unit_unit", &unit_unit_, (std::string) "");
  registerParameter ("special_null", &special_null_, (std::string) "");

  createSubConfigurables();
}

DBTableColumn::~DBTableColumn()
{
}


