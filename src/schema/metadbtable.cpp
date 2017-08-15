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
 * MetaDBTable.cpp
 *
 *  Created on: Jul 28, 2012
 *      Author: sk
 */

#include <algorithm>

#include "dbtablecolumn.h"
#include "dbtable.h"
#include "metadbtable.h"
#include "dbschema.h"
#include "atsdb.h"
#include "logger.h"
#include "metadbtablewidget.h"

MetaDBTable::MetaDBTable(const std::string &class_id, const std::string &instance_id, DBSchema *parent)
    : Configurable (class_id, instance_id, parent), schema_(*parent), main_table_ (nullptr), widget_(nullptr)
{
    registerParameter ("name", &name_, "");
    registerParameter ("info", &info_, "");
    registerParameter ("main_table_name", &main_table_name_, "");

    logdbg  << "MetaDBTable: constructor: name " << name_ << " main table '" << main_table_name_ << "'";

    assert (schema_.hasTable (main_table_name_));
    main_table_ = &schema_.table (main_table_name_);
    assert (main_table_);

    createSubConfigurables ();

    updateColumns();
}

MetaDBTable::~MetaDBTable()
{
    for (auto it : sub_table_definitions_)
        delete it.second;

    sub_table_definitions_.clear();

    sub_tables_.clear(); // are only pointers to tables from schema
    columns_.clear();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

void MetaDBTable::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "MetaDBTable: generateSubConfigurable: " << class_id_ << " instance " << instance_id_;

    if (class_id == "SubTableDefinition") // subtable
    {
        logdbg  << "MetaDBTable: generateSubConfigurable: generating sub table definition";

        SubTableDefinition *def = new SubTableDefinition (class_id, instance_id, this);
        sub_table_definitions_.insert (std::pair <std::string, SubTableDefinition*> (def->subTableName(), def));

        assert (schema_.hasTable(def->subTableName()));
        assert (sub_tables_.count(def->subTableName()) == 0);
        sub_tables_.insert( std::pair<std::string, const DBTable&> (def->subTableName(), schema_.table(def->subTableName())) );
    }
    else
        throw std::runtime_error ("MetaDBTable: generateSubConfigurable: unknown class_id "+class_id);
}

void MetaDBTable::name (const std::string &name)
{
    name_=name;
    schema_.updateMetaTables();
}

//void MetaDBTable::mainTableName (const std::string &main_table_name)
//{
//    assert (main_table_name.size() != 0);
//    main_table_name_=main_table_name;
//}

const DBTable &MetaDBTable::tableFor (const std::string &column) const
{
    assert (hasColumn(column));
    return  columns_.at(column).table();
}

std::string MetaDBTable::subTableNames () const
{
    if (sub_tables_.size() == 0)
        return "";
    else
    {
        std::stringstream ss;

        for (auto it : sub_tables_)
        {
            if (ss.str().size() != 0)
                ss << ", ";
            ss << it.second.name();
        }
        return ss.str();
    }
}

void MetaDBTable::addSubTable (const std::string &local_key, const std::string &sub_table_name, const std::string &sub_table_key)
{
    loginf << "MetaDBTable: addSubTable: local key " << local_key << " sub table name " << sub_table_name << " key " << sub_table_key;
    assert (!hasSubTable(sub_table_name));

    std::string instance_id = "SubTableDefinition"+name_+sub_table_name+"0";

    Configuration &configuration = addNewSubConfiguration ("SubTableDefinition", instance_id);
    configuration.addParameterString ("main_table_key", local_key);
    configuration.addParameterString ("sub_table_name", sub_table_name);
    configuration.addParameterString ("sub_table_key", sub_table_key);
    generateSubConfigurable ("SubTableDefinition", instance_id);

    assert (hasSubTable(sub_table_name));
}

void MetaDBTable::removeSubTable (const std::string& name)
{
    assert (hasSubTable(name));
    delete sub_table_definitions_.at(name);
    sub_table_definitions_.erase (name);

    sub_tables_.erase (name);

    updateColumns();
}

MetaDBTableWidget *MetaDBTable::widget ()
{
    if (!widget_)
    {
        widget_ = new MetaDBTableWidget (*this);
    }
    return widget_;
}

void MetaDBTable::updateColumns ()
{
    columns_.clear();

    for (auto it : main_table_->columns ())
    {
        assert (columns_.find(it.first) == columns_.end());
        columns_.insert (std::pair <std::string, const DBTableColumn&> (it.second->identifier(), *it.second));
    }

    for (auto it: sub_tables_)
    {
        for (auto it2 : it.second.columns())
        {
            //if (columns_.find(it.first) == columns_.end())
            assert (columns_.find(it2.second->identifier()) == columns_.end());
            columns_.insert (std::pair <std::string, const DBTableColumn&> (it2.second->identifier(), *it2.second));
        }
    }
}

//void MetaDBTable::setSubTablesIfRequired ()
//{
//    if (sub_table_definitions_.size() == 0)
//        return;

//    if (sub_table_definitions_.size() == sub_tables_.size())
//        return;

//    logdbg  << "MetaDBTable: setSubTablesIfRequired: for name " << name_ << " table "  << table_name_;

//    std::vector <SubTableDefinition *>::iterator it;

//    DBSchema &current_schema = ATSDB::getInstance().getCurrentSchema ();

//    for (it = sub_table_definitions_.begin(); it != sub_table_definitions_.end(); it++)
//    {
//        SubTableDefinition *def = *it;
//        logdbg  << "MetaDBTable: setSubTablesIfRequired: adding sub table " << def->getSubTableName();
//        assert (columns_.find (def->getLocalKey()) != columns_.end());
//        assert (current_schema.hasMetaTable(def->getSubTableName()));

//        MetaDBTable *sub_table = current_schema.getMetaTable(def->getSubTableName());
//        if (sub_tables_.find(def) != sub_tables_.end())
//        {
//            // already exists
//            continue;
//        }
//        sub_tables_[def] = sub_table;

//        std::map <std::string, DBTableColumn *> columns =  sub_table->getColumns ();
//        std::map <std::string, DBTableColumn *>::iterator it;

//        for (it = columns.begin(); it != columns.end(); it++)
//        {
//            if (columns_.find(it->first) == columns_.end())
//                columns_[it->first] = it->second;
//            else
//            {
//                std::string othername = sub_table->getName()+"."+it->first;
//                if (columns_.find(othername) != columns_.end())
//                    logwrn  << "MetaDBTable: setSubTablesIfRequired: " << class_id_ << " instance " << instance_id_ << " already got column name " << othername;
//                else
//                {
//                    columns_[othername] = it->second;
//                }
//            }
//        }
//    }
//    assert ((sub_table_definitions_.size() == sub_tables_.size()));
//}
