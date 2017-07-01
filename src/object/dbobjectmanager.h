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
#include <qobject.h>

#include "singleton.h"
#include "configurable.h"
#include "global.h"

class ATSDB;
class DBObject;
class DBObjectManagerWidget;
class DBObjectManagerInfoWidget;
class DBOVariable;
class DBOVariableSet;
class DBSchemaManager;

/**
 * @brief For management of all DBObjects
 *
 * Singleton which creates and holds all DBObjects defined in its configuration.
 */
class DBObjectManager : public QObject, public Configurable
{
    Q_OBJECT
public slots:
    void loadSlot ();
    void updateSchemaInformationSlot ();
    void databaseOpenedSlot ();

signals:
    void dbObjectsChangedSignal ();
    void databaseOpenedSignal ();
    void schemaChangedSignal ();

    void loadingStartedSignal ();

public:
    /// @brief Constructor
    DBObjectManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb);

    /// @brief Returns if an object of type exists
    bool exists (const std::string &dbo_name);
    /// @brief Returns the object of type, if existing
    DBObject &object (const std::string &dbo_name);
    void remove (const std::string &dbo_name);

    /// @brief Returns defined DBOVariable, if existing
    //DBOVariable *getDBOVariable (const std::string &dbo_type, std::string id);
    /// @brief Returns if defined DBOVariable exists
    //bool existsDBOVariable (const std::string &dbo_type, std::string id);
    /// @brief Returns container with all DBOVariables
    //std::map <std::string, DBOVariable*> &getDBOVariables (const std::string &dbo_type);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    /// @brief Returns container with all DBObjects
    std::map <std::string, DBObject*>& objects () { return objects_; }
    /// @brief Returns of any DBObjects exist
    bool hasObjects () { return objects_.size() > 0; }

    /// @brief Destructor
    virtual ~DBObjectManager();

    DBObjectManagerWidget *widget();
    DBObjectManagerInfoWidget *infoWidget();

protected:
    /// Container with all DBOs (DBO name -> DBO pointer)
    std::map <std::string, DBObject*> objects_;
    //bool registered_parent_variables_;

    DBObjectManagerWidget *widget_;
    DBObjectManagerInfoWidget *info_widget_;

    virtual void checkSubConfigurables ();

    /// @brief Small hack for minimum/maximum update. Refer to DBObject::registerParentVariables() for details.
    //void registerParentVariablesIfRequired ();
};

#endif /* DBOBJECTMANAGER_H_ */
