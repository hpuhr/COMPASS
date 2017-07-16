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
 * DBOVariable.h
 *
 *  Created on: Apr 25, 2012
 *      Author: sk
 */

#ifndef DBOVARIABLE_H_
#define DBOVARIABLE_H_

#include <string>
#include <vector>

#include "global.h"
#include "property.h"
#include "configurable.h"

class DBTableColumn;

/**
 * @brief Definition of DBOVariable by DBO type and string identifier.
 *
 * Used by DBOVariable.
 */
class DBOVariableDefinition : public Configurable
{
public:
  DBOVariableDefinition(const std::string &class_id, const std::string &instance_id, Configurable *parent)
  : Configurable (class_id, instance_id, parent)
  {
    registerParameter ("dbo_name", &dbo_name_, "");
    registerParameter ("dbo_variable_name", &dbo_variable_name_, "");

    assert (dbo_variable_name_.size() > 0);
  }
  virtual ~DBOVariableDefinition() {}

  const std::string &dboName () { return dbo_name_; }
  void dboName (const std::string &dbo_name) { dbo_name_=dbo_name; }

  const std::string &variableName () { return dbo_variable_name_; }
  void variableName (const std::string &dbo_variable_name) { dbo_variable_name_=dbo_variable_name; }

protected:
  std::string dbo_name_;
  std::string dbo_variable_name_;
};

/**
 * @brief Definition of a variable, based on identifiers of the schema, meta table and variable name.
 *
 * Used by DBOVariable.
 */
class DBOSchemaVariableDefinition : public Configurable
{
public:
  DBOSchemaVariableDefinition(const std::string &class_id, const std::string &instance_id, Configurable *parent) : Configurable (class_id, instance_id, parent)
  {
    registerParameter ("schema", &schema_, "");
    registerParameter ("meta_table", &meta_table_, "");
    registerParameter ("variable", &variable_, "");
  }
  virtual ~DBOSchemaVariableDefinition() {}

  const std::string &getSchema () { return schema_; }
  void setSchema(std::string schema) { schema_=schema; }

  const std::string &getMetaTable () { return meta_table_; }
  void setMetaTable(std::string meta_table) { meta_table_=meta_table; }

  const std::string &getVariable () { return variable_; }
  void setVariable(std::string variable) { variable_=variable; }

protected:
  std::string schema_;
  std::string meta_table_;
  std::string variable_;
};

class DBObject;
class MetaDBTable;
//class DBOVariableMinMaxObserver;
class DBOVariableWidget;

/**
 * @brief Variable of a DBObject
 *
 * Abstracted variable, which has two basic mechanisms.
 *
 * For one, the variable might not really exist in a table, but is a surrogate abstraction for a number of variables in different
 * DBObjects (meta variable) which carry the same content. When used, depending on the DBO type, one can get the really existing
 * DBOVariable using the getFor function.
 *
 * For the second, a DBOVariable is an abstraction of the underlying variable in the meta table which may differ for different
 * schemas. Therefore, a DBOSchemaVariableDefinition is used, which defines the all possible underlying variables.
 *
 * Based on Property (data type definition).
 */
class DBOVariable : public Property, public Configurable
{
public:
  /// @brief Constructor
  DBOVariable(const std::string &class_id, const std::string &instance_id, DBObject *parent);
  /// @brief Desctructor
  virtual ~DBOVariable();

  /// @brief Comparison operator
  bool operator==(const DBOVariable &var);

  /// @brief Sets the value string representation
//  void setStringRepresentation (STRING_REPRESENTATION representation);
//  /// @brief Returns value string representation
//  STRING_REPRESENTATION getRepresentation () { return (STRING_REPRESENTATION)representation_int_;}
//  /// @brief Returns value from void pointer as string
//  std::string getValueFrom (void *ptr);
//  /// @brief Returns value string from representation string
//  std::string getValueFromRepresentation (std::string representation_string, bool transform=false, bool* ok=NULL);
//  /// @brief Returns representation string from value string
//  std::string getRepresentationFromValue (std::string value_string);

  /// @brief Prints information for debugging
  void print ();

  virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

  /// @brief Returns variable identifier
  const std::string &name () const { return name_; }
  /// @brief Sets variable identifier
  void name (const std::string &name) { name_=name; }

  const std::string &dboName () const;

  /// @brief Returns variable description
  const std::string &description () { return description_; }
  /// @brief Sets variable description
  void description (const std::string &description) { description_=description; }

  /// @brief Returns of schema is present in schema_variables_
  bool hasSchema (const std::string &schema);
  /// @brief Returns meta table identifier for a given schema
  const std::string &metaTable (const std::string &schema);
  /// @brief Returns variable identifier for a given schema
  const std::string &variableName (const std::string &schema);

  bool hasCurrentDBColumn ();
  const DBTableColumn &currentDBColumn ();

  /// @brief Returns if current schema is present in schema_variables_
  bool hasCurrentSchema ();
  /// @brief Returns meta table identifier for current schema
  const std::string &currentMetaTableString ();
  /// @brief Returns meta table for current schema
  const MetaDBTable &currentMetaTable ();
  /// @brief Returns variable identifier for current schema
  const std::string &currentVariableName ();

  /// @brief Returns if unit information is present
  bool hasUnit () { return dimension_.size() != 0;}
  /// @brief Returns unit dimension
  std::string &dimension () { return dimension_; }
  /// @brief  Returns unit unit
  std::string &unit () { return unit_; }

  DBObject &dbObject () const { return dbo_parent_; }

  /// @brief Returns flag if minimum/maximum information is available
  //bool hasMinMaxInfo ();
  /// @brief Triggers generation of the minimum/maximum information
  //void buildMinMaxInfo ();
  /// @brief Callback used to the the minimum/maximum information
  //void setMinMax (std::string min, std::string max);

  //std::string getMinString ();
  //std::string getMaxString ();

  /// @brief Registers an observer of the minimum/maximum information
  //void addMinMaxObserver (DBOVariableMinMaxObserver *observer);
  ///// @brief Removes an observer of the minimum/maximum information
  ///void removeMinMaxObserver (DBOVariableMinMaxObserver *observer);

  /// @brief Checks if min/max info is present for meta-variables
  //void subVariableHasMinMaxInfo ();

  DBOVariableWidget *widget ();

protected:
  /// DBO parent
  DBObject &dbo_parent_;
  /// Value representation type, based on enum STRING_REPRESENTATION
  unsigned int representation_int_;
  /// Description
  std::string description_;

//  /// Minimum as string
//  std::string min_;
//  /// Maximum as string
//  std::string max_;

  /// Container with all minimum/maximum information observers
  //std::vector <DBOVariableMinMaxObserver*> min_max_observers_;

  /// Unit dimension such as time
  std::string dimension_;
  /// Unit unit such as seconds
  std::string unit_;

  /// Container with schema identified->schema-variable definitions
  std::map <std::string, DBOSchemaVariableDefinition*> schema_variables_;

  DBOVariableWidget *widget_;

  virtual void checkSubConfigurables ();
  /// @brief Notifies minimum/maximum information observers
  //void notifyMinMaxObservers ();
};

#endif /* DBOVARIABLE_H_ */
