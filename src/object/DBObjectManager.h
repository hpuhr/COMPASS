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
 * DBObjectManager.h
 *
 *  Created on: Apr 6, 2012
 *      Author: sk
 */

#ifndef DBOBJECTMANAGER_H_
#define DBOBJECTMANAGER_H_

#include <vector>
#include "Singleton.h"
#include "Configurable.h"
#include "Global.h"

class DBObject;
class DBOVariable;
class DBOVariableSet;

/**
 * @brief For management of all DBObjects
 *
 * Singleton which creates and holds all DBObjects defined in its configuration.
 */
class DBObjectManager : public Configurable
{
public:
    /// @brief Constructor
    DBObjectManager(const std::string &class_id, const std::string &instance_id, Configurable *parent);

    /// @brief Returns if an object of type exists
    bool existsDBObject (const std::string &dbo_type);
    /// @brief Returns the object of type, if existing
    DBObject &getDBObject (const std::string &dbo_type);

    /// @brief Returns defined DBOVariable, if existing
    //DBOVariable *getDBOVariable (const std::string &dbo_type, std::string id);
    /// @brief Returns if defined DBOVariable exists
    //bool existsDBOVariable (const std::string &dbo_type, std::string id);
    /// @brief Returns container with all DBOVariables
    //std::map <std::string, DBOVariable*> &getDBOVariables (const std::string &dbo_type);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    /// @brief Returns container with all DBObjects
    const std::map <std::string, DBObject>& getDBObjects () { return objects_; }
    /// @brief Returns of any DBObjects exist
    bool hasObjects () { return objects_.size() > 0; }

    /// @brief Destructor
    virtual ~DBObjectManager();

protected:
    /// Container with all DBOs (DBO type -> DBO pointer)
    std::map <std::string, DBObject> objects_;
    //bool registered_parent_variables_;

    virtual void checkSubConfigurables ();

    /// @brief Small hack for minimum/maximum update. Refer to DBObject::registerParentVariables() for details.
    //void registerParentVariablesIfRequired ();
};

#endif /* DBOBJECTMANAGER_H_ */
