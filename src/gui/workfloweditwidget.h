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


#ifndef WORKFLOWEDITWIDGET_H
#define WORKFLOWEDITWIDGET_H

#include "Global.h"

#include <vector>

#include <QTreeWidget>
#include <QTreeWidgetItem>

class Workflow;
class ComputationElement;
class Transformation;
class BufferFilter;


/**
@brief The base class for all tree widget items of the WorkflowEditWidget's tree view

The base class for all tree widget items of the WorkflowEditWidget's tree view.
Defines the user item types for all derived items.
  */
class WorkflowEditWidgetItem : public QTreeWidgetItem
{
public:
    /// Define user tree widget item types for derived classes here
    enum MyUserTypes{ TypeWorkflow=QTreeWidgetItem::UserType,
                      TypeComputationElement,
                      TypeTransformation,
                      TypeFilter,
                      TypeView,
                      TypeGenerator };

    /// @brief Constructor
    WorkflowEditWidgetItem( QTreeWidgetItem::ItemType type );
    /// @brief Destructor
    virtual ~WorkflowEditWidgetItem();
};

/**
@brief Represents a tree widget item for a workflow

Represents a tree widget item for a workflow.
  */
class WorkflowTreeItem : public WorkflowEditWidgetItem
{
public:
    /// @brief Constructor
    WorkflowTreeItem( Workflow* workflow );
    /// @brief Destructor
    virtual ~WorkflowTreeItem();

    /** @brief Returns the workflow represented by the item.

    Returns the workflow represented by the item.
    @return Represented workflow.
    */
    Workflow* getWorkflow() { return workflow_; }

private:
    /// Workflow the entry represents
    Workflow* workflow_;
};

/**
@brief Represents a tree widget item for a computation
  */
class ComputationElementTreeItem : public WorkflowEditWidgetItem
{
public:
    /// @brief Constructor
    ComputationElementTreeItem( ComputationElement* element, Workflow* wf );
    /// @brief Destructor
    virtual ~ComputationElementTreeItem();

    /**
    @brief Returns the represented computation element.

    Returns the represented computation element.
    @return Represented element.
      */
    ComputationElement* getElement() { return element_; }

    /**
    @brief Returns the represented computations workflow.

    Returns the represented computations workflow.
    @return Workflow.
      */
    Workflow* getWorkflow() { return wf_; }

private:
    /// Represented computation element
    ComputationElement* element_;
    /// The elements workflow
    Workflow* wf_;
};

/**
@brief Represents a tree widget item for a transformation
  */
class TransformationTreeItem : public WorkflowEditWidgetItem
{
public:
    /// @brief Constructor
    TransformationTreeItem( Transformation* trafo, int dbo_type, ComputationElement* elem );
    /// @brief Destructor
    virtual ~TransformationTreeItem();

    /**
    @brief Returns the represented transformation

    Returns the represented transformation.
    @return Represented transformation.
      */
    Transformation* getTransformation() { return trafo_; }

private:
    /// @brief Updates the items text
    void updateText();

    /// DBO type assigned to the transformation
    int dbo_type_;
    /// Computation element the Transformation belongs to
    ComputationElement* elem_;
    /// The represented transformation
    Transformation* trafo_;
};

/**
@brief Represents a tree widget item for a buffer filter
  */
class BufferFilterTreeItem : public WorkflowEditWidgetItem
{
public:
    /// @brief Constructor
    BufferFilterTreeItem( BufferFilter* filter );
    /// @brief Destructor
    virtual ~BufferFilterTreeItem();

    /**
    @brief Returns the represented buffer filter

    Returns the represented buffer filter.
    @return Represented buffer filter.
      */
    BufferFilter* getBufferFilter() { return filter_; }

private:
    /// Represented buffer filter.
    BufferFilter* filter_;
};

/**
@brief A widget to edit one or more workflows.

Add, insert remove your workflows and always call updateTree() to update the widget.
Note that the changes made to the workflows by this widget are immediate.

One can edit nearly all aspects of a workflow using this widget.

@todo The workflows are not backupped at the moment, so when editing their components
they will immediately get changed, which isn't good practice. This feature is implemented
partially in other edit dialogs that are opened from inside this widget. Pulling this through
would require deep copy abilities of Computation, ComputationElement, Workflow.
  */
class WorkflowEditWidget : public QTreeWidget
{
    Q_OBJECT
public:
    /// @brief Constructor
    WorkflowEditWidget( QWidget* parent=NULL );
    /// @brief Destructor
    virtual ~WorkflowEditWidget();

    /// @brief Adds a new workflow to the widget
    void addWorkflow( Workflow* wf );
    /// @brief Inserts a new workflow into the widget
    void insertWorkflow( Workflow* wf, int idx=-1 );
    /// @brief Removes a workflow from the widget
    void removeWorkflow( int idx );
    /// @brief Returns the index position of the given workflow in the tree
    int findWorkflow( Workflow* wf ) const;
    /// @brief Returns the number of workflows added to the widget
    int numberOfWorkflows() const;

    /// @brief Updates the tree and displays the added workflows
    virtual void updateTree();

protected slots:
    /// @brief Starts an edit dialog for a single workflow
    void editWorkflowSlot();
    /// @brief Deletes a computation
    void deleteComputationSlot();
    /// @brief Adds a new computation
    void addParentComputationSlot();
    /// @brief Adds a new computation
    void addChildComputationSlot();
    /// @brief Starts an edit dialog for a computation
    void editComputationSlot();
    /// @brief Starts an edit dialog for a transformation
    void editTransformationSlot();
    /// @brief Starts an edit dialog for an input buffer filter
    void editFilterSlot();

protected:
    /// @brief Shows a context menu, depending on the click position
    virtual void contextMenuEvent ( QContextMenuEvent* event );

    /// @brief Adds menu entries to the computation context menu
    virtual void getComputationMenu( QMenu& menu, const QVariant& item_data );

    /// @brief Shows the main context menu
    void contextMenuMain();
    /// @brief Shows the workflow context menu
    void contextMenuWorkflow( QTreeWidgetItem* item );
    /// @brief Shows the computation context menu
    void contextMenuComputation( QTreeWidgetItem* item );
    /// @brief Shows the transformation context menu
    void contextMenuTransformation( QTreeWidgetItem* item );
    /// @brief Shows the buffer filter context menu
    void contextMenuFilter( QTreeWidgetItem* item );

    /// @brief Encodes a tree widget item into a variant
    QVariant encode( QTreeWidgetItem* item ) const;
    /// @brief Retrieves a tree widget item from a variant
    QTreeWidgetItem* decode( const QVariant& var ) const;

    /// @brief Recursive method to build the trees entries
    void addComputationsRecursive( QTreeWidgetItem* parent, ComputationElement* elem, Workflow* wf );

    /// Added workflows
    std::vector<Workflow*> workflows_;
    /// Added computation items
    std::map<std::string,ComputationElementTreeItem*> comp_items_;
};

#endif //WORKFLOWEDITWIDGET_H
