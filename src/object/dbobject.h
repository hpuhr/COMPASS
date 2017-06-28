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
 * DBObject.h
 *
 *  Created on: Apr 6, 2012
 *      Author: sk
 */

#ifndef DBOBJECT_H_
#define DBOBJECT_H_

#include <string>
#include <qobject.h>
#include <memory>
#include "boost/date_time/posix_time/posix_time.hpp"


#include "global.h"
#include "dbovariableset.h"
#include "configurable.h"


class DBOVariable;
class PropertyList;
class MetaDBTable;
class ActiveSourcesObserver;

/**
 * @brief Definition of a meta table in a schema in a DBObject
 *
 * Simple storage class for a schema and a meta table, as strings. Used in a DBObject to save the definitions into the configuration,
 * and generate the pointers to the defined structures at from them.
 */
class DBOSchemaMetaTableDefinition : public Configurable
{
public:
    /// @brief Constructor, registers parameters
    DBOSchemaMetaTableDefinition(const std::string &class_id, const std::string &instance_id, Configurable *parent)
        : Configurable (class_id, instance_id, parent)
    {
        registerParameter ("schema", &schema_, "");
        registerParameter ("meta_table", &meta_table_, "");
    }
    /// @brief Destructor
    virtual ~DBOSchemaMetaTableDefinition() {}

    const std::string &schema () { return schema_; }
    const std::string &metaTable () { return meta_table_; }

protected:
    /// DBSchema identifier
    std::string schema_;
    /// MetaDBTable identifier
    std::string meta_table_;
};

/**
 * @brief Definition of a data source for a DBObject
 */
class DBODataSourceDefinition : public Configurable
{
public:
    /// @brief Constructor, registers parameters
    DBODataSourceDefinition(const std::string &class_id, const std::string &instance_id, Configurable *parent)
        : Configurable (class_id, instance_id, parent)
    {
        registerParameter ("schema", &schema_, "");
        registerParameter ("local_key", &local_key_, "");
        registerParameter ("meta_table", &meta_table_,  "");
        registerParameter ("foreign_key", &foreign_key_, "");
        registerParameter ("name_column", &name_column_, "");
    }
    virtual ~DBODataSourceDefinition() {}

    std::string schema () { return schema_; }
    std::string localKey () { return local_key_; }
    std::string metaTableName () { return meta_table_; }
    std::string foreignKey () { return foreign_key_; }
    std::string nameColumn () { return name_column_; }

protected:
    /// DBSchema identifier
    std::string schema_;
    /// Identifier for key in main table
    std::string local_key_;
    /// Identifier for meta table with data sources
    std::string meta_table_;
    /// Identifier for key in meta table with data sources
    std::string foreign_key_;
    /// Identifier for sensor name column in meta table with data sources
    std::string name_column_;
};

class DBObjectWidget;
class DBObjectInfoWidget;
class Buffer;
class Job;
class DBJob;
class DBOReadDBJob;

/**
 * @brief Abstract data description of an object stored in a database
 *
 * A database object serves as definition of a data container stored in a database. It is identified by name and a type,
 * and mainly consists of one or a number of meta tables. One meta table is considered the main meta table, all others
 * are sub meta tables. Columns from all such meta tables are collected and abstracted as DBOVariables of an DBObject.
 *
 * The meta table information is depended on the DBSchema, since for different schemas different table structures might exist.
 * With such a construct it is possible to abstract from different database schemas, by creating one set of DBObjects, which
 * are based on different meta tables (depended on the used schema). All DBOVariable instances have a different table variable,
 * also based on the schema.
 *
 * From an outside view, a DBObject is a collection of DBOVariables. However, a specialization exists, which is called the
 * meta DBObject (of type DBO_UNKNOWN), which serves only as an collection of meta DBOVariables and does not have a meta table.
 *
 * The distinction between meta DBOjects and normal ones can be made using the is_meta_ flag or is_loadable_ flag.
 *
 * For loadable DBObjects, a read list is held. This list contains the information, which variables are to be loaded on
 * read statements from the database. This read list is only held, and other classes can add variables to the retrieved
 * reference of the list.
 *
 * Also holds functionality about its data sources. If such information is present (marked in the database schema),
 * such information can be generated and is executed asynchronously. Interested instances have to be registered as observer
 * and receive a callback once the information is present.
 *
 * \todo Check if DBOVariables can exist only in some schemas, finish checkVariables
 */
class DBObject : public QObject, public Configurable
{
    Q_OBJECT
public slots:
    void schemaChangedSlot ();

    void readJobIntermediateSlot (std::shared_ptr<Buffer> buffer);
    void readJobObsoleteSlot ();
    void readJobDoneSlot();

    void databaseOpenedSlot ();

public:
    /// @brief Constructor
    DBObject(std::string class_id, std::string instance_id, Configurable *parent);
    /// @brief Desctructor
    virtual ~DBObject();

