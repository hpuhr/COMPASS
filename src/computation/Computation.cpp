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

#include "Computation.h"
#include "TransformationJob.h"
#include "Transformation.h"
#include "Buffer.h"
#include "Logger.h"
#include "DBOVariable.h"
#include "TransformationFactory.h"

#include "boost/bind.hpp"
#include <boost/function.hpp>
#include "String.h"

using namespace Utils::String;

/*****************************************************************************************
TransformationEntry
******************************************************************************************/

/**
Constructor.
@param dbo_type DBO type the entry is assigned to.
@param id String id the stored transformation is constructed from. Note that the transformation
    pointer may remain NULL if the string id is an empty string.
  */
TransformationEntry::TransformationEntry( int dbo_type,
                                          const std::string& id )
:   id_( id ),
    dbo_type_( dbo_type ),
    trafo_( NULL )
{
    if( !id_.empty() )
        trafo_ = TransformationFactory::getInstance().createTransformation( id_ );
}

/**
Configurable constructor.
Don't forget to set initial values for the registered parameters through the Configuration ( use getConfig() for convenience ).
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
  */
TransformationEntry::TransformationEntry( const std::string& class_id,
                                          const std::string& instance_id,
                                          Configurable* parent )
:   Configurable( class_id, instance_id, parent ),
    trafo_( NULL )
{
    registerParameter( "id", &id_, "" );
    registerParameter( "dbo_type", &dbo_type_, -1 );

    createSubConfigurables();
}

/**
Destructor.
  */
TransformationEntry::~TransformationEntry()
{
    if( trafo_ )
        delete trafo_;
}

/**
Retrieves a Configuration with the given data filled in. Use this method for your convenience when generating a Configurable.
@param config The config the data is filled in.
@param dbo_type DBO type the entry is assigned to.
@param id String id of the stored transformation.
  */
void TransformationEntry::getConfig( Configuration& config, int dbo_type, const std::string& id )
{
    config.addParameterString( "id", id );
    config.addParameterInt( "dbo_type", dbo_type );
}

/**
  */
void TransformationEntry::generateSubConfigurable( std::string class_id, std::string instance_id )
{
    if( class_id == "Transformation" )
    {
        if( trafo_ )
            delete trafo_;
        trafo_ = TransformationFactory::getInstance().createTransformation( class_id, instance_id, this, id_ );
    }
}

/**
  */
void TransformationEntry::checkSubConfigurables()
{
    //generate trafo if possible
    if( !trafo_ && !id_.empty() )
    {
        Configuration& config = addNewSubConfiguration( "Transformation" );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );
    }
}

/**
Returns the stored transformations string identifier.
@return The stored transformations string identifier.
  */
const std::string& TransformationEntry::getID() const
{
    return id_;
}

/**
Returns the DBO type the TransformationEntry is assigned to.
@return DBO type the TransformationEntry is assigned to.
  */
int TransformationEntry::getDBOType() const
{
    return dbo_type_;
}

/**
Returns the stored transformation. Note that the pointer may be NULL if
no valid (e.g. an empty) transformation string id has been set.
@return Pointer to the stored transformation.
  */
Transformation* TransformationEntry::getTransformation()
{
    return trafo_;
}

/**
Stores the given external Transformation by cloning it.
@param trafo External Transformation to be stored.
  */
void TransformationEntry::setTransformation( Transformation* trafo )
{
    if( trafo_ )
        delete trafo_;

    id_ = trafo->getId();

    if( unusable_ )
    {
        trafo_ = trafo->clone();
    }
    else
    {
        Configuration& config = addNewSubConfiguration( "Transformation" );
        trafo_ = trafo->clone( config.getClassId(), config.getInstanceId(), this, true );
    }
}

/**
Updates the stored transformation using the given transformation string identifier.
Note that all configurations made to the current transformation will get lost.
@param is Transformation string identifier to generate a new Transformation.
  */
void TransformationEntry::setTransformation( const std::string& id )
{
    if( trafo_ )
        delete trafo_;

    id_ = id;
    if( id_.empty() )
        return;

    if( unusable_ )
    {
        trafo_ = TransformationFactory::getInstance().createTransformation( id_ );
    }
    else
    {
        Configuration& config = addNewSubConfiguration( "Transformation" );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );
    }
}

/*****************************************************************************************
Computation
******************************************************************************************/

/**
Constructor.
@param name Name of the Computation.
  */
