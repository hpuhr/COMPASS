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
#include "Global.h"
#include "Property.h"
#include "Configurable.h"

class DBTableColumn;

/**
 * @brief Definition of DBOVariable by DBO type and string identifier.
 *
 * Used by DBOVariable.
 */
class DBOVariableDefinition : public Configurable
{
public:
  DBOVariableDefinition(std::string class_id, std::string instance_id, Configurable *parent)
  : Configurable (class_id, instance_id, parent)
  {
    registerParameter ("dbo_type_", &dbo_type_, "");
    registerParameter ("id", &id_, "");

    assert (id_.size() > 0);
  }
  virtual ~DBOVariableDefinition() {}

  const std::string &getDBOType () { return dbo_type_; }
  void setDBOType (const std::string &dbo_type) { dbo_type_=dbo_type; }

  std::string getId () { return id_; }
  void setId (std::string id) { id_=id; }

protected:
  std::string dbo_type_;
  std::string id_;
};

/**
 * @brief Definition of a variable, based on identifiers of the schema, meta table and variable name.
 *
 * Used by DBOVariable.
 */
class DBOSchemaVariableDefinition : public Configurable
{
public:
  DBOSchemaVariableDefinition(std::string class_id, std::string instance_id, Configurable *parent) : Configurable (class_id, instance_id, parent)
  {
    registerParameter ("schema", &schema_, "");
    registerParameter ("meta_table", &meta_table_, "");
    registerParameter ("variable", &variable_, "");
  }
  virtual ~DBOSchemaVariableDefinition() {}

  std::string getSchema () { return schema_; }
  void setSchema(std::string schema) { schema_=schema; }

  std::string getMetaTable () { return meta_table_; }
  void setMetaTable(std::string meta_table) { meta_table_=meta_table; }

  std::string getVariable () { return variable_; }
  void setVariable(std::string variable) { variable_=variable; }

protected:
  std::string schema_;
  std::string meta_table_;
  std::string variable_;
};

class DBOVariableMinMaxObserver;

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
  /// DBO type
  std::string dbo_type_;
  /// Value representation type, based on enum STRING_REPRESENTATION
  unsigned int representation_int_;
  /// Description
  std::string description_;


  /// @brief Constructor
  DBOVariable(std::string class_id, std::string instance_id, Configurable *parent);
  /// @brief Desctructor
  virtual ~DBOVariable();

  /// @brief Return if variable exist in DBO of type
  bool existsIn (const std::string &dbo_type);
  /// @brief Returns variable existing in DBO of type
  DBOVariable *getFor (const std::string &dbo_type);
  /// @brief Returns first available variable
  DBOVariable *getFirst ();
  /// @brief Return variable identifier in DBO of type
  std::string getNameFor (const std::string &dbo_type);
  /// @brief Returns flag indicating if variable is meta variable
  bool isMetaVariable ();

  /// @brief Returns container with sub variable names, dbo type -> sub variable name
  std::map <std::string, std::string> &getSubVariables () { return sub_variables_; }
  /// @brief Sets sub-variable name for DBO of type
  void setSubVariable (const std::string &type, std::string name);

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

  virtual void generateSubConfigurable (std::string class_id, std::string instance_id);

  /// @brief Returns variable identifier
  std::string getName () { return id_; }
  /// @brief Sets variable identifier
  void setName (std::string name) { id_=name; };

  /// @brief Returns variable description
  std::string getInfo () { return description_; }
  /// @brief Sets variable description
  void setInfo (std::string info) { description_=info; };

  /// @brief Returns variable data type
  PropertyDataType getDataType () { return data_type_;}
  /// @brief Sets variable data type
  void setDataType (PropertyDataType type) { data_type_=type; }

  /// @brief Returns DBO type
  const std::string &getDBOType () const { return dbo_type_; }
//  /// @brief Returns DBO type string identifier
//  std::string getDBOTypeString () { return DB_OBJECT_TYPE_STRINGS.at((DB_OBJECT_TYPE)dbo_type_int_);}

  /// @brief Returns of schema is present in schema_variables_
  bool hasSchema (std::string schema);
  /// @brief Returns meta table identifier for a given schema
  std::string getMetaTable (std::string schema);
  /// @brief Returns variable identifier for a given schema
  std::string getVariableName (std::string schema);

  bool hasCurrentDBColumn ();
  DBTableColumn *getCurrentDBColumn ();

  /// @brief Returns if current schema is present in schema_variables_
  bool hasCurrentSchema ();
  /// @brief Returns meta table identifier for current schema
  std::string getCurrentMetaTable ();
  /// @brief Returns variable identifier for current schema
  std::string getCurrentVariableName ();

  /// @brief Returns if unit information is present
  bool hasUnit () { return unit_dimension_.size() != 0;}
  /// @brief Returns unit dimension
  std::string &getUnitDimension () { return unit_dimension_; }
  /// @brief  Returns unit unit
  std::string &getUnitUnit () { return unit_unit_; }

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

  /// @brief Registers meta-variables as parents of sub-variables
  void registerAsParent ();
  /// @brief Unregisters meta-variables as parents of sub-variables
  void unregisterAsParent ();
  /// @brief Checks if min/max info is present for meta-variables
  //void subVariableHasMinMaxInfo ();

protected:
  /// Minimum as string
  std::string min_;
  /// Maximum as string
  std::string max_;

  /// Container with all minimum/maximum information observers
  std::vector <DBOVariableMinMaxObserver*> min_max_observers_;
  /// Meta-variables with container this variable
  std::vector <DBOVariable*> parent_variables_;

  /// Unit dimension such as time
  std::string unit_dimension_;
  /// Unit unit such as seconds
  std::string unit_unit_;

  /// Container of sub-variable definitions, for meta variables
  std::vector<DBOVariableDefinition *> sub_variable_definitions_;
  /// Container for sub variables (DBO type -> variable identifier), for meta variables
  std::map <std::string, std::string> sub_variables_;

  /// Container with schema-variable definitions
  std::vector <DBOSchemaVariableDefinition *> schema_variables_definitions_;
  /// Container with schema, meta table, variable mappings (schema identifier -> (meta table identifier, variable identifier))
  std::map <std::string, std::pair <std::string, std::string> > schema_variables_;

  /// Flag indicating if this meta-variable has registered itself to its sub-variables
  bool registered_as_parent_;

  virtual void checkSubConfigurables ();
  /// @brief Registers a parent variable
  void registerParentVariable (DBOVariable *parent);
  /// @brief Unregisters a parent variable
  void unregisterParentVariable (DBOVariable *parent);
  /// @brief Notifies minimum/maximum information observers
  void notifyMinMaxObservers ();
};

#endif /* DBOVARIABLE_H_ */
