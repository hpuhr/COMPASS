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
 * DBTableColumnComboBox.cpp
 *
 *  Created on: Aug 29, 2012
 *      Author: sk
 */

#include "DBTableColumn.h"
#include "DBTableColumnComboBox.h"
#include "DBSchema.h"
#include "DBSchemaManager.h"
#include "DBOVariable.h"
#include "MetaDBTable.h"

DBTableColumnComboBox::DBTableColumnComboBox(std::string schema, std::string meta_table, DBOVariable *variable, QWidget * parent)
 : schema_ (schema), meta_table_ (meta_table), variable_(variable)
{
  std::string variable_name;
  if (variable_->hasSchema (schema))
    variable_name = variable_->getVariableName (schema);

  MetaDBTable *meta = DBSchemaManager::getInstance().getSchema(schema_)->getMetaTable(meta_table_);
  std::map <std::string, DBTableColumn*> &cols =  meta->getColumns ();
  std::map <std::string, DBTableColumn*>::iterator it;

  addItem ("");

  int index=-1;
  unsigned int cnt=1;
  for (it = cols.begin(); it != cols.end(); it++)
  {
    if (variable_name.compare(it->second->getName()) == 0)
      index=cnt;

    addItem (it->second->getName().c_str());
    cnt++;
  }

  if (index != -1)
  {
    setCurrentIndex (index);
  }
}

DBTableColumnComboBox::~DBTableColumnComboBox()
{

}
