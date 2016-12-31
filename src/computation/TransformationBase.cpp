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

#include "TransformationBase.h"
#include "PropertyList.h"
#include "Buffer.h"


/**
Constructor.
  */
TransformationBase::TransformationBase()
{
    //create variables
    vars_in_     = new TransformationVariables;
    vars_out_    = new TransformationVariables;
    vars_hidden_ = new TransformationVariables;
}

/**
Configurable constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
  */
TransformationBase::TransformationBase( std::string class_id, std::string instance_id, Configurable *parent )
:   Configurable( class_id, instance_id, parent ),
    vars_in_( NULL ),
    vars_out_( NULL ),
    vars_hidden_( NULL )
{
    //call createSubconfigurables() from the derived class!
}

/**
Copy constructor.
@param cpy Instance to copy.
  */
TransformationBase::TransformationBase( const TransformationBase& cpy )
{
    *this = cpy;
}

/**
Destructor.
  */
TransformationBase::~TransformationBase()
{
    if( vars_in_ )
        delete vars_in_;
    if( vars_out_ )
        delete vars_out_;
    if( vars_hidden_ )
        delete vars_hidden_;
}

/**
Assignment operator.
@param rhs Instance to assign.
@return Reference to me.
  */
TransformationBase& TransformationBase::operator=( const TransformationBase& rhs )
{
    if( vars_in_ )
        delete vars_in_;
    if( vars_out_ )
        delete vars_out_;
    if( vars_hidden_ )
        delete vars_hidden_;
    vars_in_     = new TransformationVariables( *(rhs.vars_in_)     );
    vars_out_    = new TransformationVariables( *(rhs.vars_out_)    );
    vars_hidden_ = new TransformationVariables( *(rhs.vars_hidden_) );

    return *this;
}

/**
Return the number of input transformation variables.
@return Number of input transformation variables.
  */
unsigned int TransformationBase::numberInputVariables() const
{
    return vars_in_->numberOfVariables();
}

/**
Returns the idx'th input transformation variable.
@param idx Index of the desired input transformation variable.
@return The desired input transformation variable.
  */
TransformationVariable* TransformationBase::getInputVariable( int idx )
{
    return vars_in_->getVariable( idx );
}

/**
Returns the input transformation variable of the given name.
@param name Name of the desired input transformation variable.
@return The desired input transformation variable.
  */
TransformationVariable* TransformationBase::getInputVariable( const std::string& name )
{
    return vars_in_->getVariable( name );
}

/**
Sets the input transformation variables.
@param vars Set of input transformation variables.
  */
void TransformationBase::setInputVariables( const TransformationVariables& vars )
{
    *vars_in_ = vars;
}

/**
Return the number of output transformation variables.
@return Number of output transformation variables.
  */
unsigned int TransformationBase::numberOutputVariables() const
{
    return vars_out_->numberOfVariables();
}

/**
Returns the idx'th output transformation variable.
@param idx Index of the desired output transformation variable.
@return The desired output transformation variable.
  */
TransformationVariable* TransformationBase::getOutputVariable( int idx )
{
    return vars_out_->getVariable( idx );
}

/**
Returns the output transformation variable of the given name.
@param name Name of the desired output transformation variable.
@return The desired output transformation variable.
  */
TransformationVariable* TransformationBase::getOutputVariable( const std::string& name )
{
    return vars_out_->getVariable( name );
}

/**
Sets the input transformation variables.
@param vars Set of output transformation variables.
  */
void TransformationBase::setOutputVariables( const TransformationVariables& vars )
{
    *vars_out_ = vars;
}

/**
Return the number of hidden transformation variables.
@return Number of hidden transformation variables.
  */
unsigned int TransformationBase::numberHiddenVariables() const
{
    return vars_hidden_->numberOfVariables();
}

/**
Returns the idx'th hidden transformation variable.
@param idx hidden of the desired input transformation variable.
@return The desired hidden transformation variable.
  */
TransformationVariable* TransformationBase::getHiddenVariable( int idx )
{
    return vars_hidden_->getVariable( idx );
}

