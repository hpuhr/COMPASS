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

#include "configurationmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "dbovariableset.h"
#include "dbovariableorderedset.h"
#include "dbovariableorderedsetwidget.h"
#include "dbobjectmanager.h"
#include "atsdb.h"

DBOVariableOrderedSet::DBOVariableOrderedSet(const std::string &class_id, const std::string &instance_id, Configurable *parent)
    : Configurable (class_id, instance_id, parent), widget_(nullptr)
{
    createSubConfigurables ();
}

DBOVariableOrderedSet::~DBOVariableOrderedSet()
{
    if (widget_)
    {
        delete widget_;
        widget_=nullptr;
    }

    for (auto it : variable_definitions_)
    {
        delete it.second;
    }
    variable_definitions_.clear();
}

void DBOVariableOrderedSet::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "DBOVariableOrderedSet: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
    if (class_id.compare("DBOVariableOrderDefinition") == 0)
    {
        DBOVariableOrderDefinition *definition = new DBOVariableOrderDefinition (class_id, instance_id, this);

        DBObjectManager &manager = ATSDB::instance().objectManager();

        if (definition->dboName() == META_OBJECT_NAME )
        {
            if (!manager.existsMetaVariable(definition->variableName()))
            {
                logwrn << "DBOVariableOrderedSet: generateSubConfigurable: outdated meta variable "
                       << definition->variableName();
                delete definition;
                return;
            }
        }
        else if (!manager.existsObject(definition->dboName()) || !manager.object(definition->dboName()).hasVariable(definition->variableName()))
        {
            logwrn << "DBOVariableOrderedSet: generateSubConfigurable: outdated name " << definition->dboName() << " variable "
                   << definition->variableName();
            delete definition;
            return;
        }

        unsigned int index_new = definition->getIndex();
        assert (variable_definitions_.find(index_new) == variable_definitions_.end());
        variable_definitions_.insert (std::pair <unsigned int, DBOVariableOrderDefinition*> (index_new, definition));
    }
    else
        throw std::runtime_error ("DBOVariableOrderedSet: generateSubConfigurable: unknown class_id "+class_id );
}

void DBOVariableOrderedSet::checkSubConfigurables ()
{

}

void DBOVariableOrderedSet::add (DBOVariable &var)
{
    if (!hasVariable(var))
    {
        std::string var_name = var.name();

        Configuration &id_configuration = addNewSubConfiguration ("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0");
        id_configuration.addParameterString ("dbo_name", var.dboName());
        id_configuration.addParameterString ("dbo_variable_name", var_name);
        id_configuration.addParameterUnsignedInt ("index", (unsigned int)variable_definitions_.size());
        generateSubConfigurable("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0");

        emit setChangedSignal();
    }
}

void DBOVariableOrderedSet::add (MetaDBOVariable &var)
{
    if (!hasMetaVariable(var))
    {
        std::string var_name = var.name();

        Configuration &id_configuration = addNewSubConfiguration ("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0");
        id_configuration.addParameterString ("dbo_name", META_OBJECT_NAME);
        id_configuration.addParameterString ("dbo_variable_name", var_name);
        id_configuration.addParameterUnsignedInt ("index", (unsigned int)variable_definitions_.size());
        generateSubConfigurable("DBOVariableOrderDefinition", "DBOVariableOrderDefinition"+var_name+"0");

        emit setChangedSignal();
    }
}

//void DBOVariableOrderedSet::add (const DBOVariableOrderedSet &set)
//{
//    //const std::vector <DBOVariable &> &setset = set.getSet();
//    //std::vector <DBOVariable*>::const_iterator it;

//    for (auto it : set.getSet())
//    {
//        if (!hasVariable (it.second))
//        {
//            add (it.second);
//        }
//    }
//}

//void DBOVariableOrderedSet::addOnly (DBOVariableOrderedSet &set, const std::string &dbo_type)
//{
//  logdbg  << "DBOVariableOrderedSet: addOnly: type " << dbo_type;
//  std::vector <DBOVariable*> &setset = set.getSet();

//  //loginf  << "DBOVariableOrderedSet: addOnly: getset";
//  std::vector <DBOVariable*>::iterator it;

