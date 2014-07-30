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

#include "TransformationVariable.h"
#include "Buffer.h"
#include "DBOVariable.h"
#include "DBObjectManager.h"


/**************************************************************************
TransformationVariablePropertyEntry
 ***************************************************************************/

/**
Constructor.
@param dbo_type DBO type assigned to the entry.
@param property The entries Property.
@param var Optional DBOVariable to be stored with the entry.
 */
TransformationVariablePropertyEntry::TransformationVariablePropertyEntry( DB_OBJECT_TYPE dbo_type,
        const Property& property,
        DBOVariable* var )
{
    setup( dbo_type, property, var );
}

/**
Configurable constructor.
Don't forget to set initial values for the registered parameters through the Configuration ( use getConfig() for convenience ).
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
 */
TransformationVariablePropertyEntry::TransformationVariablePropertyEntry( std::string class_id,
        std::string instance_id,
        Configurable *parent )
:   Configurable( class_id, instance_id, parent ),
    dbo_type_( DBO_UNDEFINED ),
    id_( "" ),
    data_type_( P_TYPE_INT ),
    var_( NULL ),
    has_variable_( false )
{
    registerParameter( "dbo_type", &dbo_type_, (int)DBO_UNDEFINED );
    registerParameter( "id", &id_, "" );
    registerParameter( "data_type", &data_type_, (int)P_TYPE_INT );
    registerParameter( "has_variable", &has_variable_, false );
}

/**
Copy constructor.
@param cpy Instance to copy.
 */
TransformationVariablePropertyEntry::TransformationVariablePropertyEntry( const TransformationVariablePropertyEntry& cpy )
{
    dbo_type_ = cpy.dbo_type_;
    id_ = cpy.id_;
    data_type_ = cpy.data_type_;
    var_ = cpy.var_;
    has_variable_ = cpy.has_variable_;
}

/**
Destructor.
 */
TransformationVariablePropertyEntry::~TransformationVariablePropertyEntry()
{
}

/**
Retrieves a Configuration with the given data filled in. Use this method for your convenience when generating a Configurable.
@param config The config the data is filled in.
@param dbo_type DBO type assigned to the entry.
@param property The entries Property.
@param is_var Flag if the Property was added to the TransformationVariable from a DBOVariable.
 */
void TransformationVariablePropertyEntry::getConfig( Configuration& config, DB_OBJECT_TYPE dbo_type, const Property& prop, bool is_var )
{
    config.addParameterInt( "dbo_type", (int)dbo_type );
    config.addParameterString( "id", prop.id_ );
    config.addParameterInt( "data_type", (int)prop.data_type_int_ );
    config.addParameterBool( "has_variable", is_var );
}

/**
Retrieves a Configuration with the given data filled in. Use this method for your convenience when generating a Configurable.
@param config The config the data is filled in.
@param dbo_type DBO type assigned to the entry.
@param id Property string identifier.
 */
void TransformationVariablePropertyEntry::getConfig( Configuration& config, DB_OBJECT_TYPE dbo_type, const std::string& id )
{
    config.addParameterInt( "dbo_type", (int)dbo_type );
    config.addParameterString( "id", id );
    config.addParameterInt( "data_type", (int)P_TYPE_INT );
    config.addParameterBool( "has_variable", false );
}

/**
Sets the entries data.
@param dbo_type DBO type assigned to the entry.
@param property The entries Property.
@param is_var Flag if the Property was added to the TransformationVariable from a DBOVariable.
 */
void TransformationVariablePropertyEntry::setup( DB_OBJECT_TYPE dbo_type,
        const Property& property,
        DBOVariable* var )
{
    dbo_type_ = dbo_type;
    id_ = property.id_;
    data_type_ = property.data_type_int_;
    var_ = var;
    has_variable_ = ( var != NULL );
}

/**
Returns the DBO type that is assigned to the entry.
@return DBO type that is assigned to the entry.
 */
DB_OBJECT_TYPE TransformationVariablePropertyEntry::getDBOType() const
{
    return (DB_OBJECT_TYPE)dbo_type_;
}

/**
Returns the Property that is stored in the entry.
@return Property that is stored in the entry.
 */
