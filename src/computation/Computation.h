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

#ifndef COMPUTATION_H
#define COMPUTATION_H

//#include <QObject>

#include <set>
#include <map>
#include <string>
#include <vector>

#include <boost/thread.hpp>

#include "ComputationPipeline.h"
#include "ComputationPipelineQueue.h"
#include "Global.h"
#include "Property.h"
#include "TimedThread.h"
#include "BufferFilter.h"

#include <boost/signals2.hpp>
#include <boost/function.hpp>

class Buffer;
class Transformation;
class ComputationThread;


/**
@brief Represents a configurable Transformation entry in the Computation class.

This class serves as a storage for the Transformation entries that can be added to a Computation.
Transformations will be produced by a TransformationFactory if they are not already present as
child Configurable. The entry is also assigned a specific DBO type, since transformation in a
Computation is supposed to be buffer specific.
  */
class TransformationEntry : public Configurable
{
public:
    /// @brief Constructor
    TransformationEntry( int dbo_type,
                         const std::string& id="" );
    /// @brief Configurable constructor
    TransformationEntry( const std::string& class_id,
                         const std::string& instance_id,
                         Configurable* parent );
    /// @brief Destructor
    virtual ~TransformationEntry();

    /// @brief Returns the transformation string id
    const std::string& getID() const;
    /// @brief Returns the entries DBO type
    int getDBOType() const;
    /// @brief Returns the stored transformation
    Transformation* getTransformation();

    /// @brief Clones and sets an external transformation
    void setTransformation( Transformation* trafo );
    /// @brief Sets a new transformation through a transformation string id
    void setTransformation( const std::string& id );

    /// @brief Retrieves a configuration and fills in the given transformation entry data
    static void getConfig( Configuration& config, int dbo_type, const std::string& id="" );

    virtual void generateSubConfigurable( std::string class_id, std::string instance_id );

protected:
    virtual void checkSubConfigurables();

private:
    /// The string id of the stored transformation
    std::string id_;
    /// The DBO type the entry is assigned to
    int dbo_type_;
    /// The stored transformation
    Transformation* trafo_;
};

/**
@brief Represents a single computation unit in the computation framework.

This class serves as a single unit of computation in the computation framework. Buffers handed to the Computation will first
be filtered using a BufferFilter. Depending on the result they will be either
BLOCKED: The buffer will not be handled further.
FORWARDED: The buffer will be passed on to all attached computations.
TRANSFORMED: The buffer will be shallow copied and transformed using previously added transformations.

Transformations can be added on a per-DBO-type basis using a Transformation identifier. Instead of to a specific DBO type they can
also be added as common transformations, meaning that if common transformations are enabled, all buffers will get transformed
by the same set of transformations. It is possible to add more than one common/specific Transformation, in that case the
transformations will be applied in the order they were added to the Computation.

If a buffer is to be transformed, a ComputationPipeline will be produced for it, using the transformations retrieved for the buffer.
The ComputationPipeline represents the buffers way through all of its transformations and it will pass the buffer from transformation
to transformation until it has finished transforming. It will also handle the assignment of multithreading jobs for the specific transformations.
If a ComputationPipeline has finished, the resulting buffer will be passed on to subsequent computations for further treatment.

The Computation consists of various queues and a ComputationThread that works on them. Buffers that are accepted by the computations
BufferFilter will be inserted into an input queue. The thread will then pop them and add a ComputationPipeline for each. The pipelines
will constantly be checked by the thread for their state. Buffers from finished pipelines will be inserted into an output queue and one
after another passed on to subsequent computations by the thread.

Special treatment is needed for sustainable transformations (see class Transformation). For this special kind of Transformation there
is only a single Transformation object that is shared for all pipelines (transformations will normally get cloned for each pipeline).
Since a sustainable transformation may obtain internal datastructures that are updated with each buffer, the order incoming buffers are transformed
is especially important. The order of transformation should actually be equal to the order the buffer are handed to the Computation.
To keep the order of transformation correct throughout all pipelines a sustainable transformation is part of, a datastructure named
ComputationPipelineQueue will be used. A ComputationPipeline that encounters a sustainable transformation that is already in use will wait
until the ComputationPipelineQueue signals its ok. If the pipelines turn has finally come, the sustainable transformation will be seized and
applied to the buffer. Note that because of this, many sustainable transformations in a single computation will block parallel execution
of the computations pipelines and result in delays.

If the computation gets aborted and there are pending pipelines which have not stopped yet, new buffers may be added to the input queue,
but transformation will be postponed until all old pipelines have stopped.

When destroyed the computation will block the main thread until all pipelines have stopped.

There are various boost signals to retrieve information from, the most important beeing distribute_signal_, from which the result buffers
can be retrieved.

@todo Maybe use a single working thread for all computations, which wouldn't be that hard to realize.
@todo Calling createSubconfigurables() in the derives ComputationElement class is not so cool, since this prevents the direct usage of Computation...
  */
