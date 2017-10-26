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

#ifndef DBOVARIABLE_H_
#define DBOVARIABLE_H_

#include <string>
#include <vector>
#include <QObject>

#include "global.h"
#include "property.h"
#include "configurable.h"
#include "stringconv.h"

//#include <boost/algorithm/string.hpp>

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

        // DBOVAR LOWERCASE HACK
        //boost::algorithm::to_lower(dbo_variable_name_);

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
        registerParameter ("variable_identifier", &variable_identifier, "");
    }
    virtual ~DBOSchemaVariableDefinition() {}

    const std::string &getSchema () { return schema_; }
    void setSchema(std::string schema) { schema_=schema; }

    const std::string &getMetaTable () { return meta_table_; }
    void setMetaTable(std::string meta_table) { meta_table_=meta_table; }

    const std::string &getVariableIdentifier () { return variable_identifier; }
    void setVariableIdentifier(std::string variable) { variable_identifier=variable; }

protected:
    std::string schema_;
    std::string meta_table_;
    std::string variable_identifier;
};

class DBObject;
class MetaDBTable;
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
class DBOVariable : public QObject, public Property, public Configurable
{
    Q_OBJECT
public:
    /// @brief Constructor
    DBOVariable(const std::string &class_id, const std::string &instance_id, DBObject *parent);
    /// @brief Desctructor
    virtual ~DBOVariable();

    /// @brief Comparison operator
    bool operator==(const DBOVariable &var);

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
    bool hasSchema (const std::string &schema) const;
    /// @brief Returns meta table identifier for a given schema
    const std::string &metaTable (const std::string &schema) const;
    /// @brief Returns variable identifier for a given schema
    const std::string &variableName (const std::string &schema) const;

    bool hasCurrentDBColumn () const;
    const DBTableColumn &currentDBColumn () const;

    /// @brief Returns if current schema is present in schema_variables_
    bool hasCurrentSchema () const;
    /// @brief Returns meta table identifier for current schema
    const std::string &currentMetaTableString () const;
    /// @brief Returns meta table for current schema
    const MetaDBTable &currentMetaTable () const;
    /// @brief Returns variable identifier for current schema
    const std::string &currentVariableIdentifier () const;

    /// @brief Returns if dimension information is present
    bool hasDimension () { return dimension_.size() > 0;}
    /// @brief Returns unit dimension
    const std::string &dimensionConst () const{ return dimension_; } //TODO should be const
    std::string &dimension () { return dimension_; } //TODO should be const
    /// @brief  Returns unit unit
    const std::string &unitConst () const { return unit_; }
    std::string &unit () { return unit_; }

    DBObject &dbObject () const { return dbo_parent_; }

    std::string getMinString ();
    std::string getMaxString ();
    std::string getMinStringRepresentation ();
    std::string getMaxStringRepresentation ();

    DBOVariableWidget *widget ();

    Utils::String::Representation representation() const;
    void representation(const Utils::String::Representation &representation);

protected:
    /// DBO parent
    DBObject &dbo_parent_;
    /// Value representation type, based on enum STRING_REPRESENTATION
    std::string representation_str_;
    Utils::String::Representation representation_;

    /// Description
    std::string description_;

    bool min_max_set_{false};
    /// Minimum as string
    std::string min_;
    /// Maximum as string
    std::string max_;

    /// Unit dimension such as time
    std::string dimension_;
    /// Unit unit such as seconds
    std::string unit_;

    /// Container with schema identified->schema-variable definitions
    std::map <std::string, DBOSchemaVariableDefinition*> schema_variables_;

    DBOVariableWidget *widget_;

    virtual void checkSubConfigurables ();
    void setMinMax ();
};

#endif /* DBOVARIABLE_H_ */
