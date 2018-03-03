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
#include "dbobject.h"

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
    DBOSchemaVariableDefinition(const std::string &class_id, const std::string &instance_id, Configurable *parent)
        : Configurable (class_id, instance_id, parent)
    {
        registerParameter ("schema", &schema_, "");
        registerParameter ("variable_identifier", &variable_identifier, "");
    }
    virtual ~DBOSchemaVariableDefinition() {}

    const std::string &getSchema () { return schema_; }
    void setSchema(std::string schema) { schema_=schema; }

    const std::string &getVariableIdentifier () { return variable_identifier; }
    void setVariableIdentifier(std::string variable) { variable_identifier=variable; }

protected:
    std::string schema_;
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
    enum class Representation {
        STANDARD,
        SECONDS_TO_TIME,
        DEC_TO_OCTAL,
        DEC_TO_HEX,
        FEET_TO_FLIGHTLEVEL,
        DATA_SRC_NAME
    };

    static Representation stringToRepresentation (const std::string &representation_str);
    static std::string representationToString (Representation representation);
    static const std::map<Representation, std::string>& Representations() { return representation_2_string_; }

    /// @brief Constructor
    DBOVariable(const std::string& class_id, const std::string& instance_id, DBObject* parent);
    /// @brief Desctructor
    virtual ~DBOVariable();

    /// @brief Comparison operator
    bool operator==(const DBOVariable& var);

    /// @brief Prints information for debugging
    void print ();

    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id);

    /// @brief Returns variable identifier
    const std::string& name () const { return name_; }
    /// @brief Sets variable identifier
    void name (const std::string& name) { name_=name; }

    const std::string& dboName () const;

    /// @brief Returns variable description
    const std::string& description () { return description_; }
    /// @brief Sets variable description
    void description (const std::string& description) { description_=description; }

    /// @brief Returns of schema is present in schema_variables_
    bool hasSchema (const std::string& schema) const;
    /// @brief Returns meta table identifier for a given schema
    const std::string& metaTable (const std::string& schema) const;

    bool hasVariableName (const std::string& schema) const;
    /// @brief Returns variable identifier for a given schema
    const std::string& variableName (const std::string& schema) const;
    void setVariableName (const std::string& schema, const std::string& name);

    bool hasCurrentDBColumn () const;
    const DBTableColumn& currentDBColumn () const;

    /// @brief Returns if current schema is present in schema_variables_
    bool hasCurrentSchema () const;
    /// @brief Returns meta table identifier for current schema
    const std::string& currentMetaTableString () const;
    /// @brief Returns meta table for current schema
    const MetaDBTable& currentMetaTable () const;
    /// @brief Returns variable identifier for current schema
    const std::string& currentVariableIdentifier () const;

    /// @brief Returns if dimension information is present
    bool hasDimension () { return dimension_.size() > 0;}
    /// @brief Returns unit dimension
    const std::string& dimensionConst () const{ return dimension_; } //TODO should be const
    std::string& dimension () { return dimension_; } //TODO should be const
    /// @brief  Returns unit unit
    const std::string& unitConst () const { return unit_; }
    std::string& unit () { return unit_; }

    DBObject& dbObject () const { return db_object_; }

    std::string getMinString ();
    std::string getMaxString ();
    std::string getMinStringRepresentation ();
    std::string getMaxStringRepresentation ();

    DBOVariableWidget* widget ();

    Representation representation() const;
    const std::string& representationString () const;
    void representation(const Representation& representation);

    template <typename T> std::string getAsSpecialRepresentationString (T value) const
    {
        std::ostringstream out;
        try
        {
            if (representation_ == DBOVariable::Representation::SECONDS_TO_TIME)
            {
                return Utils::String::timeStringFromDouble (value);
            }
            else if (representation_ == DBOVariable::Representation::DEC_TO_OCTAL)
            {
                out << std::oct << std::setfill ('0') << std::setw (4) << value;
            }
            else if (representation_ == DBOVariable::Representation::DEC_TO_HEX)
            {
                out << std::uppercase << std::hex << value;
            }
            else if (representation_ == DBOVariable::Representation::FEET_TO_FLIGHTLEVEL)
            {
                out << value/100.0;
            }
            else if (representation_ == DBOVariable::Representation::DATA_SRC_NAME)
            {
                if (db_object_.hasDataSources())
                {
                    std::map<int, DBODataSource>& data_sources = db_object_.dataSources();

                    for (auto& ds_it : data_sources)
                    {
                        if (std::to_string(ds_it.first) == std::to_string(value))
                        {
                            if (ds_it.second.hasShortName())
                                return ds_it.second.shortName();
                            else
                                return ds_it.second.name();
                        }
                    }
                    // not found, return original
                }
                // has no datasources, return original

                return std::to_string(value);
            }
            else
            {
                throw std::runtime_error ("DBOVariable: getAsSpecialRepresentationString: unknown representation");
            }
        }
        catch(std::exception& e)
        {
            logerr  << "DBOVariable: getAsSpecialRepresentationString: exception thrown: " << e.what();
        }
        catch(...)
        {
            logerr  << "DBOVariable: getAsSpecialRepresentationString: exception thrown";;
        }

        return out.str();
    }

    std::string getRepresentationStringFromValue (const std::string& value_str) const;
    std::string getValueStringFromRepresentation (const std::string& representation_str) const;

    std::string multiplyString (const std::string& value_str, double factor) const;
    const std::string& getLargerValueString (const std::string& value_a_str, const std::string& value_b_str) const;
    const std::string& getSmallerValueString (const std::string& value_a_str, const std::string& value_b_str) const;

    void lock ();
    void unlock ();

protected:
    static std::map<Representation, std::string> representation_2_string_;
    static std::map<std::string, Representation> string_2_representation_;

    /// DBO parent
    DBObject& db_object_;
    /// Value representation type, based on enum STRING_REPRESENTATION
    std::string representation_str_;
    Representation representation_;

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

    DBOVariableWidget* widget_ {nullptr};

    bool locked_ {false};

    virtual void checkSubConfigurables ();
    void setMinMax ();
};

Q_DECLARE_METATYPE(DBOVariable*)

#endif /* DBOVARIABLE_H_ */