class Computation : public Configurable
{
public:
    friend class ComputationThread;

    typedef std::vector<TransformationEntry*> Transformations;
    typedef std::map<int,Transformations> TransformationMap;
    typedef std::vector<ComputationPipeline*> ComputationPipelines;
    typedef std::vector<Buffer*> BufferQueue;

    /// @brief Constructor
    Computation( const std::string& name );
    /// @brief Configurable constructor
    Computation( std::string class_id,
                 std::string instance_id,
                 Configurable* parent );
    /// @brief Destructor
    virtual ~Computation();

    /// @brief Adds a new transformation for the given DBO type
    Transformation* addTransformation( DB_OBJECT_TYPE dbo_type, const std::string& trafo_id );
    /// @brief Adds a new transformation for the given DBO type
    void addTransformation( DB_OBJECT_TYPE dbo_type, Transformation* trafo );
    /// @brief Deepcopies the transformations of the given Computation
    void copyTransformations( Computation* comp );
    /// @brief Deletes all added transformations
    void deleteTransformations();
    /// @brief Deletes all transformations added for the given DBO type
    void deleteTransformations( DB_OBJECT_TYPE dbo_type );
    /// @brief Returns the idx'th transformation added for the given DBO type
    Transformation* getTransformation( DB_OBJECT_TYPE dbo_type, int idx );

    /// @brief Adds a new transformation common to all DBO types
    Transformation* addCommonTransformation( const std::string& trafo_id );
    /// @brief Adds a new transformation common to all DBO types
    void addCommonTransformation( Transformation* trafo );
    /// @brief Deletes all common transformations
    void deleteCommonTransformations();
    /// @brief Enables/Disables use of common transformations
    void enableCommonTransformation( bool enable );
    /// @brief Checks if common transformations are enabled for this computation
    bool commonTransformationEnabled() const { return use_common_; }
    /// @brief Returns the idx'th common transformation
    Transformation* getCommonTransformation( int idx );

    /// @brief Returns all added transformations
    const TransformationMap& getTransformations() { return trafos_; }

    /// @brief Returns the computations name
    const std::string& name() const;

    /// @brief Locks the input queues ability to accept buffers
    void lock( bool ok );
    /// @brief Clears the Computation in a blocking way
    void clearBlocking();
    /// @brief Clears the Computation in a non-blocking way
    void clearSlot();

    /// @brief Main method to add buffers to the Computation
    void processBufferSlot( Buffer* buffer );

    /// @brief Returns the computations input buffer filter
    BufferFilter* getFilter();

    /// @brief Checks if the computation is cleared
    bool isCleared();

