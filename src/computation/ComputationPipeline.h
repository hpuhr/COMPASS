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

#ifndef COMPUTATIONPIPELINE_H
#define COMPUTATIONPIPELINE_H

#include "JobOrderer.h"
#include "Transformation.h"

#include <QObject>

#include <vector>

class Buffer;
class Job;
class TransformationJob;
class ComputationPipelineQueue;


/**
@brief Transforms a buffer in a Computation using one or more transformations.

This class serves as a transformation pipeline for a single buffer. It encapsulates
all the mechanisms that are needed to hand the buffer from one transformation to the next,
keep track of the state, order jobs for the transformations from the multithreading
environment and so on.

The pipeline always has a certain state. There are active states like running, waiting and aborting,
and inactive states like fresh, finished, aborted, error. Check ComputationPipelineState for a
description of each state.

When constructing a ComputationPipeline, a vector of transformations has to be provided. Non-sustainable
transformations will be cloned in order to obtain a unique version of the transformation for the pipeline.
Sustainable transformations won't be cloned since they may share internal datastructures among
more than one pipeline. For this reason the constructor is also provided with a shared ComputationPipelineQueue.

When beeing the next transformation in line for a certain buffer, a sustainable transformation may already
be occupied by another pipeline. This is what the CPL_WAITING state is for, indicating that a pipeline is waiting
for a sustainable transformation to complete its current task. When the transformation finally has finished,
there may be several pipelines waiting, but it would be undefined which one would succeed in grabbing the
transformation for itself.

Since sustainable transformations may rely on the natural order of buffers (of the DB data),
like the lines transformation for example, an external datastructure like the ComputationPipelineQueue is needed
to keep the order of buffers flowing into the sustainable transformation correct, more specifically to keep it the
same as the order of buffers flowing into the Computation.

This is why in the constructor the pipeline registers itself with all its sustainable transformations at the
ComputationPipelineQueue, this way queueing in for usage of the transformations. Note that the order of pipeline
construction in the Computation equals the order of buffers flowing into the Computation!
  */
class ComputationPipeline : public JobOrderer
{
public:
    /**
    Defines the states a ComputationPipeline may have
    CPL_FRESH: The pipeline has never been worked (the state right after construction)
    CPL_RUNNING: The pipeline is working
    CPL_WAITING: The pipeline waits for a sustainable transformation
    CPL_ABORTING: The pipeline is aborting its current transformation job
    CPL_FINISHED: The pipeline has finished all transformation without problems
    CPL_ABORTED: The pipeline has been aborted all jobs have been halted
    CPL_ERROR: The pipeline has finished in an error state, e.g. because of an error in a transformation
    */
    enum ComputationPipelineState { CPL_FRESH, CPL_RUNNING, CPL_WAITING, CPL_ABORTING, CPL_FINISHED, CPL_ABORTED, CPL_ERROR };

    /// @brief Constructor
    ComputationPipeline( Buffer* input, const std::vector<Transformation*>& trafos, ComputationPipelineQueue* queue );
    /// @brief Destructor
    virtual ~ComputationPipeline();

    /// @brief Starts the execution of the pipeline
    bool execute();
    /// @brief Aborts the execution of the pipeline
    bool abort();

    /// @brief Returns the resulting buffer at the end of the transformation pipeline
    Buffer* getOutput();
    /// @brief Checks if the pipeline is working
    bool isRunning() const;
    /// @brief Checks if the pipeline has stopped
    bool hasStopped() const;
    /// @brief Checks if the output buffer may be deleted
    bool outputDeletable() const;
    /// @brief Checks the priority of the pipeline in the ComputationPipelineQueue
    int getPriority() const;
    /// @brief Returns the state of the pipeline
    ComputationPipelineState getState();

    /// @brief Sets the priority of the pipeline in the ComputationPipelineQueue
    void setPriority( int p );

    /// @brief Starts the next transformation in line
    void next();

    /// @brief Returns an error string
    const std::string& getError() const;

private:
    /// @brief Triggered if a transformation job has finished
    void trafoFinished( Job* job );
    /// @brief Triggered if a transformation job has been aborted
    void trafoAborted( Job* job );
    /// @brief Registers/unregisters the pipeline and its sustainable transformations with the ComputationPipelineQueue
    void registerInQueue( bool ok );
    /// @brief Enters waiting state for sustainable transformations
    void waitForTrafo();
    /// @brief Clears all deletable buffers
    void clearBuffers();
    /// @brief Clears all transformations
    void clearTrafos();

    /// Transformations to be applied to the buffer
    std::vector<Transformation*> trafos_;
    /// Input buffer to be transformed
    Buffer* input_;
    /// Output buffer (the result, may equal the input buffer!)
    Buffer* output_;
    /// Current intermediate buffer
    Buffer* current_input_;
    /// Currently ordered TransformationJob
    TransformationJob* current_job_;
    /// Currently applied transformation
    Transformation* current_trafo_;
    /// Current transformation step counter
    int current_;
    /// Deletable intermediate buffers
    std::vector<Buffer*> buffer_tbd_;
    /// Current pipeline state
    ComputationPipelineState state_;
    /// Queue to manage pipeline transformation order with sustainable transformations
    ComputationPipelineQueue* queue_;
    /// Priority in the ComputationPipelineQueue
    int priority_;
    /// Mutex to safeguard the aborting mechanism
    boost::mutex mutex_abort_;
    /// Error message that may be set or not
    std::string error_msg_;
};

#endif //COMPUTATIONPIPELINE_H
