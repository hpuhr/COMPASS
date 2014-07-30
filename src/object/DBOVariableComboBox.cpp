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
 * DBOVariableComboBox.cpp
 *
 *  Created on: Aug 29, 2012
 *      Author: sk
 */

#include "DBTableColumn.h"
#include "DBOVariableComboBox.h"
#include "DBObjectManager.h"
#include "DBOVariable.h"
#include "MetaDBTable.h"
#include "DBObject.h"
#include "Logger.h"

DBOVariableComboBox::DBOVariableComboBox(DB_OBJECT_TYPE dbo_type, DBOVariable *variable, QWidget * parent)
 : dbo_type_ (dbo_type),  variable_(variable)
{
  assert (variable);
  logdbg  << "DBOVariableComboBox: constructor: dbo " << dbo_type << " variable " << variable->getName();
  DBObject *object = DBObjectManager::getInstance().getDBObject(dbo_type_);
  std::map<std::string, DBOVariable*> &variables = object->getVariables ();
  std::map<std::string, DBOVariable*>::iterator it;

  addItem ("");

  std::string selection;

  if (variable_->isMetaVariable())
  {
    logdbg  << "DBOVariableComboBox: constructor: variable is meta";
    if (variable_->existsIn(dbo_type_))
    {
      logdbg  << "DBOVariableComboBox: constructor: exists in dbo " << dbo_type_;
      selection = variable_->getNameFor(dbo_type_);
      logdbg  << "DBOVariableComboBox: constructor: selection set to " << selection;
    }
  }
  logdbg  << "DBOVariableComboBox: constructor: selection is '" << selection << "'";

  int index=-1;
  unsigned int cnt=1;

  for (it = variables.begin(); it != variables.end(); it++)
  {
    if (selection.size() > 0 && selection.compare(it->second->getName()) == 0)
      index=cnt;

    addItem (it->second->getName().c_str());
    cnt++;
  }

  if (index != -1)
  {
    setCurrentIndex (index);
  }
  else
    logdbg  << "DBOVariableComboBox: constructor: variable not found, empty index";

  connect(this, SIGNAL( activated(const QString &) ), this, SLOT( changed() ));
}

DBOVariableComboBox::~DBOVariableComboBox()
{

}


void DBOVariableComboBox::changed ()
{
  std::string variable_name = currentText().toStdString();
  variable_->setSubVariable (dbo_type_, variable_name);
  logdbg  << "DBOVariableComboBox: changed: type " << dbo_type_ << " varname " << variable_name;
}
