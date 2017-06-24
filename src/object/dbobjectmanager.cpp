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
#include "dbovariableset.h"
#include "dbobjectmanager.h"
#include "logger.h"
#include "dbobjectmanagerwidget.h"
//#include "structureDescriptionManager.h"

/**
 * Creates sub-configurables.
 */
DBObjectManager::DBObjectManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb)
: Configurable (class_id, instance_id, atsdb, "conf/config_dbo.xml"), widget_(nullptr) //, registered_parent_variables_ (false)
{
    logdbg  << "DBObjectManager: constructor: creating subconfigurables";

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
    for (auto it = objects_.begin(); it != objects_.end(); it++)
    {
        delete it->second;
    }
    objects_.clear();

    delete widget_;
    widget_ = nullptr;
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
        loginf  << "DBObjectManager: generateSubConfigurable: adding object type " << object->name();
        assert (objects_.find(object->name()) == objects_.end());
        objects_.insert(std::pair <std::string, DBObject*> (object->name(), object));
        connect (this, SIGNAL(schemaChangedSignal()), object, SLOT(schemaChangedSlot()));
        connect (this, SIGNAL(databaseOpenedSignal()), object, SLOT(databaseOpenedSlot()));
        // TODO what if generation after db opening?
    }
    else
        throw std::runtime_error ("DBObjectManager: generateSubConfigurable: unknown class_id "+class_id );
}

void DBObjectManager::checkSubConfigurables ()
{
    // nothing to do, must be defined in configuration
}

bool DBObjectManager::exists (const std::string &dbo_name)
{
    //registerParentVariablesIfRequired();

    return (objects_.find(dbo_name) != objects_.end());
}

DBObject &DBObjectManager::object (const std::string &dbo_name)
{
    logdbg  << "DBObjectManager: get: name " << dbo_name;

    //registerParentVariablesIfRequired();

    assert (objects_.find(dbo_name) != objects_.end());

    return *objects_.at(dbo_name);
}

void DBObjectManager::remove (const std::string &dbo_name)
{
    logdbg  << "DBObjectManager: del: name " << dbo_name;
    assert (exists(dbo_name));
    delete objects_.at(dbo_name);
    objects_.erase(dbo_name);

    emit dbObjectsChangedSignal();
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

void DBObjectManager::updateSchemaInformationSlot ()
{
    emit schemaChangedSignal();
}

void DBObjectManager::databaseOpenedSlot ()
{
    emit databaseOpenedSignal();
}
