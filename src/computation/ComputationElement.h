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

#ifndef COMPUTATIONELEMENT_H
#define COMPUTATIONELEMENT_H

#include "TransformationVariable.h"
#include "Computation.h"

#include <map>

class Buffer;
class Workflow;


/**
@brief Stores the name of a computation elements parent or child, optionally as a configurable.

Stores the name of a computation elements parent or child as a unique id, in order to restore the correct pointers
later on when the configurables are loaded. ( See ComputationElement , Workflow )
  */
class ComputationElementID : public Configurable
{
public:
    /**
    @brief Constructor

    Constructor.
    @param id Name of a computation element.
      */
    ComputationElementID( const std::string& id ) : id_( id ) {}

    /**
    @brief Configurable constructor

    Configurable constructor.
    Don't forget to set initial values for the registered parameters through the Configuration.
    @param class_id Configurable class id.
    @param instance_id Configurable instance id.
    @param parent Configurable parent.
      */
    ComputationElementID( std::string class_id,
                          std::string instance_id,
                          Configurable* parent ) : Configurable( class_id, instance_id, parent ) { registerParameter( "id", &id_, "" ); }

    /**
    @brief Destructor

    Destructor.
      */
    virtual ~ComputationElementID() {}

    /**
    @brief Returns the stored computation element name

    Returns the stored computation element name.
    @return Stored computation element name.
      */
    const std::string& getID() const { return id_; }
    /**
    @brief Stores a new computation element name

    Stores a new computation element name.
    @param id New computation element name.
      */
    void setID( const std::string& id ) { id_ = id; }

private:
    /// Computation element name serving as an unique id
    std::string id_;
};

/**
@brief Represents a pluggable Computation element in a computation tree governed by a Workflow.

This class derives from Computation and adds functionality to build trees of calculation from one or more
Computation. Such a tree is created and governed by the Workflow class, which has EXCLUSIVE access to the
computation elements structuring methods. Use the Workflow class to govern your Computation trees.

The workflow class holds all computation elements as sub configurables. These are restored with the workflow
but are not interconnected at this point. That is why the computations hold the names of their parents and children
as configurable IDs, so that they can be restored at a later point. The buildTree() method is called from the
Workflow to restore the pointer structure in the computation elements using these computation element IDs.
  */
class ComputationElement : public Computation
{
public:
    friend class Workflow;

    typedef std::vector<ComputationElementID*> ComputationElementIDs;
    typedef std::vector<ComputationElement*> ComputationElements;

    /// @brief Constructor
    ComputationElement( const std::string& name, ComputationElement* parent=NULL );
    /// @brief Configurable constructor
    ComputationElement( std::string class_id,
                        std::string instance_id,
                        Configurable* parent );
    /// @brief Destructor
    virtual ~ComputationElement();

    /// @brief Returns the parent computation element
    ComputationElement* parent();

    /// @brief Returns the computation elements child elements
    const ComputationElements& getChildren() const;
    /// @brief Returns the computation elements child elements
    ComputationElements& getChildren();
    /// @brief Returns the computation elements idx'th child element
    ComputationElement* getChild( int idx );
    /// @brief Returns the child element of the given name
    ComputationElement* getChild( const std::string& name );
    /// @brief Returns the number of child elements
    int numChildren() const;

    /// @brief Recursively clears the computation and all of its child computations
    void clearTree();

    virtual void generateSubConfigurable( std::string class_id, std::string instance_id );

    /// Signals that the name of the computation changed (e.g. for attached generators or views)
    boost::signal<void (const std::string& name)> name_changed_signal_;

protected:
    virtual void checkSubConfigurables();

private:
    /// @brief Sets the computation elements parent element
    void setParent( ComputationElement* parent );
    /// @brief Adds a child element to the computation element
    void addChild( ComputationElement* child );
    /// @brief Inserts the given element as a child element at the idx'th position
    void insertChild( ComputationElement* child, int idx );
    /// @brief Inserts the given element between this element and its parent into tree
    void insertChildBefore( ComputationElement* child );
    /// @brief Removes the idx'th child element (without deleting it)
    ComputationElement* removeChild( int idx );
    /// @brief Removes the child element of the given name (without deleting it)
    ComputationElement* removeChild( const std::string& name );
    /// @brief Removes all child elements (without deleting them)
    void removeChildren();
    /// @brief Deletes the idx'th child element
    void deleteChild( int idx );
    /// @brief Deletes the child element of the given name
    void deleteChild( const std::string& name );
    /// @brief Deletes all child elements
    void deleteChildren();
    /// @brief Returns the index of the given child element
    int indexOfChild( ComputationElement* child ) const;

    /// @brief Detaches the element from its parent
    void detach();
    /// @brief Removes the element from the tree it is part of
    void remove( bool attach_children=false );
    /// @brief Sets the name of the computation
    void setName( const std::string& name );

    /// @brief Sets up all parent and child pointers from the stored id's
    void buildTree( Workflow* workflow );

    /// @brief Connects the element to another element (mostly a child element)
    void connect( ComputationElement* elem, bool ok=true );

    /// The parent computation element
    ComputationElement* parent_;
    /// The child elements
    ComputationElements children_;
    /// The parent elements id
    ComputationElementID* parent_id_;
    /// The child elements id's
    ComputationElementIDs child_ids_;
};

#endif //COMPUTATIONELEMENT_H
