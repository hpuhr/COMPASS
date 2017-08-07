
//#include <boost/algorithm/string.hpp>

#include "metadbovariable.h"
#include "metadbovariablewidget.h"
#include "dbovariable.h"
#include "dbobject.h"

MetaDBOVariable::MetaDBOVariable(const std::string &class_id, const std::string &instance_id, DBObjectManager *object_manager)
    :Configurable (class_id, instance_id, object_manager), object_manager_(*object_manager), widget_(nullptr)
{
    registerParameter("name", &name_, "");
    registerParameter("description", &description_, "");

    // DBOVAR LOWERCASE HACK
    //boost::algorithm::to_lower(name_);

    assert (name_.size() > 0);

    createSubConfigurables();
}

MetaDBOVariable::~MetaDBOVariable ()
{
    if (widget_)
    {
        delete widget_;
        widget_=nullptr;
    }

    for (auto it : definitions_)
        delete it.second;
    definitions_.clear();
    variables_.clear();
}

void MetaDBOVariable::checkSubConfigurables ()
{
    // nothing to do here
}

void MetaDBOVariable::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id.compare("DBOVariableDefinition") == 0)
    {
        DBOVariableDefinition *definition = new DBOVariableDefinition (class_id, instance_id, this);

        const std::string &dbo_name = definition->dboName();
        std::string dbovar_name = definition->variableName();

        // DBOVAR LOWERCASE HACK
        //boost::algorithm::to_lower(dbovar_name);

        assert (object_manager_.existsObject(dbo_name));
        assert (object_manager_.object(dbo_name).hasVariable(dbovar_name));
        assert (variables_.find(dbo_name) == variables_.end());

        definitions_[dbo_name] = definition;
        variables_.insert(std::pair<std::string, DBOVariable&> (dbo_name, object_manager_.object(dbo_name).variable(dbovar_name)));
    }
    else
        throw std::runtime_error ("DBOVariable: generateSubConfigurable: unknown class_id "+class_id);
}

bool MetaDBOVariable::existsIn (const std::string &dbo_name)
{
    return variables_.count(dbo_name) > 0;
}

DBOVariable &MetaDBOVariable::getFor (const std::string &dbo_name)
{
    assert (existsIn(dbo_name));
    return variables_.at(dbo_name);
}

std::string MetaDBOVariable::getNameFor (const std::string &dbo_name)
{
    assert (existsIn (dbo_name));
    return variables_.at(dbo_name).name();
}

void MetaDBOVariable::removeVariable (const std::string &dbo_name)
{
    loginf  << "MetaDBOVariable " << name_ << ": removeVariable: dbo " << dbo_name;
    assert (existsIn (dbo_name));
    delete definitions_.at(dbo_name);
    definitions_.erase(dbo_name);
    variables_.erase(dbo_name);
}

void MetaDBOVariable::addVariable (const std::string &dbo_name, const std::string &dbovariable_name)
{
    loginf  << "MetaDBOVariable " << name_ << ": addVariable: dbo " << dbo_name << " varname " << dbovariable_name;

    assert (!existsIn(dbo_name));

    std::string instance_id = "DBOVariableDefinition"+dbo_name+dbovariable_name+"0";

    Configuration &config = addNewSubConfiguration ("DBOVariableDefinition", instance_id);
    config.addParameterString ("dbo_name", dbo_name);
    config.addParameterString ("dbo_variable_name", dbovariable_name);
    generateSubConfigurable ("DBOVariableDefinition", instance_id);
}

MetaDBOVariableWidget *MetaDBOVariable::widget ()
{
    if (!widget_)
    {
        widget_ = new MetaDBOVariableWidget (*this);
    }
    assert (widget_);
    return widget_;
}

std::string MetaDBOVariable::name() const
{
    return name_;
}

void MetaDBOVariable::name(const std::string &name)
{
    name_ = name;
}

std::string MetaDBOVariable::description() const
{
    return description_;
}

void MetaDBOVariable::description(const std::string &description)
{
    description_ = description;
}

PropertyDataType MetaDBOVariable::dataType ()
{
    assert (hasVariables());

    PropertyDataType data_type = variables_.begin()->second.dataType();

    for (auto variable_it : variables_)
    {
        if (variable_it.second.dataType() != data_type)
            logerr << "MetaDBOVariable: dataType: meta var " << name_ << " has different data types in sub variables";
    }

    return data_type;
}

const std::string &MetaDBOVariable::dataTypeString()
{
    assert (hasVariables());
    return Property::asString(dataType());
}

Utils::String::Representation MetaDBOVariable::representation ()
{
    assert (hasVariables());

    Utils::String::Representation representation = variables_.begin()->second.representation();

    for (auto variable_it : variables_)
    {
        if (variable_it.second.representation() != representation)
            logerr << "MetaDBOVariable: dataType: meta var " << name_ << " has different representations in sub variables";
    }

    return representation;
}

