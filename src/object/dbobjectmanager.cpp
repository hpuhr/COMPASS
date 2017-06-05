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

#include "configurationmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "dbobjectmanager.h"
#include "logger.h"
//#include "structureDescriptionManager.h"

/**
 * Creates sub-configurables.
 */
DBObjectManager::DBObjectManager(const std::string &class_id, const std::string &instance_id, Configurable *parent)
: Configurable (class_id, instance_id, parent, "conf/config_dbo.xml") //, registered_parent_variables_ (false)
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

//    std::map <std::string, DBObject*>::iterator it;

//    for (it = objects_.begin(); it != objects_.end(); it++)
//    {
//        delete it->second;
//        it->second=0;
//    }
    objects_.clear();
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
        objects_.insert(std::pair <std::string, DBObject> (object->name(), *object));
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

DBObject &DBObjectManager::get (const std::string &dbo_name)
{
    logdbg  << "DBObjectManager: get: name " << dbo_name;

    //registerParentVariablesIfRequired();

    assert (objects_.find(dbo_name) != objects_.end());

    return objects_.at(dbo_name);
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
