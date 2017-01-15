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
 * DBOVariableOrderedSetWidget.h
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#ifndef DBOVARIABLEORDEREDSETWIDGET_H_
#define DBOVARIABLEORDEREDSETWIDGET_H_

class QListWidget;

#include <QWidget>
#include <QMenu>
#include "DBOVariableOrderedSet.h"
#include "DBOVariableSelectionWidget.h"

/**
 * @brief Widget representation of a DBOVariableOrderedSet
 */
class DBOVariableOrderedSetWidget : public QWidget
{
    Q_OBJECT

public slots:
    /// @brief Updates the variables list
    void updateVariableList ();
    /// @brief Removes the selected variable
    void remove ();
    /// @brief Moves the selected variable up
    void moveUp ();
    /// @brief Moves the selected variable down
    void moveDown ();

protected slots:
    /// @brief Called when menu action is executed
    void triggerSlot( QAction* action );
    /// @brief Shows the context menu
    void showMenuSlot();

signals:
    /// @brief Emitted when set is changed
    void setChanged();

public:
    /// @brief Constructor
    DBOVariableOrderedSetWidget(DBOVariableOrderedSet *set, QWidget * parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBOVariableOrderedSetWidget();

protected:
    /// Represented set
    DBOVariableOrderedSet *set_;
    /// Context menu for adding a variable
    QMenu menu_;

    /// Variable list
    QListWidget *list_widget_;

    /// @brief Creates GUI elements
    void createElements ();
    /// @brief Updates variables list
    void updateEntries();

};

#endif /* DBOVariableOrderedSetWidget_H_ */