Property TransformationVariablePropertyEntry::getProperty() const
{
    return Property( id_, (PROPERTY_DATA_TYPE)data_type_ );
}

/**
Returns the DBOVariable that yielded the entry. Lazy loaded.
@return DBOVariable that yielded the entry.
 */
DBOVariable* TransformationVariablePropertyEntry::getVariable()
{
    if( has_variable_ && !var_ )
    {
        if( !DBObjectManager::getInstance().existsDBOVariable( (DB_OBJECT_TYPE)dbo_type_, id_ ) )
            throw std::runtime_error( "TransformationVariablePropertyEntry: getVariable: Saved property is not a dbo variable." );
        var_ = DBObjectManager::getInstance().getDBOVariable( (DB_OBJECT_TYPE)dbo_type_, id_ );
    }

    return var_;
}

/**************************************************************************
TransformationVariable
 ***************************************************************************/

/**
Constructor.
With this constructor the data type check will be disabled by default.
@param name Name of the variable.
 */
TransformationVariable::TransformationVariable( const std::string& name )
:   name_( name ),
    default_prop_( NULL ),
    optional_( false ),
    data_type_( P_TYPE_INT ),
    check_data_type_( false )
{
}

/**
Constructor.
With this constructor the data type check will be enabled by default. The default Property id
will be used together with the variables data type to set up a default Property.
@param name Name of the variable.
@param data_type The variables data type.
@param default_id Optional default Property id.
 */
TransformationVariable::TransformationVariable( const std::string& name, PROPERTY_DATA_TYPE data_type, const std::string& default_id )
:   name_( name ),
    default_prop_( NULL ),
    optional_( false ),
    data_type_( data_type ),
    check_data_type_( true )
{
    if( !default_id.empty() )
        default_prop_ = new TransformationVariablePropertyEntry( DBO_UNDEFINED, Property( default_id, data_type ), NULL );
}

/**
Configurable constructor.
Don't forget to set initial values for the registered parameters through the Configuration ( use getConfig() for convenience ).
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
 */
TransformationVariable::TransformationVariable( std::string class_id, std::string instance_id, Configurable *parent )
:   Configurable( class_id, instance_id, parent ),
    name_( "" ),
    default_prop_( NULL ),
    optional_( false ),
    data_type_( P_TYPE_INT ),
    check_data_type_( false )
{
    registerParameter( "name", &name_, "" );
    registerParameter( "optional", &optional_, false );
    registerParameter( "data_type", &data_type_, (int)P_TYPE_INT );
    registerParameter( "check_data_type", &check_data_type_, false );

    createSubConfigurables();
}

/**
Copy constructor.
@param cpy Instance to copy.
 */
TransformationVariable::TransformationVariable( const TransformationVariable& cpy )
:   default_prop_( NULL )
{
    name_ = cpy.name_;
    if( cpy.default_prop_ )
        default_prop_ = new TransformationVariablePropertyEntry( *(cpy.default_prop_) );

    PropertyMap::const_iterator it, itend = cpy.properties_.end();
    for( it=cpy.properties_.begin(); it!=itend; ++it )
        properties_[ it->first ] = new TransformationVariablePropertyEntry( *(it->second) );

    optional_ = cpy.optional_;
    data_type_ = cpy.data_type_;
    check_data_type_ = cpy.check_data_type_;
}

/**
Destructor.
 */
TransformationVariable::~TransformationVariable()
{
    deleteEntries();
}

/**
Retrieves a Configuration with the given data filled in. Use this method for your convenience when generating a Configurable.
@param config The config the data is filled in.
@param name Name of the variable.
 */
void TransformationVariable::getConfig( Configuration& config, const std::string& name )
{
    config.addParameterString( "name", name );
    config.addParameterBool( "optional", false );
    config.addParameterInt( "data_type", (int)P_TYPE_INT );
    config.addParameterBool( "check_data_type", false );
}

/**
Retrieves a Configuration with the given data filled in. Use this method for your convenience when generating a Configurable.
@param config The config the data is filled in.
@param name Name of the variable.
@param data_type The variables data type.
@param default_id Optional default Property id.
 */
