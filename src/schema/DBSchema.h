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
 * DBSchema.h
 *
 *  Created on: Aug 13, 2012
 *      Author: sk
 */

#ifndef DBSCHEMA_H_
#define DBSCHEMA_H_

#include "Configurable.h"

class DBTable;
class MetaDBTable;

/**
 * @brief Encapsulates a database schema
 *
 * Consists of a collection of database tables and meta tables.
 *
 */
class DBSchema : public Configurable
{
public:
  /// @brief Constructor
  DBSchema(std::string class_id, std::string instance_id, Configurable *parent);
  /// @brief Destructor
  virtual ~DBSchema();

  virtual void generateSubConfigurable (std::string class_id, std::string instance_id);

  /// @brief Sets the schema name
  void setName (std::string name) { assert (name.size() != 0); name_=name; }
  /// @brief Returns the schema name
  std::string getName () { return name_; }

  /// @brief Returns the DBTable with the supplied name
  DBTable *getTable (std::string name);
  /// @brief returns flag if a table with the given name exists
  bool hasTable (std::string name);
  std::string getTableName (std::string db_table_name);

  MetaDBTable *getMetaTable (std::string name);
  bool hasMetaTable (std::string name);

  /// @brief Returns container with all tables
  std::map <std::string, DBTable*> &getTables () { return tables_; }
  /// @brief Returns container with all meta-tables
  std::map <std::string, MetaDBTable*> &getMetaTables () { return meta_tables_; }

  /// @brief Updates table container (if name of a table changed)
  void updateTables ();
  /// @brief Updates meta-table container (if name of meta-table changed)
  void updateMetaTables ();

protected:
  virtual void checkSubConfigurables ();

private:
  /// Name of the schema
  std::string name_;

  /// Container with all tables (table name -> DBTable)
  std::map <std::string, DBTable*> tables_;
  /// Container with all meta-tables (meta-table name -> MetaDBTable)
  std::map <std::string, MetaDBTable*> meta_tables_;
};

#endif /* DBSCHEMA_H_ */
