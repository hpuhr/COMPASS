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

#include "dbschema.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
#include "dbtableinfo.h"
#include "dbtablewidget.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "logger.h"

DBTable::DBTable(const std::string& class_id, const std::string& instance_id, DBSchema& schema)
    : Configurable (class_id, instance_id, &schema), schema_(schema)
{
    registerParameter ("name", &name_, "");
    registerParameter ("info", &info_, "");
    registerParameter ("key_name", &key_name_, "");

    createSubConfigurables();
}

DBTable::~DBTable()
{
    logdbg << "DBTable: ~DBTable: name " << name_;

    for (auto it : columns_)
        delete it.second;
    columns_.clear();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

void DBTable::generateSubConfigurable (const std::string& class_id, const std::string& instance_id)
{
    logdbg  << "DBTable: generateSubConfigurable: " << class_id_ << " instance " << instance_id_;

    if (class_id == "DBTableColumn")
    {
        DBTableColumn *column = new DBTableColumn ("DBTableColumn", instance_id, this);
        assert (column->name().size() != 0);
        assert (columns_.find(column->name()) == columns_.end());
        columns_.insert (std::pair <std::string, DBTableColumn*> (column->name(), column));

        if (column->isKey())
            key_name_ = column->name();
    }
    else
        throw std::runtime_error ("DBTable: generateSubConfigurable: unknown class_id "+class_id);
}
void DBTable::checkSubConfigurables ()
{
    // move along, sir.
}

bool DBTable::hasColumn (const std::string& name) const
{
    return columns_.find(name) != columns_.end();
}

const DBTableColumn& DBTable::column (const std::string& name) const
{
    assert (columns_.find(name) != columns_.end());
    return *columns_.at(name);
}

void DBTable::deleteColumn (const std::string &name)
{
    assert (hasColumn(name));
    columns_.erase(columns_.find(name));
}

void DBTable::populate ()
{
    loginf << "DBTable: populate: table " << name_;

    assert (ATSDB::instance().ready());
    for (auto it : ATSDB::instance().interface().tableInfo().at(name_).columns ())
    {
        if (columns_.count(it.first) == 0)
        {
            Configuration &config = addNewSubConfiguration ("DBTableColumn", it.first);
            config.addParameterString ("name", it.first);
            config.addParameterString ("type", it.second.type());
            config.addParameterBool ("is_key", it.second.key());
            config.addParameterString ("comment", it.second.comment());
            generateSubConfigurable("DBTableColumn", it.first);
        }
    }
}

void DBTable::lock ()
{
    locked_ = true;

    if (widget_)
        widget_->lock();
}

DBTableWidget* DBTable::widget ()
{
    if (!widget_)
    {
        widget_ = new DBTableWidget (*this);

        if (locked_)
            widget_->lock();
    }

    return widget_;
}
