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

#ifndef DBOBJECTEDITWIDGET_H_
#define DBOBJECTEDITWIDGET_H_

#include <QWidget>
#include <map>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QGridLayout;
class QPushButton;
class QTextEdit;

class DBObject;
class DBODataSourceDefinition;
class DBOVariable;
class DBSchemaSelectionComboBox;
class DBOVariableDataTypeComboBox;
class StringRepresentationComboBox;

/**
 * @brief Edit widget for a DBObject
 */
class DBObjectWidget : public QWidget
{
    Q_OBJECT

  signals:
    void changedDBOSignal();

  public slots:
    void updateDataSourcesGridSlot();

    void editNameSlot();
    void editInfoSlot();

    void editDBOVariableNameSlot();
    void editDBOVariableDescriptionSlot();
    void editDBOVariableDBColumnSlot(const QString& text);
    void deleteDBOVarSlot();

    void updateDBOVarsGridSlot();

    void showLabelDefinitionWidgetSlot();

  public:
    DBObjectWidget(DBObject* object, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~DBObjectWidget();

  private:
    DBObject* object_{nullptr};

    QLineEdit* name_edit_{nullptr};
    QLineEdit* info_edit_{nullptr};

    QPushButton* edit_label_button_{nullptr};

    QGridLayout* ds_grid_{nullptr};

    /// @brief grid with all meta tables per schema
    QGridLayout* meta_table_grid_{nullptr};

    QPushButton* new_meta_button_{nullptr};

    /// @brief Grid with all DBOVariables
    QGridLayout* dbovars_grid_{nullptr};
};

#endif /* DBOBJECTEDITWIDGET_H_ */
