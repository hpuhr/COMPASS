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

#ifndef TRANSFORMATIONVARIABLE_H
#define TRANSFORMATIONVARIABLE_H

#include "Property.h"
#include "DBOVariableSet.h"
#include "Configurable.h"

#include <set>
#include <map>
#include <vector>

class Buffer;
class DBOVariable;


/**
@brief Represents a Property realization of a TransformationVariable for a specific DBO type.

This is a configurable entry for a Property added to a TransformationVariable.
When generated from a DBOVariable, the variable may be stored in the entry. The variable
will be lazy-loaded from the DBObjectManager when retrieved from a restored configurable entry.
  */
class TransformationVariablePropertyEntry : public Configurable
{
public:
    /// @brief Constructor
    TransformationVariablePropertyEntry( const std::string &dbo_type,
                                         const Property& property,
                                         DBOVariable* var=NULL );
    /// @brief Configurable constructor
    TransformationVariablePropertyEntry( std::string class_id,
                                         std::string instance_id,
                                         Configurable *parent );
    /// @brief Copy constructor
    TransformationVariablePropertyEntry( const TransformationVariablePropertyEntry& cpy );
    /// @brief Destructor
    virtual ~TransformationVariablePropertyEntry();

    /// @brief Sets the entries data
    void setup( const std::string &dbo_type,
                const Property& property,
                DBOVariable* var );

    /// @brief Returns the DBO type that is assigned to the entry
    const std::string &getDBOType() const;
    /// @brief Returns the entries Property
    Property getProperty() const;
    /// @brief Returns the DBOVariable that yielded the entry
    DBOVariable* getVariable();

    /// @brief Retrieves a configuration and fills in the given entry data
    static void getConfig( Configuration& config, const std::string &dbo_type, const Property& prop, bool is_var );
    /// @brief Retrieves a configuration and fills in the given entry data
    static void getConfig( Configuration& config, const std::string &dbo_type, const std::string& id );

private:
    /// The entries DBO type as integer number
    std::string dbo_type_;
    /// The properties string identifier
    std::string id_;
    /// The properties data type
    PropertyDataType data_type_;
    std::string data_type_str_;
    /// DBOVariable assigned to the entry
    DBOVariable* var_;
    /// DBOVariable present flag
    bool has_variable_;
};

/**
@brief Represents a variable that is used for transformations working on buffers.

This class abstracts properties varying amongst different buffers into a variable
serving a common purpose for a transformation. An example may be a variable that
represents the x-position in 3D space. This position may be realized differently
in buffers with varying DBO type, in one buffer the Property beeing named "pos_x",
in another one "pos_x_3d" etc. The advantage of using this class is that the varying
properties can be addressed under a unique name and that the retrieval of the "right"
Property can be arranged "behind the scenes". Further the variables can be used to
pre-check if the correct data is present in a buffer.

Property realizations can be added for a specific DBO type in various ways. They can be added
manually by providing a Property or by reading them from a DBOVariable.

A variable is assigned a unique name which can be used to easily access it and
an optional datatype. If this data type is provided it becomes automatically obligatory
for all added properties. Additionally, a default Property can be provided, which is
useful if the properties are the same amongst all buffers flowing into the transformation.

If no data type is provided to the variable, no data type checks on the different Properties
will be possible and the transformation has to cope with this itself. Handle with care!
(See TransformationBase)

The variable can be marked as "optional" to signal that it is not mandatory to provide
a valid variable to a transformation.

When adding a Property through a DBOVariable, the DBOVariable will be stored. Later, all
stored DBO variables may be retrieved through collectDBOVariables(). This may be used to
obtain a read set from all transformation variables.
  */
class TransformationVariable : public Configurable
{
public:
    typedef std::map<std::string,TransformationVariablePropertyEntry*> PropertyMap;

    /// @brief Constructor
    TransformationVariable( const std::string& name );
    /// @brief Constructor
    TransformationVariable( const std::string& name,
                            PropertyDataType data_type,
                            const std::string& default_id="" );
    /// @brief Configurable constructor
    TransformationVariable( std::string class_id, std::string instance_id, Configurable *parent );
    /// @brief Copy constructor
    TransformationVariable( const TransformationVariable& cpy );
    /// @brief Destructor
    ~TransformationVariable();

    /// @brief Adds a new Property for a specific DBO type
    void addProperty( const std::string &dbo_type, const Property& prop, DBOVariable* var=NULL );
    /// @brief Adds a new Property for a specific DBO type
    void addProperty( const std::string &dbo_type, const std::string& id );
    /// @brief Adds a one or more new properties from the given DBOVariable
    void addProperty( DBOVariable* var );
    /// @brief Sets the variables properties from the given DBOVariable
    void setProperties( DBOVariable* var );
    /// @brief Removes the Property assigned to the given DBO type
    void removeProperty( const std::string &dbo_type );
    /// @brief Clears all added properties
    void clearProperties();
    /// @brief Sets the default Property
    void setDefaultProperty( const Property& id );
    /// @brief Sets the default Property
    void setDefaultProperty( const std::string& id );
    /// @brief Sets the default Property
    void setDefaultProperty( DBOVariable* var );

