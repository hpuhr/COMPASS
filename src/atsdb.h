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

#ifndef ATSDB_H_
#define ATSDB_H_

#include "propertylist.h"
#include "singleton.h"
#include "configurable.h"

#include <set>
#include <map>
#include <vector>

class DBInterface;
class DBObjectManager;
class DBSchemaManager;
class FilterManager;
class TaskManager;
class ViewManager;
class SimpleConfig;

/**
 * @mainpage  ATSDB Main Page
 *
 * This library encapsulates a database system and allows reading and writing of flight surveillance data.
 *
 * The main access point is the class ATSDB. Using this singleton, surveillance data can be written using the RDL
 * tool and SQLite3 file containers or a MySQL database.
 *
 * Most of the classes were written to be persistent, meaning that local parameters can saved/restored using a sophisticated
 * Configuration framework. Also, there exists a mechanism for dynamic creation of instances based on such a Configuration,
 * which is embedded in the class Configurable. The main access point for this framework is the class ConfigurationManager.
 *
 * Based on the this configuration, an abstract representation of objects stored in the database was created, which should
 * largely be independent of the database schema. Such an object is called DBObject, all of which are managed by the DBObjectManager.
 *
 * Also based on the configuration is the database filtering system. The FilterManager is used to manage filters using the DBFilter
 * class, which itself holds filter conditions based on the DBFilterCondition class. For each DBObject, the current conditions in the
 * SQL query can be retrieved from the active filters.
 *
 * The most common data storage container is a Buffer, a dynamic and fast mechanism for data storage and retrieval. It is based on
 * memory pages based on the ArrayTemplate in interplay with the ArrayTemplateManager, and the MemoryManager which is the main
 * access point for memory allocation and management.
 * <p/> <br/>
 */

/**
 * @brief Main access point for all library function.
 *
 * @details Singleton, uses separate thread. Is started using the init function (with a connection type specifying the database
 * system and parameters). Has functionality which generates and allows access to sensor information of DBObjects. Allows reading/writing
 * data from/to the database and various access function. Can be stopped using the shutdown function.
 *
 * Note the following example code:
 * @code
 *  DBConnectionInfo *info = ...;
 *  ATSDB::getInstance().init (info);
 *  // do stuff
 *  ATSDB::getInstance().shutdown();
 * @endcode
 *
 * \todo Change sorting if export is active
 * \todo Maybe extend some classes to observer pattern
 * \todo Removed writing of new databases. Re-integrated if necessary
 */
class ATSDB : public Configurable, public Singleton
{
public:
    ///@brief Destructor.
    virtual ~ATSDB();

    //void initialize ();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    DBInterface& interface ();
    DBSchemaManager& schemaManager ();
    DBObjectManager& objectManager ();
    FilterManager& filterManager ();
    TaskManager& taskManager ();
    ViewManager& viewManager ();
    SimpleConfig& config ();

    bool ready ();

    ///@brief Shuts down the DB access.
    void shutdown ();

protected:
    //bool initialized_ {false};
    bool shut_down_ {false};

    std::unique_ptr<SimpleConfig> simple_config_;
    /// DB interface, encapsulating all database functionality.
    std::unique_ptr<DBInterface> db_interface_;
    std::unique_ptr<DBObjectManager> dbo_manager_;
    std::unique_ptr<DBSchemaManager> db_schema_manager_;
    std::unique_ptr<FilterManager> filter_manager_;
    std::unique_ptr<TaskManager> task_manager_;
    std::unique_ptr<ViewManager> view_manager_;

    virtual void checkSubConfigurables ();

    ///@brief Constructor.
    ATSDB();

public:
    ///@brief Instance access function for Singleton.
    static ATSDB& instance()
    {
        static ATSDB instance;
        return instance;
    }
};

#endif /* ATSDB_H */