/**
Returns the hidden transformation variable of the given name.
@param name Name of the desired hidden transformation variable.
@return The desired hidden transformation variable.
  */
TransformationVariable* TransformationBase::getHiddenVariable( const std::string& name )
{
    return vars_hidden_->getVariable( name );
}

/**
Adds a new input transformation variable of the given name.
@param name Name of the new variable.
@return The new variable.
  */
TransformationVariable* TransformationBase::addInputVariable( const std::string& name )
{
    return vars_in_->addVariable( name );
}

/**
Adds a new input transformation variable.
@param name Name of the new variable.
@param data_type Data type of the new variable.
@param default_id Optional default property string identifier.
@return The new variable.
  */
TransformationVariable* TransformationBase::addInputVariable( const std::string& name,
                                                           PropertyDataType data_type,
                                                           const std::string& default_id )
{
    return vars_in_->addVariable( name, data_type, default_id );
}

/**
Adds a new output transformation variable of the given name.
@param name Name of the new variable.
@return The new variable.
  */
TransformationVariable* TransformationBase::addOutputVariable( const std::string& name )
{
    return vars_out_->addVariable( name );
}

/**
Adds a new output transformation variable.
@param name Name of the new variable.
@param data_type Data type of the new variable.
@param default_id Optional default property string identifier.
@return The new variable.
  */
TransformationVariable* TransformationBase::addOutputVariable( const std::string& name,
                                                            PropertyDataType data_type,
                                                            const std::string& default_id )
{
    return vars_out_->addVariable( name, data_type, default_id );
}


/**
Adds a new hidden transformation variable of the given name.
@param name Name of the new variable.
@return The new variable.
  */
TransformationVariable* TransformationBase::addHiddenVariable( const std::string& name )
{
    return vars_hidden_->addVariable( name );
}

/**
Adds a new hidden transformation variable.
@param name Name of the new variable.
@param data_type Data type of the new variable.
@param default_id Optional default property string identifier.
@return The new variable.
  */
TransformationVariable* TransformationBase::addHiddenVariable( const std::string& name,
                                                               PropertyDataType data_type,
                                                               const std::string& default_id )
{
    return vars_hidden_->addVariable( name, data_type, default_id );
}

/**
Reads all needed input properties from the given buffer. Also reads some additional information
for later usage of the quick access methods.
@param buffer Buffer to read the properties from.
  */
void TransformationBase::createInputProperties( Buffer* buffer )
{
    assert( buffer );

    keys_in_.clear();
    indices_in_.clear();
    properties_in_.clear();

    const PropertyList &p  = buffer->properties();

    int idx;
    unsigned int i, n = vars_in_->numberOfVariables();
    for( i=0; i<n; ++i )
    {
        TransformationVariable* var = vars_in_->getVariable( i );
        idx = var->propertyIndex( buffer );
        const std::string& name = var->name();
        keys_in_[ name ] = i;
        indices_in_.push_back( idx );

        //TODO WHAT THE HELL IS HAPPENING?
        assert (false);
//        if( idx < 0 )
//            properties_in_.push_back( NULL );   //store NULL pointer if not existing in buffer
//        else
//            properties_in_.push_back( p->getProperty( idx ) );
    }
}

/**
Reads all needed output properties from the given buffer. Also reads some additional information
for later usage of the quick access methods.
@param buffer Buffer to read the properties from.
  */
void TransformationBase::createOutputProperties( Buffer* buffer )
{
    assert( buffer );

    keys_out_.clear();
    indices_out_.clear();
    properties_out_.clear();

    const PropertyList& p = buffer->properties();

    int idx;
    unsigned int i, n = vars_out_->numberOfVariables();
    for( i=0; i<n; ++i )
    {
        TransformationVariable* var = vars_out_->getVariable( i );
        idx = var->propertyIndex( buffer );
        const std::string& name = var->name();
        keys_out_[ name ] = i;
        indices_out_.push_back( idx );

        //TODO WHAT THE HELL IS HAPPENING?
        assert (false);

//        if( idx < 0 )
//            properties_out_.push_back( NULL );   //store NULL pointer if not existing in buffer
//        else
//            properties_out_.push_back( p->getProperty( idx ) );
    }
}


