#include "metadbovariable.h"

MetaDBOVariable::MetaDBOVariable(const std::string &class_id, const std::string &instance_id, DBObjectManager *object_manager)
    :Configurable (class_id, instance_id, object_manager), object_manager_(*object_manager)
{

}

MetaDBOVariable::~MetaDBOVariable ()
{

}

void MetaDBOVariable::checkSubConfigurables ()
{
    // nothing to do here
}

void MetaDBOVariable::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
//    if (class_id.compare("DBOVariableDefinition") == 0)
//    {
//        DBOVariableDefinition *definition = new DBOVariableDefinition (class_id, instance_id, this);
//        sub_variable_definitions_.push_back (definition);

//        const std::string &dbo_type = definition->getDBOType();
//        std::string name = definition->getId();

//        assert (sub_variables_.find(dbo_type) == sub_variables_.end());
//        sub_variables_[dbo_type] = name;
//    }
//    else
        throw std::runtime_error ("DBOVariable: generateSubConfigurable: unknown class_id "+class_id);
}

//bool DBOVariable::existsIn (const std::string &dbo_type)
//{
//    bool ret = (dbo_type_ == dbo_type);

//    if (!ret && sub_variables_.find(dbo_type) != sub_variables_.end() && sub_variables_[dbo_type].size() != 0)
//    {
//        DBOVariable *variable = DBObjectManager::getInstance().getDBOVariable (dbo_type, sub_variables_[dbo_type]);
//        ret |= variable->existsIn(dbo_type);
//    }

//    return ret;
//}

//DBOVariable *DBOVariable::getFor (const std::string &dbo_type)
//{
//    //assert (dbo_type != DBO_UNDEFINED);

//    if (!isMetaVariable())
//    {
//        assert (existsIn (dbo_type));
//        return this;
//    }
//    else
//    {
//        if (dbo_type == dbo_type_)
//            return this;

//        if (sub_variables_.find(dbo_type) != sub_variables_.end())
//        {
//            DBOVariable *var = DBObjectManager::getInstance().getDBOVariable (dbo_type, sub_variables_[dbo_type]);
//            assert (!var->isMetaVariable());
//            return var;
//        }

//        throw std::runtime_error ("DBOVariable: getFor: id '"+id_+"' type "+dbo_type_+": impossible for type "+dbo_type);
//    }
//}

//DBOVariable *DBOVariable::getFirst ()
//{
//    if (!isMetaVariable())
//    {
//        return this;
//    }
//    else
//    {
//        if (sub_variables_.size() == 0)
//            throw std::runtime_error ("DBOVariable: getFirst: no sub variables");

//        return DBObjectManager::getInstance().getDBOVariable (sub_variables_.begin()->first, sub_variables_.begin()->second);
//    }
//}


//std::string DBOVariable::getNameFor (const std::string &dbo_type)
//{
//    assert (existsIn (dbo_type));
//    return sub_variables_[dbo_type];
//}

//void DBOVariable::setSubVariable (const std::string &dbo_type, std::string name)
//{
//    logdbg  << "DBOVariable: changed: type " << dbo_type << " varname " << name;

//    bool set=false;
//    if (sub_variables_.find(dbo_type) != sub_variables_.end())
//    {
//        logdbg  << "DBOVariable: changed: sub variable should exist";
//        std::vector<DBOVariableDefinition *>::iterator it;
//        for (it = sub_variable_definitions_.begin(); it != sub_variable_definitions_.end(); it++)
//        {
//            if ((*it)->getDBOType() == dbo_type)
//            {
//                (*it)->setId (name);
//                set=true;
//                break;
//            }
//            else
//            {
//                logwrn  << "DBOVariable: changed: not exists at id " << (*it)->getId() <<  " type " << (*it)->getDBOType();
//            }
//        }
//        if (!set)
//            throw std::runtime_error ("DBOVariable: setSubVariable: not found though exists, type "+dbo_type+" name "+name);
//    }
//    else
//    {
//        std::string instance_id = "DBOVariableDefinition"+dbo_type+id_+name+"0";

//        logdbg  << "DBOVariable: setSubVariable: generating subvar type " << dbo_type << " name " << name << " instance " << instance_id;

//        Configuration &config = addNewSubConfiguration ("DBOVariableDefinition", instance_id);
//        config.addParameterString ("dbo_type", dbo_type);
//        config.addParameterString ("id", name);
//        generateSubConfigurable ("DBOVariableDefinition", instance_id);
//    }
//}

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
