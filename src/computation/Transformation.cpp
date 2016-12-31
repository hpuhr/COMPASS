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
 * Transformation.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: sk
 */

#include "Buffer.h"
#include "Transformation.h"
#include "DBObjectManager.h"
#include "DBOVariable.h"

#include <stdexcept>


/**
Constructor.
Starts in appending mode.
@param input Input buffer.
  */
Transformation::Transformation( Buffer* input )
:   id_( "Transformation" ),
    input_( input ),
    output_( input ),
    ready_buffer_( NULL ),
    append_( true ),
    sustainable_( false ),
    available_( true ),
    state_( TRAFO_OK ),
    error_msg_( "Unknown error" )
{
}

/**
Configurable constructor.
Don't forget to set initial values for the registered parameters through the Configuration.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
  */
Transformation::Transformation( std::string class_id,
                                std::string instance_id,
                                Configurable* parent )
:   TransformationBase( class_id, instance_id, parent ),
    id_( "Transformation" ),
    input_( NULL ),
    output_( NULL ),
    ready_buffer_( NULL ),
    append_( true ),
    sustainable_( false ),
    available_( true ),
    state_( TRAFO_OK ),
    error_msg_( "Unknown error" )
{
    registerParameter( "append", &append_, append_ );
    registerParameter( "sustainable", &sustainable_, sustainable_ );

    //!
    createSubConfigurables();
}

/**
Copy constructor.
@param copy The instance to copy.
  */
Transformation::Transformation( const Transformation& copy )
{
    *this = copy;
}

/**
Nothing gets cleaned up, handle cleanup of the buffers on the outside!
  */
Transformation::~Transformation()
{
}

/**
Assignment operator.
@param rhs Instance to assign.
  */
Transformation& Transformation::operator=( const Transformation& rhs )
{
    TransformationBase::operator=( rhs );

    /// @todo Oo copy pointers! Think about this more...
    id_ = rhs.id_;
    input_ = rhs.input_;
    append_ = rhs.append_;
    output_ = rhs.output_;
    ready_buffer_ = rhs.ready_buffer_;
    sustainable_ = rhs.sustainable_;
    state_ = rhs.state_;
    available_ = rhs.available_;

    return *this;
}

/**
Executes the transformation and does additional error handling.
@return Returns ok if the transformations could be executed without errors, otherwise false.
  */
bool Transformation::doExecute()
{
    //input buffer has to be set
    if( !input_ )
    {
        error_msg_ = "No input buffer set";
        state_ = TRAFO_ERROR;
        logerr  << "Transformation: doExecute: No input buffer set.";

        return false;
    }

    //input variables not configured properly
    if( !checkInputVariables( input_ ) )
    {
        std::string vars = getErrorVariables( input_ );
        error_msg_ = "Input variables not found in input buffer." + vars;
        state_ = TRAFO_ERROR;
        logerr  << "Transformation: doExecute: Input variables not valid for input buffer.";

        return false;
    }

    bool ok;

    try
    {
        /// @todo Hidden properties not automatically handled here...
        resetProperties();
        bufferFromOutputVariables( &output_, input_->dboType() );
        createInputProperties( input_ );
        createOutputProperties( output_ );
        ok = execute();
    }
    catch( const std::runtime_error& err )
    {
        //error encountered
        error_msg_ = err.what();
        state_ = TRAFO_ERROR;
        logerr  << err.what();

        return false;
    }

    //execution not ok
    if( !ok )
    {
        error_msg_ = "Error during transformation.";
        state_ = TRAFO_ERROR;
    }

    return ok;
}

/**
Returns the output buffer.
This is just the buffer the transformation currently writes to.
Output buffers that are ready to be fetched should be transferred to the ready buffer
using outputReady() or using checkBuffer() (which will transfer it if its full). Then
get the ready buffer on the outside using fetchReadyBuffer(), which will also clean it up.
@todo Think about the impact of making this one available.
@return Output buffer.
  */
Buffer* Transformation::getOutput ()
{
    return output_;
}

/**
Returns the input buffer.
@return Input buffer.
  */
Buffer* Transformation::getInput ()
{
    return input_;
}

/**
Returns the ready buffer. Think about calling fetchReadyBuffer()!
@return Ready buffer.
  */
Buffer* Transformation::getReadyBuffer() const
{
    return ready_buffer_;
}

/**
Clears and optionally deletes the ready buffer.
@param delete_buffer If true deletes the ready buffer.
  */
