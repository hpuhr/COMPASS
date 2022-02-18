/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DBFILTERWIDGET_H_
#define DBFILTERWIDGET_H_

#include <QFrame>
#include <QMenu>

#include "dbfilter.h"

class QWidget;
class QCheckBox;
class QVBoxLayout;
class QPushButton;

/**
 * @brief Qt frame widget, which represents a DBFilter
 *
 * Has general elements, like a label with the filter name, show/hide button and a manage button for
 * generic filters. A child widget is embedded, which is the filters widget itself. Emits
 * possibleFilterChange when something is changed, but the filter decides of the change is
 * propagated to the FilterManager (based on the active_ flag).
 */
class DBFilterWidget : public QFrame
{
    Q_OBJECT
  private slots:
    /// @brief Visibility toggle
    void toggleVisible();
    /// @brief And operator toggle, not used yet
    void toggleAnd();
    /// @brief Active toggle
    void toggleActive();
    /// @brief Slot for sub-filters to propagate their changes. Not used yet.
    void possibleSubFilterChange();
    /// @brief Reset function
    void reset();
    /// @brief Deletes the filter
    void deleteFilter();
    /// @brief Value changed in edit field
    void filterEditSlot();

  protected slots:
    /// @brief Shows the management menu
    void showMenuSlot();

  signals:
    /// @brief Emitted when the filter is changed
    void possibleFilterChange();
    /// @brief Emitted when the file should be edited
    void filterEdit(DBFilter* filter);
    void deleteFilterSignal(DBFilter* filter);

  public:
    /// @brief Constructor
    DBFilterWidget(DBFilter& filter);
    /// @brief Destructor
    virtual ~DBFilterWidget();

    /// @brief Adds a child widget
    void addChildWidget(QWidget* widget);
    /// @brief Updates the child widget
    void updateChildWidget();

    /// @brief Updates the management elements
    virtual void update(void);

    void setInvisible()
    {
        filter_.widgetVisible(false);
        child_->setVisible(false);
    }

  protected:
    DBFilter& filter_;

    QWidget* child_ {nullptr}; // Child widget from DBFilter

    QCheckBox* visible_checkbox_ {nullptr};
    QCheckBox* active_checkbox_ {nullptr};

    QPushButton* manage_button_ {nullptr};

    QVBoxLayout* child_layout_ {nullptr};

    QMenu menu_;

    void createMenu();
};

#endif /* DBFILTERWIDGET_H_ */
