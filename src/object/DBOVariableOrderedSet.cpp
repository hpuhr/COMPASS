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
 * DBOVariableOrderedSet.cpp
 *
 *  Created on: May 22, 2012
 *      Author: sk
 */

#include <algorithm>
#include "ConfigurationManager.h"
#include "DBOVariable.h"
#include "DBOVariableSet.h"
#include "DBOVariableOrderedSet.h"
#include "DBObjectManager.h"

DBOVariableOrderedSet::DBOVariableOrderedSet(std::string class_id, std::string instance_id, Configurable *parent)
 : Configurable (class_id, instance_id, parent), changed_(false)
{
  createSubConfigurables ();
}

DBOVariableOrderedSet::~DBOVariableOrderedSet()
{
  std::map <unsigned int, DBOVariableOrderDefinition*>::iterator it;
  for (it = variable_definitions_.begin(); it != variable_definitions_.end(); it++)
  {
    delete it->second;
  }
  variable_definitions_.clear();
}

void DBOVariableOrderedSet::generateSubConfigurable (std::string class_id, std::string instance_id)
{
  logdbg  << "DBOVariableOrderedSet: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
  if (class_id.compare("DBOVariableOrderDefinition") == 0)
  {
    DBOVariableOrderDefinition *definition = new DBOVariableOrderDefinition (class_id, instance_id, this);

    if (!DBObjectManager::getInstance().existsDBOVariable(definition->getDBOType(), definition->getId()))
    {
      logwrn << "DBOVariableOrderedSet: generateSubConfigurable: outdated type " << definition->getDBOType() << " variable "
          << definition->getId();
      delete definition;
      return;
    }

    unsigned int index_new = definition->getIndex();
    assert (variable_definitions_.find(index_new) == variable_definitions_.end());
    variable_definitions_[index_new] = definition;

    updateDBOVariableSet();
  }
  else
    throw std::runtime_error ("DBOVariableOrderedSet: generateSubConfigurable: unknown class_id "+class_id );
}

void DBOVariableOrderedSet::checkSubConfigurables ()
{

}

void DBOVariableOrderedSet::add (DBOVariable *var)
{
  assert (variable_definitions_.size() == set_.size());

  if (find (set_.begin(), set_.end(), var) == set_.end())
  {
    std::string var_name = var->getName();

    Configuration &id_configuration = addNewSubConfiguration ("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0");
    id_configuration.addParameterString ("dbo_type", var->getDBOType());
    id_configuration.addParameterString ("id", var_name);
    id_configuration.addParameterUnsignedInt ("index", (unsigned int)set_.size());
    generateSubConfigurable("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0");

    assert (variable_definitions_.size() == set_.size());
    changed_=true;
  }
}

void DBOVariableOrderedSet::add (const DBOVariable *var)
{
  //lol hack
  add ((DBOVariable *)var);
}

void DBOVariableOrderedSet::add (DBOVariableOrderedSet &set)
{
  std::vector <DBOVariable*> &setset = set.getSet();
  std::vector <DBOVariable*>::iterator it;

  for (it=setset.begin(); it != setset.end(); it++)
  {
    if (find (set_.begin(), set_.end(), *it) == set_.end())
    {
      add (*it);
    }
  }
}

void DBOVariableOrderedSet::addOnly (DBOVariableOrderedSet &set, const std::string &dbo_type)
{
  logdbg  << "DBOVariableOrderedSet: addOnly: type " << dbo_type;
  std::vector <DBOVariable*> &setset = set.getSet();

  //loginf  << "DBOVariableOrderedSet: addOnly: getset";
  std::vector <DBOVariable*>::iterator it;

  //loginf  << "DBOVariableOrderedSet: addOnly: iterating";
  for (it=setset.begin(); it != setset.end(); it++)
  {
    if (find (set_.begin(), set_.end(), *it) == set_.end())
    {
      //loginf  << "DBOVariableOrderedSet: addOnly: new var";
      if ((*it)->existsIn(dbo_type))
      {
        logdbg  << "DBOVariableOrderedSet: addOnly: pushback";
        add ((*it)->getFor(dbo_type));
      }
    }
  }
  //loginf  << "DBOVariableOrderedSet: addOnly: done";
}

void DBOVariableOrderedSet::removeVariableAt (unsigned int index)
{
  assert (index < set_.size());
  assert (index < variable_definitions_.size());

  std::map <unsigned int, DBOVariableOrderDefinition*>::iterator it;
  std::map <unsigned int, DBOVariableOrderDefinition*>::iterator it_tobeerased;
  it = variable_definitions_.find(index);
  assert (it != variable_definitions_.end());
  delete it->second;

  it_tobeerased=it;

  for (; it != variable_definitions_.end(); it++)
  {
    it->second->setIndex(it->first-1);
    const_cast<unsigned int&>(it->first) = it->first-1;
  }

  variable_definitions_.erase(it_tobeerased);

  set_.erase(set_.begin()+index);

  changed_=true;
}