Computation::Computation( const std::string& name )
:   name_( name ),
    shutdown_( false ),
    aborting_( false ),
    use_common_( false ),
    active_pipes_( 0 ),
    buffers_waiting_( 0 ),
    buffers_in_cnt_( 0 ),
    buffers_out_cnt_( 0 ),
    buffers_aborted_cnt_( 0 ),
    buffers_error_cnt_( 0 ), 
    dispatch_thread_( NULL ),
    thread_running_( false ),
    idle_( true )
{
    filter_ = new BufferFilter;

    dispatch_thread_ = new ComputationThread( this, 20 );
    startThread();
}

/**
Configurable constructor.
Don't forget to set initial values for the registered parameters through the Configuration.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
  */
Computation::Computation( std::string class_id,
                          std::string instance_id,
                          Configurable* parent )
:   Configurable( class_id, instance_id, parent ),
    shutdown_( false ),
    aborting_( false ),
    use_common_( false ),
    active_pipes_( 0 ),
    buffers_waiting_( 0 ),
    buffers_in_cnt_( 0 ),
    buffers_out_cnt_( 0 ),
    buffers_aborted_cnt_( 0 ),
    buffers_error_cnt_( 0 ),
    filter_( NULL ),
    dispatch_thread_( NULL ),
    thread_running_( false ),
    idle_( true )
{
    registerParameter( "name", &name_, "" );
    registerParameter( "use_common_trafo", &use_common_, false );

    //create thread and start it right away
    dispatch_thread_ = new ComputationThread( this, 20 );
    startThread();

    //createSubconfigurables() called from subclass! (ComputatationElement)
}

/**
Destructor.
  */
Computation::~Computation()
{
    //set both idle and shutdown, nothing has to happen at this moment
    idle_ = true;
    shutdown_ = true;

    //inform attached components (generators, views)
    delete_signal_();

    stopThread();// MUI IMPORTANTE
    delete dispatch_thread_;

    clearSlot();

    //Blocking wait for abort of all pipes
    while( active_pipes_ )
        checkActivePipelines();

    deleteTransformations();

    if( filter_ )
        delete filter_;
}

/**
Returns the Computation name.
@return Name of the Computation.
  */
const std::string& Computation::name() const
{
    return name_;
}

/**
Returns the computations input buffer filter.
@return Input buffer filter.
  */
BufferFilter* Computation::getFilter()
{
    return filter_;
}

/**
Starts the working thread.
  */
void Computation::startThread()
{
    if( thread_running_ )
        return;
    thread_running_ = true;
    dispatch_thread_->go();
}

/**
Stops the working thread.
  */
void Computation::stopThread()
{
    if( !thread_running_ )
        return;
    dispatch_thread_->shutdown();

    //blocking wait
    while( !dispatch_thread_->isShutdown() )
        ;

    thread_running_ = false;
}

/**
Adds a new DBO type specific Transformation to the Computation. It is possible to add more
than one Transformation for each DBO type. The order this method is called then equals the
order the transformations are applied to a buffer.
@param dbo_type DBO type the Transformation is assigned to.
@param trafo_id String identifier of the Transformation to be added.
@return The created Transformation.
  */
Transformation* Computation::addTransformation( DB_OBJECT_TYPE dbo_type, const std::string& trafo_id )
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    assert( !trafo_id.empty() );

    if( unusable_ )
    {
        TransformationEntry* entry = new TransformationEntry( (int)dbo_type, trafo_id );
        trafos_[ dbo_type ].push_back( entry );

        //register the transformation with the computation queue if sustainable
        if( entry->getTransformation()->isSustainable() )
            computation_queue_.registerTransformation( entry->getTransformation() );

        return entry->getTransformation();
    }
    else
    {
        unsigned int size = trafos_[ dbo_type ].size();

        Configuration& config = addNewSubConfiguration( "TransformationEntry" );
        TransformationEntry::getConfig( config, (int)dbo_type, trafo_id );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );

        assert( trafos_[ dbo_type ].size() > size );

        return trafos_[ dbo_type ].back()->getTransformation();
    }
}

/**
Adds a new DBO type specific Transformation to the Computation. It is possible to add more
than one Transformation for each DBO type. The order this method is called then equals the
order the transformations are applied to a buffer.
@param dbo_type DBO type the Transformation is assigned to.
@param trafo An external Transformation that will be cloned and added.
  */
