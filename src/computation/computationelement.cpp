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

#include "ComputationElement.h"
#include "PropertyList.h"
#include "Buffer.h"
#include "Workflow.h"


/**
Constructor.
@param name The computations name.
@param parent The parent computation element (may be NULL).
  */
//ComputationElement::ComputationElement( const std::string& name, ComputationElement* parent )
//:   Computation( name ),
//    parent_( NULL ),
//    parent_id_( NULL )
//{
//    if( parent )
//        parent->addChild( this );
//}

/**
Configurable constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
  */
ComputationElement::ComputationElement( std::string class_id,
                                        std::string instance_id,
                                        Configurable* parent )
:   Computation( class_id, instance_id, parent ),
    parent_( NULL ),
    parent_id_( NULL )
{
    createSubConfigurables();
}

/**
Destructor.
  */
ComputationElement::~ComputationElement()
{
    //delete IDs
    if( parent_id_ )
        delete parent_id_;

    unsigned int i, n = child_ids_.size();
    for( i=0; i<n; ++i )
        delete child_ids_[ i ];
}

/**
Sets the parent computation element.
@param parent New parent computation element. May be NULL.
  */
void ComputationElement::setParent( ComputationElement* parent )
{
    parent_ = parent;

//    if( unusable_ )
//        return;

    if( parent_ )
    {
        if( !parent_id_ )
        {
            Configuration& config = addNewSubConfiguration( "ComputationElementIDParent", "ComputationElementIDParent0" );
            config.addParameterString( "id", parent_->name() );
            generateSubConfigurable( "ComputationElementIDParent", "ComputationElementIDParent0" );
        }

        assert( parent_id_ );
        parent_id_->setID( parent_->name() );
    }
    else
    {
        if( parent_id_ )
        {
            delete parent_id_;
            parent_id_ = NULL;
        }
    }
}

/**
Returns the parent computation element.
@return Parent computation element.
  */
ComputationElement* ComputationElement::parent()
{
    return parent_;
}

/**
Returns all child computation elements.
@return Child computation elements.
  */
const ComputationElement::ComputationElements& ComputationElement::getChildren() const
{
    return children_;
}

/**
Returns all child computation elements.
@return Child computation elements.
  */
ComputationElement::ComputationElements& ComputationElement::getChildren()
{
    return children_;
}

/**
Returns the idx'th child computation element.
@param idx Index of the desired child.
@return Desired child computation element.
  */
ComputationElement* ComputationElement::getChild( int idx )
{
    if( idx < 0 || idx >= (signed)children_.size() )
        throw( "ComputationElement: getChild: Index out of bounds." );
    return children_[ idx ];
}

/**
Returns the child computation element of the given name.
@param name Name of the desired child.
@return Desired child computation element.
  */
ComputationElement* ComputationElement::getChild( const std::string& name )
{
    int i, n = children_.size();
    for( i=0; i<n; ++i )
    {
        if( children_[ i ]->name() == name )
            return children_[ i ];
    }

    throw( "ComputationElement: getChild: No child of given name." );
}

/**
Adds the given computation element as a child element.
@param child Computation element to be added as a child element.
  */
void ComputationElement::addChild( ComputationElement* child )
{
    assert( child );

    children_.push_back( child );
    child->setParent( this );
    connect( child );

//    if( unusable_ )
//        return;

    Configuration& config = addNewSubConfiguration( "ComputationElementIDChild" );
    config.addParameterString( "id", child->name() );
    generateSubConfigurable( config.getClassId(), config.getInstanceId() );
}

/**
Inserts the given computation element as a child element at the idx'th position.
@param child Computation element to be added as a child element.
@param idx Index the element should be inserted at.
  */
void ComputationElement::insertChild( ComputationElement* child, int idx )
{
    if( idx < 0 || idx >= (signed)children_.size() )
        throw( "ComputationElement: insertChild: Index out of bounds." );

    assert( child );

    children_.insert( children_.begin()+idx, child );
    child->setParent( this );
    connect( child );

//    if( unusable_ )
//        return;

    Configuration& config = addNewSubConfiguration( "ComputationElementIDChild", "" );
    ComputationElementID* id = new ComputationElementID( config.getClassId(), config.getInstanceId(), this );
    id->setID( child->name() );
    child_ids_.insert( child_ids_.begin()+idx, id );
}

/**
Inserts the given computation element between this element and its parent into the tree.
@param child Computation element to be inserted.
  */
void ComputationElement::insertChildBefore( ComputationElement* child )
{
    if( !parent_ )
        throw( "ComputationElement: addChildBefore: Cannot add before root node." );
    parent_->removeChild( this->name() );
    parent_->addChild( child );
    child->addChild( this );
}

/**
Removes the idx'th child element without deleting it.
@param idx Index of the child element to be removed.
@return The removed child element.
  */
ComputationElement* ComputationElement::removeChild( int idx )
{
    if( idx < 0 || idx >= (signed)children_.size() )
        throw( "ComputationElement: removeChild: Index out of bounds." );

    ComputationElement* elem = children_[ idx ];
    children_.erase( children_.begin()+idx );
    elem->setParent( NULL );

    connect( elem, false );

//    if( !unusable_ )
//    {
//        delete child_ids_[ idx ];
//        child_ids_.erase( child_ids_.begin()+idx );
//    }

    return elem;
}

/**
Removes the child element of the given name without deleting it.
@param name Name of the child element to be removed.
@return The removed child element.
  */
