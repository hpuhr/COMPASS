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
    registerParameter("name", &name_, std::string());

    // DBOVAR LOWERCASE HACK
    // boost::algorithm::to_lower(name_);

    assert(name_.size() > 0);

    createSubConfigurables();

    checkSubVariables();
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

        const std::string& dbcontent_name = definition->dbContentName();
        std::string dbovar_name = definition->variableName();

        // DBOVAR LOWERCASE HACK
        // boost::algorithm::to_lower(dbovar_name);

        if (!object_manager_.existsDBContent(dbcontent_name))
        {
            logerr << "MetaVariable: generateSubConfigurable: name " << name_
                   << " dbovariable definition " << instance_id << " has unknown dbo, ignoring";
            // delete definition;
            return;
        }

        if (!object_manager_.dbContent(dbcontent_name).hasVariable(dbovar_name))
        {
            logerr << "MetaVariable: generateSubConfigurable: name " << name_
                   << " dbovariable definition " << instance_id << " has unknown dbo variable, ignoring";
            // delete definition;
            return;
        }

        if (variables_.find(dbcontent_name) != variables_.end())
        {
            logerr << "MetaVariable: generateSubConfigurable: name " << name_
                   << " dbovariable definition " << instance_id << " has already defined dbo, ignoring";
            // delete definition;
            return;
        }

        assert(object_manager_.existsDBContent(dbcontent_name));
        assert(object_manager_.dbContent(dbcontent_name).hasVariable(dbovar_name));
        assert(variables_.find(dbcontent_name) == variables_.end());

        definitions_[dbcontent_name] = definition;
        variables_.insert(std::pair<std::string, Variable&>(
            dbcontent_name, object_manager_.dbContent(dbcontent_name).variable(dbovar_name)));
    }
    else
        throw std::runtime_error("MetaVariable: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

bool MetaVariable::existsIn(const std::string& dbcontent_name)
{
    return variables_.count(dbcontent_name) > 0;
}

Variable& MetaVariable::getFor(const std::string& dbcontent_name)
{
    assert(existsIn(dbcontent_name));
    return variables_.at(dbcontent_name);
}

std::string MetaVariable::getNameFor(const std::string& dbcontent_name)
{
    assert(existsIn(dbcontent_name));
    return variables_.at(dbcontent_name).name();
}

void MetaVariable::set(Variable& var)
{
    string dbcontent_name = var.dbObject().name();

    loginf << "MetaVariable " << name_ << ": set: dbo " << dbcontent_name << " name " << var.name();

    if (existsIn(dbcontent_name))
        removeVariable(dbcontent_name);

    addVariable(dbcontent_name, var.name());
}

void MetaVariable::removeVariable(const std::string& dbcontent_name)
{
    loginf << "MetaVariable " << name_ << ": removeVariable: dbo " << dbcontent_name;
    assert(existsIn(dbcontent_name));
    delete definitions_.at(dbcontent_name);
    definitions_.erase(dbcontent_name);
    variables_.erase(dbcontent_name);

    updateDescription();
}

void MetaVariable::addVariable(const std::string& dbcontent_name, const std::string& dbovariable_name)
{
    loginf << "MetaVariable " << name_ << ": addVariable: dbo " << dbcontent_name << " varname "
           << dbovariable_name;

    assert(!existsIn(dbcontent_name));

    std::string instance_id = "VariableDefinition" + dbcontent_name + dbovariable_name + "0";

    auto config = Configuration::create("VariableDefinition", instance_id);
    config->addParameter<std::string>("dbcontent_name", dbcontent_name);
    config->addParameter<std::string>("variable_name", dbovariable_name);

    generateSubConfigurableFromConfig(std::move(config));
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

std::string MetaVariable::info() const
{
    std::ostringstream ss;

    ss << "Name: '" << name_ << "'" << endl;
    ss << "Data Type: " << dataTypeString() << endl;
    ss << "Description: " << description() << endl;

    return ss.str();
}

PropertyDataType MetaVariable::dataType() const
{
    assert(hasVariables());

    //checked in checkSubVariables

    return variables_.begin()->second.dataType();
}

const std::string& MetaVariable::dataTypeString() const
{
    assert(hasVariables());
    return Property::asString(dataType());
}

Variable::Representation MetaVariable::representation()
{
    assert(hasVariables());

    //checked in checkSubVariables

    return variables_.begin()->second.representation();
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

    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

    for (auto var_it = definitions_.begin(); var_it != definitions_.end();)
    {
        delete_var = false;

        if (!obj_man.existsDBContent(var_it->second->dbContentName()))
            delete_var = true;
        else if (!obj_man.dbContent(var_it->second->dbContentName())
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
        description_ += variable_it.second.info() + "\n\n";
    }
}

void MetaVariable::checkSubVariables()
{
    if (hasVariables())
    {
        PropertyDataType data_type = variables_.begin()->second.dataType();

        for (auto variable_it : variables_)
        {
            if (variable_it.second.dataType() != data_type)
            {
                logerr << "MetaVariable: checkSubVariables: meta var " << name_
                       << " has different data types in sub variables ("
                       << Property::asString(data_type) << ", " << variable_it.second.dataTypeString() << ")";
                throw std::runtime_error("Conflicting data types in metavariable '" + name_ + "' (" +
                                         Property::asString(data_type) + " =/= " + variable_it.second.dataTypeString() + ")");
            }
        }

        string rep_str = variables_.begin()->second.representationString();

        for (auto variable_it : variables_)
        {
            if (variable_it.second.representationString() != rep_str)
            {
                logerr << "MetaVariable: checkSubVariables: meta var " << name_
                       << " has different representations in sub variables ("
                       << rep_str << ", " << variable_it.second.representationString() << ")";
                throw std::runtime_error("Conflicting representations in metavariable '" + name_ + "' (" +
                                         rep_str + " =/= " + variable_it.second.representationString() + ")");
            }
        }
    }
}

}