void Computation::addTransformation( DB_OBJECT_TYPE dbo_type, Transformation* trafo )
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    if( unusable_ )
    {
        TransformationEntry* entry = new TransformationEntry( (int)dbo_type );
        entry->setTransformation( trafo );
        trafos_[ dbo_type ].push_back( entry );

        //register the transformation with the computation queue if sustainable
        if( entry->getTransformation()->isSustainable() )
            computation_queue_.registerTransformation( entry->getTransformation() );
    }
    else
    {
        unsigned int size = trafos_[ dbo_type ].size();

        Configuration& config = addNewSubConfiguration( "TransformationEntry" );
        TransformationEntry::getConfig( config, (int)dbo_type );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );

        assert( trafos_[ dbo_type ].size() > size );

        TransformationEntry* entry = trafos_[ dbo_type ].back();
        entry->setTransformation( trafo );

        //register the transformation with the computation queue if sustainable
        if( entry->getTransformation()->isSustainable() )
            computation_queue_.registerTransformation( entry->getTransformation() );
    }
}

/**
Deepcopies the transformations of the given Computation. The stored transformations
will be deleted in this process.
@param comp The Computation the transformations shall be copied from.
  */
void Computation::copyTransformations( Computation* comp )
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    //clear stored trafos first
    deleteTransformations();

    const TransformationMap& trafo_map = comp->getTransformations();
    unsigned int i, n;
    TransformationMap::const_iterator it, itend = trafo_map.end();
    for( it=trafo_map.begin(); it!=itend; ++it )
    {
        const Transformations& trafos = it->second;
        n = trafos.size();
        for( i=0; i<n; ++i )
        {
            //common or DBO type specific?
            if( it->first == -1 )
                addCommonTransformation( trafos[ i ]->getTransformation() );
            else
                addTransformation( (DB_OBJECT_TYPE)it->first, trafos[ i ]->getTransformation() );
        }
    }
}

/**
Deletes all stored transformations.
  */
void Computation::deleteTransformations()
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    unsigned int i, n;
    TransformationMap::iterator it, itend = trafos_.end();
    for( it=trafos_.begin(); it!=itend; ++it )
    {
        const Transformations& trafos = it->second;
        n = trafos.size();
        for( i=0; i<n; ++i )
        {
            //unregister the transformation with the computation queue if sustainable
            if( trafos[ i ]->getTransformation()->isSustainable() )
                computation_queue_.unregisterTransformation( trafos[ i ]->getTransformation() );

            delete trafos[ i ];
        }
    }

    trafos_.clear();
}

/**
Deletes all stored trafos assigned to the given DBO type.
@param dbo_type DBO type of the Transformation entries to be deleted.
  */
void Computation::deleteTransformations( DB_OBJECT_TYPE dbo_type )
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    if( trafos_.find( dbo_type ) == trafos_.end() )
        return;

    Transformations& trafos = trafos_[ dbo_type ];
    unsigned int i, n = trafos.size();
    for( i=0; i<n; ++i )
    {
        //unregister the transformation with the computation queue if sustainable
        if( trafos[ i ]->getTransformation()->isSustainable() )
            computation_queue_.unregisterTransformation( trafos[ i ]->getTransformation() );

        delete trafos[ i ];
    }

    trafos_.erase( dbo_type );
}

/**
Returns the idx'th transformation added for the given DBO type.
@param dbo_type DBO type of the desired Transformation.
@param idx Index of the desired Transformation.
@return The desired Transformation.
  */
Transformation* Computation::getTransformation( DB_OBJECT_TYPE dbo_type, int idx )
{
    if( trafos_.find( dbo_type ) == trafos_.end() )
        throw std::runtime_error( "Computation: getTransformation: DBO type not found." );
    if( idx < 0 || idx >= (signed)trafos_[ dbo_type ].size() )
        throw std::runtime_error( "Computation: getTransformation: Index out of bounds." );
    return trafos_[ dbo_type ][ idx ]->getTransformation();
}

/**
Adds a new common Transformation to the Computation. As with the specific transformations,
it is possible to add more than one Transformation of this kind. The order this method is called
then equals the order the transformations are applied to a buffer. The usage of common transformations
has to be enabled manually by calling enableCommonTransformation(). When enabled, all accepted buffers
will be transformed using this set of transformations instead of their specific ones.
@param trafo_id String identifier of the Transformation to be added.
@return The created Transformation.
  */
