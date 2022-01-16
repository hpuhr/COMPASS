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

#ifndef DBCONTENT_VARIABLEORDEREDSETWIDGET_H_
#define DBCONTENT_VARIABLEORDEREDSETWIDGET_H_

class QListWidget;

#include <QMenu>
#include <QWidget>

#include "dbcontent/variable/variableorderedset.h"
#include "dbcontent/variable/variableselectionwidget.h"

namespace dbContent
{

class VariableOrderedSetWidget : public QWidget
{
    Q_OBJECT

  public slots:
    /// @brief Updates the variables list
    void updateVariableListSlot();
    /// @brief Removes the selected variable
    void removeSlot();
    /// @brief Moves the selected variable up
    void moveUpSlot();
    /// @brief Moves the selected variable down
    void moveDownSlot();

  protected slots:
    /// @brief Called when menu action is executed
    void triggerSlot(QAction* action);
    /// @brief Shows the context menu
    void showMenuSlot();

  public:
    /// @brief Constructor
    VariableOrderedSetWidget(VariableOrderedSet& set, QWidget* parent = 0,
                                Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~VariableOrderedSetWidget();

  protected:
    /// Represented set
    VariableOrderedSet& set_;
    /// Context menu for adding a variable
    QMenu menu_;

    /// Variable list
    QListWidget* list_widget_{nullptr};
    int current_index_{-1};

    /// @brief Updates variables list
    void updateMenuEntries();
};

}

#endif /* DBCONTENT_VARIABLEORDEREDSETWIDGET_H_ */