    /// @brief Returns the variables name
    const std::string& name() const;
    /// @brief Checks if the variable obtains a Property for the given DBO type
    bool hasProperty( const std::string &dbo_type ) const;
    /// @brief Checks if this variable is marked as optional
    bool isOptional() const;
    /// @brief Returns the data type of this variable
    PropertyDataType dataType() const;
    /// @brief Checks if the data type check is enabled
    bool dataTypeChecked() const;
    /// @brief Returns the property that is assigned the given DBO type
    Property getProperty( const std::string& dbo_type );
    /// @brief Sets the variable optional
    void setOptional( bool optional );
    /// @brief Returns the if correct Property exists in the buffer
    bool hasProperty( Buffer* buffer );
    /// @brief Returns the correct Property in the buffer according to its DBO type
    const Property& property( Buffer* buffer );
    /// @brief Returns the correct Property index in the buffer according to its DBO type
    int propertyIndex( Buffer* buffer );
    /// @brief Returns all added properties
    const PropertyMap& getProperties() const;
    /// @brief Retrieves the default Property
    bool getDefaultProperty( Property& prop ) const;

    /// @brief Sets the variables data type
    void setDataType( PropertyDataType data_type, bool clear=false );
    /// @brief Enables/disables the data type check for newly added properties
    void enableDataTypeCheck( bool enable );

    /// @brief Collects all DBO variables that were used to set the variables properties
    void collectDBOVariables( DBOVariableSet& varset ) const;

    /// @brief Prints the variables properties
    void print();

    virtual void generateSubConfigurable( std::string class_id, std::string instance_id );

    /// @brief Retrieves a configuration and fills in the given variable data
    static void getConfig( Configuration& config, const std::string& name );
    /// @brief Retrieves a configuration and fills in the given variable data
    static void getConfig( Configuration& config,
                           const std::string& name,
                           PropertyDataType data_type,
                           const std::string& default_id );


protected:
    virtual void checkSubConfigurables();

private:
    /// @brief Checks if the given DBOVariable exists in the DBO manager
    bool existsDBOVariable( const std::string & dbo_type, const std::string& id );
    /// @brief Deletes all properties including the default property
    void deleteEntries();
    /// @brief Checks all properties including the default property against the current data type
    bool checkDataTypes();

    /// Added properties
    PropertyMap properties_;
    /// Variable name
    std::string name_;
    /// Default property
    TransformationVariablePropertyEntry* default_prop_;
    /// Optional flag
    bool optional_;
    /// Variable data type
    PropertyDataType data_type_;
    std::string data_type_str_;
    /// Data type check flag
    bool check_data_type_;
};

/**
@brief Represents a set of transformation variables.

Provides an interface to manipulate the stored transformation variables.
  */
class TransformationVariables : public Configurable
{
public:
    typedef std::map<std::string,TransformationVariable*> TransformationVariableMap;
    typedef std::vector<TransformationVariable*> TransformationVariableVector;

    /// @brief Constructor
    TransformationVariables();
    /// @brief Copy constructor
    TransformationVariables( const TransformationVariables& cpy );
    /// @brief Configurable constructor
    TransformationVariables( std::string class_id, std::string instance_id, Configurable *parent );
    /// @brief Destructor
    ~TransformationVariables();

    /// @brief Returns the number of stored variables
    unsigned int numberOfVariables() const;
    /// @brief Returns the idx'th variable
    TransformationVariable* getVariable( int idx );
    /// @brief Returns the variable of the given name
    TransformationVariable* getVariable( const std::string& name );
    /// @brief Adds a new variable of the given name
    TransformationVariable* addVariable( const std::string& name );
    /// @brief Adds the given variable by copying it
    TransformationVariable* addVariable( TransformationVariable* var );
    /// @brief Adds a new variable
    TransformationVariable* addVariable( const std::string& name, PropertyDataType data_type, const std::string& default_id="" );
    /// @brief Removes and deletes the variable of the given name
    void removeVariable( const std::string& name );
    /// @brief Clears all variables
    void clearVariables();
    /// @brief Checks is a variable of the given name exists in the container
    bool exists( const std::string& name );

    /// @brief Assignment operator
    TransformationVariables& operator=( const TransformationVariables& rhs );

    virtual void generateSubConfigurable( std::string class_id, std::string instance_id );

protected:
    virtual void checkSubConfigurables();

private:
    /// String identifier access to the stored variables
    TransformationVariableMap map_;
    /// Stored variables
    TransformationVariableVector vars_;
};

#endif //TRANSFORMATIONVARIABLE_H
