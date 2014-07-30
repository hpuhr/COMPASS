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

#ifndef COMPUTATIONPIPELINEQUEUE_H
#define COMPUTATIONPIPELINEQUEUE_H

#include "ComputationPipeline.h"

#include <map>
#include <set>


/**
@brief Organizes the access on sustainable transformations shared between one or more computation pipelines in a Computation.

Sustainable transformations obtain inner data structures which are updated by each Buffer that is transformed
by them, one example beeing the lines transformation CalculateLines. Because of their nature they are shared among multiple
computation pipelines, but no one can guarantee in which order the pipelines access these transformations.

Buffers flow into a Computation in a specific order and for a sustainable transformation this order may often be crucial,
for example when relying on some specific ordering in the buffers.

Another problem is that multiple threads may want to access free sustainable transformations at the same time.

This datastructure assures that pipelines access sustainable transformations in the same order they are created and gives
the exclusive right to the transformation to exactly one pipeline.

Transformations that should be organized can be registered at the queue, which will e.g. be done by a Computation when adding new
transformations. Pipelines, when created, also register themselves for all their sustainable transformations, this way obtaining a priority
code. When they need access to a transformation they may signal it calling waiting() and by doing so they are in line for a certain transformation
until their priority number enables them to gain access. In that case the queue will reenable the pipeline from its waiting state and start
the transformation execution.

Note that the queue is checked at periodic intervals by the ComputationThread of the Computation it is part of. The thread just calls handle(),
which works all the magic.

To sum it up, the interaction through the queue works as follows:
- Register sustainable transformations that should be organized in the queue.
- Register a pipeline with all its sustainable transformations at the queue on creation. If the pipeline registers its first
  transformation it will be assigned a priority number.
- If longing for access to a transformation, call waiting() from the pipeline to queue in for the transformation.

@todo Put this functionality directly into the Transformation class, maybe derive a new SustainableTransformation. This way
      the pipelines could just issue a Job and the Job would idle until it may use the trafo.
  */
class ComputationPipelineQueue
{
public:
    typedef std::map<int,ComputationPipeline*> RegisteredPipes;
    typedef std::set<ComputationPipeline*> WaitingPipes;
    typedef std::map<Transformation*,RegisteredPipes> PipeMap;
    typedef std::map<Transformation*,WaitingPipes> WaitingMap;

    /// @brief Constructor
    ComputationPipelineQueue();
    /// @brief Copy constructor
    ComputationPipelineQueue( const ComputationPipelineQueue& cpy );
    /// @brief Destructor
    ~ComputationPipelineQueue();

    /// @brief Registers a new (mostly sustainable) transformation with the queue
    void registerTransformation( Transformation* trafo );
    /// @brief Unregisters a new (mostly sustainable) transformation from the queue
    void unregisterTransformation( Transformation* trafo );
    /// @brief Registers a ComputationPipeline with the given transformation
    void registerPipeline( Transformation* trafo, ComputationPipeline* pipe );
    /// @brief Unregisters a ComputationPipeline with the given transformation
    void unregisterPipeline( Transformation* trafo, ComputationPipeline* pipe );
    /// @brief Signals that the given pipeline is waiting for the given transformation
    void waiting( Transformation* trafo, ComputationPipeline* pipe );

    /// @brief Clears the registered pipelines
    void clear();
    /// @brief Checks if there are pipelines registered or waiting
    bool empty();
    /// @brief Works the queue, called by the ComputationThread of a Computation
    void handle();

private:
    /// @brief Returns a new priority ticket
    int priority();

    /// Registered pipelines
    PipeMap registered_;
    /// Waiting pipelines
    WaitingMap waiting_;
    ///
    boost::mutex mutex_;
    /// Priority counter
    int priority_;
};

#endif //COMPUTATIONPIPELINEQUEUE_H