void Transformation::clearReadyBuffer( bool delete_buffer )
{
    if( !ready_buffer_ )
        throw std::runtime_error( "Transformation::clearReadyBuffer(): Already cleaned up." );

    if( delete_buffer )
    {
        //in append mode not a good idea
        if( append_ || input_ == ready_buffer_ )
            throw std::runtime_error( "Transformation::clearReadyBuffer(): Input == ReadyBuffer, cant delete." );
        delete ready_buffer_;
    }

    ready_buffer_ = NULL;
}

/**
Returns the input buffer and resets it.
Should be used when someone else cleans up the input later, so cleanup() wont be confused.
  */
Buffer* Transformation::fetchInputBuffer()
{
    Buffer* rbuffer = input_;
    input_ = NULL;
    return rbuffer;
}

/**
Returns the output buffer and resets it.
Should be used when someone else cleans up the output later, so cleanup() wont be confused.
  */
Buffer* Transformation::fetchOutputBuffer()
{
    Buffer* rbuffer = output_;
    output_ = NULL;
    return rbuffer;
}

/**
Returns the ready buffer, leaving the cleanup to some other authority.
  */
Buffer* Transformation::fetchReadyBuffer()
{
    //not ready yet
    if( !isOutputReady() )
        throw std::runtime_error( "Transformation::fetchReadyBuffer(): Not yet ready." );

    Buffer* rbuffer = ready_buffer_;
    clearReadyBuffer( false );
    return rbuffer;
}

/**
Returns the transformations id. At the moment used to identify the transformation
in the TransformationFactory.
@return The transformations id.
  */
const std::string& Transformation::getId() const
{
    return id_;
}

/**
Checks if the transformation is in appending mode.
@return True if appending, false otherwise.
  */
bool Transformation::isAppending() const
{
    return append_;
}

/**
Returns if there is output at the ready buffer, waiting to be fetched.
Check on this before fetching a result via fetchReadyBuffer().
@return True if there is a result waiting to be fetched, false otherwise.
  */
bool Transformation::isOutputReady() const
{
    return ready_buffer_ != NULL;
}

/**
Returns if the transformation is sustainable or not, which means
it is used more often than once for a calculation.
@return True if sustainable, false otherwise.
  */
bool Transformation::isSustainable() const
{
    return sustainable_;
}

/**
Flags the transformation as available/occupied.
Should be called when a sustainable transformation is used / not used anaymore.
@param available Flag to be set.
  */
void Transformation::makeAvailable( bool available )
{
    available_ = available;
}

/**
Returns if the buffer is available or not. Non-sustainable buffers
are always available.
@return True if available, false otherwise.
  */
bool Transformation::isAvailable() const
{
    return !sustainable_ || available_;
}

/**
Transfers the output buffer to the ready buffer.
CAREFUL: only transfer full buffers!
  */
void Transformation::outputReady()
{
    //nothing to transfer.
    if( !output_ || output_->size() == 0 )
        throw std::runtime_error( "Transformation::outputReady(): Nothing to transfer! Empty or NULL." );

    if( !ready_buffer_ )
    {
        //first buffer
        ready_buffer_ = output_;
        output_ = NULL;
    }
    else
    {
        if( append_ )
            throw std::runtime_error( "Transformation::outputReady(): Only call once when appending." );

        //add to existing ready buffer
        ready_buffer_->seizeBuffer( *output_ );
        delete output_;
        output_ = NULL;
    }
}

/**
Checks if the output buffer is full, if yes its transferred to the
ready buffer and a new output buffer is created. Call this at the end
of your execute() method if in sustainable mode.
  */
void Transformation::checkOutputBuffer()
{
    if( append_ )
        throw std::runtime_error( "Transformation::checkOutputBuffer(): Not valid in appending mode." );

    if( output_ && output_->isFull() )
    {
        outputReady();
        changeBuffer();
    }
}

/**
Switches to a new output buffer. Maybe backup your old one before calling this.
  */
void Transformation::changeBuffer()
{
    output_ = NULL;
    bufferFromOutputVariables( &output_, input_->dboType() );
}

/**
Sets a new input buffer.
@param buffer New input buffer.
  */
