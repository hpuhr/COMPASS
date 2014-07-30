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

/*
 * DBFilterWidget.h
 *
 *  Created on: May 4, 2012
 *      Author: sk
 */

#ifndef DBFILTERWIDGET_H_
#define DBFILTERWIDGET_H_

#include <QFrame>
#include <QMenu>
#include "Configurable.h"

class QWidget;
class QCheckBox;
class QVBoxLayout;
class DBFilter;
class QPushButton;

/**
 * @brief Qt frame widget, which represents a DBFilter
 *
 * Has general elements, like a label with the filter name, show/hide button and a manage button for generic filters.
 * A child widget is embedded, which is the filters widget itself. Emits possibleFilterChange when something is changed,
 * but the filter decides of the change is propagated to the FilterManager (based on the active_ flag).
 */
class DBFilterWidget : public QFrame, public Configurable
{
  Q_OBJECT
private slots:
  /// @brief Visibility toggle
  void toggleVisible();
  /// @brief And operator toggle, not used yet
  void toggleAnd();
  /// @brief Active toggle
  void toggleActive ();
  /// @brief Invert function, not used yet
  void invert ();
  /// @brief Slot for sub-filters to propagate their changes. Not used yet.
  void possibleSubFilterChange ();
  /// @brief Reset function
  void reset ();
  /// @brief Deletes the filter
  void deleteFilter ();
  /// @brief Value changed in edit field
  void filterEditSlot ();

protected slots:
  /// @brief Shows the management menu
  void showMenuSlot();

signals:
  /// @brief Emitted when the filter is changed
  void possibleFilterChange();
  /// @brief Emitted when the file should be edited
  void filterEdit (DBFilter *filter);

public:
  /// @brief Constructor
  DBFilterWidget(DBFilter &filter, std::string class_id, std::string instance_id);
  /// @brief Destructor
  virtual ~DBFilterWidget();

  /// @brief Adds a child widget
  void addChildWidget (QWidget *widget);
  /// @brief Updates the child widget
  void updateChildWidget ();

  /// @brief Updates the management elements
  virtual void update (void);

protected:
  /// DBFilter that is represented by this widget
  DBFilter &filter_;
  /// Visibility flag
  bool visible_;
  /// Child widget from DBFilter
  QWidget *child_;
  /// Visibility checkbox
  QCheckBox *visible_checkbox_;
  /// Active checkbox
  QCheckBox *active_checkbox_;
  //QCheckBox *and_checkbox_;
  //QCheckBox *invert_checkbox_;
  /// Manage filter button
  QPushButton *manage_button_;

  /// Layout for child widget
  QVBoxLayout *child_layout_;

  /// Filter management menu
  QMenu menu_;

  /// Creates widget GUI elements
  virtual void createGUIElements ();
  /// Creates the menu_
  void createMenu ();
};

#endif /* DBFILTERWIDGET_H_ */
