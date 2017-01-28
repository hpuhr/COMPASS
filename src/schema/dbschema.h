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

#include "configurable.h"

class DBTable;
class MetaDBTable;
class DBSchemaWidget;

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
  DBSchema(const std::string &class_id, const std::string &instance_id, Configurable *parent);
  /// @brief Destructor
  virtual ~DBSchema();

  virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

  /// @brief Sets the schema name
  void name (const std::string &name) { assert (name.size() != 0); name_=name; }
  /// @brief Returns the schema name
  const std::string &name () const { return name_; }

  /// @brief Returns the DBTable with the supplied name
  const DBTable &table (const std::string &name) const  {  assert (tables_.find(name) != tables_.end()); return tables_.at(name); }
  /// @brief returns flag if a table with the given name exists
  bool hasTable (const std::string &name) const { return tables_.find(name) != tables_.end(); }
  const std::string &tableName (const std::string &db_table_name) const;

  bool hasMetaTable (const std::string &name) const;
  const MetaDBTable &metaTable (const std::string &name) const { assert (hasMetaTable(name)); return meta_tables_.at(name);}

  /// @brief Returns container with all tables
  const std::map <std::string, DBTable> &tables () const { return tables_; }
  /// @brief Returns container with all meta-tables
  const std::map <std::string, MetaDBTable> &metaTables ()  const{ return meta_tables_; }

  /// @brief Updates table container (if name of a table changed)
  void updateTables ();
  /// @brief Updates meta-table container (if name of meta-table changed)
  void updateMetaTables ();

  DBSchemaWidget *widget ();

protected:
  virtual void checkSubConfigurables () {};

private:
  /// Name of the schema
  std::string name_;

  /// Container with all tables (table name -> DBTable)
  std::map <std::string, DBTable> tables_;
  /// Container with all meta-tables (meta-table name -> MetaDBTable)
  std::map <std::string, MetaDBTable> meta_tables_;

  DBSchemaWidget *widget_;
};

#endif /* DBSCHEMA_H_ */
