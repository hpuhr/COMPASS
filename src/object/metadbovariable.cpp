#include "metadbovariable.h"
#include "metadbovariablewidget.h"
#include "dbovariable.h"
#include "dbobject.h"

MetaDBOVariable::MetaDBOVariable(const std::string &class_id, const std::string &instance_id, DBObjectManager *object_manager)
    :Configurable (class_id, instance_id, object_manager), object_manager_(*object_manager), widget_(nullptr)
{
    registerParameter("name", &name_, "");
    registerParameter("description", &description_, "");

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
    assert (existsIn (dbo_name));
    delete definitions_.at(dbo_name);
    definitions_.erase(dbo_name);
    variables_.erase(dbo_name);
}

void MetaDBOVariable::addVariable (const std::string &dbo_name, const std::string &dbovariable_name)
{
    logdbg  << "MetaDBOVariable: addVariable: dbo " << dbo_name << " varname " << dbovariable_name;

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

/**
 * Bit of a hack, only to be called on meta variables. Basically, only for normal (= non-meta) variables the
 * minimum/maximum information can be generated. If such information should be generated for a meta-variable,
 * jobs are generated for all sub-variables. When the information is set for the sub-variables, the meta-variable
 * needs an update to inform its observers. Therefore, the meta-variable registers itself as parent to all its
 * sub-variables, and is updated when all sub-variables have the minimum/maximum information. Look into setMinMax()
 * and subVariableHasMinMaxInfo() for the details.
 */
//void DBOVariable::registerAsParent ()
//{
//    logdbg << "DBOVariable: registerAsParent: " << id_;
//    assert (isMetaVariable());
//    assert (!registered_as_parent_);
//    std::map <std::string, std::string>::iterator it;
//    for (it = sub_variables_.begin(); it != sub_variables_.end(); it++)
//    {
//        assert (DBObjectManager::getInstance().existsDBOVariable(it->first, it->second));
//        DBOVariable *var = DBObjectManager::getInstance().getDBOVariable(it->first, it->second);
//        var->registerParentVariable(this);

//        if (data_type_ != var->data_type_)
//            logwrn << "DBOVariable: registerAsParent: meta variable " << id_ << " has different data type " <<
//                      data_type_str_ << " than sub variable " << var->id_ << " data type "
//                   << var->data_type_str_;
//    }
//    registered_as_parent_=true;
//}

//void DBOVariable::unregisterAsParent ()
//{
//    logdbg << "DBOVariable: unregisterAsParent: " << id_;
//    assert (isMetaVariable());
//    assert (registered_as_parent_);
//    std::map <std::string, std::string>::iterator it;
//    for (it = sub_variables_.begin(); it != sub_variables_.end(); it++)
//    {
//        assert (DBObjectManager::getInstance().existsDBOVariable(it->first, it->second));
//        DBOVariable *var = DBObjectManager::getInstance().getDBOVariable(it->first, it->second);
//        var->unregisterParentVariable(this);
//    }
//    registered_as_parent_=false;
//}
