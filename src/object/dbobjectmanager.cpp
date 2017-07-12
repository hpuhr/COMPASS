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
 * DBObjectManager.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: sk
 */

#include "atsdb.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "dbovariableset.h"
#include "dbobjectmanager.h"
#include "logger.h"
#include "dbobjectmanagerwidget.h"
#include "dbobjectmanagerloadwidget.h"
//#include "structureDescriptionManager.h"
#include "stringconv.h"
#include "viewmanager.h"

using Utils::String;

/**
 * Creates sub-configurables.
 */
DBObjectManager::DBObjectManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb)
: Configurable (class_id, instance_id, atsdb, "conf/config_dbo.xml"), widget_(nullptr), load_widget_(nullptr) //, registered_parent_variables_ (false)
{
    logdbg  << "DBObjectManager: constructor: creating subconfigurables";

    registerParameter("use_filters", &use_filters_, false);
    registerParameter("use_limit", &use_limit_, false);
    registerParameter("limit_min", &limit_min_, 0);
    registerParameter("limit_max", &limit_max_, 1000);

    createSubConfigurables ();
}
/**
 * Deletes all DBOs.
 */
DBObjectManager::~DBObjectManager()
{
//    if (registered_parent_variables_)
//    {
//        //loginf << "DBObjectManager: registerParentVariablesIfRequired: registering";
//        std::map <std::string, DBObject*>::iterator it;

//        for (it = objects_.begin(); it != objects_.end(); it++)
//            if (it->second->isMeta())
//                it->second->unregisterParentVariables();

//        registered_parent_variables_=false;
//    }
    for (auto it : objects_)
        delete it.second;
    objects_.clear();

    for (auto it : meta_variables_)
        delete it.second;
    meta_variables_.clear();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    if (load_widget_)
    {
        delete load_widget_;
        load_widget_ = nullptr;
    }
}

/**
 * Can create DBOs.
 */
void DBObjectManager::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "DBObjectManager: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
    if (class_id.compare ("DBObject") == 0)
    {
        DBObject *object = new DBObject (class_id, instance_id, this);
        logdbg  << "DBObjectManager: generateSubConfigurable: adding object type " << object->name();
        assert (objects_.find(object->name()) == objects_.end());
        objects_.insert(std::pair <std::string, DBObject*> (object->name(), object));
        connect (this, SIGNAL(schemaChangedSignal()), object, SLOT(schemaChangedSlot()));
        connect (this, SIGNAL(databaseOpenedSignal()), object, SLOT(databaseOpenedSlot()));
        // TODO what if generation after db opening?
    }
    else if (class_id.compare ("MetaDBOVariable") == 0)
    {
        MetaDBOVariable *meta_var = new MetaDBOVariable (class_id, instance_id, this);
        logdbg  << "DBObjectManager: generateSubConfigurable: adding meta var type " << meta_var->name();
        assert (meta_variables_.find(meta_var->name()) == meta_variables_.end());
        meta_variables_.insert(std::pair <std::string, MetaDBOVariable*> (meta_var->name(), meta_var));
    }
    else
        throw std::runtime_error ("DBObjectManager: generateSubConfigurable: unknown class_id "+class_id );
}

void DBObjectManager::checkSubConfigurables ()
{
    // nothing to do, must be defined in configuration
}

bool DBObjectManager::existsObject (const std::string &dbo_name)
{
    //registerParentVariablesIfRequired();

    return (objects_.find(dbo_name) != objects_.end());
}

DBObject &DBObjectManager::object (const std::string &dbo_name)
{
    logdbg  << "DBObjectManager: object: name " << dbo_name;

    //registerParentVariablesIfRequired();

    assert (objects_.find(dbo_name) != objects_.end());

    return *objects_.at(dbo_name);
}

void DBObjectManager::deleteObject (const std::string &dbo_name)
{
    logdbg  << "DBObjectManager: deleteObject: name " << dbo_name;
    assert (existsObject(dbo_name));
    delete objects_.at(dbo_name);
    objects_.erase(dbo_name);

    emit dbObjectsChangedSignal();
}

bool DBObjectManager::existsMetaVariable (const std::string &var_name)
{
    return (meta_variables_.find(var_name) != meta_variables_.end());
}

MetaDBOVariable &DBObjectManager::metaVariable (const std::string &var_name)
{
    logdbg  << "DBObjectManager: metaVariable: name " << var_name;

    assert (meta_variables_.find(var_name) != meta_variables_.end());

    return *meta_variables_.at(var_name);
}

