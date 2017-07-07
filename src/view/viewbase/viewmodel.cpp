
#include "viewmodel.h"
#include "view.h"
#include "viewwidget.h"


/**
@brief Constructor.
@param class_id Configurable class id.
@param instance_id Configurable instance id.
@param view The view the model is part of, configurable parent.
*/
ViewModel::ViewModel(const std::string &class_id, const std::string &instance_id, View* view )
:   Configurable (class_id, instance_id, view), view_(view), widget_(nullptr)
{
    assert(view);
    assert(view->getWidget());

    widget_ = view->getWidget();

    //connect the model to a selection change
    ViewSelection& sel = ViewSelection::getInstance();
    connect( &sel, SIGNAL(selectionChanged(bool)), this, SLOT(enableSelection(bool)) );

    //connect a proposed selection change in the widget to the model
    connect( widget_, SIGNAL(itemsSelected(ViewSelectionEntries&)), this, SLOT(sendSelection(ViewSelectionEntries&)) );
}

/**
@brief Destructor.
*/
ViewModel::~ViewModel()
{
}

/**
@brief Reacts on a change in the ViewSelection.

Implement for view specific interpretation of selection change.
@param enable If true does a selection, if false a deselection.
  */
void ViewModel::enableSelection( bool enable )
{
}

/**
@brief Alters the proposed selection coming from a view widget before it is sent to the selection.

Implement to alter the view widgets selection before it is set in the ViewSelection.
@param entries Proposed selection from the view widget.
@return True if the selection may be sent to the selection, false otherwise.
  */
bool ViewModel::confirmSelection(ViewSelectionEntries& entries)
{
    return true;
}

/**
@brief Sends the altered selection of the view widget to the ViewSelection.

Calls confirmSelection() to give the derived classes a chance to alter the widgets
proposed selection or even to cancel it.
@param entries Selection to be sent.
  */
void ViewModel::sendSelection(ViewSelectionEntries& entries)
{
    if( confirmSelection(entries))
        ViewSelection::getInstance().addSelection(entries);
}