/**
Reads all needed hidden properties from the given buffer. Also reads some additional information
for later usage of the quick access methods.
@param buffer Buffer to read the properties from.
  */
void TransformationBase::createHiddenProperties( Buffer* buffer )
{
    assert( buffer );

    keys_hidden_.clear();
    indices_hidden_.clear();
    properties_hidden_.clear();

    const PropertyList& p = buffer->properties();

    int idx;
    unsigned int i, n = vars_hidden_->numberOfVariables();
    for( i=0; i<n; ++i )
    {
        TransformationVariable* var = vars_hidden_->getVariable( i );
        idx = var->propertyIndex( buffer );
        const std::string& name = var->name();
        keys_hidden_[ name ] = i;
        indices_hidden_.push_back( idx );

        //TODO WHAT THE HELL IS HAPPENING?
        assert (false);

//        if( idx < 0 )
//            properties_hidden_.push_back( NULL );   //store NULL pointer if not existing in buffer
//        else
//            properties_hidden_.push_back( p->getProperty( idx ) );
    }
}

/**
Checks all input variables for existence in the given buffer. This method can be
used to pre-check the validity (availability) of the input variables for the given buffer before
the actual transformation. Optional variables will be left out in this check.
@param buffer Buffer to check the validity of input variables for.
@return True if the input variables are valid for the given buffer, false otherwise.
  */
bool TransformationBase::checkInputVariables( Buffer* buffer )
{
    assert( buffer );

    unsigned int i, n = vars_in_->numberOfVariables();
    for( i=0; i<n; ++i )
    {
        //optional variables are skipped here
        if( !vars_in_->getVariable( i )->isOptional() &&
            !vars_in_->getVariable( i )->hasProperty( buffer ) )
        {
            //DBOVariable *notfound =  vars_in_->getVariable( i )->getVariable();
            logerr << "TransformationBase: checkInputVariables: variable not found: '" << vars_in_->getVariable( i )->name() << "'";
              //      << "' with DBO type " << notfound->getDBOTypeString() << " name " << notfound->getName();
            return false;
        }
    }
    return true;
}

/**
Returns all erroneous input variables in a printable string format. This is for
debug purposes.
@param Buffer to check the validity of input variables for.
@return Input variables that are invalid for the given buffer, returned in a printable string format.
  */
std::string TransformationBase::getErrorVariables( Buffer* buffer )
{
    assert( buffer );

    std::string ret = "";
    unsigned int i, n = vars_in_->numberOfVariables();
    for( i=0; i<n; ++i )
    {
        if( !vars_in_->getVariable( i )->hasProperty( buffer ) &&
            !vars_in_->getVariable( i )->isOptional() )
        {
            ret += "<br>\t" + vars_in_->getVariable( i )->name();
        }
    }
    return ret;
}

/**
Checks if the output variables are present in the given buffer and adds them if needed.
Can be used to automatically check/create the output buffer from the output variables.
If the buffer pointer points to NULL the buffer will be newly created.
This method will also check the data type of the variable and buffer property on equality,
eventually resulting in an exception.
@param output Pointer to a buffer pointer.
@param dbo_type The buffers DBO type. Needed because the buffer may not be created yet.
  */
