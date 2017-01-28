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
 * DBSchema.cpp
 *
 *  Created on: Aug 13, 2012
 *      Author: sk
 */

#include "dbtablecolumn.h"
#include "dbtable.h"
#include "metadbtable.h"
#include "dbschema.h"
#include "dbschemawidget.h"
#include "logger.h"

DBSchema::DBSchema(const std::string &class_id, const std::string &instance_id, Configurable *parent)
    : Configurable (class_id, instance_id, parent), widget_(nullptr)
{
    registerParameter ("name", &name_, (std::string) "");
    assert (name_.size() != 0);

    createSubConfigurables();
}

DBSchema::~DBSchema()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    for (auto it : tables_)
        delete it.second;
    tables_.clear();

    for (auto it : meta_tables_)
        delete it.second;
    meta_tables_.clear();
}

void DBSchema::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "DBSchema: generateSubConfigurable: " << class_id_ << " instance " << instance_id_;

    if (class_id.compare("DBTable") == 0)
    {
        logdbg  << "DBSchema '" << name_ << "': generateSubConfigurable: generating DBTable " << instance_id;
        DBTable *table = new DBTable ("DBTable", instance_id, this);
        assert (table->name().size() != 0);
        assert (tables_.find(table->name()) == tables_.end());
        tables_.insert (std::pair <std::string, DBTable*> (table->name(), table));
        logdbg  << "DBSchema '" << name_ << "': generateSubConfigurable: generated DBTable " << table->name();
    }
    else if (class_id.compare("MetaDBTable") == 0)
    {
        logdbg  << "DBSchema '" << name_ << "': generateSubConfigurable: generating MetaDBTable " << instance_id;
        MetaDBTable *meta_table = new MetaDBTable ("MetaDBTable", instance_id, this);
        assert (meta_table->name().size() != 0);
        assert (meta_tables_.find(meta_table->name()) == meta_tables_.end());
        meta_tables_.insert(std::pair<std::string, MetaDBTable*>(meta_table->name(), meta_table));
        logdbg  << "DBSchema '" << name_ << "': generateSubConfigurable: generated MetaDBTable " << meta_table->name();
    }
    else
        throw std::runtime_error ("DBSchema: generateSubConfigurable: unknown class_id "+class_id);
}

void DBSchema::addTable(const std::string &name)
{
    assert (!hasTable(name));
    assert (children_.count("DBTable"+name) == 0);

    Configuration &table_config = addNewSubConfiguration ("DBTable", name);
    table_config.addParameterString ("name", name);

    generateSubConfigurable("DBTable", name);
    assert (hasTable(name));
}

void DBSchema::deleteTable (const std::string &name)
{
    assert (hasTable(name));
    delete tables_.at(name);
    tables_.erase (name);

    // update meta tables
}

void DBSchema::populateTable (const std::string &name)
{
    assert (hasTable(name));
    tables_.at(name)->populate();

    // update meta tables
}

bool DBSchema::hasMetaTable (const std::string &name) const
{
    return meta_tables_.find(name) != meta_tables_.end();
}

void DBSchema::updateTables ()
{
    logdbg  << "DBSchema: updateTables";
    std::map <std::string, DBTable*> old_tables = tables_;
    tables_.clear();

    for (auto it : old_tables)
    {
        tables_.insert (std::pair <std::string, DBTable*> (it.second->name(), it.second));
    }
}

void DBSchema::updateMetaTables ()
{
    logdbg  << "DBSchema: updateMetaTables";
    std::map <std::string, MetaDBTable*> old_meta_tables = meta_tables_;
    meta_tables_.clear();

    for (auto it : old_meta_tables)
    {
        meta_tables_.insert (std::pair <std::string, MetaDBTable*> (it.second->name(), it.second));
    }
}

DBSchemaWidget *DBSchema::widget ()
{
    if (!widget_)
    {
        widget_ = new DBSchemaWidget (*this);
    }

    return widget_;
}

