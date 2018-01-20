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

#include "viewselection.h"

#include <algorithm>
#include <stdexcept>

//Ogre::ColourValue ViewSelection::selection_col_ = Ogre::ColourValue( 1.0, 1.0, 0.0, 1.0 );
float ViewSelection::selection_alpha_ = 0.0;


/******************************************************************************************
ViewSelectionEntry
******************************************************************************************/

/**
@brief Constructor.
  */
ViewSelectionEntry::ViewSelectionEntry()
    :   id_( -1, -1 ), type_( TYPE_UNDEFINED ) //, dobj_( NULL )
{
}

/**
@brief Constructor.
@param id Entry identifier.
@param type Entry type.
  */
ViewSelectionEntry::ViewSelectionEntry( const ViewSelectionId& id, unsigned char type )
    :   id_( id ), type_( type ) //,dobj_( NULL )
{
}

/**
@brief Copy constructor.
@param cpy Instance to copy.
  */
ViewSelectionEntry::ViewSelectionEntry( const ViewSelectionEntry& cpy )
    :   id_( cpy.id_ ), type_( cpy.type_ )//, dobj_( cpy.dobj_ )
{
}

/**
@brief Destructor.
  */
ViewSelectionEntry::~ViewSelectionEntry()
{
}

/**
@brief Sets the entries data.
@param id Entry identifier.
@param type Entry type.
  */
void ViewSelectionEntry::set( const ViewSelectionId& id, unsigned char type )
{
    id_ = id;
    type_ = type;
}

/**
@brief Checks if the entry represents a billboard.
@return True if the entry represents a billboard, false otherwise.
  */
bool ViewSelectionEntry::isBillboard() const
{
    return ( type_ & TYPE_BILLBOARD ) != 0;
}

/**
@brief Checks if the entry represents a display object.
@return True if the entry represents a display object, false otherwise.
  */
bool ViewSelectionEntry::isDisplayObject() const
{
    return ( type_ & TYPE_DISPLAY_OBJECT ) != 0;
}

/**
@brief Checks if the entry stems from the database.
@return True if the entry stems from the database, false otherwise.
  */
bool ViewSelectionEntry::isDBO() const
{
    return id_.first > 0;
}

/**
@brief Checks if the entry stems from a generic item and not from the database.
@return True if generic entry, false otherwise.
  */
bool ViewSelectionEntry::isGeneric() const
{
    return id_.first == 0;
}

/**
@brief Ordering for selection entries.
@param rhs Entry to compare the entry to.
@return True if smaller, false otherwise.
  */
bool ViewSelectionEntry::operator<( const ViewSelectionEntry& rhs ) const
{
    return id_ < rhs.id_;
}

/******************************************************************************************
ViewSelection
******************************************************************************************/

/**
@brief Constructor.
  */
ViewSelection::ViewSelection()
{
}

/**
@brief Destructor.
  */
ViewSelection::~ViewSelection()
{
}

/**
@brief Sets a new selection.

Will signal all views connected to the selection that the selection changed.
The views will handle selection of the new items individually.
@param entries The new entries.
  */
void ViewSelection::setSelection( const ViewSelectionEntries& entries )
{
    entries_ = entries;

    ids_.clear();
    unsigned int i, n = entries_.size();
    for( i=0; i<n; ++i )
        ids_.insert( entries_[ i ].id_ );

    //inform views
    emit selectionChanged( true );
    emit selectionUpdated();
}

/**
@brief Adds the given entries to the existing selection.

Will signal all views connected to the selection that the selection changed.
The views will handle selection of the new items individually.
@param entries Entries to be added to the selection.
  */
void ViewSelection::addSelection( const ViewSelectionEntries& entries )
{
    entries_.insert( entries_.end(), entries.begin(), entries.end() );

    unsigned int i, n = entries.size();
    for( i=0; i<n; ++i )
        ids_.insert( entries[ i ].id_ );

    //inform views
    emit selectionChanged( true );
    emit selectionUpdated();
}

/**
@brief Clears the selection.

This is always called before a new selection happens.
The views will handle deselection of the old items individually.
  */
void ViewSelection::clearSelection()
{
    //inform views to unselect
    emit selectionChanged( false );

    entries_.clear();
    ids_.clear();
    emit selectionUpdated();
}

