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

#ifndef WORKFLOW_H
#define WORKFLOW_H

#include "Configurable.h"

#include <map>
#include <string>

#include "signalslib.h"
#include <boost/function.hpp>

class ComputationElement;
class Buffer;


/**
@brief Encapsulates a Computation tree.

This class serves as a container for a tree of computation elments. It provides interfaces
to manipulate the tree hierarchy, e.g. remove elements, add or insert new ones, etc.

On creation there will always be a root ComputationElement present, which cannot be removed.
Its initial configuration is just to forward all buffers, but it may be configured otherwise.

The interface to pass data to the Workflow is the process() method, which takes a buffer and
distributes it over the Computation tree. Results may be retrieved from any ComputationElement
in the tree by connecting to its distribute_signal_ boost signal. Examples for classes that may
want to attach themselves on a ComputationElement include display object generators and views.

Note that this is the only place in which the computation elements connection structure may be
manipulated, which makes administration of the elements much easier.

Additionally there exist various debug signals one may retrieve debug output from. Those are rerouted
from all computations in the workflow.

@code
Workflow my_workflow("wf");
ComputationElement* c1 = my_workflow.addComputation( my_workflow.getRoot(), "C1" );
ComputationElement* c2 = my_workflow.addComputation( my_workflow.getRoot(), "C2" );
ComputationElement* c3 = my_workflow.addComputationBefore( c1, "C3" );
my_workflow.renameComputation( "C3", "AddBeforeComputation" );
@endcode

The example code produces the following tree.

"root"----"AddBeforeComputation"----"C1"
      \
       ------"C2"
  */
class Workflow : public Configurable
{
public:
    typedef std::map<std::string,ComputationElement*> ComputationElements;

    /// @brief Constructor
    Workflow( const std::string& name );
    /// @brief Configurable constructor
    Workflow( std::string class_id,
              std::string instance_id,
              Configurable* parent );
    /// @brief Destructor
    virtual ~Workflow();

    /// @brief Pushes buffers to the Workflow and starts distribution over the Computation tree
    void process( Buffer* buffer );

    /// @brief Returns the name of the workflow
    const std::string& name() const;
    /// @brief Sets the name of the workflow
    void setName( const std::string& name );

    /// @brief Checks if a ComputationElement of the given name exists in the workflow
    bool hasComputation( const std::string& name ) const;
    /// @brief Returns the root ComputationElement
    ComputationElement* getRoot();
    /// @brief Returns the ComputationElement of the given name
    ComputationElement* getComputation( const std::string& name );
    /// @brief Adds a new ComputationElement by creating a new child of the given parent element
    ComputationElement* addComputation( ComputationElement* parent, const std::string& name );
    /// @brief Adds a new ComputationElement to the Workflow by creating a new element between the given element and its parent
    ComputationElement* addComputationBefore( ComputationElement* child, const std::string& name );
    /// @brief Inserts an existing ComputationElement into the given elements child list
    void insertComputation( ComputationElement* parent, int idx, ComputationElement* computation );
    /// @brief Inserts an existing ComputationElement between the given element and its parent
    void insertComputationBefore( ComputationElement* child, ComputationElement* computation );
    /// @brief Removes the ComputationElement of the given name without deleting it
    ComputationElement* removeComputation( const std::string& name );
    /// @brief Deletes the ComputationElement of the given name
    void deleteComputation( const std::string& name );
    /// @brief Deletes the whole computation tree (except the root element)
    void clearComputations();
    /// @brief Renames the ComputationElement of the given name
    void renameComputation( const std::string& name, const std::string& new_name );

    /// Clears all computations in the tree
    void clear();

    virtual void generateSubConfigurable( std::string class_id, std::string instance_id );

    /// Sends errors produced in the computations
    boost::signal<void (const std::string& msg)> error_signal_;
    /// Sends warnings produced in the computations
    boost::signal<void (const std::string& msg)> warn_signal_;
    /// Sends filter debug output produced in the computations
    boost::signal<void (const std::string& msg)> filter_signal_;

protected:
    virtual void checkSubConfigurables();

private:
    /// Created computation elements
    ComputationElements computations_;
    /// Root element
    ComputationElement* root_;
    /// Workflow name
    std::string name_;
};

#endif //WORKFLOW_H