Transformation* Computation::addCommonTransformation( const std::string& trafo_id )
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    if( unusable_ )
    {
        TransformationEntry* entry = new TransformationEntry( -1, trafo_id );
        trafos_[ -1 ].push_back( entry );

        //register the transformation with the computation queue if sustainable
        if( entry->getTransformation()->isSustainable() )
            computation_queue_.registerTransformation( entry->getTransformation() );

        return entry->getTransformation();
    }
    else
    {
        unsigned int size = trafos_[ -1 ].size();

        Configuration& config = addNewSubConfiguration( "TransformationEntry" );
        TransformationEntry::getConfig( config, -1, trafo_id );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );

        assert( trafos_[ -1 ].size() > size );

        return trafos_[ -1 ].back()->getTransformation();
    }
}

/**
Adds a new common Transformation to the Computation. As with the specific transformations,
it is possible to add more than one Transformation of this kind. The order this method is called
then equals the order the transformations are applied to a buffer. The usage of common transformations
has to be enabled manually by calling enableCommonTransformation(). When enabled, all accepted buffers
will be transformed using this set of transformations instead of their specific ones.
@param trafo An external Transformation that will be cloned and added.
  */
void Computation::addCommonTransformation( Transformation* trafo )
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    if( unusable_ )
    {
        TransformationEntry* entry = new TransformationEntry( -1 );
        entry->setTransformation( trafo );
        trafos_[ -1 ].push_back( entry );

        //register the transformation with the computation queue if sustainable
        if( entry->getTransformation()->isSustainable() )
            computation_queue_.registerTransformation( entry->getTransformation() );
    }
    else
    {
        unsigned int size = trafos_[ -1 ].size();

        Configuration& config = addNewSubConfiguration( "TransformationEntry" );
        TransformationEntry::getConfig( config, -1 );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );

        assert( trafos_[ -1 ].size() > size );

        TransformationEntry* entry = trafos_[ -1 ].back();
        entry->setTransformation( trafo );

        //register the transformation with the computation queue if sustainable
        if( entry->getTransformation()->isSustainable() )
            computation_queue_.registerTransformation( entry->getTransformation() );
    }
}

/**
Deletes all stored common transformations.
  */
void Computation::deleteCommonTransformations()
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    if( trafos_.find( -1 ) == trafos_.end() )
        return;

    Transformations& trafos = trafos_[ -1 ];
    unsigned int i, n = trafos.size();
    for( i=0; i<n; ++i )
    {
        //unregister the transformation with the computation queue if sustainable
        if( trafos[ i ]->getTransformation()->isSustainable() )
            computation_queue_.unregisterTransformation( trafos[ i ]->getTransformation() );
        delete trafos[ i ];
    }

    trafos_.erase( -1 );
}

/**
Enables/Disables the usage of common transformations.
@param enable If true the stored common transformations will be applied to
    all accepted buffers no matter what DBO type they obtain. If false the
    usage of DBO type specific transformations will be reenabled.
  */
void Computation::enableCommonTransformation( bool enable )
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    use_common_ = enable;
}

/**
Returns the idx'th stored common transformation.
@param idx Index of the desired common Transformation.
@return The desired common Transformation.
  */
Transformation* Computation::getCommonTransformation( int idx )
{
    if( trafos_.find( -1 ) == trafos_.end() )
        throw std::runtime_error( "Computation: getCommonTransformation: No common transformations set." );
    if( idx < 0 || idx >= (signed)trafos_[ -1 ].size() )
        throw std::runtime_error( "Computation: getCommonTransformation: Index out of bounds." );
    return trafos_[ -1 ][ idx ]->getTransformation();
}

/**
Clears the computations data. This does not include the configuration of
the Computation, e.g. stored transformations, the input buffer filter etc.
  */
