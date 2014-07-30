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

#include "ConfigurationManager.h"
#include "DBObject.h"
#include "DBOVariable.h"
#include "DBOVariableSet.h"
#include "DBObjectManager.h"
#include "DBOVariable.h"
#include "Logger.h"
#include "StructureDescriptionManager.h"

/**
 * Creates sub-configurables.
 */
DBObjectManager::DBObjectManager()
: Configurable ("DBObjectManager", "DBObjectManager0", 0, "conf/config_dbo.xml"), registered_parent_variables_ (false)
{
    logdbg  << "DBObjectManager: constructor: creating subconfigurables";

    createSubConfigurables ();

}
/**
 * Deletes all DBOs.
 */
DBObjectManager::~DBObjectManager()
{
    if (registered_parent_variables_)
    {
        //loginf << "DBObjectManager: registerParentVariablesIfRequired: registering";
        if (objects_.find(DBO_UNDEFINED) != objects_.end())
            objects_[DBO_UNDEFINED]->unregisterParentVariables();
        registered_parent_variables_=false;
    }

    std::map <DB_OBJECT_TYPE, DBObject*>::iterator it;

    for (it = objects_.begin(); it != objects_.end(); it++)
    {
        delete it->second;
        it->second=0;
    }
    objects_.clear();
}

/**
 * Can create DBOs.
 */
void DBObjectManager::generateSubConfigurable (std::string class_id, std::string instance_id)
{
    logdbg  << "DBObjectManager: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
    if (class_id.compare ("DBObject") == 0)
    {
        DBObject *object = new DBObject (class_id, instance_id, this);
        loginf  << "DBObjectManager: generateSubConfigurable: adding object type " << object->getName();
        assert (objects_.find(object->getType()) == objects_.end());
        objects_[object->getType()] = object;
    }
    else
        throw std::runtime_error ("DBObjectManager: generateSubConfigurable: unknown class_id "+class_id );
}

void DBObjectManager::checkSubConfigurables ()
{
    // nothing to do, must be defined in configuration
}

bool DBObjectManager::existsDBObject (DB_OBJECT_TYPE type)
{
    registerParentVariablesIfRequired();

    return (objects_.find(type) != objects_.end());
}

DBObject *DBObjectManager::getDBObject (DB_OBJECT_TYPE type)
{
    logdbg  << "DBObjectManager: getDBObject: type " << type;

    registerParentVariablesIfRequired();

    assert (objects_.find(type) != objects_.end());
    assert (objects_.at(type));

    return objects_.at(type);
}

/**
 * Checks if variable exists, returns it if found.
 *
 * \exception std::runtime_error if variable not found.
 */
DBOVariable *DBObjectManager::getDBOVariable (DB_OBJECT_TYPE type, std::string id)
{
    logdbg  << "DBObjectManager: getDBOVariable: type " << type << " id " << id;

    registerParentVariablesIfRequired();

    assert (existsDBObject(type));
    assert (id.size() > 0);

    if (!existsDBOVariable (type, id))
    {
        logerr  << "DBObjectManager: getDBOVariable: variable unknown type " << type << " id " << id;
        throw std::runtime_error("DBObjectManager: getDBOVariable: variable unknown");
    }

    return getDBObject (type)->getVariable(id);
}

std::map <std::string, DBOVariable*> &DBObjectManager::getDBOVariables (DB_OBJECT_TYPE type)
{
    registerParentVariablesIfRequired();

    assert (existsDBObject(type));
    return getDBObject (type)->getVariables();
}

bool DBObjectManager::existsDBOVariable (DB_OBJECT_TYPE type, std::string id)
{
    registerParentVariablesIfRequired();

    if (!existsDBObject(type))
        return false;
    return getDBObject (type)->hasVariable(id);
}

void DBObjectManager::registerParentVariablesIfRequired ()
{
    if (!registered_parent_variables_)
    {
        //loginf << "DBObjectManager: registerParentVariablesIfRequired: registering";
        registered_parent_variables_=true;
        if (objects_.find(DBO_UNDEFINED) != objects_.end())
            objects_[DBO_UNDEFINED]->registerParentVariables();
    }
}
