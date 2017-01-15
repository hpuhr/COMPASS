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

#include "ComputationPipeline.h"
#include "ComputationPipelineQueue.h"
#include "Buffer.h"
#include "TransformationJob.h"

#include "boost/bind.hpp"
#include <boost/function.hpp>


/**
Constructor.
@param input Input buffer to be transformed by the pipeline.
@param trafos Vector of transformations to be applied to the input buffer.
@param queue Queue to synchronize reservation of sustainable transformations for the pipeline.
  */
ComputationPipeline::ComputationPipeline( Buffer* input, const std::vector<Transformation*>& trafos, ComputationPipelineQueue* queue )
:   input_( input ),
    output_( NULL ),
    current_input_( NULL ),
    current_job_( NULL ),
    current_trafo_( NULL ),
    current_( -1 ),
    state_( CPL_FRESH ),
    queue_( queue ),
    priority_( -1 ),
    error_msg_( "" )
{
    Transformation* trafo;
    unsigned int i, n = trafos.size();
    for( i=0; i<n; ++i )
    {
        //sustainable trafos are reused, all other trafos cloned
        if( trafos[ i ]->isSustainable() )
            trafo = trafos[ i ];
        else
            trafo = trafos[ i ]->clone();

        trafos_.push_back( trafo );
    }

    registerInQueue( true );
}

/**
Destructor.
  */
ComputationPipeline::~ComputationPipeline()
{
    logdbg << "ComputationPipeline: ~ComputationPipeline(): active jobs=" << active_jobs_.size() << ", state=" << state_;

    //unregister the pipeline from the queue to make room for others
    registerInQueue( false );

    clearBuffers();
    clearTrafos();
}

/**
Registers/unregisters the pipeline and its sustainable transformations with the ComputationPipelineQueue.
Transformation are preregistered in the Computation when added. A call to this method will register the
pipeline with already registered sustainable transformations and insert the pipeline into the transformations
waiting queues. Then, if the time has come, the ComputationPipelineQueue will call next() on the waiting pipeline.
Note that unregistering the pipeline is extremely important, since the pipeline would otherwise block the
ComputationPipelineQueue infinitely!
@param ok Registers the pipelines sustainable transformations if true, unregisters if false.
  */
void ComputationPipeline::registerInQueue( bool ok )
{
    logdbg << "ComputationPipeline: registerInQueue: " << ok;

    unsigned int i, n = trafos_.size();
    for( i=0; i<n; ++i )
    {
        //has to be sustainable
        if( !trafos_[ i ]->isSustainable() )
            continue;

        if( ok )
            queue_->registerPipeline( trafos_[ i ], this );
        else
            queue_->unregisterPipeline( trafos_[ i ], this );
    }
}

/**
Returns the result buffer of the pipeline. This buffer may equal the input buffer, so
check with outputDeletable() if the buffer may be deleted.
  */
Buffer* ComputationPipeline::getOutput()
{
    return output_;
}

/**
Checks if the pipeline is currently working.
@return True if the pipeline is working, otherwise false.
  */
bool ComputationPipeline::isRunning() const
{
    return ( state_ == CPL_RUNNING );
}

/**
Checks if the pipeline is in an inactive state.
@return True if the pipeline has stopped, otherwise false.
  */
bool ComputationPipeline::hasStopped() const
{
    return ( state_ == CPL_FINISHED || state_ == CPL_ABORTED || state_ == CPL_ERROR );
}

/**
Checks if the result buffer may be deleted or not.
  */
bool ComputationPipeline::outputDeletable() const
{
    return ( output_ && input_ != output_ );
}

/**
Returns the state the pipeline is currently in.
@return Current pipeline state.
  */
ComputationPipeline::ComputationPipelineState ComputationPipeline::getState()
{
    return state_;
}

/**
Sets the priority of the pipeline in the ComputationPipelineQueue. Used by the
ComputationPipelineQueue to distribute priorities to the pipelines.
@param p Transformation priority.
  */
void ComputationPipeline::setPriority( int p )
{
    priority_ = p;
}

/**
Returns the priority of the pipeline in the ComputationPipelineQueue. Used by the
ComputationPipelineQueue to check the priority of its registered pipelines.
@return The pipelines transformation priority.
  */
int ComputationPipeline::getPriority() const
{
    return priority_;
}

/**
Clears all deletable intermediate buffers. This may include the input buffer, depending on the transformations.
  */
void ComputationPipeline::clearBuffers()
{
    unsigned int i, n = buffer_tbd_.size();
    for( i=0; i<n; ++i )
        delete buffer_tbd_[ i ];
    buffer_tbd_.clear();
}

/**
Clears all cloned transformations.
  */
void ComputationPipeline::clearTrafos()
{
    unsigned int i, n = trafos_.size();
    for( i=0; i<n; ++i )
    {
        if( !trafos_[ i ]->isSustainable() )
            delete trafos_[ i ];
    }
    trafos_.clear();
}

/**
Starts the next transformation in line. This method is also called by the
ComputationPipelineQueue to reactivate pipelines waiting for a sustainable transformation.
Note that the state between two transformations should be CPL_WAITING.
  */
void ComputationPipeline::next()
{
    logdbg << "ComputationPipeline: next: state=" << state_ << ", trafo " << current_ << " / " << trafos_.size();

    assert( current_trafo_ );

    //we should be in waiting state here
    if( state_ != CPL_WAITING )
    {
        //for convenience, maybe the queue is calling this when already in abort state...
        if( state_ == CPL_ABORTING )
        {
            registerInQueue( false );
            state_ = CPL_ABORTED;
        }
        return;
    }

    //grab the sustainable trafo
    if( current_trafo_->isSustainable() )
        current_trafo_->makeAvailable( false );

    current_trafo_->setInputBuffer( current_input_ );

    //running again
    state_ = CPL_RUNNING;
    current_job_ = new TransformationJob( this,
                                          boost::bind( &ComputationPipeline::trafoFinished, this, _1 ),
                                          boost::bind( &ComputationPipeline::trafoAborted , this, _1 ),
                                          current_trafo_ );
}

