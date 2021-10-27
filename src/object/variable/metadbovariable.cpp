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

//#include <boost/algorithm/string.hpp>

#include "metadbovariable.h"

#include "compass.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "metadbovariablewidget.h"

MetaDBOVariable::MetaDBOVariable(const std::string& class_id, const std::string& instance_id,
                                 DBObjectManager* object_manager)
    : Configurable(class_id, instance_id, object_manager),
      object_manager_(*object_manager),
      widget_(nullptr)
{
    registerParameter("name", &name_, "");
    registerParameter("description", &description_, "");

    // DBOVAR LOWERCASE HACK
    // boost::algorithm::to_lower(name_);

    assert(name_.size() > 0);

    createSubConfigurables();
}

MetaDBOVariable::~MetaDBOVariable()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (auto it : definitions_)
        delete it.second;
    definitions_.clear();
    variables_.clear();
}

void MetaDBOVariable::checkSubConfigurables()
{
    // nothing to do here
}

void MetaDBOVariable::generateSubConfigurable(const std::string& class_id,
                                              const std::string& instance_id)
{
    if (class_id.compare("DBOVariableDefinition") == 0)
    {
        DBOVariableDefinition* definition = new DBOVariableDefinition(class_id, instance_id, this);

        const std::string& dbo_name = definition->dboName();
        std::string dbovar_name = definition->variableName();

        // DBOVAR LOWERCASE HACK
        // boost::algorithm::to_lower(dbovar_name);

        if (!object_manager_.existsObject(dbo_name) ||
            !object_manager_.object(dbo_name).hasVariable(dbovar_name) ||
            variables_.find(dbo_name) != variables_.end())
        {
            logerr << "MetaDBOVariable: generateSubConfigurable: name " << name_
                   << " dbovariable definition " << instance_id << " has error, ignoring";
            // delete definition;
            return;
        }

        assert(object_manager_.existsObject(dbo_name));
        assert(object_manager_.object(dbo_name).hasVariable(dbovar_name));
        assert(variables_.find(dbo_name) == variables_.end());

        definitions_[dbo_name] = definition;
        variables_.insert(std::pair<std::string, DBOVariable&>(
            dbo_name, object_manager_.object(dbo_name).variable(dbovar_name)));
    }
    else
        throw std::runtime_error("DBOVariable: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

bool MetaDBOVariable::existsIn(const std::string& dbo_name)
{
    return variables_.count(dbo_name) > 0;
}

DBOVariable& MetaDBOVariable::getFor(const std::string& dbo_name)
{
    assert(existsIn(dbo_name));
    return variables_.at(dbo_name);
}

std::string MetaDBOVariable::getNameFor(const std::string& dbo_name)
{
    assert(existsIn(dbo_name));
    return variables_.at(dbo_name).name();
}

void MetaDBOVariable::removeVariable(const std::string& dbo_name)
{
    loginf << "MetaDBOVariable " << name_ << ": removeVariable: dbo " << dbo_name;
    assert(existsIn(dbo_name));
    delete definitions_.at(dbo_name);
    definitions_.erase(dbo_name);
    variables_.erase(dbo_name);
}

void MetaDBOVariable::addVariable(const std::string& dbo_name, const std::string& dbovariable_name)
{
    loginf << "MetaDBOVariable " << name_ << ": addVariable: dbo " << dbo_name << " varname "
           << dbovariable_name;

    assert(!existsIn(dbo_name));

    std::string instance_id = "DBOVariableDefinition" + dbo_name + dbovariable_name + "0";

    Configuration& config = addNewSubConfiguration("DBOVariableDefinition", instance_id);
    config.addParameterString("dbo_name", dbo_name);
    config.addParameterString("dbo_variable_name", dbovariable_name);
    generateSubConfigurable("DBOVariableDefinition", instance_id);
}

bool MetaDBOVariable::uses(const DBOVariable& variable)
{
    for (auto& var_it : variables_)
        if (var_it.second == variable)
            return true;

    return false;
}

MetaDBOVariableWidget* MetaDBOVariable::widget()
{
    if (!widget_)
    {
        widget_ = new MetaDBOVariableWidget(*this);

        if (locked_)
            widget_->lock();
    }
    assert(widget_);
    return widget_;
}

void MetaDBOVariable::unlock()
{
    if (!locked_)
        return;

    locked_ = false;

    if (widget_)
        widget_->unlock();
}

void MetaDBOVariable::lock()
{
    if (locked_)
        return;

    locked_ = true;

    if (widget_)
        widget_->lock();
}

std::string MetaDBOVariable::name() const { return name_; }

void MetaDBOVariable::name(const std::string& name) { name_ = name; }

std::string MetaDBOVariable::description() const { return description_; }

void MetaDBOVariable::description(const std::string& description) { description_ = description; }

PropertyDataType MetaDBOVariable::dataType() const
{
    assert(hasVariables());

    PropertyDataType data_type = variables_.begin()->second.dataType();

    for (auto variable_it : variables_)
    {
        if (variable_it.second.dataType() != data_type)
            logerr << "MetaDBOVariable: dataType: meta var " << name_
                   << " has different data types in sub variables";
    }

    return data_type;
}

const std::string& MetaDBOVariable::dataTypeString() const
{
    assert(hasVariables());
    return Property::asString(dataType());
}

DBOVariable::Representation MetaDBOVariable::representation()
{
    assert(hasVariables());

    DBOVariable::Representation representation = variables_.begin()->second.representation();

    for (auto variable_it : variables_)
    {
        if (variable_it.second.representation() != representation)
            logerr << "MetaDBOVariable: dataType: meta var " << name_
                   << " has different representations in sub variables";
    }

    return representation;
}

//std::string MetaDBOVariable::getMinString() const
//{
//    std::string value_string;

//    for (auto variable_it : variables_)
//    {
//        if (value_string.size() == 0)
//            value_string = variable_it.second.getMinString();
//        else
//            value_string = variable_it.second.getSmallerValueString(
//                value_string, variable_it.second.getMinString());
//    }
//    // assert (value_string.size());
//    return value_string;
//}

//std::string MetaDBOVariable::getMaxString() const
//{
//    std::string value_string;

//    for (auto variable_it : variables_)
//    {
//        if (value_string.size() == 0)
//            value_string = variable_it.second.getMaxString();
//        else
//            value_string = variable_it.second.getLargerValueString(
//                value_string, variable_it.second.getMaxString());
//    }
//    // assert (value_string.size());
//    return value_string;
//}

//std::string MetaDBOVariable::getMinStringRepresentation() const
//{
//    assert(variables_.size());
//    return variables_.begin()->second.getRepresentationStringFromValue(getMinString());
//}

//std::string MetaDBOVariable::getMaxStringRepresentation() const
//{
//    assert(variables_.size());
//    return variables_.begin()->second.getRepresentationStringFromValue(getMaxString());
//}

void MetaDBOVariable::removeOutdatedVariables()
{
    loginf << "MetaDBOVariable " << name() << ": removeOutdatedVariables";

    bool delete_var;

    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    for (auto var_it = definitions_.begin(); var_it != definitions_.end();)
    {
        delete_var = false;

        if (!obj_man.existsObject(var_it->second->dboName()))
            delete_var = true;
        else if (!obj_man.object(var_it->second->dboName())
                      .hasVariable(var_it->second->variableName()))
            delete_var = true;

        if (delete_var)
        {
            loginf << "MetaDBOVariable: removeOutdatedVariables: removing var " << var_it->first;
            assert(variables_.count(var_it->first));
            variables_.erase(var_it->first);

            delete var_it->second;
            definitions_.erase(var_it++);
        }
        else
            ++var_it;
    }
}