void TransformationVariable::getConfig( Configuration& config,
        const std::string& name,
        PROPERTY_DATA_TYPE data_type,
        const std::string& default_id )
{
    config.addParameterString( "name", name );
    config.addParameterBool( "optional", false );
    config.addParameterInt( "data_type", (int)data_type );
    config.addParameterBool( "check_data_type", true );

    //add default property as a new subconfiguration
    if( !default_id.empty() )
    {
        Configuration& def_config = config.addNewSubConfiguration( "TransformationVariablePropertyEntryDefault",
                "TransformationVariablePropertyEntryDefault0" );
        TransformationVariablePropertyEntry::getConfig( def_config, DBO_UNDEFINED, Property( default_id, data_type ), NULL );
    }
}

/**
Deletes all properties including the default property.
 */
void TransformationVariable::deleteEntries()
{
    clearProperties();

    if( default_prop_ )
    {
        delete default_prop_;
        default_prop_ = NULL;
    }
}

/**
Adds a new Property for a specific DBO type.
@param dbo_type DBO type the Property is assigned to.
@param prop The Property to be added.
@param var Optional pointer to the DBOVariable that yielded the entry.
 */
void TransformationVariable::addProperty( DB_OBJECT_TYPE dbo_type, const Property& prop, DBOVariable* var )
{
    //check data type consistency if activated
    if( check_data_type_ && ( (signed)prop.data_type_int_ != data_type_ ) )
        throw std::runtime_error( "TransformationVariable: addProperty: Wrong Data Type, name=" + name_ + ", id=" + prop.id_ );

    //if not added from a variable check if the name is not used by another variable
    if( !var && existsDBOVariable( dbo_type, prop.id_ ) )
        throw std::runtime_error( "TransformationVariable: addProperty: DBOVariable already exists, name=" + name_ + ", id=" + prop.id_ );

    if( unusable_ )
    {
        if( properties_.find( dbo_type ) != properties_.end() )
            delete properties_[ dbo_type ];
        properties_[ dbo_type ] = new TransformationVariablePropertyEntry( dbo_type, prop , var );
    }
    else
    {
        Configuration &config = addNewSubConfiguration( "TransformationVariablePropertyEntry" );
        TransformationVariablePropertyEntry::getConfig( config, dbo_type, prop, var!=NULL );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );
    }
}

/**
Adds a new Property for a specific DBO type. The data type will be inferred from the variables internal data type.
Data type checking must be enabled.
@param dbo_type DBO type the Property is assigned to.
@param id String identifier of the new Property.
 */
void TransformationVariable::addProperty( DB_OBJECT_TYPE dbo_type, const std::string& id )
{
    //no data type provided...
    if( !check_data_type_ )
        throw std::runtime_error( "TransformationVariable: addProperty: No data type provided." );

    //check if the name is not used by another variable
    if( existsDBOVariable( dbo_type, id ) )
        throw std::runtime_error( "TransformationVariable: addProperty: DBOVariable already exists, name=" + name_ + ", id=" + id );

    if( unusable_ )
    {
        if( properties_.find( dbo_type ) != properties_.end() )
            delete properties_[ dbo_type ];
        properties_[ dbo_type ] = new TransformationVariablePropertyEntry( dbo_type, Property( id, (PROPERTY_DATA_TYPE)data_type_ ), NULL );
    }
    else
    {
        std::string name = "TransformationVariablePropertyEntry_" + id + "0";

        Configuration &config = addNewSubConfiguration( "TransformationVariablePropertyEntry", name );
        TransformationVariablePropertyEntry::getConfig( config, dbo_type, Property( id, (PROPERTY_DATA_TYPE)data_type_ ), false );
        generateSubConfigurable( "TransformationVariablePropertyEntry", name );
    }
}

/**
Adds a one or more new properties from the given DBOVariable.
If the variable is meta, the Property of every subvariable will be added.
@param var DBOVariable to add the properties from.
 */
