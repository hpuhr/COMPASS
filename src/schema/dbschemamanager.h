/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBSCHEMAMANAGER_H_
#define DBSCHEMAMANAGER_H_

#include <qobject.h>

#include "configurable.h"

class COMPASS;
class DBSchema;
class DBSchemaManagerWidget;
class DBInterface;

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
    void schemaChangedSignal();
    void schemaLockedSignal();

  public slots:
    void databaseContentChangedSlot();

  public:
    /// @brief Constructor
    DBSchemaManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass,
                    DBInterface& db_interface);

    /// @brief Destructor
    virtual ~DBSchemaManager();

    /// @brief Returns the current DBSchema
    DBSchema& getCurrentSchema();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    DBSchemaManagerWidget* widget();

    void lock();
    bool isLocked() { return locked_; }

  protected:
    DBInterface& db_interface_;
    bool locked_{false};

    /// Container with all defined schemas (schema name -> DBSchema)
    std::unique_ptr<DBSchema> schema_;

    DBSchemaManagerWidget* widget_{nullptr};

    // void loadDBSchema (); // outdated method
    virtual void checkSubConfigurables();
};

#endif /* DBSCHEMAMANAGER_H_ */
