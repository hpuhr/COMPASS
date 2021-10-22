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

#include "dbschemamanager.h"

#include <QApplication>

#include "compass.h"
#include "buffer.h"
#include "configurationmanager.h"
#include "dbinterface.h"
#include "dbobjectmanager.h"
#include "dbschema.h"
#include "dbschemamanagerwidget.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
#include "logger.h"
#include "metadbtable.h"

/**
 * Registers current_schema as parameter, creates sub-configurables (schemas), checks if
 * current_schema exists (if defined).
 */
DBSchemaManager::DBSchemaManager(const std::string& class_id, const std::string& instance_id,
                                 COMPASS* compass, DBInterface& db_interface)
    : Configurable(class_id, instance_id, compass, "db_schema.json"), db_interface_(db_interface)
{
    createSubConfigurables();

    assert (schema_);
}

/**
 * Deletes all schemas
 */
DBSchemaManager::~DBSchemaManager()
{
    schema_ = nullptr;

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

/**
 * Can create DBSchemas.
 *
 * \exception std::runtime_error if unknown class_id
 */
void DBSchemaManager::generateSubConfigurable(const std::string& class_id,
                                              const std::string& instance_id)
{
    logdbg << "DBSchemaManager: generateSubConfigurable: " << classId() << " instance "
           << instanceId();

    if (class_id.compare("DBSchema") == 0)
    {
        logdbg << "DBSchemaManager: generateSubConfigurable: generating schema";
        assert (!schema_);

        schema_.reset(new DBSchema("DBSchema", instance_id, this, db_interface_));
        assert (schema_);
    }
    else
        throw std::runtime_error("DBSchema: generateSubConfigurable: unknown class_id " + class_id);
}

void DBSchemaManager::checkSubConfigurables()
{
    assert (schema_); // TODO create empty?
}

DBSchema& DBSchemaManager::getCurrentSchema()
{
    assert(schema_);
    return *schema_;
}

DBSchemaManagerWidget* DBSchemaManager::widget()
{
    if (!widget_)
    {
        widget_ = new DBSchemaManagerWidget(*this);

        if (locked_)
            widget_->lock();
    }

    assert(widget_);
    return widget_;
}

void DBSchemaManager::lock()
{
    if (locked_)
        return;

    locked_ = true;

    assert (schema_);
    schema_->lock();

    if (widget_)
        widget_->lock();

    emit schemaLockedSignal();
}

void DBSchemaManager::databaseContentChangedSlot()
{
    loginf << "DBSchemaManager: databaseContentChangedSlot";

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    assert (schema_);
    schema_->updateOnDatabase();

    QApplication::restoreOverrideCursor();
}