void TransformationVariable::addProperty( DBOVariable* var )
{
    assert( var );

    if( var->isMetaVariable() )
    {
        std::map<DB_OBJECT_TYPE,std::string>& subs = var->getSubVariables();
        std::map<DB_OBJECT_TYPE,std::string>::iterator it, itend = subs.end();
        for( it=subs.begin(); it!=itend; ++it )
        {
            if( !DBObjectManager::getInstance().existsDBOVariable( it->first, it->second ) )
                continue;

            //get the subvariable and store it with the Property
            DBOVariable* subvar = DBObjectManager::getInstance().getDBOVariable( it->first, it->second );
            addProperty( it->first, Property( it->second, (PROPERTY_DATA_TYPE)var->data_type_int_ ), subvar );
        }
        return;
    }

    //not meta, just add and store the variable
    addProperty( var->getDBOType(), *var, var );
}

/**
Sets the variables properties from the given DBOVariable.
Basically the same as addProperty(), but clears the entries before.
@param var DBOVariable to set the properties from.
 */
void TransformationVariable::setProperties( DBOVariable* var )
{
    assert( var );
    clearProperties();
    addProperty( var );
}

/**
Removes the Property assigned to the given DBO type.
@param dbo_type DBO type of the entry that should be removed.
 */
void TransformationVariable::removeProperty( DB_OBJECT_TYPE dbo_type )
{
    if( properties_.find( dbo_type ) == properties_.end() )
        throw std::runtime_error( "TransformationVariable: removeProperty: DBO Type not found." );
    delete properties_[ dbo_type ];
    properties_.erase( dbo_type );
}

/**
Clears all added properties.
 */
void TransformationVariable::clearProperties()
{
    PropertyMap::iterator it, itend = properties_.end();
    for( it=properties_.begin(); it!=itend; ++it )
        delete it->second;
    properties_.clear();
}

/**
Returns the variables name.
@return The variables name.
 */
const std::string& TransformationVariable::name() const
{
    return name_;
}

/**
Checks if the variable obtains a Property for the given DBO type.
@param dbo_type DBO type that should be checked.
@return True if a Property exists for the given DBO type, otherwise false.
 */
bool TransformationVariable::hasProperty( DB_OBJECT_TYPE dbo_type ) const
{
    return ( properties_.find( dbo_type ) != properties_.end() || default_prop_ != NULL );
}

/**
Returns the property that is assigned the given DBO type.
@param DBO type assigned to the desired Property entry.
@return Property that is assigned the given DBO type.
 */
Property TransformationVariable::getProperty( DB_OBJECT_TYPE dbo_type )
{
    if( properties_.find( dbo_type ) == properties_.end() )
    {
        if( !default_prop_ )
            throw std::runtime_error( "TransformationVariable: getProperty: No property set for dbo type." );
        return default_prop_->getProperty();
    }

    assert( properties_[ dbo_type ] );

    return properties_[ dbo_type ]->getProperty();
}

/**
Returns the correct Property realization of the variable in the given buffer.
Note that additionally to the name, the data type is also checked.
@param buffer Buffer to return the Property from.
@return Property realization of the variable in the given buffer. May return NULL if not found.
 */
Property* TransformationVariable::property( Buffer* buffer )
{
    assert( buffer );

    DB_OBJECT_TYPE type = buffer->getDBOType();
    PropertyList* props = buffer->getPropertyList();
    Property prop;
    int idx;

    //check property entries first
    if( properties_.find( type ) != properties_.end() )
    {
        assert( properties_[ type ] );

        prop = properties_[ type ]->getProperty();
        logdbg << "TransformationVariable: property; prop found '" << prop.id_ << "'";

        if( props->hasProperty( prop.id_ ) )
        {
            idx = props->getPropertyIndex( prop.id_ );

            //check data type
            if( props->getProperty( idx )->data_type_int_ == prop.data_type_int_ )
                return props->getProperty( idx );
            else
                logerr << "TransformationVariable: property: prop found in buffer but wrong data type "
                << props->getProperty( idx )->data_type_int_ << " instead of " << prop.data_type_int_;
        }
        else
            logerr << "TransformationVariable: property: prop not found in buffer (prop size " << props->getNumProperties()
            << ")";
    }

    if(default_prop_ )
    {
        //no entry found, check default prop
        Property def = default_prop_->getProperty();
        logdbg << "TransformationVariable: property; def prop found '" << def.id_ << "'";

        if( props->hasProperty( def.id_ ) )
        {
            idx = props->getPropertyIndex( def.id_ );

            //check data type
            if( props->getProperty( idx )->data_type_int_ == def.data_type_int_ )
                return props->getProperty( idx );
            else
                logerr << "TransformationVariable: property: def prop found in buffer but wrong data type "
                << props->getProperty( idx )->data_type_int_ << " instead of " << def.data_type_int_;
        }
    }
    logerr << "TransformationVariable: property: DBO type " << type << " not found";

    return NULL;
}