ComputationElement* ComputationElement::removeChild( const std::string& name )
{
    int i, n = children_.size();
    for( i=0; i<n; ++i )
    {
        if( children_[ i ]->name() == name )
        {
            ComputationElement* elem = children_[ i ];
            children_.erase( children_.begin()+i );
            elem->setParent( NULL );

            connect( elem, false );

//            if( !unusable_ )
//            {
//                delete child_ids_[ i ];
//                child_ids_.erase( child_ids_.begin()+i );
//            }

            return elem;
        }
    }

    throw( "ComputationElement: removeChild: No child of given name." );
}

/**
Deletes the idx'th child element.
@param idx Index of the child element to be deleted.
  */
void ComputationElement::deleteChild( int idx )
{
    ComputationElement* elem = getChild( idx );
    elem->remove( false );
    delete elem;
}

/**
Deletes the child element of the given name.
@param name Name of the child element to be deleted.
  */
void ComputationElement::deleteChild( const std::string& name )
{
    ComputationElement* elem = getChild( name );
    elem->remove( false );
    delete elem;
}

/**
Removes all child computation elements without deleting them.
  */
void ComputationElement::removeChildren()
{
    int i, n = children_.size();
    for( i=n; i>=0; --i )
        removeChild( i );
}

/**
Deletes all child computation elements.
  */
void ComputationElement::deleteChildren()
{
    int i, n = children_.size();
    for( i=n; i>=0; --i )
        deleteChild( i );
}

/**
Returns the number of child computation elements.
@return Number of child computation elements.
  */
int ComputationElement::numChildren() const
{
    return child_ids_.size();
}

/**
Detaches the element (and the subtree it represents) from its parent.
  */
void ComputationElement::detach()
{
    if( !parent_ )
        return;

    //remove me from parent (will set my parent to NULL)
    parent_->removeChild( name() );
}

/**
Removes the element from the tree hierarchy and attaches the children to the parent node if needed.
@param attach_children If true the elements children will be added to the elements parent as new children.
    If false the elements children will become parentless.
  */
void ComputationElement::remove( bool attach_children )
{
    if( !parent_ )
        return;

    int i, n = children_.size();

    //restructure children...
    if( attach_children )
    {
        for( i=0; i<n; ++i )
        {
            connect( children_[ i ], false );
            parent_->addChild( children_[ i ] );
        }
    }
    else
    {
        for( i=0; i<n; ++i )
        {
            children_[ i ]->setParent( NULL );
            connect( children_[ i ], false );
        }
    }

    //...an detach this one from its parent
    detach();
}

/**
Sets the name of the computation.
@param name New computation name.
  */
void ComputationElement::setName( const std::string& name )
{
    name_ = name;

    //inform attached units (generators, views, etc. may rely on the name of the comp. they are attached to)
    name_changed_signal_( name_ );
}

/**
Returns the index of the given child element.
@param child Child element the index shall be retrieved for.
@return Index of the given child element in the child vector. May return -1 if
    the given child has not been found.
  */
int ComputationElement::indexOfChild( ComputationElement* child ) const
{
    int i, n = children_.size();
    for( i=0; i<n; ++i )
    {
        if( children_[ i ] == child )
            return i;
    }

    return -1;
}

/**
Connects/disconnects the element to a subsequent element, most likely a child. Feel free
to add more communication here.
@param elem Computation element to connect/disconnect.
@param ok Connects if true, disconnects if false.
  */
void ComputationElement::connect( ComputationElement* elem, bool ok )
{
    if( ok )
        distribute_signal_.connect( boost::bind( &ComputationElement::processBufferSlot, elem, _1) );
    else
        distribute_signal_.disconnect( boost::bind( &ComputationElement::processBufferSlot, elem, _1) );
}

/**
Clears the element and its subtree recursively.
  */
void ComputationElement::clearTree()
{
    clearSlot();

    unsigned int i, n = children_.size();
    for( i=0; i<n; ++i )
        children_[ i ]->clearTree();
}

/**
Sets up all parent and child pointers from the stored id's.
This method is used to restore the connection between the several computation
elements governed as configurables in a workflow, by using the given workflow and
the stored computation element IDs (no more than the unique element names).

  */
void ComputationElement::buildTree( Workflow* workflow )
{
    assert( workflow );

    //retrieve the parent from workflow
    if( parent_id_ )
        parent_ = workflow->getComputation( parent_id_->getID() );

    //retrieve the children from workflow
    unsigned int i, n = child_ids_.size();
    for( i=0; i<n; ++i )
    {
        ComputationElement* child = workflow->getComputation( child_ids_[ i ]->getID() );
        connect( child, true );
        children_.push_back( child );
    }
}

/**
  */
void ComputationElement::generateSubConfigurable( std::string class_id, std::string instance_id )
{
    Computation::generateSubConfigurable( class_id, instance_id );

    if( class_id == "ComputationElementIDChild" )
    {
        ComputationElementID* id = new ComputationElementID( class_id, instance_id, this );
        child_ids_.push_back( id );
    }

    if( class_id == "ComputationElementIDParent" )
    {
        if( parent_id_ )
            delete parent_id_;
        parent_id_ = new ComputationElementID( class_id, instance_id, this );
    }

    if( !hasSubConfigurable( class_id, instance_id ) )
        throw std::runtime_error( "ComputationElement: generateSubConfigurable: Subconfigurable could not be added: " + class_id + "," + instance_id );
}

/**
  */
void ComputationElement::checkSubConfigurables()
{
    Computation::checkSubConfigurables();
}
