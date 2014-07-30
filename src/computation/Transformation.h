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
 * Transformation.h
 *
 *  Created on: Apr 24, 2012
 *      Author: sk
 */

#ifndef TRANSFORMATION_H_
#define TRANSFORMATION_H_

#include "Global.h"
#include "Property.h"
#include "TransformationVariable.h"
#include "TransformationBase.h"

#include <string>
#include <vector>
#include <set>
#include <map>

class Buffer;
class DBOVariable;

/// State information
enum TransformationState { TRAFO_OK, TRAFO_ERROR };


/**
@brief Base class for executable transformations

The main method to call is doExecute(). It needs at least a correctly set input buffer to operate.
A derived transformation needs to implement the execute() methos, which does the actual transformation work.
The output buffer will be set internally by the transformation, depending on special flags.

A derived transformation further has to implement the createVariables() method, which creates the needed
transformation variables, declared in the base class. These variables then have to be configured correctly
from the outside, in order to make the transformation work with various buffer types.

A derived transformation further has to implement two different clone() methods, one that just clones the
transformation and another one that returns a cloned transformation as a child of some externally
provided configurable. These methods are important to be able to provide the transformation in the
TransformationFactory.

There are three buffer types. The input buffer serves as the data source for the transformation and data
may also written back to it. The output buffer is the buffer the transformed data is written back to, it
may equal the input buffer. The ready buffer is the buffer the result can be retrieved from. The reasons why
this buffer was introduced are cleared out further below.

There are two important properties of a transformation. The first one is the append property, which defines
if the transformed data should be appended to the input buffer, or to a newly created buffer. The output buffer
will internally be set accordingly.

The second property is the sustainable property. A normal transformation will be used to transform the rows
of a single buffer independent of all other buffers. A sustainable transformation is not destroyed after transforming
a single buffer, instead it keeps some internal state that is updated with every buffer it transforms and that influences
the execution on the next buffer. These transformations are treated specially when used in a Computation of the computation
framework, see the Computation class for more on this.

A buffer added to a view filter is expected to be "full", which means that its size is a multiple of the buffer allocation
size. Buffers loaded from the database will always come in full-size except maybe the last buffer, which is ok.
For non-appending non-sustainable transformations there is at the moment no way to handle the creation of unfull buffers,
so if implementing such a transformation ALWAYS GENERATE FULL BUFFERS. This may easily be done by filling up holes with NaN
values and checking on them in your display object generators. For ALL appending transformations (sustainable and non-sustainable)
this is actually not a problem, just call outputReady() at the end of your execute() method to move the output buffer
to the ready buffer. For non-appending sustainable transformations this can be handled by calling checkOutputBuffer()
at the end of your execute() method. If the output buffer is full it will be appended to the ready buffer, otherwise it
will be reused for the next input buffer.

Scenarios:
--------------------------------------------------------------------------
Sustainable/appending:         Call outputReady() at the end of execute()
Sustainable/non-appending:     Call checkOutputBuffer() at the end of execute()
Non-sustainable/appending:     Call outputReady() at the end of execute()
Non-sustainable/non-appending: Always create full buffers, call outputReady() at the end of execute()

The transformation result should always be fetched via the fetchReadyBuffer() method, which has some internal logic. In
general be careful how to do your cleanup. When cleaning up externally in some special way, be careful to call the
fetch* methods to retrieve the buffers to be deleted, which will internally set them to NULL. Also be aware that some
of the three buffers may be equal to each other! The cleanup() method may be used to clean ALL internal buffers.
Note that per se no buffers are freed automatically.

@todo There may be flaws in the internal logic, this should be reviewed for all methods and state flags. Additionally I would
suggest to hide more internal logic from derived classes via private. The outputReady(), checkOutputBuffer() stuff should maybe
get called in doExecute() depending on the state flags? Also the cleanup logic is a little confused at the moment. So all in all
this class would need a rework.
  */
class Transformation : public TransformationBase
{
public:
    /// @brief Constructor
    Transformation( Buffer* input=NULL );
    /// @brief Configurable constructor
    Transformation( std::string class_id,
                    std::string instance_id,
                    Configurable* parent );
    /// @brief Copy constructor
    Transformation( const Transformation& copy );
    /// @brief Destructor
    virtual ~Transformation();

    //implement!
    /// @brief Assignment operator
    Transformation& operator=( const Transformation& rhs );
    /// @brief Clone function
    virtual Transformation* clone() = 0;
    /// @brief Configurable clone function
    virtual Transformation* clone( const std::string& class_id,
                                   const std::string& instance_id,
                                   Configurable* parent,
                                   bool assign ) = 0;

    /// @brief Returns if property exists in input_, sets index if true
    bool getPropertyIndex( const std::string& var_name, bool is_meta, unsigned int& index );
    /// @brief Returns if DBOVariable exists in input, sets index if true
    bool getPropertyIndex( DBOVariable* var, unsigned int& index );

    /// @brief Returns the output buffer
    Buffer *getOutput ();
    /// @brief Returns the input buffer
    Buffer *getInput ();
    /// @brief Returns the read buffer
    Buffer *getReadyBuffer() const;
    /// @brief Returns and clears input buffer
    Buffer *fetchInputBuffer ();
    /// @brief Returns an clears output buffer
    Buffer *fetchOutputBuffer ();
    /// @brief Returns and clears ready buffer
    Buffer *fetchReadyBuffer();

    /// @brief Returns identifier
    const std::string& getId () const;
    /// @brief Returns if data is appended to input buffer
    bool isAppending() const;
    /// @brief Returns if ready buffer is set
    bool isOutputReady() const;
    /// @brief Returns if reuse is possible
    bool isSustainable() const;
    /// @brief Returns if the buffer is available
    bool isAvailable() const;
    /// @brief Sets the input buffer
    void setInputBuffer( Buffer* buffer );
    /// @brief Sets the append flag
    void setAppend( bool append );
    /// @brief Sets the buffer as available
    void makeAvailable( bool available );
    /// @brief Checks if state indicates no errors
    bool isOk() const;
    /// @brief Returns the error message
    const std::string& getError() const;

    /// @brief Executes the transformation and does additional error handling
    bool doExecute();

    /// @brief Clears intermediate data
    virtual void clearIntermediateData() {}
    /// @brief Deletes and clears all buffers
    void cleanup();

    virtual void generateSubConfigurable( std::string class_id, std::string instance_id );

protected:
    virtual void checkSubConfigurables();

    /// @brief Sets the sustainable flag
    void setSustainable( bool sustainable );

    /// @brief For output buffer switching in derived classes
    void changeBuffer();
    /// @brief Creates the needed transformation variables
    virtual void createVariables() = 0;

    /// @brief Moves output buffer to ready buffer if full
    void checkOutputBuffer();
    /// @brief Declares the output as ready and transfers it to the ready buffer
    void outputReady();
    /// @brief Clears the ready buffer and optionally deletes it.
    void clearReadyBuffer( bool delete_buffer );

    /// @brief Main operation function, executes the transformation using the set input and ouput buffers
    virtual bool execute ()=0;

    /// Identifier
    std::string id_;
    /// Input buffer
    Buffer *input_;
    /// Output buffer
    Buffer *output_;
    /// Ready buffer
    Buffer *ready_buffer_;
    /// Append flag
    bool append_;
    /// Sustainable flag
    bool sustainable_;
    /// Available flag
    bool available_;

    /// State variable
    TransformationState state_;
    /// Error message
    std::string error_msg_;
};

#endif /* TRANSFORMATION_H_ */