/**
Returns the index to the correct Property realization of the variable in the given buffer.
Note that additionally to the name, the data type is also checked.
@param buffer Buffer to return the Property index from.
@return Index to the correct Property realization of the variable in the given buffer. May return -1 if not found.
 */
int TransformationVariable::propertyIndex( Buffer* buffer )
{
    assert( buffer );

    DB_OBJECT_TYPE type = buffer->getDBOType();
    PropertyList* props = buffer->getPropertyList();
    Property prop;
    int idx;

    //check property entries first
    if( properties_.find( type ) != properties_.end() )
    {
        assert( properties_[ type ] );

        prop = properties_[ type ]->getProperty();
        if( props->hasProperty( prop.id_ ) )
        {
            idx = props->getPropertyIndex( prop.id_ );

            //check data type
            if( props->getProperty( idx )->data_type_int_ == prop.data_type_int_ )
                return idx;
        }
    }

    if( !default_prop_ )
        return -1;

    //no entry found, check default prop
    Property def = default_prop_->getProperty();
    if( props->hasProperty( def.id_ ) )
    {
        int idx = props->getPropertyIndex( def.id_ );

        //check data type
        if( props->getProperty( idx )->data_type_int_ == def.data_type_int_ )
            return idx;
    }

    return -1;
}

/**
Returns all added properties.
@return Added Property realizations.
 */
const TransformationVariable::PropertyMap& TransformationVariable::getProperties() const
{
    return properties_;
}

/**
Sets a default Property. The default property will be used if no specific entry can be found for a buffer.
@param prop The new default Property.
 */
void TransformationVariable::setDefaultProperty( const Property& prop )
{
    //check data type if enabled
    if( check_data_type_ && (signed)prop.data_type_int_ != data_type_ )
        throw std::runtime_error( "TransformationVariable: setDefaultProperty: Wrong Data Type." );

    if( unusable_ )
    {
        if( default_prop_ )
            delete default_prop_;
        default_prop_ = new TransformationVariablePropertyEntry( DBO_UNDEFINED, prop, NULL );
    }
    else
    {
        std::string name = "TransformationVariablePropertyEntryDefault0";

        Configuration &config = addNewSubConfiguration( "TransformationVariablePropertyEntryDefault", name );
        TransformationVariablePropertyEntry::getConfig( config, DBO_UNDEFINED, prop, false );
        generateSubConfigurable( "TransformationVariablePropertyEntryDefault", name );
    }
}

/**
Sets a default Property. The default property will be used if no specific entry can be found for a buffer.
In this version the internal data type of the variable will be used. Data type check needs to be enabled.
@param id Default Property string identifier.
 */
void TransformationVariable::setDefaultProperty( const std::string& id )
{
    //No data type provided...
    if( !check_data_type_ )
        throw std::runtime_error( "TransformationVariable: setDefaultProperty: No data type provided." );

    if( unusable_ )
    {
        if( default_prop_ )
            delete default_prop_;
        default_prop_ = new TransformationVariablePropertyEntry( DBO_UNDEFINED, Property( id, (PROPERTY_DATA_TYPE)data_type_ ), NULL );
    }
    else
    {
        std::string name = "TransformationVariablePropertyEntryDefault0";

        Configuration &config = addNewSubConfiguration( "TransformationVariablePropertyEntryDefault", name );
        TransformationVariablePropertyEntry::getConfig( config, DBO_UNDEFINED, Property( id, (PROPERTY_DATA_TYPE)data_type_ ), NULL );
        generateSubConfigurable( "TransformationVariablePropertyEntryDefault", name );
    }
}

