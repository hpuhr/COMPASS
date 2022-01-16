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

#include "dbcontent/variable/metavariable.h"

#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariablewidget.h"

using namespace std;

namespace dbContent
{

MetaVariable::MetaVariable(const std::string& class_id, const std::string& instance_id,
                                 DBContentManager* object_manager)
    : Configurable(class_id, instance_id, object_manager),
      object_manager_(*object_manager),
      widget_(nullptr)
{
    registerParameter("name", &name_, "");

    // DBOVAR LOWERCASE HACK
    // boost::algorithm::to_lower(name_);

    assert(name_.size() > 0);

    createSubConfigurables();

    updateDescription();
}

MetaVariable::~MetaVariable()
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

void MetaVariable::checkSubConfigurables()
{
    // nothing to do here
}

void MetaVariable::generateSubConfigurable(const std::string& class_id,
                                              const std::string& instance_id)
{
    if (class_id.compare("VariableDefinition") == 0)
    {
        VariableDefinition* definition = new VariableDefinition(class_id, instance_id, this);

        const std::string& dbo_name = definition->dboName();
        std::string dbovar_name = definition->variableName();

        // DBOVAR LOWERCASE HACK
        // boost::algorithm::to_lower(dbovar_name);

        if (!object_manager_.existsObject(dbo_name))
        {
            logerr << "MetaVariable: generateSubConfigurable: name " << name_
                   << " dbovariable definition " << instance_id << " has unknown dbo, ignoring";
            // delete definition;
            return;
        }

        if (!object_manager_.object(dbo_name).hasVariable(dbovar_name))
        {
            logerr << "MetaVariable: generateSubConfigurable: name " << name_
                   << " dbovariable definition " << instance_id << " has unknown dbo variable, ignoring";
            // delete definition;
            return;
        }

        if (variables_.find(dbo_name) != variables_.end())
        {
            logerr << "MetaVariable: generateSubConfigurable: name " << name_
                   << " dbovariable definition " << instance_id << " has already defined dbo, ignoring";
            // delete definition;
            return;
        }

        assert(object_manager_.existsObject(dbo_name));
        assert(object_manager_.object(dbo_name).hasVariable(dbovar_name));
        assert(variables_.find(dbo_name) == variables_.end());

        definitions_[dbo_name] = definition;
        variables_.insert(std::pair<std::string, Variable&>(
            dbo_name, object_manager_.object(dbo_name).variable(dbovar_name)));
    }
    else
        throw std::runtime_error("MetaVariable: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

bool MetaVariable::existsIn(const std::string& dbo_name)
{
    return variables_.count(dbo_name) > 0;
}

Variable& MetaVariable::getFor(const std::string& dbo_name)
{
    assert(existsIn(dbo_name));
    return variables_.at(dbo_name);
}

std::string MetaVariable::getNameFor(const std::string& dbo_name)
{
    assert(existsIn(dbo_name));
    return variables_.at(dbo_name).name();
}

void MetaVariable::set(Variable& var)
{
    string dbo_name = var.dbObject().name();

    loginf << "MetaVariable " << name_ << ": set: dbo " << dbo_name << " name " << var.name();

    if (existsIn(dbo_name))
        removeVariable(dbo_name);

    addVariable(dbo_name, var.name());
}

void MetaVariable::removeVariable(const std::string& dbo_name)
{
    loginf << "MetaVariable " << name_ << ": removeVariable: dbo " << dbo_name;
    assert(existsIn(dbo_name));
    delete definitions_.at(dbo_name);
    definitions_.erase(dbo_name);
    variables_.erase(dbo_name);

    updateDescription();
}

void MetaVariable::addVariable(const std::string& dbo_name, const std::string& dbovariable_name)
{
    loginf << "MetaVariable " << name_ << ": addVariable: dbo " << dbo_name << " varname "
           << dbovariable_name;

    assert(!existsIn(dbo_name));

    std::string instance_id = "VariableDefinition" + dbo_name + dbovariable_name + "0";

    Configuration& config = addNewSubConfiguration("VariableDefinition", instance_id);
    config.addParameterString("dbcontent_name", dbo_name);
    config.addParameterString("variable_name", dbovariable_name);
    generateSubConfigurable("VariableDefinition", instance_id);

    updateDescription();
}

bool MetaVariable::uses(const Variable& variable)
{
    for (auto& var_it : variables_)
        if (var_it.second == variable)
            return true;

    return false;
}

MetaVariableWidget* MetaVariable::widget()
{
    if (!widget_)
    {
        widget_ = new MetaVariableWidget(*this);

        if (locked_)
            widget_->lock();
    }
    assert(widget_);
    return widget_;
}

void MetaVariable::unlock()
{
    if (!locked_)
        return;

    locked_ = false;

    if (widget_)
        widget_->unlock();
}

void MetaVariable::lock()
{
    if (locked_)
        return;

    locked_ = true;

    if (widget_)
        widget_->lock();
}

std::string MetaVariable::name() const { return name_; }

void MetaVariable::name(const std::string& name) { name_ = name; }

std::string MetaVariable::description() const { return description_; }

PropertyDataType MetaVariable::dataType() const
{
    assert(hasVariables());

    PropertyDataType data_type = variables_.begin()->second.dataType();

    for (auto variable_it : variables_)
    {
        if (variable_it.second.dataType() != data_type)
            logerr << "MetaVariable: dataType: meta var " << name_
                   << " has different data types in sub variables";
    }

    return data_type;
}

const std::string& MetaVariable::dataTypeString() const
{
    assert(hasVariables());
    return Property::asString(dataType());
}

Variable::Representation MetaVariable::representation()
{
    assert(hasVariables());

    Variable::Representation representation = variables_.begin()->second.representation();

    for (auto variable_it : variables_)
    {
        if (variable_it.second.representation() != representation)
            logerr << "MetaVariable: dataType: meta var " << name_
                   << " has different representations in sub variables";
    }

    return representation;
}

//std::string MetaVariable::getMinString() const
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

//std::string MetaVariable::getMaxString() const
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

//std::string MetaVariable::getMinStringRepresentation() const
//{
//    assert(variables_.size());
//    return variables_.begin()->second.getRepresentationStringFromValue(getMinString());
//}

//std::string MetaVariable::getMaxStringRepresentation() const
//{
//    assert(variables_.size());
//    return variables_.begin()->second.getRepresentationStringFromValue(getMaxString());
//}

void MetaVariable::removeOutdatedVariables()
{
    loginf << "MetaVariable " << name() << ": removeOutdatedVariables";

    bool delete_var;

    DBContentManager& obj_man = COMPASS::instance().objectManager();

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
            loginf << "MetaVariable: removeOutdatedVariables: removing var " << var_it->first;
            assert(variables_.count(var_it->first));
            variables_.erase(var_it->first);

            delete var_it->second;
            definitions_.erase(var_it++);
        }
        else
            ++var_it;
    }
}

void MetaVariable::updateDescription()
{
    description_ = "";

    for (auto& variable_it : variables_)
    {
        description_ += "For " + variable_it.first +" ("+ variable_it.second.dataTypeString()+"):\n";
        description_ += variable_it.second.description() + "\n\n";
    }
}

}