/**
Starts the execution of the pipeline. Note that the execution
will only start with fresh or finished pipelines.
@return True if the execution was successful, otherwise false.
  */
bool ComputationPipeline::execute()
{
    logdbg << "ComputationPipeline: execute: state=" << state_;

    if( !( state_ == CPL_FINISHED || state_ == CPL_FRESH ) )
        return false;

    state_ = CPL_WAITING;

    //clear old garbage
    clearBuffers();

    output_ = NULL;
    current_ = 0;
    current_input_ = input_;
    current_trafo_ = trafos_[ current_ ];

    if( !current_trafo_->isSustainable() )
        next();
    else
        waitForTrafo();

    return true;
}

/**
Aborts the execution of the pipeline.
@return True if the pipeline already could be aborted, false if it is aborting asynchronously.
  */
bool ComputationPipeline::abort()
{
    boost::mutex::scoped_lock l( mutex_abort_ );

    logdbg << "ComputationPipeline: abort: state=" << state_;

    //already aborting...
    if( state_ == CPL_ABORTING )
        return false;

    //states in which we can abort immediately
    if( state_ == CPL_WAITING  || state_ == CPL_FRESH || hasStopped() )
    {
        registerInQueue( false );
        state_ = CPL_ABORTED;
        return true;
    }

    //abort jobs
    state_ = CPL_ABORTING;
    setJobsObsolete();

    return false;
}

/**
Triggered if a transformation job has finished. This one is invoked
by a multithreading Job and may also be caused by a transformation error.
@param job Finished job.
  */
void ComputationPipeline::trafoFinished( Job* job )
{
    boost::mutex::scoped_lock l( mutex_abort_ );

    logdbg << "ComputationPipeline: trafoFinished: state=" << state_ << ", " << active_jobs_.size() << " left";

    //aborting, won't need result
    if( state_ == CPL_ABORTING )
    {
        delete job;
        registerInQueue( false );
        state_ = CPL_ABORTED;
        return;
    }

    //limbo state
    state_ = CPL_WAITING;

    TransformationJob* tjob = (TransformationJob*)job;
    Transformation* trafo = tjob->getTransformation();
    delete tjob;
    current_job_ = NULL;

    //error?
    if( !trafo->isOk() )
    {
        logdbg << "ComputationPipeline: trafoFinished: Trafo error";

        //At the moment we totally cleanup sustainable trafos that go wrong,
        //the internal data may be corrupted anyway...
        /// @todo Not sure if this is correct way to go in all cases...
        if( trafo->isSustainable() )
        {
            trafo->cleanup();
            trafo->clearIntermediateData();

            //Not sure if needed, but maybe unblocking is a good idea before making the trafo available...
            queue_->unregisterPipeline( trafo, this );

            trafo->makeAvailable( true );
        }

        registerInQueue( false );
        state_ = CPL_ERROR;
        error_msg_ = trafo->getId() + ": " + trafo->getError();
        return;
    }

    Buffer* buffer_in = trafo->fetchInputBuffer();
    output_ = trafo->fetchReadyBuffer();
    if( !trafo->isAppending() )
        buffer_tbd_.push_back( buffer_in );

    //sustainable?
    if( trafo->isSustainable() )
    {
        if( input_->getLastOne() )
            trafo->clearIntermediateData();
        queue_->unregisterPipeline( trafo, this );
        trafo->makeAvailable( true );
    }

    //last one? Then we are finished, YAY!
    ++current_;
    if( current_ == (signed)trafos_.size() )
    {
        logdbg << "ComputationPipeline: trafoFinished: Pipeline finished";
        registerInQueue( false );
        state_ = CPL_FINISHED;
        return;
    }

    //next trafo
    current_input_ = output_;
    current_trafo_ = trafos_[ current_ ];

    if( !current_trafo_->isSustainable() )
        next();
    else
        waitForTrafo();
}

/**
Triggered if a transformation job has been aborted. This one is invoked
by a multithreading Job. The ultimate cause may be a call to abort(), anyway
the pipeline will go into aborted state and finish execution.
@param job Aborted job.
  */
void ComputationPipeline::trafoAborted( Job* job )
{
    boost::mutex::scoped_lock l( mutex_abort_ );

    logdbg << "ComputationPipeline: trafoAborted: state=" << state_ << ", " << active_jobs_.size() << " left";

    TransformationJob* tjob = (TransformationJob*)job;

    Transformation* T = tjob->getTransformation();
    if( T->isSustainable() )
    {
        queue_->unregisterPipeline( T, this );  //Not sure if needed
        /// @todo Maybe the internal data of the sustainable trafo should get cleared here too...but the Computation should clear it anyway.
        T->makeAvailable( true );
    }

    delete tjob;
    current_job_ = NULL;
    registerInQueue( false );
    state_ = CPL_ABORTED;
}

/**
Enters waiting state for sustainable transformations. The method will set waiting state and
signalize to the ComputationPipelineQueue that it is now ready for the sustainable trafo it is waiting for.
  */
void ComputationPipeline::waitForTrafo()
{
    logdbg << "ComputationPipeline: waitForTrafo: Informing queue, state=" << state_;

    state_ = CPL_WAITING;
    queue_->waiting( current_trafo_, this );
}

/**
Returns an error string. The string may be empty if no errors occurred.
@return Error string.
  */
const std::string& ComputationPipeline::getError() const
{
    return error_msg_;
}