/**
Sets a default Property. The default property will be used if no specific entry can be found for a buffer.
In this version the Proeprty is assigned from a DBOVariable. The variable has to be of non-meta type.
@param var DBOVariable to set the default Property from. Can be used to unset the default Property by providing NULL.
 */
void TransformationVariable::setDefaultProperty( DBOVariable* var )
{
    //unset default property
    if( !var )
    {
        if( default_prop_ )
        {
            delete default_prop_;
            default_prop_ = NULL;
        }
        return;
    }

    //no metas allowed
    if( var->isMetaVariable() )
        throw std::runtime_error( "TransformationVariable: setDefaultProperty: Cannot set from DBO meta variable." );

    //check data type if needed
    if( check_data_type_ && (signed)var->data_type_int_ != data_type_ )
        throw std::runtime_error( "TransformationVariable: setDefaultProperty: Wrong Data Type." );

    if( unusable_ )
    {
        if( default_prop_ )
            delete default_prop_;
        default_prop_ = new TransformationVariablePropertyEntry( DBO_UNDEFINED, *var, NULL );
    }
    else
    {
        std::string name = "TransformationVariablePropertyEntryDefault0";

        Configuration &config = addNewSubConfiguration( "TransformationVariablePropertyEntryDefault", name );
        TransformationVariablePropertyEntry::getConfig( config, DBO_UNDEFINED, *var, false );
        generateSubConfigurable( "TransformationVariablePropertyEntryDefault", name );
    }
}

/**
Retrieves the default Property.
@param prop The Property to store the default Property to.
@return True if the default Property could be retreived, otherwise false.
 */
bool TransformationVariable::getDefaultProperty( Property& prop ) const
{
    if( !default_prop_ )
        return false;

    prop = default_prop_->getProperty();
    return true;
}

/**
Sets/unsets the variable optional.
@param optional Optional if true, non-optional if false.
 */
void TransformationVariable::setOptional( bool optional )
{
    optional_ = optional;
}

/**
Checks if this variable is marked as optional.
@return True if the variable is optional, otherwise false.
 */
bool TransformationVariable::isOptional() const
{
    return optional_;
}

/**
Returns the data type of this variable.
@return The variables data type.
 */
PROPERTY_DATA_TYPE TransformationVariable::dataType() const
{
    return (PROPERTY_DATA_TYPE)data_type_;
}

/**
Checks if the data type check is enabled.
@return True if data type check is enabled, otherwise false.
 */
bool TransformationVariable::dataTypeChecked() const
{
    return check_data_type_;
}

/**
Prints the variables properties.
 */
void TransformationVariable::print()
{
    PropertyMap::iterator it, itend = properties_.end();
    for( it=properties_.begin(); it!=itend; ++it )
        loginf << "Variable " << name_.c_str() << ": dbo_type=" << (int)it->first << ", id=" << it->second->getProperty().id_.c_str();
}

/**
Collects all DBO variables that were used to set the variables properties.
@param varset A set to insert the DBO variables.
 */
void TransformationVariable::collectDBOVariables( DBOVariableSet& varset ) const
{
    PropertyMap::const_iterator it, itend = properties_.end();
    for( it=properties_.begin(); it!=itend; ++it )
    {
        DBOVariable* var = it->second->getVariable();
        if( !var )
            continue;
        varset.add( var );
    }
}

/**
Checks if the given DBOVariable exists in the DBO manager.
@param dbo_type The DBOVariable's DBO type.
@param id The DBOVariable's string identifier.
@return True if the DBOVariable exists in the DBO manager, otherwise false.
 */
bool TransformationVariable::existsDBOVariable( DB_OBJECT_TYPE dbo_type, const std::string& id )
{
    return ( DBObjectManager::getInstance().existsDBOVariable( dbo_type, id ) );
}

/**
Sets the variables data type.
If the variable is not cleared after the data type change, the stored properties may
be checked against the new data type, which may result in an exception.
@param data_type New data type.
@param clear If true the variable will be emptied after the data type change.
 */
