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

#ifndef DBCONTENT_DBCONTENTMANAGERWIDGET_H_
#define DBCONTENT_DBCONTENTMANAGERWIDGET_H_

#include <QWidget>

#include <map>

class DBContent;
class DBContentWidget;
class DBContentManager;

namespace dbContent
{
class MetaVariable;
class MetaVariableWidget;
}

class QGridLayout;
class QScrollArea;
class QPushButton;
class QLineEdit;
class QComboBox;

class DBContentManagerWidget : public QWidget
{
    Q_OBJECT

  public slots:
    /// @brief Adds a DBObject
    void addDBOSlot();
    /// @brief Is called when a DBObject was changed
    void changedDBOSlot();
    /// @brief Edits a DBObject
    void editDBOSlot();
    /// @brief Deletes a DBObject
    void deleteDBOSlot();
    /// @brief Updates the DBObject list
    void updateDBOsSlot();

    void addMetaVariableSlot();
    void editMetaVariableSlot();
    void deleteMetaVariableSlot();
    void addAllMetaVariablesSlot();
    void updateMetaVariablesSlot();

    /// @brief Unlocks editing functionality
    // void databaseOpenedSlot ();

  public:
    /// @brief Constructor
    DBContentManagerWidget(DBContentManager& object_manager);
    /// @brief Destructor
    virtual ~DBContentManagerWidget();

    //    void lock ();
    //    void unlock ();

  private:
    DBContentManager& object_manager_;

    /// Grid with all DBObjects
    QGridLayout* dbobjects_grid_{nullptr};
    QGridLayout* meta_variables_grid_{nullptr};

    /// Editing functionality unlocked flag
    // bool locked_ {false};

    /// New DBO add button
    QPushButton* add_dbo_button_{nullptr};
    QPushButton* add_metavar_button_{nullptr};

    /// Container with DBO edit buttons
    std::map<QPushButton*, DBContent*> edit_dbo_buttons_;
    /// Container with DBO edit buttons
    std::map<QPushButton*, DBContent*> delete_dbo_buttons_;

    /// Container with already existing edit DBO widgets
    std::map<DBContent*, DBContentWidget*> edit_dbo_widgets_;

    std::map<QPushButton*, dbContent::MetaVariable*> edit_meta_buttons_;
    /// Container with DBO edit buttons
    std::map<QPushButton*, dbContent::MetaVariable*> delete_meta_buttons_;
    std::map<dbContent::MetaVariable*, dbContent::MetaVariableWidget*> edit_meta_widgets_;
};

#endif /* DBCONTENT_DBCONTENTMANAGERWIDGET_H_ */
