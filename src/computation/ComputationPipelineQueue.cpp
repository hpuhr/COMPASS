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

#include "ComputationPipelineQueue.h"
#include "Logger.h"


/**
Constructor.
  */
ComputationPipelineQueue::ComputationPipelineQueue()
:   priority_( 0 )
{
}

/**
Copy constructor.
@param cpy Instance to copy.
  */
ComputationPipelineQueue::ComputationPipelineQueue( const ComputationPipelineQueue& cpy )
{
    waiting_ = cpy.waiting_;
    registered_ = cpy.registered_;
    priority_ = cpy.priority_;
}

/**
Destructor.
  */
ComputationPipelineQueue::~ComputationPipelineQueue()
{
}

/**
Clears all registered pipelines.
  */
void ComputationPipelineQueue::clear()
{
    boost::mutex::scoped_lock l( mutex_ );

    {
        WaitingMap::iterator it, itend = waiting_.end();
        for( it=waiting_.begin(); it!=itend; ++it )
            it->second.clear();
    }

    {
        PipeMap::iterator it, itend = registered_.end();
        for( it=registered_.begin(); it!=itend; ++it )
            it->second.clear();
    }

    priority_ = 0;
}

/**
Returns a new priority ticket.
@return New priority ticket.
  */
int ComputationPipelineQueue::priority()
{
    ++priority_;
    return priority_;
}

/**
Registers a new transformation.
@param trafo Transformation to be registered.
  */
void ComputationPipelineQueue::registerTransformation( Transformation* trafo )
{
    boost::mutex::scoped_lock l( mutex_ );

    if( registered_.find( trafo ) != registered_.end() )
        throw std::runtime_error( "ComputationTicketMachine: registerTrafo: Duplicate trafo." );

    //create datastructures for the registered trafo
    registered_[ trafo ];
    waiting_[ trafo ];
}

/**
Unregisters a transformation.
@param trafo Transformation to be unregistered.
  */
void ComputationPipelineQueue::unregisterTransformation( Transformation* trafo )
{
    boost::mutex::scoped_lock l( mutex_ );

    if( registered_.find( trafo ) == registered_.end() )
        throw std::runtime_error( "ComputationTicketMachine: registerTrafo: No such trafo." );

    //erase datastructures for the registered trafo
    registered_.erase( trafo );
    waiting_.erase( trafo );
}

/**
Registers the given pipeline at the given transformation.
@param trafo Transformation the pipeline is registered with.
@param pipe Pipeline to be registered with the given transfromation.
  */
void ComputationPipelineQueue::registerPipeline( Transformation* trafo, ComputationPipeline* pipe )
{
    boost::mutex::scoped_lock l( mutex_ );

    //Not registered...
    if( registered_.find( trafo ) == registered_.end() )
        return;

    //If no priority ticket yet, hand one out
    int p = pipe->getPriority();
    if( p < 0 )
    {
        p = priority();
        pipe->setPriority( p );
    }

    //Sort in the pipe at its priority number
    registered_[ trafo ][ p ] = pipe;
}

/**
Unregisters the given pipeline from the given transformation.
@param trafo Transformation the pipeline is unregistered from.
@param pipe Pipeline to be unregistered from the given transfromation.
  */
void ComputationPipelineQueue::unregisterPipeline( Transformation* trafo, ComputationPipeline* pipe )
{
    boost::mutex::scoped_lock l( mutex_ );

    //Not registered...
    if( registered_.find( trafo ) == registered_.end() )
        return;

    registered_[ trafo ].erase( pipe->getPriority() );
    waiting_[ trafo ].erase( pipe );
}

/**
Signals that the given pipeline is waiting for the given transformation.
@param trafo Transformation the pipeline is currently waiting for.
@param pipe Pipeline that is waiting.
  */
void ComputationPipelineQueue::waiting( Transformation* trafo, ComputationPipeline* pipe )
{
    boost::mutex::scoped_lock l( mutex_ );

    assert( trafo );
    assert( pipe );

    logdbg << "ComputationPipelineQueue: waiting: trafo=" << trafo->getId().c_str() << ", pipe_pr=" << pipe->getPriority();

    //Not registered...
    if( registered_.find( trafo ) == registered_.end() )
        return;

    //Queue pipe in as waiting for the given trafo
    waiting_[ trafo ].insert( pipe );
}

/**
Main working routine.
  */
void ComputationPipelineQueue::handle()
{
    boost::mutex::scoped_lock l( mutex_ );

    PipeMap::iterator it, itend = registered_.end();
    WaitingPipes::iterator waiting;
    ComputationPipeline* current;

    int num_waiting = 0;

    //iterate over all registered trafos
    for( it=registered_.begin(); it!=itend; ++it )
    {
        num_waiting += waiting_[ it->first ].size();

        //get the pipe with the smallest priority ticket
        current = it->second.begin()->second;

        //check if this pipe is already waiting for the trafo
        waiting = waiting_[ it->first ].find( current );
        if( waiting == waiting_[ it->first ].end() )
            continue;

        //give trafo free, start pipeline and erase from waiting queue
        //the pipe will automatically unregister itself from the trafo when finished
        (*waiting)->next();
        waiting_[ it->first ].erase( waiting );
        --num_waiting;
    }
}

/**
Checks if there are pipelines registered or waiting.
@return True if there are registered or waiting pipelines, false otherwise.
  */
bool ComputationPipelineQueue::empty()
{
    {
        WaitingMap::iterator it, itend = waiting_.end();
        for( it=waiting_.begin(); it!=itend; ++it )
        {
            if( !it->second.empty() )
                return false;
        }
    }

    {
        PipeMap::iterator it, itend = registered_.end();
        for( it=registered_.begin(); it!=itend; ++it )
        {
            if( !it->second.empty() )
                return false;
        }
    }

    return true;
}
