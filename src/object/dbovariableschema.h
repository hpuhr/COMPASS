#ifndef DBOVARIABLESCHEMA_H
#define DBOVARIABLESCHEMA_H

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

    DBOVariableDefinition& operator=(DBOVariableDefinition&& other)
    {
        dbo_name_ = other.dbo_name_;
        other.dbo_name_ = "";

        dbo_variable_name_ = other.dbo_variable_name_;
        other.dbo_variable_name_ = "";

        return *this;
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

    DBOSchemaVariableDefinition& operator=(DBOSchemaVariableDefinition&& other)
    {
        schema_ = other.schema_;
        other.schema_ = "";

        variable_identifier = other.variable_identifier;
        other.variable_identifier = "";

        return *this;
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

#endif // DBOVARIABLESCHEMA_H
