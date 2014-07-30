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

#include "Workflow.h"
#include "ComputationElement.h"


/**
Constructor.
@param name Name of the workflow.
  */
Workflow::Workflow( const std::string& name )
:   name_( name )
{
    //root node always present, forward all buffers is default
    root_ = new ComputationElement( "root" );
    root_->getFilter()->setBaseRule( BufferFilter::FORWARD );
    computations_[ "root" ] = root_;
}

/**
Configurable constructor.
Don't forget to set initial values for the registered parameters through the Configuration.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param parent Configurable parent.
  */
Workflow::Workflow( std::string class_id,
                    std::string instance_id,
                    Configurable* parent )
:   Configurable( class_id, instance_id, parent ),
    root_( NULL ),
    name_( "" )
{
    registerParameter( "name", &name_, "" );

    //create root node
    //load any saved back computation elements, they wont be interconnected to a proper tree yet!
    createSubConfigurables();

    //the interconnections happens here, the elements know about their parent and child elements
    ComputationElements::iterator it, itend = computations_.end();
    for( it=computations_.begin(); it!=itend; ++it )
        it->second->buildTree( this );
}

/**
Destructor.
  */
Workflow::~Workflow()
{
    clearComputations();
    delete root_;
}

/**
Returns the name of the workflow.
@return Workflow name.
  */
const std::string& Workflow::name() const
{
    return name_;
}

/**
Sets the workflows name.
@param name New workflow name.
  */
void Workflow::setName( const std::string& name )
{
    name_ = name;
}

/**
Returns the root ComputationElement.
@return Root element.
  */
ComputationElement* Workflow::getRoot()
{
    return root_;
}

/**
Checks if a ComputationElement of the given name exists in the workflow.
@param name Name of the desired element.
@return True if such an element exists, otherwise false.
  */
bool Workflow::hasComputation( const std::string& name ) const
{
    return ( computations_.find( name ) != computations_.end() );
}

/**
Returns the ComputationElement of the given name.
@param name Name of the desired element.
@return The desired element.
  */
ComputationElement* Workflow::getComputation( const std::string& name )
{
    if( computations_.find( name ) == computations_.end() )
        throw std::runtime_error( "Workflow: getComputation: Name not found." );
    return computations_[ name ];
}

/**
Adds a new ComputationElement by creating a new child of the given parent element.
@param parent The parent of the new element.
@param name The new elements name.
@return The created element.
  */
ComputationElement* Workflow::addComputation( ComputationElement* parent, const std::string& name )
{
    assert( parent );

    if( name == "root" )
        throw std::runtime_error( "Workflow: addComputation: Cannot create a second root node." );

    if( computations_.find( name ) != computations_.end() )
        throw std::runtime_error( "Workflow: addComputation: Duplicate name." );

    if( unusable_ )
    {
        ComputationElement* elem = new ComputationElement( name, parent );
        computations_[ name ] = elem;
    }
    else
    {
        Configuration& config = addNewSubConfiguration( "ComputationElement" );
        config.addParameterString( "name", name );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );
        if( parent )
            parent->addChild( computations_[ name ] );
    }

    //reroute debug signals
    computations_[ name ]->error_signal_.connect( error_signal_ );
    computations_[ name ]->filter_signal_.connect( filter_signal_ );
    computations_[ name ]->warn_signal_.connect( warn_signal_ );

    return computations_[ name ];
}

/**
Adds a new ComputationElement to the Workflow by creating a new element between the given element and its parent.
@param child The child element to-be of the new element.
@param name The new elements name.
@return The created element.
  */
ComputationElement* Workflow::addComputationBefore( ComputationElement* child, const std::string& name )
{
    assert( child );
    assert( child->parent() );

    if( name == "root" )
        throw std::runtime_error( "Workflow: addComputation: Cannot create a second root node." );

    if( computations_.find( name ) != computations_.end() )
        throw std::runtime_error( "Workflow: addComputation: Duplicate name." );

    if( unusable_ )
    {
        ComputationElement* elem = new ComputationElement( name, child->parent() );
        child->detach();
        elem->addChild( child );

        computations_[ name ] = elem;
    }
    else
    {
        Configuration& config = addNewSubConfiguration( "ComputationElement" );
        config.addParameterString( "name", name );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );

        //add new element to child's parent and add child to the new element
        child->parent()->addChild( computations_[ name ] );
        child->detach();
        computations_[ name ]->addChild( child );
    }

    computations_[ name ]->error_signal_.connect( error_signal_ );
    computations_[ name ]->filter_signal_.connect( filter_signal_ );
    computations_[ name ]->warn_signal_.connect( warn_signal_ );

    return computations_[ name ];
}

/**
Inserts an existing ComputationElement into the given elements child list.
This will also include the whole subtree attached to 'computation'.
@param parent New parent element.
@param idx Index in parents child list to insert the given element at.
@param computation Element to be inserted.
  */
