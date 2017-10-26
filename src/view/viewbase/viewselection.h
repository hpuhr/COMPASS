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

#ifndef VIEWSELECTION_H
#define VIEWSELECTION_H

#include <vector>
#include <set>
//#include <OGRE/Ogre.h>
#include <QObject>

#include "singleton.h"
//#include "DisplayObject.h"

/**
Identifies the source of a selection entry. The pair consists of a
source id and a key. The source id is actually just a DB_OBJECT_TYPE,
for non-database entries this can be set to DBO_UNDEFINED. They key can
be a database key or a DisplayObject id.
  */
typedef std::pair<unsigned char,unsigned int> ViewSelectionId;


/**
@brief Represents a unique entry in the view selection.

Entries are assigned a type, which can be used to quickly distinguish between
them. The entry also has a unique selection id to identify it's source data.
It is also possible to directly store a pointer to a display object in the entry.
  */
class ViewSelectionEntry
{
public:
    /// Type of the entry
    enum ViewSelectionEntryType { TYPE_UNDEFINED      = 0,
                                  TYPE_BILLBOARD      = 1<<1,
                                  TYPE_DISPLAY_OBJECT = 1<<2 };

    ViewSelectionEntry();
    ViewSelectionEntry( const ViewSelectionId& id, unsigned char type );
    ViewSelectionEntry( const ViewSelectionEntry& cpy );
    ~ViewSelectionEntry();

    void set( const ViewSelectionId& id, unsigned char type );

    bool isBillboard() const;
    bool isDisplayObject() const;
    bool isDBO() const;
    bool isGeneric() const;

    bool operator<( const ViewSelectionEntry& rhs ) const;

    /// Identifier of the entry, relates the entry to the database
    ViewSelectionId id_;
    /// Entry type
    unsigned char type_;
    /// DisplayObject assigned to the entry, not used at the moment
    //DisplayObject* dobj_; // TODO what?
};

/// A vector of selection entries.
typedef std::vector<ViewSelectionEntry> ViewSelectionEntries;

/**
@brief Stores cross view selection entries and propagates them to the views.

This singleton class is used for cross selection between several views.
The entries represent items in the database which may be visualized differently
in each view. Only visual items which relate to unique database items may be
used for cross selection.

The selection process works as follows.
- A ViewWidget generates a new selection (e.g. scene queries in OgreViewWidget).
- It clears the selection, which sends an unselect event to all views.
- The view models use the old selection to unselect the currently selected items.
- A ViewModel may change/convert/cancel the new selection and then send it to the selection (or not).
- The selection sends a select event to all views.
- The view models use the new selection entries to select the items.

@todo A ViewSelectionEntry can be used to store a display object, which actually makes only sense
for a view specific selection.
  */
class ViewSelection : public QObject, public Singleton
{
    Q_OBJECT
public:
    virtual ~ViewSelection();

    /// @brief Returns the instance to the singleton.
    static ViewSelection& getInstance()
    {
        static ViewSelection instance;
        return instance;
    }

    const ViewSelectionEntries& getEntries() const;
    ViewSelectionEntries& getEntries();
    ViewSelectionEntry& getEntry( unsigned int idx );
    bool hasEntry( const ViewSelectionId& id ) const;

    /// Global selection color
    //static Ogre::ColourValue selection_col_;
    /// Global selection alpha (unused)
    static float selection_alpha_;

signals:
    /// @brief Sends a signal to update views on a selection change.
    void selectionChanged( bool selected );
    /// @brief Sends a signal that the selection has updated.
    void selectionUpdated();
    /// @brief Sends a follow signal, to synchronize iteration over the selection in more than one view.
    void follow( unsigned int idx );

public slots:
    void setSelection( const ViewSelectionEntries& entries );
    void addSelection( const ViewSelectionEntries& entries );
    void clearSelection();

protected:
    ViewSelection();

private:
    /// Stored selection entries
    ViewSelectionEntries entries_;
    /// All stored entry id's
    std::set<ViewSelectionId> ids_;
};

/**
@brief An iteration container for ViewSelection entries.

Used to iterate over selections entries. Just create an instance and start
iterating via nextItem(), previousItem() etc.

The instances of this class can be snychronized by enabling the follow mode.
If follow mode is active, current item changes will be propagated over the selection
to all other instances with active follow mode and then synchronized.

Changes of the current item can be catched from the outside by connecting
to currentItemChanged().
  */
class ViewSelectionItemContainer : public QObject
{
    Q_OBJECT
public:
    ViewSelectionItemContainer();
    virtual ~ViewSelectionItemContainer();

    bool empty() const;
    void reset();
    void nextItem();
    void update();
    void previousItem();
    ViewSelectionEntry* currentItem();

    void enableFollowMode( bool enable );

protected slots:
    void selectionUpdated();
    void followSlot( unsigned int idx );

signals:
    /// @brief Signals that the current item has changed.
    void currentItemChanged();
    /// @brief Snychronizes the selection of the given item in all other instances with active follow mode.
    void follow( unsigned int idx );

private:
    /// Current entry index
    unsigned int idx_;
    /// Follow mode flag
    bool follow_;
};

#endif //VIEWSELECTION_H
