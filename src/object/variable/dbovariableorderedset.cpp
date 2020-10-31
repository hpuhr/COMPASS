/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbovariableorderedset.h"

#include <algorithm>

#include "compass.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "dbovariableorderedsetwidget.h"
#include "dbovariableset.h"
#include "metadbovariable.h"

DBOVariableOrderedSet::DBOVariableOrderedSet(const std::string& class_id,
                                             const std::string& instance_id, Configurable* parent)
    : Configurable(class_id, instance_id, parent), widget_(nullptr)
{
    createSubConfigurables();
}

DBOVariableOrderedSet::~DBOVariableOrderedSet()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (auto it : variable_definitions_)
    {
        delete it.second;
    }
    variable_definitions_.clear();
}

void DBOVariableOrderedSet::generateSubConfigurable(const std::string& class_id,
                                                    const std::string& instance_id)
{
    logdbg << "DBOVariableOrderedSet: generateSubConfigurable: class_id " << class_id
           << " instance_id " << instance_id;

    if (class_id.compare("DBOVariableOrderDefinition") == 0)
    {
        DBOVariableOrderDefinition* definition =
            new DBOVariableOrderDefinition(class_id, instance_id, this);

        DBObjectManager& manager = COMPASS::instance().objectManager();

        if (definition->dboName() == META_OBJECT_NAME)
        {
            if (!manager.existsMetaVariable(definition->variableName()))
            {
                logwrn << "DBOVariableOrderedSet: generateSubConfigurable: outdated meta variable "
                       << definition->variableName();
                delete definition;
                return;
            }
        }
        else if (!manager.existsObject(definition->dboName()) ||
                 !manager.object(definition->dboName()).hasVariable(definition->variableName()))
        {
            logwrn << "DBOVariableOrderedSet: generateSubConfigurable: outdated name "
                   << definition->dboName() << " variable " << definition->variableName();
            delete definition;
            return;
        }

        unsigned int index_new = definition->getIndex();
        assert(variable_definitions_.find(index_new) == variable_definitions_.end());
        variable_definitions_.insert(
            std::pair<unsigned int, DBOVariableOrderDefinition*>(index_new, definition));
    }
    else
        throw std::runtime_error(
            "DBOVariableOrderedSet: generateSubConfigurable: unknown class_id " + class_id);
}

void DBOVariableOrderedSet::checkSubConfigurables() {}

void DBOVariableOrderedSet::add(DBOVariable& var)
{
    if (!hasVariable(var))
    {
        std::string var_name = var.name();

        Configuration& id_configuration = addNewSubConfiguration("DBOVariableOrderDefinition");
        id_configuration.addParameterString("dbo_name", var.dboName());
        id_configuration.addParameterString("dbo_variable_name", var_name);
        id_configuration.addParameterUnsignedInt("index",
                                                 (unsigned int)variable_definitions_.size());
        generateSubConfigurable("DBOVariableOrderDefinition", id_configuration.getInstanceId());

        emit setChangedSignal();
        emit variableAddedChangedSignal();
    }
}

void DBOVariableOrderedSet::add(MetaDBOVariable& var)
{
    if (!hasMetaVariable(var))
    {
        std::string var_name = var.name();

        Configuration& id_configuration = addNewSubConfiguration("DBOVariableOrderDefinition");
        id_configuration.addParameterString("dbo_name", META_OBJECT_NAME);
        id_configuration.addParameterString("dbo_variable_name", var_name);
        id_configuration.addParameterUnsignedInt("index",
                                                 (unsigned int)variable_definitions_.size());
        generateSubConfigurable("DBOVariableOrderDefinition", id_configuration.getInstanceId());

        emit setChangedSignal();
        emit variableAddedChangedSignal();
    }
}

// void DBOVariableOrderedSet::add (const DBOVariableOrderedSet &set)
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

// void DBOVariableOrderedSet::addOnly (DBOVariableOrderedSet &set, const std::string &dbo_type)
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

void DBOVariableOrderedSet::removeVariableAt(unsigned int index)
{
    loginf << "DBOVariableOrderedSet: removeVariableAt: index " << index;

    assert(index < variable_definitions_.size());
    assert(variable_definitions_.count(index) == 1);

    // unsigned int tmp_i = variable_definitions_.at(index);
    DBOVariableOrderDefinition* tmp = variable_definitions_.at(index);
    variable_definitions_.erase(index);

    delete tmp;

    reorderVariables();

    emit setChangedSignal();
}

void DBOVariableOrderedSet::removeVariable(const DBOVariable& variable)
{
    assert (hasVariable(variable));

    int index = -1;
    for (auto it : variable_definitions_)
        if (it.second->variableName() == variable.name() &&
            it.second->dboName() == variable.dbObject().name())
        {
            index = it.first;
            break;
        }

    assert (index >= 0);
    removeVariableAt(index);
}

void DBOVariableOrderedSet::removeMetaVariable(const MetaDBOVariable& variable)
{
    assert (hasMetaVariable(variable));

    int index = -1;
    for (auto it : variable_definitions_)
        if (it.second->variableName() == variable.name() &&
            it.second->dboName() == META_OBJECT_NAME)
        {
            index = it.first;
            break;
        }

    assert (index >= 0);
    removeVariableAt(index);
}


void DBOVariableOrderedSet::reorderVariables ()
{
    std::map<unsigned int, DBOVariableOrderDefinition*> new_variable_definitions;

    unsigned int key{0};

    for (auto& def_it : variable_definitions_)
    {
        def_it.second->setIndex(key);
        new_variable_definitions[key] = def_it.second;
        ++key;
    }

    variable_definitions_ = new_variable_definitions;
}

