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


#ifndef DBOVARIABLESELECTIONWIDGET_H
#define DBOVARIABLESELECTIONWIDGET_H

#include "dbovariable.h"

#include <QGroupBox>
#include <QMenu>

class QPushButton;
class QLabel;


//typedef struct {
//    std::string id;
//    unsigned int dbo_type;
//} SelectionId;

/**
 * @brief Widget for selection of a DBOVariable
 *
 * Selection is made using a context menu (which is triggered by a button), definition of the variable is shown in two
 * QLineEdit fields. Provides getter and setter methods for access.
 */
class DBOVariableSelectionWidget : public QGroupBox
{
    Q_OBJECT
public:
    /// @brief Constructor without variable
    DBOVariableSelectionWidget (bool show_title=true, bool h_box=false, QWidget* parent=nullptr );
    /// @brief Constructor with given variable
    DBOVariableSelectionWidget (DBOVariable *init, bool show_title=true, bool h_box=false, QWidget* parent=nullptr );
    /// @brief Destructor
    ~DBOVariableSelectionWidget();

    /// @brief Returns if a variable is selected
    bool hasVariable() const { return sel_var_ != nullptr; }
    /// @brief Return selected variable
    DBOVariable* getSelectedVariable() const;
    /// @brief Sets the selected variable
    void setSelectedVariable (DBOVariable *var);

protected slots:
    /// @brief Slot for menu triggered action
    void triggerSlot( QAction* action );
    /// @brief Slot for show menu
    void showMenuSlot();

signals:
    /// @brief Signal if variable was changed
    void selectionChanged();

private:
    /// @brief Creates GUI elements
    void createControls();
    /// @brief Updates selection menu entries
    void updateEntries();

    /// Select variable button
    QPushButton* sel_button_;
    /// Variable type information
    QLabel* sel_edit_type_;
    /// Variable name information
    QLabel* sel_edit_id_;
    /// Context menu for variable selection
    QMenu menu_;
    /// Selected variable
    DBOVariable *sel_var_;
    /// Layout horizontal flag
    bool h_box_;
};

#endif //DBOVARIABLESELECTIONWIDGET_H