//  //loginf  << "DBOVariableOrderedSet: addOnly: iterating";
//  for (it=setset.begin(); it != setset.end(); it++)
//  {
//    if (find (set_.begin(), set_.end(), *it) == set_.end())
//    {
//      //loginf  << "DBOVariableOrderedSet: addOnly: new var";
//      if ((*it)->existsIn(dbo_type))
//      {
//        logdbg  << "DBOVariableOrderedSet: addOnly: pushback";
//        add ((*it)->getFor(dbo_type));
//      }
//    }
//  }
//  //loginf  << "DBOVariableOrderedSet: addOnly: done";
//}

void DBOVariableOrderedSet::removeVariableAt (unsigned int index)
{
    assert (index < variable_definitions_.size());

    auto it = variable_definitions_.find(index);
    assert (it != variable_definitions_.end());
    delete it->second;

    auto it_tobeerased=it;

    for (; it != variable_definitions_.end(); it++)
    {
        it->second->setIndex(it->first-1);
        const_cast<unsigned int&>(it->first) = it->first-1;
    }

    variable_definitions_.erase(it_tobeerased);

    emit setChangedSignal();
}

void DBOVariableOrderedSet::moveVariableUp (unsigned int index)
{
    logdbg  << "DBOVariableOrderedSet: moveVariableUp: index " << index;
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

    emit setChangedSignal();
}
void DBOVariableOrderedSet::moveVariableDown (unsigned int index)
{
    logdbg  << "DBOVariableOrderedSet: moveVariableDown: index " << index;
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

    emit setChangedSignal();
}


//DBOVariableSet *DBOVariableOrderedSet::getFor (const std::string &dbo_type)
//{
//  logdbg  << "DBOVariableOrderedSet: getFor: type " << dbo_type;

//  DBOVariableSet *type_set = new DBOVariableSet ();
//  std::vector <DBOVariable*>::iterator it;

//  for (it=set_.begin(); it != set_.end(); it++)
//  {
//    if ((*it)->existsIn(dbo_type))
//    {
//      logdbg  << "DBOVariableOrderedSet: getFor: add";
//      type_set->add ((*it)->getFor(dbo_type));
//    }
//  }

//  return type_set;
//}

//DBOVariableSet DBOVariableOrderedSet::getUnorderedSet () const
//{
//  logdbg  << "DBOVariableOrderedSet: getSet";

//  DBOVariableSet type_set;

//  for (auto it : set_)
//  {
//      logdbg  << "DBOVariableOrderedSet: getFor: add";
//      type_set.add (it.second);
//  }

//  return type_set;
//}

DBOVariableOrderDefinition &DBOVariableOrderedSet::variableDefinition (unsigned int index) const
{
    assert (index < variable_definitions_.size());
    return *variable_definitions_.at(index);
}

//void DBOVariableOrderedSet::print () const
//{
//    logdbg  << "DBOVariableOrderedSet: print: size" << set_.size() << " changed " << changed_;
//    //std::vector <DBOVariable*>::iterator it;

//    for (auto it : set_)
//    {
//        it.second.print();
//    }
//}

bool DBOVariableOrderedSet::hasVariable (const DBOVariable &variable) const
{
    for (auto it : variable_definitions_)
        if (it.second->variableName() == variable.name() && it.second->dboName() == variable.dbObject().name())
            return true;

    return false;
}

bool DBOVariableOrderedSet::hasMetaVariable (const MetaDBOVariable &variable) const
{
    for (auto it : variable_definitions_)
        if (it.second->variableName() == variable.name() && it.second->dboName() == META_OBJECT_NAME)
            return true;

    return false;
}

//PropertyList DBOVariableOrderedSet::getPropertyList (const std::string &dbo_type)
//{
//  std::vector <DBOVariable*>::iterator it;
//  PropertyList list;

//  for (it=set_.begin(); it != set_.end(); it++)
//  {
//    if ((*it)->existsIn(dbo_type))
//    {
//      logdbg  << "DBOVariableOrderedSet: getPropertyList: getfor";
//      DBOVariable *var = (*it)->getFor(dbo_type);
//      list.addProperty (var->getId(), var->getDataType());
//    }
//  }

//  return list;
//}

DBOVariableOrderedSetWidget *DBOVariableOrderedSet::widget ()
{
    if (!widget_)
    {
        widget_ = new DBOVariableOrderedSetWidget (*this);
        connect (this, SIGNAL(setChangedSignal()), widget_, SLOT(updateVariableListSlot()));
    }

    assert (widget_);
    return widget_;
}

