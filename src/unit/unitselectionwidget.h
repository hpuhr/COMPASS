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

#ifndef UNITSELECTIONWIDGET_H_
#define UNITSELECTIONWIDGET_H_

#include <QPushButton>
#include <QMenu>

/**
 * @brief Sets a Unit using a context menu
 */
class UnitSelectionWidget : public QPushButton
{
    Q_OBJECT


protected slots:
    /// @brief Called when menu action is executed
    void triggerSlot( QAction* action );
    /// @brief Shows the context menu
    void showMenuSlot();

public:
    /// @brief Constructor
    UnitSelectionWidget (std::string &quantity, std::string &unit);
    /// @brief Destructor
    virtual ~UnitSelectionWidget();

protected:
    /// Unit dimension reference
    std::string &quantity_;
    /// Unit unit reference
    std::string &unit_;

    /// Context menu
    QMenu menu_;

    void createMenu();
};

#endif /* UNITSELECTIONWIDGET_H_ */
