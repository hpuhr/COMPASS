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

#ifndef DBTABLEEDITWIDGET_H_
#define DBTABLEEDITWIDGET_H_

#include <QWidget>

#include "configurable.h"

class DBTable;
class DBTableColumn;
class QLineEdit;
class QComboBox;
class QGridLayout;
class QScrollArea;
class QPushButton;
class UnitSelectionWidget;

/**
 * @brief Edit widget for a DBTable
 */
class DBTableWidget : public QWidget
{
    Q_OBJECT

signals:

public slots:
    void infoSlot (const QString& value);
//    void setSpecialNull (const QString &text);

public:
    /// @brief Constructor
    DBTableWidget(DBTable& table, QWidget* parent=0, Qt::WindowFlags f=0);
    /// @brief Destructor
    virtual ~DBTableWidget();

    void lock ();

protected:
    /// Represented table
    DBTable& table_;

    /// Table info edit field
    QLineEdit* info_edit_ {nullptr};

    /// Grid with all table columns
    QGridLayout* column_grid_ {nullptr};

    /// Container with all column special null edit fields
    //std::map <QLineEdit *, DBTableColumn* > column_grid_special_nulls_;

    std::map <UnitSelectionWidget*, DBTableColumn*> column_unit_selection_widgets_;

    /// @brief Updates the table columns grid
    void updateColumnGrid ();
};

#endif /* DBTABLEEDITWIDGET_H_ */
