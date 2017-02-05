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
 * DBTableColumn.h
 *
 *  Created on: Aug 13, 2012
 *      Author: sk
 */

#ifndef DBTABLECOLUMN_H_
#define DBTABLECOLUMN_H_

#include <string>
#include "configurable.h"

class DBTable;

/**
 * @brief Database table column definition
 *
 * Holds some parameters which define a database table column
 */
class DBTableColumn : public Configurable
{
public:
  /// @brief Constructor
  DBTableColumn(const std::string &class_id, const std::string &instance_id, DBTable *table);
  /// @brief Destructor
  virtual ~DBTableColumn();

  /// @brief Sets the column name
  void name (const std::string &name) { name_=name; }
  /// @brief Returns the column name
  const std::string &name() const { return name_; }

  /// @brief Sets the data type
  void type (const std::string &type) { type_=type; }
  /// @brief Returns the data type
  const std::string &type() const { return type_; }

  /// @brief Sets key flag
  void isKey (bool is_key) { is_key_=is_key;}
  /// @brief Returns key flag
  bool isKey () const { return is_key_; }

  void comment (const std::string &comment) { comment_ = comment; }
  const std::string &comment () const { return comment_; }

  /// @brief Returns if column has an assigned unit
  bool unit () const { return unit_dimension_.size() != 0; }
  /// @brief Returns unit dimension
  const std::string &unitDimension () const { return unit_dimension_; }
  /// @brief Returns unit
  const std::string &unitUnit () const { return unit_unit_; }

  /// @brief Returns database table name which holds this column
  const std::string &dbTableName () const { return db_table_name_; }

  bool hasSpecialNull () const { return special_null_.size() > 0; }
  void specialNull (std::string special_null) { special_null_ = special_null; }
  const std::string &specialNull () const { return special_null_; }

  void createSubConfigurables () {}

protected:
  DBTable &table_;
  /// Name of the column
  std::string name_;
  /// Data type
  std::string type_;
  /// Key flag
  bool is_key_;
  /// Data type
  std::string comment_;
  /// Unit dimension
  std::string unit_dimension_;
  /// Unit
  std::string unit_unit_;
  /// Database table name which holds this column
  std::string db_table_name_;
  /// Special value signifying null value
  std::string special_null_;
};

#endif /* DBTABLECOLUMN_H_ */
