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
 * MetaDBTable.h
 *
 *  Created on: Jul 28, 2012
 *      Author: sk
 */

#ifndef METADBTABLE_H_
#define METADBTABLE_H_

#include <string>
#include <vector>

#include "configurable.h"
#include "dbtablecolumn.h"
//#include "DBTable.h"
//#include "DBSchema.h"

//class DBTableColumn;
class DBTable;
class DBSchema;

/**
 * @brief Sub-table definition for a DBMetaTable
 *
 * Defines a contained meta sub-table. To a MetaDBTable additional meta sub-tables
 * can be added which link main table columns (local key) to a foreign key (sub-table key).
 *
 * \todo Remove SQL statement generation, maybe move to SQLGenerator
 */
class SubTableDefinition : public Configurable
{
public:
    /// @brief Constructor
    SubTableDefinition(const std::string &class_id, const std::string &instance_id, Configurable *parent)
        : Configurable (class_id, instance_id, parent)
    {
        registerParameter ("local_key", &local_key_, (std::string)"");
        registerParameter ("sub_table_name", &sub_table_name_,  (std::string)"");
        registerParameter ("sub_table_key", &sub_table_key_,  (std::string)"");
    }
    /// @brief Destructor
    virtual ~SubTableDefinition() {}

    /// @brief Returns local key
    const std::string &getLocalKey () { return local_key_; }
    /// @brief Returns meta sub-table name
    const std::string &getSubTableName () { return sub_table_name_; }
    /// @brief Returns meta sub-table key
    const std::string &getSubTableKey () { return sub_table_key_; }

protected:
    /// Local key
    std::string local_key_;
    /// Meta sub-table name
    std::string sub_table_name_;
    /// Meta sub-table key
    std::string sub_table_key_;

};

/**
 * @brief Aggregation and abstraction of DBTables
 */
class MetaDBTable : public Configurable
{
public:
    /// @brief Constructor
    MetaDBTable(const std::string &class_id, const std::string &instance_id, DBSchema *parent);
    /// @brief Destructor
    virtual ~MetaDBTable();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    /// @brief Returns name
    const std::string &name () const { return name_; }
    /// @brief Sets name
    void name (const std::string &name) { assert(name.size() != 0 ); name_=name; }

    /// @brief Returns description
    const std::string &info () const { return info_; }
    /// @brief Sets description
    void info (const std::string &info) { info_=info; }

    /// @brief Returns name of the main database table
    const std::string &tableName () const { return table_name_; }
    /// @brief Sets name of the main database table
    void tableName (const std::string &table_name) { assert (table_name.size() != 0); table_name_=table_name; }

    /// @brief Returns database table name for a variable
    const std::string &tableDBNameForVariable (std::string variable_name) const { return columns_.at(variable_name).dbTableName(); }

    /// @brief Returns number of columns
    unsigned int numColumns () const { return columns_.size(); }
    /// @brief Returns if column with a given name exists
    bool hasColumn (const std::string &column) const { return columns_.find (column) != columns_.end(); }
    /// @brief Returns container with all columns
    const std::map <std::string, const DBTableColumn&>& columns () const { return columns_; }
    /// @brief Returns column with a given name
    const DBTableColumn &column (const std::string &column) const { return columns_.at(column); }

    /// @brief Returns if meta sub-tables are defined
    bool hasSubTables () const { return sub_table_definitions_.size() > 0; }
    /// @brief Returns container with all meta sub-tables
    const std::map <SubTableDefinition, const DBTable &> &subTables () const { return sub_tables_; }
    /// @brief Returns string with concatenated, comma separated meta sub-tables
    std::string subTableNames () const;
    /// @brief Returns string with all table names (own and sub metas) as comma separated list
    std::string allTableNames () const { return tableName()+ ", " + subTableNames (); }
    /// @brief Returns container with all table names
    std::vector<std::string> allTableNamesVector () const;

    /// @brief Returns SQL where clause with all used meta sub-tables
    //std::string subTablesWhereClause (std::vector <std::string> &used_tables) const;
    /// @brief Returns SQL key clause for a give meta sub-table
    //std::string subTableKeyClause (const std::string &sub_table_name) const;

    /// @brief Returns container with all sub meta-table definitions
    const std::vector <SubTableDefinition>& getSubTableDefinitions () const { return sub_table_definitions_; }

    /// @brief Returns main database table
    const DBTable &getTable() const { return *table_; }

private:
    /// Name
    std::string name_;
    /// Description
    std::string info_;
    /// Main table name
    std::string table_name_;

    /// Parent schema
    const DBSchema &schema_;
    /// Main database table
    const DBTable *table_;

    /// Container with all meta sub-table definitions
    std::vector <SubTableDefinition> sub_table_definitions_;
    /// Container with all meta sub-tables
    std::map <SubTableDefinition, const DBTable&> sub_tables_;
    /// Container with all table columns
    std::map <std::string, const DBTableColumn&> columns_;

    /// @brief Creates sub meta-tables from their definitions (if required)
    //void setSubTablesIfRequired ();

protected:
    virtual void checkSubConfigurables () {}
};

#endif /* METADBTABLE_H_ */
