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

MetaDBTable::MetaDBTable(const std::string &class_id, const std::string &instance_id, DBSchema *parent)
    : Configurable (class_id, instance_id, parent), schema_(*parent), table_ (nullptr)
{
    registerParameter ("name", &name_, (std::string)"");
    registerParameter ("info", &info_, (std::string)"");
    registerParameter ("table", &table_name_, (std::string)"");

    logdbg  << "MetaDBTable: constructor: name " << name_ << " table " << table_name_;

    assert (schema_.hasTable (table_name_));
    table_ = &schema_.table (table_name_);
    assert (table_);

    for (auto it : table_->columns ())
    {
        assert (columns_.find(it.first) == columns_.end());
        columns_.insert (std::pair <std::string, DBTableColumn> (it.first, it.second));
    }

    createSubConfigurables ();

    assert (false);
    //TODO check init
}

MetaDBTable::~MetaDBTable()
{
    //  std::map <std::pair<std::string, std::string>, MetaDBTable*>::iterator it;
    //
    //  for (it=sub_tables_.begin(); it != sub_tables_.end(); it++)
    //    delete it->second;

    //  std::vector <SubTableDefinition *>::iterator it;
    //  for (it=sub_table_definitions_.begin(); it != sub_table_definitions_.end(); it++)
    //    delete (*it);

    sub_tables_.clear(); // are only pointers to tables from schema
    sub_table_definitions_.clear();
}

void MetaDBTable::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "MetaDBTable: generateSubConfigurable: " << class_id_ << " instance " << instance_id_;

    if (class_id == "SubTableDefinition") // subtable
    {
        logdbg  << "MetaDBTable: generateSubConfigurable: generating sub table definition";

        SubTableDefinition *def = new SubTableDefinition (class_id, instance_id, this);
        sub_table_definitions_.push_back (*def);
    }
    else
        throw std::runtime_error ("MetaDBTable: generateSubConfigurable: unknown class_id "+class_id);
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

std::vector<std::string> MetaDBTable::allTableNamesVector () const
{
    std::vector <std::string> table_names;

    table_names.push_back (tableName());

    for (auto it : sub_tables_)
        table_names.push_back(it.second.name());

    return table_names;
}

//std::string MetaDBTable::subTablesWhereClause(std::vector <std::string> &used_tables) const
//{
//    std::stringstream ss;

//    bool first=true;

//    for (auto it : sub_tables_)
//    {
//        if (find (used_tables.begin(), used_tables.end(), it->second->getTableDBName()) == used_tables.end())
//            continue;

//        if (!first)
//            ss << " AND ";

//        ss << getTableDBName() << "." << it.first.localKey() << "=" << it->second->getTableDBName() << "." << it->first->getSubTableKey();
//        first=false;
//    }

//    return ss.str();
//}

//std::string MetaDBTable::sSubTableKeyClause (std::string sub_table_name)
//{
//    setSubTablesIfRequired();

//    std::stringstream ss;

//    std::map <SubTableDefinition*, MetaDBTable*>::iterator it;

//    for (it=sub_tables_.begin(); it != sub_tables_.end(); it++)
//    {
//        if (it->second->getTableDBName().compare (sub_table_name) != 0)
//            continue;

//        // found subtable

//        ss << getTableDBName() << "." << it->first->getLocalKey() << "=" << it->second->getTableDBName() << "." << it->first->getSubTableKey();
//        return ss.str();
//    }

//    throw std::runtime_error ("MetaDBTable: getSubTableKeyClause: sub_table_name "+sub_table_name+" not found");
//}

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