    /// @brief Returns flag indication if a DBOVariable identified by id exists
    bool hasVariable (const std::string &id) const;
    /// @brief Returns variable identified by id
    DBOVariable &variable (std::string variable_id);
    /// @brief Deletes a variable identified by id
    void deleteVariable (std::string id);

    /// @brief Returns container with all variables
    const std::map<std::string, DBOVariable*> &variables () const;
    /// @brief Returns number of existing variables
    unsigned int numVariables () const { return variables_.size(); }

    /// @brief Returns container with all variables in a DBTable identified by table
    //std::vector <DBOVariable*> getVariablesForTable (std::string table);
    /// @brief Returns name of the object
    const std::string &name () const { return name_; }
    /// @brief Sets name of the object
    void name (const std::string &name) { assert (name.size()>0); name_=name; }

    /// @brief Returns description of the object
    const std::string &info () const { return info_; }
    /// @brief Sets description of the object
    void info(std::string info) { info_=info; }

    /// @brief Returns if an object can be loaded
    bool loadable () const { return is_loadable_; }
    void load ();

    /// @brief Returns if incremental read for DBO type was prepared
    bool isLoading ();
    /// @brief Returns if incremental read for DBO type was finished
    bool wasLoadingPerformed ();
    /// @brief Returns if DBO exists and has data in the database
    bool hasData ();
    /// @brief Returns number of elements for DBO type
    size_t count ();
    size_t loadedCount ();

    /// @brief Returns container with all meta tables
    const std::map <std::string, std::string> &metaTables () const { return meta_tables_; }
    /// @brief Returns identifier of main meta table under DBSchema defined by schema
    const std::string &metaTable (const std::string &schema) const;

    /// @brief Returns main meta table for current schema
    const MetaDBTable &currentMetaTable () const;
    /// @brief Returns if current schema has main meta table
    bool hasCurrentMetaTable () const;

    /// @brief Returns if object is meta
    //bool isMeta () { return is_meta_; }

    /// @brief Returns if a data source is defined in the current schema
    bool hasCurrentDataSource () const;
    /// @brief Returns current data source definition
    const DBODataSourceDefinition &currentDataSource () const;
    /// @brief Returns container with all data source definitions
    const std::map <std::string, DBODataSourceDefinition *> &dataSourceDefinitions () const { return data_source_definitions_; }

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    //  /// @brief Registers an observer to the active data sources information
    //  void addActiveSourcesObserver (ActiveSourcesObserver *observer);
    //  /// @brief Removes an observer to the active data sources information
    //  void removeActiveSourcesObserver (ActiveSourcesObserver *observer);

    //  /// @brief Return if active data sources info is available
    //  bool hasActiveDataSourcesInfo ();
    //  /// @brief Triggers build process of the active data sources ino
    //  void buildActiveDataSourcesInfo ();
    //  /// @brief Sets the container with the active data sources information
    //  void setActiveDataSources (std::set<int> active_data_sources);

    /// @brief Returns container with the active data sources information
    const std::set<int> getActiveDataSources () const { return active_data_sources_; }

    /// @brief In meta object, registers meta variables as parent variables
    //void registerParentVariables ();
    /// @brief In meta object, unregisters meta variables as parent variables
    //void unregisterParentVariables ();

    std::string status ();

    DBObjectWidget *widget ();
    DBObjectInfoWidget *infoWidget ();

protected:
    /// DBO name
    std::string name_;
    /// DBO description
    std::string info_;
    /// DBO is loadable flag
    bool is_loadable_; // loadable on its own
    size_t count_;
    /// DBO is meta flag
    //bool is_meta_;
    std::shared_ptr <DBOReadDBJob> read_job_;
    std::vector <std::shared_ptr<Buffer>> read_job_data_;

    std::shared_ptr<Buffer> data_;

    /// Container with all DBOSchemaMetaTableDefinitions
    std::vector <DBOSchemaMetaTableDefinition*> meta_table_definitions_;
    /// Container with the main meta tables for schemas (schema identifier -> meta_table identifier)
    std::map <std::string, std::string> meta_tables_;

    /// Container with data source definitions (schema identifier -> data source definition pointer)
    std::map <std::string, DBODataSourceDefinition*> data_source_definitions_;
    /// Container with all variables (variable identifier -> variable pointer)
    std::map<std::string, DBOVariable*> variables_;

    /// Container with all observers of the active data sources information
    std::vector <ActiveSourcesObserver *> active_sources_observers_;
    /// Container with all active data sources
    std::set<int> active_data_sources_;

    /// Current (in the current schema) main meta table
    const MetaDBTable *current_meta_table_; // TODO rework

    /// Flag indicating if varaibles where checked. Not really used yet.
    bool variables_checked_;
    /// Flag indicating if the active data sources information is present
    bool has_active_data_sources_info_;

    DBObjectWidget *widget_;
    DBObjectInfoWidget *info_widget_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    virtual void checkSubConfigurables ();
    /// @brief Checks if variables really exist. Not used yet.
    void checkVariables ();

    /// Notifies all observers of the active data sources information
    void notifyActiveDataSourcesObservers ();
};

#endif /* DBOBJECT_H_ */