void Computation::clearSlot()
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    //enable aborting mode
    aborting_ = true;
    //disable idle, otherwise the thread will not dispatch the unfinished pipes
    idle_ = false;

    //abort pending pipes
    {
        std::vector<ComputationPipeline*>::iterator it, itend = pipelines_.end();
        for( it=pipelines_.begin(); it!=itend; ++it )
            (*it)->abort();
    }

    //clear sustainable queue
    computation_queue_.clear();

    //clear transformed garbage buffers
    unsigned int i, n = buffers_tbd_.size();
    for( i=0; i<n; ++i )
        delete buffers_tbd_[ i ];
    buffers_tbd_.clear();

    //clear transformed buffers
    buffers_tr_.clear();

    //clear forwarded buffers
    buffers_forward_.clear();

    //clear pending untransformed buffers
    n = buffers_tbt_.size();
    for( i=0; i<n; ++i )
        delete buffers_tbt_[ i ];
    buffers_tbt_.clear();
    buffers_waiting_ = 0;

    //reset counters
    buffers_in_cnt_ = 0;
    buffers_out_cnt_ = 0;
    buffers_aborted_cnt_ = 0;
    buffers_error_cnt_ = 0;

    //inform attached components (e.g. generators, views)
    clear_signal_( this );
}

/**
Processes the given buffer. This is the main method for pushing data to the Computation.
It is also connected to the distribute_signal_ of other computations to pass data between them.
@param buffer Buffer to be processed by the Computation.
  */
void Computation::processBufferSlot( Buffer* buffer )
{
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    assert( buffer );

    BufferFilter::BufferFilterRule rule = filter_->getRule( buffer );

    //Do not accept buffers when already shutting down
    if( shutdown_ )
        return;

    //check buffer input filter if buffer is accepted
    if( shutdown_ || rule == BufferFilter::BLOCK )
    {
        std::string msg = "[COMPUTATION:" + name() + "][FILTER] Blocked. DBO type=" + intToString( (int)buffer->getDBOType() );
        filter_signal_( msg );
        return;
    }

    //check the buffer input filter for required buffer properties
    if( !filter_->checkProperties( buffer ) )
    {
        std::string props = filter_->getErrorProperties( buffer );
        std::string msg = "[COMPUTATION:" + name() + "][FILTER] Property-filtered. DBO type=" + intToString( (int)buffer->getDBOType() ) + props;
        filter_signal_( msg );

        return;
    }

    //check if buffer shall just get forwarded, if yes add to forward queue
    if( rule == BufferFilter::FORWARD )
    {
        buffers_forward_.push_back( buffer );
        idle_ = false;
        ++buffers_in_cnt_;

        std::string msg = "[COMPUTATION:" + name() + "][FILTER] Forwarded. DBO type=" + intToString( (int)buffer->getDBOType() );
        filter_signal_( msg );

        return;
    }

    //shallow copy buffer for the transform and add to transformation queue
    buffers_tbt_.push_back( buffer->getShallowCopy() );
    ++buffers_waiting_;

    //reenable dispatching
    idle_ = false;
}

/**
Transforms the given buffer. Note that the buffer at this point already has been accepted
by the input buffer filter and shallow copied.
@param buffer Buffer to be transformed.
@return True if the buffer was transformed (transformations could be found), false otherwise.
  */
bool Computation::transform( Buffer* buffer )
{
    int type = (int)buffer->getDBOType();
    if( use_common_ )
        type = -1;

    ++buffers_in_cnt_;

    //if no trafo, delete (shallow copied) buffer
    if( trafos_.find( type ) == trafos_.end() || trafos_[ type ].empty() )
    {
        delete buffer;
        ++buffers_error_cnt_;
        logdbg << "Computation: transform: No trafo found";

        std::string msg = "[COMPUTATION:" + name() + "][WRN] No trafo found. DBO type=" + intToString( (int)buffer->getDBOType() );
        warn_signal_( msg );

        return false;
    }

    logdbg << "Computation: transform: New pipe for buffer, dbo_type=" << (int)buffer->getDBOType();

    //fetch trafos from the entries
    std::vector<TransformationEntry*>& trafo_entries = trafos_[ type ];
    unsigned int i, n = trafo_entries.size();
    std::vector<Transformation*> trafos( n );
    for( i=0; i<n; ++i )
        trafos[ i ] = trafo_entries[ i ]->getTransformation();

    //add new pipe for buffer
    ComputationPipeline* pipe = new ComputationPipeline( buffer, trafos, &computation_queue_ );
    pipe->execute();
    pipelines_.push_back( pipe );
    ++active_pipes_;

    return true;
}

/**
Clears the internal data of all sustainable transformations.
  */
