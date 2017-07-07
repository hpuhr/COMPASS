
#ifndef VIEWMODEL_H
#define VIEWMODEL_H

#include <QObject>

#include <map>

#include "global.h"
#include "viewselection.h"
#include "configurable.h"

class View;
class ViewWidget;


/**
@brief Manages a view's data and configures the view.

The model holds the views datastructures and it is the views main interface
to configure the display, add data to the view, manage it, etc. It thus contains
most of the view's logic.

This base class just implements some basic selection handling.
  */
class ViewModel : public QObject, public Configurable
{
    Q_OBJECT
public:
    ViewModel(const std::string &class_id, const std::string &instance_id, View* view );
    virtual ~ViewModel();

    /// @brief Redraws the data
    virtual void redrawData() = 0;
    /// @brief Clears the data and updates the display if wished.
    virtual void clear( bool update=true ) = 0;

    /// @brief Returns the view
    View* getView() { return view_; }
    /// @brief Returns the widget
    ViewWidget* getWidget() { return widget_; }
    /// @brief Checks if the model obtains a Workflow, reimplement for convenience
    virtual bool hasWorkflow() const { return false; }

protected slots:
    void sendSelection( ViewSelectionEntries& entries );
    virtual void enableSelection( bool enable );

protected:
    virtual bool confirmSelection( ViewSelectionEntries& entries );

    /// The view the model is part of
    View* view_;
    /// The view's widget
    ViewWidget* widget_;
};

#endif //VIEWMODEL_H