void DBObjectManager::deleteMetaVariable (const std::string &var_name)
{
    logdbg  << "DBObjectManager: deleteMetaVariable: name " << var_name;
    assert (existsMetaVariable(var_name));
    delete meta_variables_.at(var_name);
    meta_variables_.erase(var_name);
}


/**
 * Checks if variable exists, returns it if found.
 *
 * \exception std::runtime_error if variable not found.
 */
//DBOVariable *DBObjectManager::getDBOVariable (const std::string &dbo_type, std::string id)
//{
//    logdbg  << "DBObjectManager: getDBOVariable: type " << dbo_type << " id " << id;

//    //registerParentVariablesIfRequired();

//    assert (existsDBObject(dbo_type));
//    assert (id.size() > 0);

//    if (!existsDBOVariable (dbo_type, id))
//    {
//        logerr  << "DBObjectManager: getDBOVariable: variable unknown type " << dbo_type << " id " << id;
//        throw std::runtime_error("DBObjectManager: getDBOVariable: variable unknown");
//    }

//    return getDBObject (dbo_type)->getVariable(id);
//}

//std::map <std::string, DBOVariable*> &DBObjectManager::getDBOVariables (const std::string &dbo_type)
//{
//    //registerParentVariablesIfRequired();

//    assert (existsDBObject(dbo_type));
//    return getDBObject (dbo_type)->getVariables();
//}

//bool DBObjectManager::existsDBOVariable (const std::string &dbo_type, std::string id)
//{
//    //registerParentVariablesIfRequired();

//    if (!existsDBObject(dbo_type))
//        return false;
//    return getDBObject (dbo_type)->hasVariable(id);
//}

//void DBObjectManager::registerParentVariablesIfRequired ()
//{
//    if (!registered_parent_variables_)
//    {
//        //loginf << "DBObjectManager: registerParentVariablesIfRequired: registering";
//        std::map <std::string, DBObject*>::iterator it;

//        for (it = objects_.begin(); it != objects_.end(); it++)
//            if (it->second->isMeta())
//                it->second->registerParentVariables();

//        registered_parent_variables_=true;
//    }
//}

DBObjectManagerWidget *DBObjectManager::widget()
{
    if (!widget_)
    {
        widget_ = new DBObjectManagerWidget (*this);
    }

    assert (widget_);
    return widget_;
}

DBObjectManagerLoadWidget *DBObjectManager::loadWidget()
{
    if (!load_widget_)
    {
        load_widget_ = new DBObjectManagerLoadWidget (*this);
    }

    assert (load_widget_);
    return load_widget_;
}

bool DBObjectManager::useLimit() const
{
    return use_limit_;
}

void DBObjectManager::useLimit(bool use_limit)
{
    use_limit_ = use_limit;
}

unsigned int DBObjectManager::limitMin() const
{
    return limit_min_;
}

void DBObjectManager::limitMin(unsigned int limit_min)
{
    limit_min_ = limit_min;
    loginf << "DBObjectManager: limitMin: " << limit_min_;
}

unsigned int DBObjectManager::limitMax() const
{
    return limit_max_;
}

void DBObjectManager::limitMax(unsigned int limit_max)
{
    limit_max_ = limit_max;
    loginf << "DBObjectManager: limitMax: " << limit_max_;
}

bool DBObjectManager::useFilters() const
{
    return use_filters_;
}

void DBObjectManager::useFilters(bool use_filters)
{
    use_filters_ = use_filters;
    loginf << "DBObjectManager: useFilters: " << use_filters_;
}

void DBObjectManager::loadSlot ()
{
    loginf << "DBObjectManager: loadSlot";
    for (auto object : objects())
    {
        loginf << "DBObjectManagerInfoWidget: loadSlot: object " << object.first << " wanted loading " << object.second->loadingWanted();
        if (object.second->loadingWanted())
        {
            DBOVariableSet read_set = ATSDB::instance().viewManager().getReadSet(object.first);

            if (use_limit_)
            {
                std::string limit_str = String::intToString(limit_min_)+","+String::intToString(limit_max_);
                loginf << "DBObjectManager: loadSlot: use limit str " << limit_str;
                object.second->load(read_set, use_filters_, limit_str);
            }
            else
                object.second->load(read_set, use_filters_);
        }
    }
    emit loadingStartedSignal();
}

void DBObjectManager::updateSchemaInformationSlot ()
{
    emit schemaChangedSignal();
}

void DBObjectManager::databaseOpenedSlot ()
{
    emit databaseOpenedSignal();
}