void Computation::clearIntermediateData()
{
    TransformationMap::iterator it, itend = trafos_.end();
    Transformations::iterator it2, it2end;
    for( it=trafos_.begin(); it!=itend; ++it )
    {
        it2 = it->second.begin();
        it2end = it->second.end();
        for( ; it2 != it2end; ++it2 )
        {
            if( (*it2)->getTransformation()->isSustainable() )
            {
                (*it2)->getTransformation()->clearIntermediateData();
                (*it2)->getTransformation()->cleanup();
            }
        }
    }
}

/**
Checks all active pipelines on their current status and handles them.
Finished pipelines will be destroyed and their output buffers will be inserted into the output queue.
Pipelines may also finish in the states aborted and error. Note that the pipelines are handled
one after another, this way the order of incoming and outgoing buffers won't be confused.
@return Number of active pipelines.
  */
int Computation::checkActivePipelines()
{
    std::vector<ComputationPipeline*>::iterator it;
    for( it=pipelines_.begin(); it!=pipelines_.end(); )
    {
        //the current pipeline in the queue has not yet stopped, so check next time
        //this is important to retain the right buffer order, pipes are handled one after another, like in a queue
        if( !(*it)->hasStopped() )
            break;

        ComputationPipeline::ComputationPipelineState state = (*it)->getState();

        //handle various pipeline states
        if( state == ComputationPipeline::CPL_FINISHED )
        {
            logdbg << "Computation: checkActivePipelines: Pipe FINISHED, " << pipelines_.size()-1 << " active remain.";

            Buffer* output = (*it)->getOutput();
            if( !aborting_ )
            {   
                if( output )
                    buffers_tr_.push_back( output );
                else
                    logdbg << "Computation: checkActivePipelines: Pipe finished, no output data.";
            }
            else
            {
                //delete result, we are aborting anyway
                if( (*it)->outputDeletable() )
                    delete output;
            }
        }
        else if( state == ComputationPipeline::CPL_ABORTED )
        {
            ++buffers_aborted_cnt_;
            logdbg << "Computation: checkActivePipelines: Pipe ABORTED, " << pipelines_.size()-1 << " active remain.";
        }
        else if( state == ComputationPipeline::CPL_ERROR )
        {
            ++buffers_error_cnt_;
            logdbg << "Computation: checkActivePipelines: Pipe ERROR, " << pipelines_.size()-1 << " active remain.";

            std::string msg = "[COMPUTATION:" + name() + "][ERR] Trafo error.\n\t" + (*it)->getError();
            error_signal_( msg );
        }
        else
        {
            //should not happen
            std::string msg = "BAD STATE: computation=" + name();
            error_signal_( msg );

            throw std::runtime_error( "Computation: checkActivePipelines: Pipe finished in bad state." );
        }

        delete (*it);
        it = pipelines_.erase( it );
    }

    active_pipes_ = pipelines_.size();
    return active_pipes_;
}

/**
Checks if all pipes have aborted/finished when in aborting mode.
@return True if all pipelines finished, otherwise false.
  */
bool Computation::checkOnAborting()
{
    if( checkActivePipelines() > 0 )
        return false;

    //all pipelines finished, leave aborting mode
    clearIntermediateData();    //maybe not needed, just for assurance
    aborting_ = false;

    return true;
}

/**
Handles pending input buffers, which were already accepted by the input buffer filter.
Pending buffers will be transformed and popped from the queue.
@return True if a buffer was handled, otherwise false.
  */
bool Computation::checkInputBuffers()
{
    if( buffers_tbt_.empty() )
        return false;

    Buffer* next;
    while( !buffers_tbt_.empty() )
    {
        next = buffers_tbt_.front();
        transform( next );
        --buffers_waiting_;
        buffers_tbt_.erase( buffers_tbt_.begin() );
    }

    return true;
}

/**
Handles pending output buffers, which were already transformed.
Pending buffers will be popped from the queue and distributed to other attached entities.
@return True if a buffer was handled, otherwise false.
  */
bool Computation::checkTransformedBuffers()
{
    if( buffers_tr_.empty() )
        return false;

    Buffer* next;
    while( !buffers_tr_.empty() )
    {
        next = buffers_tr_.front();
        buffers_tr_.erase( buffers_tr_.begin() );
        distribute_signal_( next );
        ++buffers_out_cnt_;
    }

    return true;
}

/**
Handles pending forwarded buffers.
Pending buffers will be popped from the queue and distributed to other attached entities.
@return True if a buffer was handled, otherwise false.
  */