void TransformationVariable::setDataType( PROPERTY_DATA_TYPE data_type, bool clear )
{
    if( data_type_ == data_type )
        return;

    data_type_ = (int)data_type;

    //easy solution, just clear :b
    if( clear )
    {
        deleteEntries();
        return;
    }

    //check new data type
    if( check_data_type_ && !checkDataTypes() )
        throw std::runtime_error( "TransformationVariable: setDataType: Property data types not matching new data type." );
}

/**
Checks all properties including the default property against the current data type.
@return True if all Property entry data types are consistent, otherwise false.
 */
bool TransformationVariable::checkDataTypes()
{
    //check properties against new data type
    PropertyMap::iterator it, itend = properties_.end();
    for( it=properties_.begin(); it!=itend; ++it )
    {
        if( (signed)it->second->getProperty().data_type_int_ != data_type_ )
            return false;
    }

    //check default property against new data type
    if( default_prop_ && (signed)default_prop_->getProperty().data_type_int_ != data_type_ )
        return false;

    return true;
}

/**
Enables/disables the data type check for newly added properties.
If enabled, newly added properties will be checked on consistency with the variables current data type.
@param enable If true enables, if false disables check.
 */
void TransformationVariable::enableDataTypeCheck( bool enable )
{
    check_data_type_ = enable;

    //if enabled, check data types
    if( check_data_type_ && !checkDataTypes() )
        throw std::runtime_error( "TransformationVariable: enableDataTypeCheck: Property data types not matching data type." );
}

/**
 */
void TransformationVariable::generateSubConfigurable( std::string class_id, std::string instance_id )
{
    if( class_id == "TransformationVariablePropertyEntry" )
    {
        TransformationVariablePropertyEntry* entry = new TransformationVariablePropertyEntry( class_id, instance_id, this );
        DB_OBJECT_TYPE dbo_type = entry->getDBOType();
        if( properties_.find( dbo_type ) != properties_.end() )
            delete properties_[ dbo_type ];
        properties_[ dbo_type ] = entry;
    }

    if( class_id == "TransformationVariablePropertyEntryDefault" )
    {
        if( default_prop_ )
            delete default_prop_;
        default_prop_ = new TransformationVariablePropertyEntry( class_id, instance_id, this );
    }
}

/**
 */
void TransformationVariable::checkSubConfigurables()
{
}

/**************************************************************************
TransformationVariables
 ***************************************************************************/

/**
Constructor.
 */
TransformationVariables::TransformationVariables()
{
}

/**
Configurable constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
 */
TransformationVariables::TransformationVariables( std::string class_id, std::string instance_id, Configurable *parent )
:   Configurable( class_id, instance_id, parent )
{
    createSubConfigurables();
}

/**
Copy constructor.
@param cpy Instance to copy.
 */
TransformationVariables::TransformationVariables( const TransformationVariables& cpy )
{
    *this = cpy;
}

/**
Destructor.
 */
TransformationVariables::~TransformationVariables()
{
    clearVariables();
}

/**
Assignment operator.
@param rhs Assigned instance.
@return Reference to me.
 */
TransformationVariables& TransformationVariables::operator=( const TransformationVariables& rhs )
{
    clearVariables();

    unsigned int i, n = rhs.numberOfVariables();
    for( i=0; i<n; ++i )
        addVariable( (const_cast<TransformationVariables*>(&rhs))->getVariable( i ) );
    return *this;
}

/**
Returns the number of stored variables.
@return Number of stored variables.
 */
unsigned int TransformationVariables::numberOfVariables() const
{
    return vars_.size();
}

/**
Returns the idx'th variable.
@param idx Index of the desired variable.
@return The desired variable.
 */
TransformationVariable* TransformationVariables::getVariable( int idx )
{
    if( idx < 0 || idx >= (signed)vars_.size() )
        throw std::runtime_error( "TransformationVariables: getVariable: Out of range." );

    assert( vars_[ idx ] );
    return vars_[ idx ];
}

/**
Returns the variable of the given name.
@param name Name of the desired variable.
@return The desired variable.
 */
