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

#ifndef DBOVARIABLESETWIDGET_H_
#define DBOVARIABLESETWIDGET_H_

class QListWidget;

#include <QWidget>
#include <QMenu>
#include "DBOVariableSet.h"
#include "DBOVariableSelectionWidget.h"

/**
 * @brief Widget representation of a DBOVariableSet
 */
class DBOVariableSetWidget : public QWidget
{
    Q_OBJECT

public slots:
    /// @brief Updates the variable list
    void updateVariableList ();
    /// @brief Removes the currently selected variable
    void remove ();

protected slots:
    /// @brief Called when menu action is executed
    void triggerSlot( QAction* action );
    /// @brief Shows the context menu
    void showMenuSlot();

signals:
    /// @brief Is emitted when the set is changed
    void setChanged();

public:
    /// @brief Constructor
    DBOVariableSetWidget(DBOVariableSet &set, QWidget * parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBOVariableSetWidget();

    /// @brief Return represented set
    DBOVariableSet &getSet () { return set_; }

protected:
    /// Represented set
    DBOVariableSet set_;
    /// Context menu for adding a variable
    QMenu menu_;

    /// List of variables
    QListWidget *list_widget_;

    /// @brief Creates GUI elements
    void createElements ();
    /// @brief Updates the variables list
    void updateEntries();

};

#endif /* DBOVARIABLESetWIDGET_H_ */