bool Computation::checkForwardedBuffers()
{
    if( buffers_forward_.empty() )
        return false;

    Buffer* next;
    while( !buffers_forward_.empty() )
    {
        next = buffers_forward_.front();
        buffers_forward_.erase( buffers_forward_.begin() );
        distribute_signal_( next );
        ++buffers_out_cnt_;
    }

    return true;
}

/**
Main dispatch routine called by the ComputationThread at a predefined interval.
Here most of the magic happens, e.g. the pipelines will be checked and the queues
will be emptied.
  */
void Computation::dispatch()
{
    //This is the main mutex that guards the dispatch work of the thread.
    //Potential distubances may arise from:
    // - The main thread, adding transformations, configuring stuff etc.
    // - Working threads of other computations, passing new buffers etc.
    boost::mutex::scoped_lock l( dispatch_mutex_ );

    //nothing to do
    if( idle_ )
        return;

    //if aborting wait until all pipes aborted
    if( aborting_ && !checkOnAborting() )
    {
        loginf << "Computation: timerEvent: Waiting for aborting...";
        return;
    }

    //update the ComputationPipelineQueue for sustainable transformations
    computation_queue_.handle();

    //handle queues and pipelines
    checkInputBuffers();
    checkActivePipelines();
    checkTransformedBuffers();
    checkForwardedBuffers();

    //if nothing to do go into idle
    if( buffers_tr_.empty() && buffers_forward_.empty() && buffers_waiting_ == 0 && active_pipes_ == 0 )
    {
        logdbg << "Computation: timerEvent: Going idle. Name=" << name_.c_str() << ", in="
               << buffers_in_cnt_
               << ", out="
               << buffers_out_cnt_
               << ", aborted="
               << buffers_aborted_cnt_
               << ", errors="
               << buffers_error_cnt_;
        idle_ = true;
    }
}

/**
Checks the consistency of the computations configuration.
  */
void Computation::analyzeConfiguration()
{
    /**
    @todo Here the consistency of a computations configuration should be checked,
        e.g. if the transformation variables are valid, automatic configuration of the input filter
        depending on the transformation variables, etc. etc.
    */
}

/**
Clears the Computation in a blocking way.
@todo Not working. At the moment just calls the non-blocking clearSlot().
  */
void Computation::clearBlocking()
{
    clearSlot();

    //while( aborting_ )
    //    sleep( boost::posix_time::milliseconds(100) );
}

/**
Checks if the Computation is properly cleared. "Cleared" refers to the
computation data structures, pipelines, queues etc. and not to the configuration (transformations etc.).
@return True if the Computation is properly cleared, otherwise false.
  */
bool Computation::isCleared()
{
    if( !buffers_tr_.empty()        ) { loginf << "buffers_tr_"; return false; }
    if( !buffers_tbd_.empty()       ) { loginf << "buffers_tbd_"; return false; }
    if( !buffers_tbt_.empty()       ) { loginf << "buffers_tbt_"; return false; }
    if( !buffers_forward_.empty()   ) { loginf << "buffers_forward_"; return false; }
    if( !pipelines_.empty()         ) { loginf << "pipelines_"; return false; }
    if( !computation_queue_.empty() ) { loginf << "computation_queue_"; return false; }
    if( active_pipes_ > 0           ) { loginf << "active_pipes_"; return false; }
    if( buffers_waiting_ > 0        ) { loginf << "buffers_waiting_"; return false; }
    if( aborting_                   ) { loginf << "aborting_"; return false; }

    return true;
}

/**
  */
void Computation::checkSubConfigurables()
{
    if( !filter_ )
    {
        addNewSubConfiguration( "BufferFilter", "BufferFilter0" );
        generateSubConfigurable( "BufferFilter", "BufferFilter0" );
    }
}

/**
  */
void Computation::generateSubConfigurable( std::string class_id, std::string instance_id )
{
    if( class_id == "TransformationEntry" )
    {
        TransformationEntry* entry = new TransformationEntry( class_id, instance_id, this );
        int dbo_type = entry->getDBOType();
        trafos_[ dbo_type ].push_back( entry );

        //register the transformation with the computation queue if sustainable
        if( entry->getTransformation() && entry->getTransformation()->isSustainable() )
            computation_queue_.registerTransformation( entry->getTransformation() );
    }

    if( class_id == "BufferFilter" )
    {
        loginf << "adding filter";
        filter_ = new BufferFilter( class_id, instance_id, this );
    }
}