TransformationVariable* TransformationVariables::getVariable( const std::string& name )
{
    if( map_.find( name ) == map_.end() )
        throw std::runtime_error( "TransformationVariables: getVariable: Name not found: " + name );

    assert( map_[ name ] );
    return map_[ name ];
}

/**
Adds a new variable of the given name.
@param name Name of the new variable.
@return The new variable.
 */
TransformationVariable* TransformationVariables::addVariable( const std::string& name )
{
    if( map_.find( name ) != map_.end() )
        throw std::runtime_error( "TransformationVariables: addVariable: Duplicate name." );

    if( unusable_ )
    {
        TransformationVariable* var = new TransformationVariable( name );
        map_[ name ] = var;
        vars_.push_back( var );
    }
    else
    {
        std::string config_name = "TransformationVariable_" + name;

        Configuration &config = addNewSubConfiguration( "TransformationVariable", config_name );
        TransformationVariable::getConfig( config, name );
        generateSubConfigurable( "TransformationVariable", config_name );
    }

    return map_[ name ];
}

/**
Adds the given variable by copying it.
@param var The variable to copy.
@return Pointer to the copied variable.
 */
TransformationVariable* TransformationVariables::addVariable( TransformationVariable* var )
{
    assert( var );

    if( map_.find( var->name() ) != map_.end() )
        throw std::runtime_error( "TransformationVariables: addVariable: Duplicate name." );

    TransformationVariable* cpy = new TransformationVariable( *var );
    map_[ var->name() ] = cpy;
    vars_.push_back( cpy );

    return cpy;
}

/**
Adds a new variable.
@param name Name of the new variable.
@param data_type Data type of the new variable.
@param default_id Default Property id of the new variable.
 */
TransformationVariable* TransformationVariables::addVariable( const std::string& name, PROPERTY_DATA_TYPE data_type, const std::string& default_id )
{
    if( map_.find( name ) != map_.end() )
        throw std::runtime_error( "TransformationVariables: addVariable: Duplicate name." );

    if( unusable_ )
    {
        TransformationVariable* var = new TransformationVariable( name, data_type, default_id );
        map_[ name ] = var;
        vars_.push_back( var );
    }
    else
    {
        std::string config_name = "TransformationVariable_" + name;

        Configuration &config = addNewSubConfiguration( "TransformationVariable", config_name );
        TransformationVariable::getConfig( config, name, data_type, default_id );
        generateSubConfigurable( "TransformationVariable", config_name );
    }

    return map_[ name ];
}

/**
Removes and deletes the variable of the given name.
@param name Name of the variable to be deleted.
 */
void TransformationVariables::removeVariable( const std::string& name )
{
    if( map_.find( name ) == map_.end() )
        throw std::runtime_error( "TransformationVariables: removeVariable: Name not found." );

    TransformationVariable* var = map_[ name ];

    map_.erase( name );

    //find and erase var
    int i, n = vars_.size();
    for( i=0; i<n; ++i )
    {
        if( vars_[ i ] == var )
        {
            vars_.erase( vars_.begin() + i );
            break;
        }
    }

    delete var;
}

/**
Clears all variables.
 */
void TransformationVariables::clearVariables()
{
    unsigned int i, n = vars_.size();
    for( i=0; i<n; ++i )
        delete vars_[ i ];
    vars_.clear();
    map_.clear();
}

/**
Checks is a variable of the given name exists in the container.
@param name Name of the variable to check for.
@return True if such a variable could be found, false otherwise.
 */
bool TransformationVariables::exists( const std::string& name )
{
    return ( map_.find( name ) != map_.end() );
}

/**
 */
void TransformationVariables::generateSubConfigurable( std::string class_id, std::string instance_id )
{
    if( class_id == "TransformationVariable" )
    {
        TransformationVariable* var = new TransformationVariable( class_id, instance_id, this );
        const std::string& name = var->name();

        if( map_.find( var->name() ) != map_.end() )
            throw std::runtime_error( "TransformationVariables: generateSubConfigurable: Duplicate name." );

        map_[ name ] = var;
        vars_.push_back( var );
    }
}

/**
 */
void TransformationVariables::checkSubConfigurables()
{
}