void Workflow::insertComputation( ComputationElement* parent, int idx, ComputationElement* computation )
{
    assert( parent );
    assert( computation );

    if( computation->name() == "root" )
        throw std::runtime_error( "Workflow: insertComputation: Cannot apply this action on the root node." );

    computation->detach();
    parent->insertChild( computation, idx );
}

/**
Inserts an existing ComputationElement between the given element and its parent.
Note that after this procedure 'child' may be one amongst many children of 'computation'.
@param child The child element to-be of 'computation'.
@param computation Element to be inserted.
  */
void Workflow::insertComputationBefore( ComputationElement* child, ComputationElement* computation )
{
    assert( child );
    assert( computation );

    if( child->name() == "root" )
        throw std::runtime_error( "Workflow: insertComputationBefore: Cannot insert before root." );
    if( computation->name() == "root" )
        throw std::runtime_error( "Workflow: insertComputationBefore: Cannot apply this action on the root node." );

    computation->detach();
    child->insertChildBefore( computation );
}

/**
Removes the ComputationElement of the given name without deleting it.
@param name Name of the element to remove.
@return The removed element.
  */
ComputationElement* Workflow::removeComputation( const std::string& name )
{
    if( name == "root" )
        throw std::runtime_error( "Workflow: removeComputation: Cannot remove root." );
    if( computations_.find( name ) == computations_.end() )
        throw std::runtime_error( "Workflow: removeComputation: Name not found." );
    ComputationElement* elem = computations_[ name ];
    elem->remove( true );
    computations_.erase( name );

    return elem;
}

/**
Deletes the ComputationElement of the given name.
@param name Name of the element to be deleted.
  */
void Workflow::deleteComputation( const std::string& name )
{
    if( name == "root" )
        throw std::runtime_error( "Workflow: deleteComputation: Cannot remove root." );
    if( computations_.find( name ) == computations_.end() )
        throw std::runtime_error( "Workflow: deleteComputation: Name not found." );

    ComputationElement* elem = removeComputation( name );
    delete elem;
}

/**
Deletes the whole computation tree. The root element will be skipped by this operation.
  */
void Workflow::clearComputations()
{
    ComputationElements::iterator it, itend = computations_.end();
    for( it=computations_.begin(); it!=itend; ++it )
    {
        if( it->first == "root" )
            continue;
        delete it->second;
    }
    computations_.clear();
}

/**
Clears all computations in the tree by calling ComputationElement::clearTree() recusively on the root node.
  */
void Workflow::clear()
{
    assert( root_ );
    root_->clearTree();
}

/**
Pushes buffers to the Workflow and starts distribution over the Computation tree.
@param buffer The buffer to pass to the Workflow.
  */
void Workflow::process( Buffer* buffer )
{
    root_->processBufferSlot( buffer );
}

/**
  */
void Workflow::generateSubConfigurable( std::string class_id, std::string instance_id )
{
    if( class_id == "ComputationElement" )
    {
        ComputationElement* elem = new ComputationElement( class_id, instance_id, this );
        if( computations_.find( elem->name() ) != computations_.end() )
            throw std::runtime_error( "Workflow: generateSubConfigurable: Duplicate name." );
        computations_[ elem->name() ] = elem;

        //the elements are now just restored and registered in a map, but not interconnected. see constructor.

        //saved root
        if( elem->name() == "root" )
            root_ = elem;
    }
}

/**
  */
void Workflow::checkSubConfigurables()
{
    if( !root_ )
    {
        //always create a root node
        Configuration& config = addNewSubConfiguration( "ComputationElement" );
        config.addParameterString( "name", "root" );
        generateSubConfigurable( config.getClassId(), config.getInstanceId() );

        assert( computations_.find( "root" ) != computations_.end() );

        root_ = computations_[ "root" ];
        root_->getFilter()->setBaseRule( BufferFilter::FORWARD );   //default rule
    }
}

/**
Renames the ComputationElement of the given name.
@param name Name of the element to be renamed.
@param new_name New name.
  */
void Workflow::renameComputation( const std::string& name, const std::string& new_name )
{
    if( name == new_name )
        return;

    if( name == "root" )
        throw std::runtime_error( "Workflow: renameComputation: Root element cannot be renamed." );
    if( name.empty() )
        throw std::runtime_error( "Workflow: renameComputation: New name not valid." );

    if( computations_.find( name ) == computations_.end() )
        throw std::runtime_error( "Workflow: renameComputation: Name not found." );
    if( computations_.find( new_name ) != computations_.end() )
        throw std::runtime_error( "Workflow: renameComputation: Name already present." );

    ComputationElement* elem = computations_[ name ];
    computations_.erase( name );
    computations_[ new_name ] = elem;
    elem->setName( new_name );
}