void Transformation::setInputBuffer( Buffer* buffer )
{
    //ready buffer not fetched, not allowed!
    if( ready_buffer_ )
        throw std::runtime_error( "Transformation: setInputBuffer: ReadyBuffer not cleaned up." );

    input_ = buffer;

    //Don't do this with sustainable ops that already have their output buffer set.
    //This buffer may not be full yet and is reused until full. Setting output to input
    //with sustainable appending trafos is ok since those will will always yield full
    //buffers anayways. DRAMA: An output buffer that equals the input buffer and remains
    //set after execute(). Should not happen!
    if( !sustainable_ || output_ == NULL )
    {
        if( append_ )
            output_ = input_;
        else
            output_ = NULL;
    }
}

/**
Enables/disables append mode.
@todo Review internal logic! At the moment only call before setting the input buffer.
@param Append If true appending mode gets enabled, otherwise disabled.
  */
void Transformation::setAppend( bool append )
{
    //if( append && sustainable_ )
    //    throw std::runtime_error( "Transformation:setAppend: Append mode not possible when sustainable." );

    append_ = append;

    //This may not be enough!
    if( append_ )
        output_ = input_;
    else
        output_ = NULL;
}

/**
Enables/disables sustainable mode. This one should only be called in the constructor internally.
No need to decide externally if i'm a sustainable transformation or not.
@param sustainable If true the transformation is set to sustainable mode.
  */
void Transformation::setSustainable( bool sustainable )
{
    sustainable_ = sustainable;
}

/**
Returns if property exists in input_, sets index if true.
  */
bool Transformation::getPropertyIndex( const std::string& var_name, bool is_meta, unsigned int& index )
{
    if( !input_ )
        return false;

    const std::string &type = input_->dboType();

    if( is_meta )
    {
        // TODO
        assert (false);

//        if( !DBObjectManager::getInstance().existsDBObject( DBO_UNDEFINED ) ||
//            !DBObjectManager::getInstance().existsDBOVariable( DBO_UNDEFINED, var_name ) ||
//            !DBObjectManager::getInstance().getDBOVariable( DBO_UNDEFINED, var_name )->existsIn( type ) )
//            return false;

//        try
//        {
//            const std::string& id = DBObjectManager::getInstance().getDBOVariable( DBO_UNDEFINED, var_name )->getFor( type )->id_;
//            index = input_->getPropertyList()->getPropertyIndex( id );
//        }
//        catch( ... )
//        {
//            return false;
//        }

        return true;
    }

    if( !DBObjectManager::getInstance().existsDBOVariable( type, var_name ) )
        return false;

    try
    {
        const std::string& id = DBObjectManager::getInstance().getDBOVariable( type, var_name )->getId();
        index = input_->properties().getPropertyIndex( id );
    }
    catch( ... )
    {
        return false;
    }

    return true;
}

/**
Returns if DBOVariable exists in input, sets index if true.
  */
bool Transformation::getPropertyIndex( DBOVariable* var, unsigned int& index )
{
    if( !var )
        return false;
    const std::string &dbo_type = input_->dboType();
    if( !var->existsIn( dbo_type ) )
        return false;
    DBOVariable* var_for_dbo_type = var->getFor( dbo_type );
    if( !input_->properties().hasProperty( var_for_dbo_type->getId() ) )
        return false;
    index = input_->properties().getPropertyIndex(var_for_dbo_type->getId() );
    return true;
}

/**
Returns if the trafo is in good state.
@return True if the trafo is ok, otherwise false.
  */
bool Transformation::isOk() const
{
    return ( state_ == TRAFO_OK );
}

/**
Cleans up the transformation buffers. Be careful ere!
(e.g. use the fetch methods when cleaning up externally)
  */
void Transformation::cleanup()
{
    bool delete_out = ( input_ != output_ );
    bool delete_ready = ( output_ != ready_buffer_ && input_ != ready_buffer_ );

    if( input_ )
        delete input_;
    if( delete_out && output_ )
        delete output_;
    if( delete_ready && ready_buffer_ )
        delete ready_buffer_;

    input_ = NULL;
    output_ = NULL;
    ready_buffer_ = NULL;
}

/**
  */
void Transformation::generateSubConfigurable( std::string class_id, std::string instance_id )
{
    //!
    TransformationBase::generateSubConfigurable( class_id, instance_id );
}

/**
  */
void Transformation::checkSubConfigurables()
{
    //!
    TransformationBase::checkSubConfigurables();
}

/**
Returns a saved error message.
@return Error message string.
  */
const std::string& Transformation::getError() const
{
    return error_msg_;
}