void TransformationBase::bufferFromOutputVariables( Buffer** output, const std::string &dbo_type )
{
    assert( output );

    TransformationVariable* var;
    unsigned int i, n = vars_out_->numberOfVariables();

    //check if the buffer pointer is NULL, if yes create a buffer and add to it
    if( *output == NULL )
    {
        PropertyList list;
        for( i=0; i<n; ++i )
        {
            var = vars_out_->getVariable( i );
            if( !var->hasProperty( dbo_type ) )
            {
                //Oo variable not set and not optional...
                if( !var->isOptional() )
                    throw std::runtime_error( "TransformationBase: bufferFromOutputVariables: No rule to create buffer." );
                continue;
            }

            Property p = var->getProperty( dbo_type );
            list.addProperty( p );
        }
        *output = new Buffer( list, dbo_type );
        return;
    }

    //buffer exists already, check properties and add if needed
    const PropertyList &props = (*output)->properties();
    for( i=0; i<n; ++i )
    {
        var = vars_out_->getVariable( i );
        if( !var->hasProperty( dbo_type ) )
        {
            //Oo variable not set and not optional...
            if( !var->isOptional() )
                throw std::runtime_error( "TransformationBase: bufferFromOutputVariables: No rule to create buffer." );
            continue;
        }

        Property p = var->getProperty( dbo_type );

        //variable already present in buffer
        if( props.hasProperty( p.getId() ) )
        {
            //variable seems to exist in buffer but try to retrieve it anyways, to check datatype etc.
            if( !var->hasProperty( *output ) )
                throw std::runtime_error( "TransformationBase: bufferFromOutputVariables: Variable in buffer, but not properly configured." );
            continue;
        }

        //needs to be added to buffer
        (*output)->addProperty( p.getId(), p.getDataType() );
    }
}

/**
Adds the hidden variables to the given buffer if not already present.
Call this one e.g. to automatically add the hidden properties to your input buffer.
This method will also check the data type of the variable and buffer property on equality,
eventually resulting in an exception.
@param buffer Buffer to add the hidden variables to.
  */
void TransformationBase::addHiddenProperties( Buffer* buffer )
{
    assert( buffer );

    const std::string &dbo_type = buffer->dboType();
    const PropertyList& props = buffer->properties();

    TransformationVariable* var;
    unsigned int i, n = vars_hidden_->numberOfVariables();
    for( i=0; i<n; ++i )
    {
        var = vars_hidden_->getVariable( i );
        if( !var->hasProperty( dbo_type ) )
        {
            //Oo variable not set and not optional...
            if( !var->isOptional() )
                throw std::runtime_error( "TransformationBase: addHiddenProperties: No rule to add to buffer." );
            continue;
        }

        Property p = var->getProperty( dbo_type );

        //variable already present in buffer
        if( props.hasProperty( p.getId() ) )
        {
            //variable seems to exist in buffer but try to retrieve it anyways, to check datatype etc.
            if( !var->hasProperty( buffer ) )
                throw std::runtime_error( "TransformationBase: addHiddenProperties: Variable in buffer, but not properly configured." );
            continue;
        }

        //needs to be added to buffer
        buffer->addProperty( p.getId(), p.getDataType() );
    }
}

/**
Clears all stored Property data created by the create*Properties() methods
  */
void TransformationBase::resetProperties()
{
    keys_in_.clear();
    keys_out_.clear();
    indices_in_.clear();
    indices_out_.clear();
    properties_in_.clear();
    properties_out_.clear();
//    addresses_in_ = NULL;
//    addresses_out_ = NULL;
}

/**
  */
void TransformationBase::generateSubConfigurable( std::string class_id, std::string instance_id )
{
    if( class_id == "TransformationVarsIn" )
    {
        vars_in_ = new TransformationVariables( class_id, instance_id, this );
    }

    if( class_id == "TransformationVarsOut" )
    {
        vars_out_ = new TransformationVariables( class_id, instance_id, this );
    }

    if( class_id == "TransformationVarsHidden" )
    {
        vars_hidden_ = new TransformationVariables( class_id, instance_id, this );
    }
}

/**
  */
void TransformationBase::checkSubConfigurables()
{
    if( !vars_in_ )
    {
        addNewSubConfiguration( "TransformationVarsIn", "TransformationVarsIn0" );
        generateSubConfigurable( "TransformationVarsIn", "TransformationVarsIn0" );
    }

    if( !vars_out_ )
    {
        addNewSubConfiguration( "TransformationVarsOut", "TransformationVarsOut0" );
        generateSubConfigurable( "TransformationVarsOut", "TransformationVarsOut0" );
    }

    if( !vars_hidden_ )
    {
        addNewSubConfiguration( "TransformationVarsHidden", "TransformationVarsHidden0" );
        generateSubConfigurable( "TransformationVarsHidden", "TransformationVarsHidden0" );
    }
}
