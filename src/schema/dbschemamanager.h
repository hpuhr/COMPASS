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

#ifndef DBSCHEMAMANAGER_H_
#define DBSCHEMAMANAGER_H_

#include "configurable.h"
#include <qobject.h>

class ATSDB;
class DBSchema;
class DBSchemaManagerWidget;

/**
 * @brief Singleton for managing DBSchema instances
 *
 * Holds the current schema and a collection of all defined schemas.
 *
 * \todo Check why addRDLSchema is the same addEmptySchema.
 */
class DBSchemaManager : public QObject, public Configurable
{
    Q_OBJECT
signals:
    void schemaChangedSignal ();
    void schemaLockedSignal ();

public:
    /// @brief Constructor
    DBSchemaManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb);

    /// @brief Destructor
    virtual ~DBSchemaManager();

    /// @brief Returns flag indicating if current schema is defined and exists
    bool hasCurrentSchema ();
    /// @brief Returns name of the current schema
    const std::string &getCurrentSchemaName ();
    /// @brief Returns the current DBSchema
    DBSchema &getCurrentSchema ();
    /// @brief Returns the DBSchema with a given name
    DBSchema &getSchema (const std::string &name);
    /// @brief Returns if DBSchema with a given name exists
    bool hasSchema (const std::string &name);
    void deleteCurrentSchema ();

    /// @brief Returns container with all schemas
    std::map <std::string, DBSchema*>& getSchemas () { return schemas_; }

    /// @brief Renames the current schema
    void renameCurrentSchema (const std::string &new_name);
    /// @brief Sets the current schema

    void setCurrentSchema (const std::string &current_schema);
    bool isCurrentSchemaLocked () { return schema_locked_; }
    void lockCurrentSchema ();
    /// @brief Adds an empty schema with a given name
    void addEmptySchema (const std::string &name);
    /// @brief Adds an RDL schema with a given name
    //void addRDLSchema (std::string name);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    DBSchemaManagerWidget *widget();

protected:
    /// Name of current DBSchema
    std::string current_schema_;
    bool schema_locked_ {false};

    /// Container with all defined schemas (schema name -> DBSchema)
    std::map <std::string, DBSchema*> schemas_;

    DBSchemaManagerWidget *widget_;

    //void loadDBSchema (); // outdated method
    virtual void checkSubConfigurables ();


};

#endif /* DBSCHEMAMANAGER_H_ */
