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

#ifndef VIEWMODEL_H
#define VIEWMODEL_H

#include <QObject>
#include <map>

#include "configurable.h"
#include "global.h"
#include "viewselection.h"

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
    ViewModel(const std::string& class_id, const std::string& instance_id, View* view);
    virtual ~ViewModel();

    /// @brief Redraws the data
    virtual void redrawData() = 0;
    /// @brief Clears the data and updates the display if wished.
    virtual void clear(bool update = true) = 0;

    /// @brief Returns the view
    View* getView() { return view_; }
    /// @brief Returns the widget
    ViewWidget* getWidget() { return widget_; }
    /// @brief Checks if the model obtains a Workflow, reimplement for convenience
    virtual bool hasWorkflow() const { return false; }

  protected slots:
    void sendSelection(ViewSelectionEntries& entries);
    virtual void enableSelection(bool enable);

  protected:
    virtual bool confirmSelection(ViewSelectionEntries& entries);

    /// The view the model is part of
    View* view_;
    /// The view's widget
    ViewWidget* widget_;
};

#endif  // VIEWMODEL_H
