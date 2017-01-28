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


#ifndef DBSCHEMAWIDGET_H_
#define DBSCHEMAWIDGET_H_

#include <QWidget>
#include <QDialog>
#include <map>

class QCheckBox;
class QPushButton;
class QGridLayout;

class DBTable;
class DBTableEditWidget;
class MetaDBTable;
class MetaDBTableEditWidget;
class DBSchema;

/**
 * @brief Widget for setting the current and adding a new DBSchema
 */
class DBSchemaWidget : public QWidget
{
    Q_OBJECT

signals:
    /// @brief Is emitted when the schema was renamed
    void renamed ();

private slots:
    /// @brief Adds a DBTable
    void addTable();
    /// @brief Adds all possible DBTables
    void addAllTables();
    /// @brief Adds a MetaDBTable
    void addMetaTable();
    /// @brief Edits a DBTable
    void editTable();
    void deleteTable();
    /// @brief Edits a MetaDBTable
    void editMetaTable ();
    /// @brief Called when a DBTable was changed
    void changedTable();
    /// @brief Called when a MetaDBTable was changed
    void changedMetaTable ();

public:
    /// @brief Constructor
    DBSchemaWidget(DBSchema &schema, QWidget * parent = 0, Qt::WindowFlags f = 0);
    /// @brief Destructor
    virtual ~DBSchemaWidget();

protected:
    /// Schema to be edited
    DBSchema &schema_;

    QCheckBox *auto_populate_check_;

    /// Grid for all tables
    QGridLayout *table_grid_;
    /// Grid for all meta tables
    QGridLayout *meta_table_grid_;

    /// Container for table edit buttons
    std::map <QPushButton *, DBTable *> edit_table_buttons_;
    std::map <QPushButton *, DBTable *> delete_table_buttons_;

    /// Container for meta table edit buttons
    std::map <QPushButton *, MetaDBTable *> edit_meta_table_buttons_;

    /// @brief Creates GUI elements
    void createElements ();

    /// Updates DBTable grid
    void updateTableGrid();
    /// Updates MetaDBTable grid
    void updateMetaTablesGrid();
};

#endif /* DBSCHEMAWIDGET_H_ */
