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
#include "Configurable.h"

class DBTableColumn;
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
  SubTableDefinition(std::string class_id, std::string instance_id, Configurable *parent)
   : Configurable (class_id, instance_id, parent)
  {
    registerParameter ("local_key", &local_key_, (std::string)"");
    registerParameter ("sub_table_name", &sub_table_name_,  (std::string)"");
    registerParameter ("sub_table_key", &sub_table_key_,  (std::string)"");
  }
  /// @brief Destructor
  virtual ~SubTableDefinition() {}

  /// @brief Returns local key
  std::string getLocalKey () { return local_key_; }
  /// @brief Returns meta sub-table name
  std::string getSubTableName () { return sub_table_name_; }
  /// @brief Returns meta sub-table key
  std::string getSubTableKey () { return sub_table_key_; }

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
  MetaDBTable(std::string class_id, std::string instance_id, Configurable *parent, DBSchema *schema);
  /// @brief Destructor
  virtual ~MetaDBTable();

  virtual void generateSubConfigurable (std::string class_id, std::string instance_id);

  /// @brief Returns name
  std::string getName () { return name_; }
  /// @brief Sets name
  void setName (std::string name) { assert(name.size() != 0 ); name_=name; }

  /// @brief Returns description
  std::string getInfo () { return info_; }
  /// @brief Sets description
  void setInfo (std::string info) { info_=info; }

  /// @brief Returns name of the main database table
  std::string getTableName () { return table_name_; }
  /// @brief Sets name of the main database table
  void setTableName (std::string table_name) { assert (table_name.size() != 0); table_name_=table_name; }

  /// @brief Returns the database name of the main table
  std::string getTableDBName ();
  /// @brief Returns database table name for a variable
  std::string getTableDBNameForVariable (std::string variable_name);

  /// @brief Returns number of columns
  unsigned int getNumColumns ();
  /// @brief Returns container with all columns
  std::map <std::string, DBTableColumn*>& getColumns ();
  /// @brief Returns column with a given name
  const DBTableColumn &getTableColumn (std::string column);

  /// @brief Returns if meta sub-tables are defined
  bool hasSubTables () { return sub_table_definitions_.size() > 0; }
  /// @brief Returns container with all meta sub-tables
  std::map <SubTableDefinition*, MetaDBTable*> &getSubTables ();
  /// @brief Returns string with concatenated, comma separated meta sub-tables
  std::string getSubTableNames ();
  /// @brief Returns string with all table names (own and sub metas) as comma separated list
  std::string getAllTableNames (); //
  /// @brief Returns container with all table names
  std::vector<std::string> getAllTableNamesAsVector ();

  /// @brief Returns SQL where clause with all used meta sub-tables
  std::string getSubTablesWhereClause (std::vector <std::string> &used_tables);
  /// @brief Returns SQL key clause for a give meta sub-table
  std::string getSubTableKeyClause (std::string sub_table_name);

  /// @brief Returns if column with a given name exists
  bool hasTableColumn (std::string column);

  /// @brief Returns container with all sub meta-table definitions
  std::vector <SubTableDefinition *>& getSubTableDefinitions () { return sub_table_definitions_; }

  /// @brief Returns main database table
  DBTable *getTable() { assert(table_); return table_; }

private:
  /// Name
  std::string name_;
  /// Description
  std::string info_;
  /// Main table name
  std::string table_name_;

  /// Parent schema
  DBSchema *schema_;
  /// Main database table
  DBTable *table_;

  /// Container with all meta sub-table definitions
  std::vector <SubTableDefinition *> sub_table_definitions_;
  /// Container with all meta sub-tables
  std::map <SubTableDefinition*, MetaDBTable*> sub_tables_;
  /// Container with all table columns
  std::map <std::string, DBTableColumn*> columns_;

  /// @brief Creates sub meta-tables from their definitions (if required)
  void setSubTablesIfRequired ();

protected:
  virtual void checkSubConfigurables ();
};

#endif /* METADBTABLE_H_ */
