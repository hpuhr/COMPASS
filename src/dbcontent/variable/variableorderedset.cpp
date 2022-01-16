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

#include "dbcontent/variable/variableorderedset.h"
#include "compass.h"
#include "configurationmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableorderedsetwidget.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/variable/metavariable.h"

#include <algorithm>

namespace dbContent
{

VariableOrderedSet::VariableOrderedSet(const std::string& class_id,
                                             const std::string& instance_id, Configurable* parent)
    : Configurable(class_id, instance_id, parent), widget_(nullptr)
{
    createSubConfigurables();
}

VariableOrderedSet::~VariableOrderedSet()
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

void VariableOrderedSet::generateSubConfigurable(const std::string& class_id,
                                                    const std::string& instance_id)
{
    logdbg << "VariableOrderedSet: generateSubConfigurable: class_id " << class_id
           << " instance_id " << instance_id;

    if (class_id.compare("VariableOrderDefinition") == 0)
    {
        VariableOrderDefinition* definition =
            new VariableOrderDefinition(class_id, instance_id, this);

        DBContentManager& manager = COMPASS::instance().objectManager();

        if (definition->dboName() == META_OBJECT_NAME)
        {
            if (!manager.existsMetaVariable(definition->variableName()))
            {
                logwrn << "VariableOrderedSet: generateSubConfigurable: outdated meta variable "
                       << definition->variableName();
                delete definition;
                return;
            }
        }
        else if (!manager.existsObject(definition->dboName()) ||
                 !manager.object(definition->dboName()).hasVariable(definition->variableName()))
        {
            logwrn << "VariableOrderedSet: generateSubConfigurable: outdated name "
                   << definition->dboName() << " variable " << definition->variableName();
            delete definition;
            return;
        }

        unsigned int index_new = definition->getIndex();
        assert(variable_definitions_.find(index_new) == variable_definitions_.end());
        variable_definitions_.insert(
            std::pair<unsigned int, VariableOrderDefinition*>(index_new, definition));
    }
    else
        throw std::runtime_error(
            "VariableOrderedSet: generateSubConfigurable: unknown class_id " + class_id);
}

void VariableOrderedSet::checkSubConfigurables() {}

void VariableOrderedSet::add(Variable& var)
{
    if (!hasVariable(var))
    {
        std::string var_name = var.name();

        Configuration& id_configuration = addNewSubConfiguration("VariableOrderDefinition");
        id_configuration.addParameterString("dbcontent_name", var.dboName());
        id_configuration.addParameterString("variable_name", var_name);
        id_configuration.addParameterUnsignedInt("index",
                                                 (unsigned int)variable_definitions_.size());
        generateSubConfigurable("VariableOrderDefinition", id_configuration.getInstanceId());

        emit setChangedSignal();
        emit variableAddedChangedSignal();
    }
}

void VariableOrderedSet::add(MetaVariable& var)
{
    if (!hasMetaVariable(var))
    {
        std::string var_name = var.name();

        Configuration& id_configuration = addNewSubConfiguration("VariableOrderDefinition");
        id_configuration.addParameterString("dbcontent_name", META_OBJECT_NAME);
        id_configuration.addParameterString("variable_name", var_name);
        id_configuration.addParameterUnsignedInt("index",
                                                 (unsigned int)variable_definitions_.size());
        generateSubConfigurable("VariableOrderDefinition", id_configuration.getInstanceId());

        emit setChangedSignal();
        emit variableAddedChangedSignal();
    }
}

void VariableOrderedSet::add (const std::string& dbo_name, const std::string var_name)
{
    if (!hasVariable(dbo_name, var_name))
    {
        Configuration& id_configuration = addNewSubConfiguration("VariableOrderDefinition");
        id_configuration.addParameterString("dbcontent_name", dbo_name);
        id_configuration.addParameterString("variable_name", var_name);
        id_configuration.addParameterUnsignedInt("index",
                                                 (unsigned int)variable_definitions_.size());
        generateSubConfigurable("VariableOrderDefinition", id_configuration.getInstanceId());

        emit setChangedSignal();
        emit variableAddedChangedSignal();
    }
}


// void VariableOrderedSet::add (const VariableOrderedSet &set)
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

// void VariableOrderedSet::addOnly (VariableOrderedSet &set, const std::string &dbo_type)
//{
//  logdbg  << "VariableOrderedSet: addOnly: type " << dbo_type;
//  std::vector <DBOVariable*> &setset = set.getSet();

//  //loginf  << "VariableOrderedSet: addOnly: getset";
//  std::vector <DBOVariable*>::iterator it;

//  //loginf  << "VariableOrderedSet: addOnly: iterating";
//  for (it=setset.begin(); it != setset.end(); it++)
//  {
//    if (find (set_.begin(), set_.end(), *it) == set_.end())
//    {
//      //loginf  << "VariableOrderedSet: addOnly: new var";
//      if ((*it)->existsIn(dbo_type))
//      {
//        logdbg  << "VariableOrderedSet: addOnly: pushback";
//        add ((*it)->getFor(dbo_type));
//      }
//    }
//  }
//  //loginf  << "VariableOrderedSet: addOnly: done";
//}

void VariableOrderedSet::removeVariableAt(unsigned int index)
{
    loginf << "VariableOrderedSet: removeVariableAt: index " << index;

    assert(index < variable_definitions_.size());
    assert(variable_definitions_.count(index) == 1);

    // unsigned int tmp_i = variable_definitions_.at(index);
    VariableOrderDefinition* tmp = variable_definitions_.at(index);
    variable_definitions_.erase(index);

    delete tmp;

    reorderVariables();

    emit setChangedSignal();
}

void VariableOrderedSet::removeVariable(const Variable& variable)
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

void VariableOrderedSet::removeMetaVariable(const MetaVariable& variable)
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


void VariableOrderedSet::reorderVariables ()
{
    std::map<unsigned int, VariableOrderDefinition*> new_variable_definitions;

    unsigned int key{0};

    for (auto& def_it : variable_definitions_)
    {
        def_it.second->setIndex(key);
        new_variable_definitions[key] = def_it.second;
        ++key;
    }

    variable_definitions_ = new_variable_definitions;
}

void VariableOrderedSet::moveVariableUp(unsigned int index)
{
    logdbg << "VariableOrderedSet: moveVariableUp: index " << index;
    assert(index < variable_definitions_.size());

    auto it = variable_definitions_.find(index);
    assert(it != variable_definitions_.end());

    if (index == 0)
    {
        logwrn << "VariableOrderedSet: moveVariableUp: tried to move up first variable";
        return;
    }

    auto itprev = variable_definitions_.find(index - 1);
    assert(itprev != variable_definitions_.end());

    std::swap(it->second, itprev->second);
    it->second->setIndex(it->first);
    itprev->second->setIndex(itprev->first);

    emit setChangedSignal();
}
void VariableOrderedSet::moveVariableDown(unsigned int index)
{
    logdbg << "VariableOrderedSet: moveVariableDown: index " << index;
    assert(index < variable_definitions_.size());

    auto it = variable_definitions_.find(index);
    assert(it != variable_definitions_.end());

    if (index == variable_definitions_.size() - 1)
    {
        logerr << "VariableOrderedSet: moveVariableDown: tried to down up last variable";
        return;
    }

    auto itnext = variable_definitions_.find(index + 1);
    assert(itnext != variable_definitions_.end());

    std::swap(it->second, itnext->second);
    it->second->setIndex(it->first);
    itnext->second->setIndex(itnext->first);

    emit setChangedSignal();
}

VariableSet VariableOrderedSet::getFor(const std::string& dbo_name)
{
    loginf << "VariableOrderedSet: getFor: type " << dbo_name;

    DBContentManager& manager = COMPASS::instance().objectManager();
    VariableSet type_set;
    std::map<unsigned int, VariableOrderDefinition*>::iterator it;

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

VariableSet VariableOrderedSet::getExistingInDBFor(const std::string& dbo_name)
{
    logdbg << "VariableOrderedSet: getExistingInDBFor: type " << dbo_name;

    DBContentManager& manager = COMPASS::instance().objectManager();
    VariableSet type_set;
    std::map<unsigned int, VariableOrderDefinition*>::iterator it;

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

// DBOVariableSet VariableOrderedSet::getUnorderedSet () const
//{
//  logdbg  << "VariableOrderedSet: getSet";

//  DBOVariableSet type_set;

//  for (auto it : set_)
//  {
//      logdbg  << "VariableOrderedSet: getFor: add";
//      type_set.add (it.second);
//  }

//  return type_set;
//}

VariableOrderDefinition& VariableOrderedSet::variableDefinition(unsigned int index) const
{
    assert(index < variable_definitions_.size());
    return *variable_definitions_.at(index);
}

// void VariableOrderedSet::print () const
//{
//    logdbg  << "VariableOrderedSet: print: size" << set_.size() << " changed " << changed_;
//    //std::vector <DBOVariable*>::iterator it;

//    for (auto it : set_)
//    {
//        it.second.print();
//    }
//}

bool VariableOrderedSet::hasVariable(const Variable& variable) const
{
    for (auto it : variable_definitions_)
        if (it.second->variableName() == variable.name() &&
            it.second->dboName() == variable.dbObject().name())
            return true;

    return false;
}

bool VariableOrderedSet::hasMetaVariable(const MetaVariable& variable) const
{
    for (auto it : variable_definitions_)
        if (it.second->variableName() == variable.name() &&
            it.second->dboName() == META_OBJECT_NAME)
            return true;

    return false;
}

bool VariableOrderedSet::hasVariable(const std::string& dbo_name, const std::string& name) const
{
    for (auto it : variable_definitions_)
        if (it.second->variableName() == name &&
            it.second->dboName() == dbo_name)
            return true;

    return false;
}

// PropertyList VariableOrderedSet::getPropertyList (const std::string &dbo_type)
//{
//  std::vector <DBOVariable*>::iterator it;
//  PropertyList list;

//  for (it=set_.begin(); it != set_.end(); it++)
//  {
//    if ((*it)->existsIn(dbo_type))
//    {
//      logdbg  << "VariableOrderedSet: getPropertyList: getfor";
//      DBOVariable *var = (*it)->getFor(dbo_type);
//      list.addProperty (var->getId(), var->getDataType());
//    }
//  }

//  return list;
//}

VariableOrderedSetWidget* VariableOrderedSet::widget()
{
    if (!widget_)
    {
        widget_ = new VariableOrderedSetWidget(*this);
        connect(this, SIGNAL(setChangedSignal()), widget_, SLOT(updateVariableListSlot()));
    }

    assert(widget_);
    return widget_;
}

}