void DBOVariableOrderedSet::moveVariableUp(unsigned int index)
{
    logdbg << "DBOVariableOrderedSet: moveVariableUp: index " << index;
    assert(index < variable_definitions_.size());

    auto it = variable_definitions_.find(index);
    assert(it != variable_definitions_.end());

    if (index == 0)
    {
        logwrn << "DBOVariableOrderedSet: moveVariableUp: tried to move up first variable";
        return;
    }

    auto itprev = variable_definitions_.find(index - 1);
    assert(itprev != variable_definitions_.end());

    std::swap(it->second, itprev->second);
    it->second->setIndex(it->first);
    itprev->second->setIndex(itprev->first);

    emit setChangedSignal();
}
void DBOVariableOrderedSet::moveVariableDown(unsigned int index)
{
    logdbg << "DBOVariableOrderedSet: moveVariableDown: index " << index;
    assert(index < variable_definitions_.size());

    auto it = variable_definitions_.find(index);
    assert(it != variable_definitions_.end());

    if (index == variable_definitions_.size() - 1)
    {
        logerr << "DBOVariableOrderedSet: moveVariableDown: tried to down up last variable";
        return;
    }

    auto itnext = variable_definitions_.find(index + 1);
    assert(itnext != variable_definitions_.end());

    std::swap(it->second, itnext->second);
    it->second->setIndex(it->first);
    itnext->second->setIndex(itnext->first);

    emit setChangedSignal();
}

DBOVariableSet DBOVariableOrderedSet::getFor(const std::string& dbo_name)
{
    loginf << "DBOVariableOrderedSet: getFor: type " << dbo_name;

    DBObjectManager& manager = COMPASS::instance().objectManager();
    DBOVariableSet type_set;
    std::map<unsigned int, DBOVariableOrderDefinition*>::iterator it;

    for (it = variable_definitions_.begin(); it != variable_definitions_.end(); it++)
    {
        if (it->second->dboName() == META_OBJECT_NAME)
        {
            assert(manager.existsMetaVariable(it->second->variableName()));
            if (manager.metaVariable(it->second->variableName()).existsIn(dbo_name))
                type_set.add(manager.metaVariable(it->second->variableName()).getFor(dbo_name));
        }
        else if (it->second->dboName() == dbo_name)
        {
            assert(manager.existsObject(dbo_name));
            assert(manager.object(dbo_name).hasVariable(it->second->variableName()));
            type_set.add(manager.object(dbo_name).variable(it->second->variableName()));
        }
    }

    return type_set;
}

DBOVariableSet DBOVariableOrderedSet::getExistingInDBFor(const std::string& dbo_name)
{
    logdbg << "DBOVariableOrderedSet: getExistingInDBFor: type " << dbo_name;

    DBObjectManager& manager = COMPASS::instance().objectManager();
    DBOVariableSet type_set;
    std::map<unsigned int, DBOVariableOrderDefinition*>::iterator it;

    for (it = variable_definitions_.begin(); it != variable_definitions_.end(); it++)
    {
        if (it->second->dboName() == META_OBJECT_NAME)
        {
            assert(manager.existsMetaVariable(it->second->variableName()));
            if (manager.metaVariable(it->second->variableName()).existsIn(dbo_name) &&
                manager.metaVariable(it->second->variableName()).existsInDB() &&
                manager.metaVariable(it->second->variableName()).getFor(dbo_name).existsInDB())
                type_set.add(manager.metaVariable(it->second->variableName()).getFor(dbo_name));
        }
        else if (it->second->dboName() == dbo_name)
        {
            assert(manager.existsObject(dbo_name));
            assert(manager.object(dbo_name).hasVariable(it->second->variableName()));
            if (manager.object(dbo_name).variable(it->second->variableName()).existsInDB())
                type_set.add(manager.object(dbo_name).variable(it->second->variableName()));
        }
    }

    return type_set;
}

// DBOVariableSet DBOVariableOrderedSet::getUnorderedSet () const
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

DBOVariableOrderDefinition& DBOVariableOrderedSet::variableDefinition(unsigned int index) const
{
    assert(index < variable_definitions_.size());
    return *variable_definitions_.at(index);
}

// void DBOVariableOrderedSet::print () const
//{
//    logdbg  << "DBOVariableOrderedSet: print: size" << set_.size() << " changed " << changed_;
//    //std::vector <DBOVariable*>::iterator it;

//    for (auto it : set_)
//    {
//        it.second.print();
//    }
//}

bool DBOVariableOrderedSet::hasVariable(const DBOVariable& variable) const
{
    for (auto it : variable_definitions_)
        if (it.second->variableName() == variable.name() &&
            it.second->dboName() == variable.dbObject().name())
            return true;

    return false;
}

bool DBOVariableOrderedSet::hasMetaVariable(const MetaDBOVariable& variable) const
{
    for (auto it : variable_definitions_)
        if (it.second->variableName() == variable.name() &&
            it.second->dboName() == META_OBJECT_NAME)
            return true;

    return false;
}

// PropertyList DBOVariableOrderedSet::getPropertyList (const std::string &dbo_type)
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

DBOVariableOrderedSetWidget* DBOVariableOrderedSet::widget()
{
    if (!widget_)
    {
        widget_ = new DBOVariableOrderedSetWidget(*this);
        connect(this, SIGNAL(setChangedSignal()), widget_, SLOT(updateVariableListSlot()));
    }

    assert(widget_);
    return widget_;
}