    /// Sends a finished buffer forth. Catch the signal to retrieve buffer data from the computation.
    boost::signals2::signal<void (Buffer*)> distribute_signal_;
    /// Signals that the Computation has been cleared (e.g. for attached generators)
    boost::signals2::signal<void (Computation*)> clear_signal_;
    /// Signals that the Computation is deleting (e.g. for attached generators or views)
    boost::signals2::signal<void ()> delete_signal_;
    /// Sends an error message for debugging purpose.
    boost::signals2::signal<void (const std::string& msg)> error_signal_;
    /// Sends an warning message for debugging purpose.
    boost::signals2::signal<void (const std::string& msg)> warn_signal_;
    /// Sends an input buffer filter specific message for debugging purpose.
    boost::signals2::signal<void (const std::string& msg)> filter_signal_;

    virtual void generateSubConfigurable( std::string class_id, std::string instance_id );

protected:
    virtual void checkSubConfigurables();

    /// @brief Transforms the given buffer
    bool transform( Buffer* buffer );
    /// @brief Starts the execution thread
    void startThread();
    /// @brief Stops the execution thread
    void stopThread();
    /// @brief Main working routine called by the execution thread
    void dispatch();
    /// @brief Clears the internal data of all sustainable transformations
    void clearIntermediateData();
    /// @brief Checks all active pipelines on their current status and handles them
    int checkActivePipelines();
    /// @brief Checks the current state of the aborting phase
    bool checkOnAborting();
    /// @brief Checks pending input buffers
    bool checkInputBuffers();
    /// @brief Checks pending output buffers
    bool checkTransformedBuffers();
    /// @brief Checks pending buffers that need forwarding
    bool checkForwardedBuffers();
    /// @brief Checks the consistency of the computations configuration
    void analyzeConfiguration();

    /// Already transformed buffers
    BufferQueue buffers_tr_;
    /// Buffers that need to be deleted (intermediate buffers etc.)
    BufferQueue buffers_tbd_;
    /// Buffers that need to be transformed
    BufferQueue buffers_tbt_;
    /// Buffers that were forwarded and need to be passed on
    BufferQueue buffers_forward_;
    /// Added buffer transformations
    TransformationMap trafos_;
    /// Running buffer-specific computation pipelines
    ComputationPipelines pipelines_;
    /// Synchronization queue for sustainable transformations
    ComputationPipelineQueue computation_queue_;

    /// Name of the computation
    std::string name_;
    /// Computation shutdown flag
    bool shutdown_;
    /// Computation aborting flag
    bool aborting_;
    /// Use common transformations flag
    bool use_common_;
    /// Number of active pipelines
    int active_pipes_;
    /// Number of buffers waiting for transformation
    int buffers_waiting_;
    /// Numbers of accepted buffers
    int buffers_in_cnt_;
    /// Numbers of passed on buffers
    int buffers_out_cnt_;
    /// Number of buffers whose pipelines were aborted
    int buffers_aborted_cnt_;
    /// Number of buffers which resulted in an erroneous pipeline
    int buffers_error_cnt_;

    /// Input buffer filter
    BufferFilter* filter_;

    /// Working thread, works the queues and pipelines
    ComputationThread* dispatch_thread_;
    /// Mutex guarding the dispatching routine of the thread. See comment in dispatch().
    boost::mutex dispatch_mutex_;
    /// Flag indicating if the thread is running
    bool thread_running_;
    /// Idle state flag for the thread
    bool idle_;
};

/**
@brief The working thread of a Computation.

This thread works the pipelines and queues of a Computation by calling Computation::dispatch()
at each interval.
  */
class ComputationThread : public TimedThread
{
public:
    /**
    @brief Constructor

    Constructor.
    @param comp The Computation that shall be worked.
    @param ms Number of milliseconds between each dispatch call.
      */
    ComputationThread( Computation* comp, unsigned int ms ) : TimedThread( ms, "ComputationThread" ), comp_( comp ) {}

    /**
    @brief Destructor

    Destructor.
      */
    ~ComputationThread() {}

protected:
    /**
    @brief Calls the dispatch routine in the governed Computation.

    Calls the dispatch routine in the governed Computation.
      */
    void workFun() { comp_->dispatch(); }

    /// Governed computation
    Computation* comp_;
};

#endif //COMPUTATION_H