/**
@brief Returns the selection entries.
@return entries.
  */
const ViewSelectionEntries& ViewSelection::getEntries() const
{
    return entries_;
}

/**
@brief Returns the selection entries.
@return entries.
  */
ViewSelectionEntries& ViewSelection::getEntries()
{
    return entries_;
}

/**
@brief Returns the idx'th entry.
@param idx Index of the desired entry.
@return Desired entry.
  */
ViewSelectionEntry& ViewSelection::getEntry( unsigned int idx )
{
    if( idx >= entries_.size() )
        throw std::runtime_error( "ViewSelection::getEntry(): Index out of range." );
    return entries_[ idx ];
}

/**
@brief Checks if the given id exists as an entry.
@param id Selection id.
@return True if the given id exists in the selection, false otherwise.
  */
bool ViewSelection::hasEntry( const ViewSelectionId& id ) const
{
    return ( ids_.find( id ) != ids_.end() );
}

/******************************************************************************************
ViewSelectionItemContainer
******************************************************************************************/

/**
@brief Constructor.
  */
ViewSelectionItemContainer::ViewSelectionItemContainer()
    :   idx_( 0 ),
      follow_( false )
{
    connect( &ViewSelection::getInstance(), SIGNAL(selectionUpdated()),
             this, SLOT(selectionUpdated()) );
    connect( &ViewSelection::getInstance(), SIGNAL(follow(unsigned int)),
             this, SLOT(followSlot(unsigned int)) );
    connect( this, SIGNAL(follow(unsigned int)),
             &ViewSelection::getInstance(), SIGNAL(follow(unsigned int)) );
}

/**
@brief Destructor.
  */
ViewSelectionItemContainer::~ViewSelectionItemContainer()
{
}

/**
@brief Checks if there are entries to iterate over.
@return True if there are entries to iterate over, otherwise false.
  */
bool ViewSelectionItemContainer::empty() const
{
    return ViewSelection::getInstance().getEntries().empty();
}

/**
@brief Resets the selected entry to the first item.
  */
void ViewSelectionItemContainer::reset()
{
    idx_ = 0;
    update();
}

/**
@brief Updates the container.

Sends all important events.
  */
void ViewSelectionItemContainer::update()
{
    if( empty() )
        return;

    if( follow_ )
    {
        emit follow( idx_ );
        return;
    }

    emit currentItemChanged();
}

/**
@brief Selects the next entry.

Jumps back to the first item if overflowing.
  */
void ViewSelectionItemContainer::nextItem()
{
    const ViewSelectionEntries& entries = ViewSelection::getInstance().getEntries();
    ++idx_;
    if( idx_ >= entries.size() )
        idx_ = 0;

    update();
}

/**
@brief Selects the previous entry.

Jumps forth to the last item if underflowing.
  */
void ViewSelectionItemContainer::previousItem()
{
    const ViewSelectionEntries& entries = ViewSelection::getInstance().getEntries();
    if( idx_ == 0 )
        idx_ = entries.size()-1;
    else
        --idx_;

    update();
}

/**
@brief Returns the currently selected entry.
@return Currently selected entry.
  */
ViewSelectionEntry* ViewSelectionItemContainer::currentItem()
{
    ViewSelectionEntries& entries = ViewSelection::getInstance().getEntries();
    if( entries.empty() || idx_ >= entries.size() )
        return NULL;

    return &entries[ idx_ ];
}

/**
@brief Triggered by selection changes.

Resets the selected entry to the first item.
  */
void ViewSelectionItemContainer::selectionUpdated()
{
    idx_ = 0;
    if( follow_ && !empty() )
        emit currentItemChanged();
}

/**
@brief Enables the follow mode.
@param enable If true the current item changes will be synchronized between all instances
with activated follow mode.
  */
void ViewSelectionItemContainer::enableFollowMode( bool enable )
{
    follow_ = enable;
}

/**
@brief Synchronizes the selected item on changes in another instance.
@param idx Index of the entry to select.
  */
void ViewSelectionItemContainer::followSlot( unsigned int idx )
{
    const ViewSelectionEntries& entries = ViewSelection::getInstance().getEntries();
    if( !follow_ || idx_ >= entries.size() )
        return;

    idx_ = idx;
    emit currentItemChanged();
}
