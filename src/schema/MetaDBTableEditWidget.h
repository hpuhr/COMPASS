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
 * MetaDBTableEditWidget.h
 *
 *  Created on: Aug 26, 2012
 *      Author: sk
 */

#ifndef METADBTABLEEDITWIDGET_H_
#define METADBTABLEEDITWIDGET_H_

#include <QWidget>
#include <map>

class MetaDBTable;
class QLineEdit;
class QComboBox;
class QGridLayout;
class QScrollArea;
class QPushButton;

/**
 * @brief Edit widget for a MetaDBTable
 */
class MetaDBTableEditWidget: public QWidget
{
    Q_OBJECT

signals:
    /// @brief Emitted when changed
    void changedMetaTable();

public slots:
    /// @brief Sets name
    void editName ();
    /// @brief Sets info
    void editInfo ();

    /// @brief Adds new sub meta table
    void addSubMetaTable ();
    /// @brief Edits sub meta table
    void editSubMetaTable ();

    /// @brief Sets main database table
    void selectTable ();

    /// @brief Updates main database table selection
    void updateTableSelection();
    /// @brief Updates sub meta tables grid
    void updateSubMetaTablesGrid ();
    void updateColumnsGrid ();
    /// @brief Updates meta table selection for new meta sub table
    void updateNewMetaTableSelection();
    /// @brief Updates local key selection for new meta sub table
    void updateLocalKeySelection ();
    /// @brief Updates sub key selection for new meta sub table
    void updateSubKeySelection ();

public:
    /// @brief Constructor
    MetaDBTableEditWidget(MetaDBTable *table_structure, QWidget * parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~MetaDBTableEditWidget();

protected:
    /// Represented meta table
    MetaDBTable *meta_table_;

    /// Name edit field
    QLineEdit *name_edit_;
    /// Info edit field
    QLineEdit *info_edit_;
    /// Main database table selection field
    QComboBox *table_box_;
    /// Key selection field
    QComboBox *key_box_;

    /// Grid with all sub meta tables
    QGridLayout *sub_meta_tables_grid_;

    QGridLayout *columns_grid_;

    /// New sub meta table local key selection
    QComboBox *new_local_key_;
    /// New sub meta table meta table selection
    QComboBox *new_table_;
    /// New sub meta table sub key selection
    QComboBox *new_sub_key_;

    /// Container with all edit buttons for sub meta tables
    std::map <QPushButton *, MetaDBTable *> edit_sub_meta_table_buttons_;
    /// Container with existing sub meta table edit widgets
    std::map <MetaDBTable *, MetaDBTableEditWidget*> edit_sub_meta_table_widgets_;

    /// @brief Creates GUI elements
    void createElements ();
};

#endif /* METADBTABLEEDITWIDGET_H_ */
