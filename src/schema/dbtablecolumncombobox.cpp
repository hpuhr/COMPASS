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

#include "dbtablecolumn.h"
#include "dbtablecolumncombobox.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbovariable.h"
#include "metadbtable.h"

DBTableColumnComboBox::DBTableColumnComboBox(std::string schema, std::string meta_table, DBOVariable *variable, QWidget * parent)
 : schema_ (schema), meta_table_ (meta_table), variable_(variable)
{
  std::string variable_name;
  if (variable_->hasSchema (schema))
    variable_name = variable_->variableName (schema);

  const MetaDBTable &meta = variable_->currentMetaTable();
  auto cols =  meta.columns ();

  addItem ("");

  int index=-1;
  unsigned int cnt=1;
  for (auto it = cols.begin(); it != cols.end(); it++)
  {
    if (variable_name.compare(it->second.name()) == 0)
      index=cnt;

    addItem (it->second.name().c_str());
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