void DBOVariableOrderedSet::moveVariableUp (unsigned int index)
{
  logdbg  << "DBOVariableOrderedSet: moveVariableUp: index " << index;
  assert (index < set_.size());
  assert (index < variable_definitions_.size());

  std::map <unsigned int, DBOVariableOrderDefinition*>::iterator it, itnext;

  it = variable_definitions_.find(index);
  assert (it != variable_definitions_.end());

  if (index == variable_definitions_.size() - 1)
  {
    logerr  << "DBOVariableOrderedSet: moveVariableUp: tried to move up last variable";
    return;
  }

  itnext = variable_definitions_.find(index+1);
  assert (itnext != variable_definitions_.end());

  std::swap (it->second, itnext->second);
  it->second->setIndex(it->first);
  itnext->second->setIndex(itnext->first);

  updateDBOVariableSet();

  changed_=true;
}
void DBOVariableOrderedSet::moveVariableDown (unsigned int index)
{
  logdbg  << "DBOVariableOrderedSet: moveVariableDown: index " << index;
  assert (index < set_.size());
  assert (index < variable_definitions_.size());

  std::map <unsigned int, DBOVariableOrderDefinition*>::iterator it, itprev;

  it = variable_definitions_.find(index);
  assert (it != variable_definitions_.end());

  if (index == 0)
  {
    logwrn  << "DBOVariableOrderedSet: moveVariableDown: tried to move down first variable";
    return;
  }

  itprev = variable_definitions_.find(index-1);
  assert (itprev != variable_definitions_.end());

  std::swap (it->second, itprev->second);
  it->second->setIndex(it->first);
  itprev->second->setIndex(itprev->first);

  updateDBOVariableSet();

  changed_=true;
}


DBOVariableSet *DBOVariableOrderedSet::getFor (const std::string &dbo_type)
{
  logdbg  << "DBOVariableOrderedSet: getFor: type " << dbo_type;

  DBOVariableSet *type_set = new DBOVariableSet ();
  std::vector <DBOVariable*>::iterator it;

  for (it=set_.begin(); it != set_.end(); it++)
  {
    if ((*it)->existsIn(dbo_type))
    {
      logdbg  << "DBOVariableOrderedSet: getFor: add";
      type_set->add ((*it)->getFor(dbo_type));
    }
  }

  return type_set;
}

DBOVariableSet DBOVariableOrderedSet::getUnorderedSet ()
{
  logdbg  << "DBOVariableOrderedSet: getSet";

  DBOVariableSet type_set;
  std::vector <DBOVariable*>::iterator it;

  for (it=set_.begin(); it != set_.end(); it++)
  {
      logdbg  << "DBOVariableOrderedSet: getFor: add";
      type_set.add ((*it));
  }

  return type_set;
}

DBOVariable *DBOVariableOrderedSet::getVariable (unsigned int index)
{
  assert (index < set_.size());
  return set_.at(index);
}

void DBOVariableOrderedSet::print ()
{
  logdbg  << "DBOVariableOrderedSet: print: size" << set_.size() << " changed " << changed_;
  std::vector <DBOVariable*>::iterator it;

  for (it=set_.begin(); it != set_.end(); it++)
  {
    (*it)->print();
  }
}

bool DBOVariableOrderedSet::hasVariable (DBOVariable *variable)
{
  return find (set_.begin(), set_.end(), variable) != set_.end();
}

PropertyList DBOVariableOrderedSet::getPropertyList (const std::string &dbo_type)
{
  std::vector <DBOVariable*>::iterator it;
  PropertyList list;

  for (it=set_.begin(); it != set_.end(); it++)
  {
    if ((*it)->existsIn(dbo_type))
    {
      logdbg  << "DBOVariableOrderedSet: getPropertyList: getfor";
      DBOVariable *var = (*it)->getFor(dbo_type);
      list.addProperty (var->getId(), var->getDataType());
    }
  }

  return list;
}

void DBOVariableOrderedSet::updateDBOVariableSet ()
{
  set_.clear();
  std::map <unsigned int, DBOVariableOrderDefinition*>::iterator it;
  for (it = variable_definitions_.begin(); it != variable_definitions_.end(); it++)
  {
    const std::string &type = it->second->getDBOType();
    std::string name = it->second->getId();

    assert (DBObjectManager::getInstance().existsDBOVariable (type, name));
    DBOVariable *variable = DBObjectManager::getInstance().getDBOVariable (type, name);
    set_.push_back(variable);
  }
  assert (variable_definitions_.size() == set_.size());
  //loginf  << "DBOVariableOrderedSet: updateDBOVariableSet: set has size "  << set_.size();
}
