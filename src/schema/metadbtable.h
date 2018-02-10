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
    SubTableDefinition(const std::string& class_id, const std::string& instance_id, Configurable* parent)
        : Configurable (class_id, instance_id, parent)
    {
        registerParameter ("main_table_key", &main_table_key_, "");
        registerParameter ("sub_table_name", &sub_table_name_,  "");
        registerParameter ("sub_table_key", &sub_table_key_, "");
    }
    /// @brief Destructor
    virtual ~SubTableDefinition() {}

    /// @brief Returns local key
    const std::string& mainTableKey () { return main_table_key_; }
    /// @brief Returns meta sub-table name
    const std::string& subTableName () { return sub_table_name_; }
    /// @brief Returns meta sub-table key
    const std::string& subTableKey () { return sub_table_key_; }

protected:
    /// Local key
    std::string main_table_key_;
    /// Meta sub-table name
    std::string sub_table_name_;
    /// Meta sub-table key
    std::string sub_table_key_;

};

class MetaDBTableWidget;

/**
 * @brief Aggregation and abstraction of DBTables
 */
class MetaDBTable : public Configurable
{
public:
    /// @brief Constructor
    MetaDBTable(const std::string& class_id, const std::string& instance_id, DBSchema* parent);
    /// @brief Destructor
    virtual ~MetaDBTable();

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id);

    const std::string& name () const { return name_; }
    /// @brief Sets description
    void name (const std::string& name);

    /// @brief Returns description
    const std::string& info () const { return info_; }
    /// @brief Sets description
    void info (const std::string &info) { info_=info; }

    /// @brief Returns name of the main database table
    const std::string& mainTableName () const { return main_table_name_; }

    /// @brief Returns number of columns
    unsigned int numColumns () const { return columns_.size(); }
    /// @brief Returns if column with a given name exists
    bool hasColumn (const std::string& column) const { return columns_.find (column) != columns_.end(); }
    /// @brief Returns container with all columns
    const std::map <std::string, const DBTableColumn&>& columns () const { return columns_; }
    /// @brief Returns column with a given name
    const DBTableColumn &column (const std::string& column) const { return columns_.at(column); }
    const DBTable &tableFor (const std::string& column) const;

    /// @brief Returns if meta sub-tables are defined
    bool hasSubTables () const { return sub_table_definitions_.size() > 0; }
    bool hasSubTable (const std::string& name) const { return sub_table_definitions_.count(name) > 0; }
    /// @brief Returns container with all meta sub-tables
    const std::map <std::string, const DBTable&> &subTables () const { return sub_tables_; }
    /// @brief Returns string with concatenated, comma separated meta sub-tables
    std::string subTableNames () const;
    void addSubTable (const std::string& local_key, const std::string& sub_table_name,
                      const std::string& sub_table_key);
    void removeSubTable (const std::string& name);

    /// @brief Returns container with all sub meta-table definitions
    const std::map <std::string, SubTableDefinition*>& subTableDefinitions () const { return sub_table_definitions_; }

    /// @brief Returns main database table
    const DBTable &mainTable() const { return *main_table_; }
    const DBSchema &schema() const { return schema_; }

    MetaDBTableWidget *widget ();

    void lock ();

protected:
    /// Name
    std::string name_;
    /// Description
    std::string info_;
    /// Main table name
    std::string main_table_name_;

    /// Parent schema
    DBSchema& schema_;
    /// Main database table
    const DBTable* main_table_ {nullptr};

    bool locked_ {false};

    MetaDBTableWidget* widget_ {nullptr};

    /// Container with all meta sub-table definitions
    std::map <std::string, SubTableDefinition*> sub_table_definitions_;
    /// Container with all meta sub-tables
    std::map <std::string, const DBTable&> sub_tables_;
    /// Container with all table columns
    std::map <std::string, const DBTableColumn&> columns_;

    virtual void checkSubConfigurables () {}

    void updateColumns ();
};

#endif /* METADBTABLE_H_ */
