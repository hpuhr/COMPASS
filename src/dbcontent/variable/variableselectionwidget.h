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

#ifndef DBCONTENT_VARIABLESELECTIONWIDGET_H
#define DBCONTENT_VARIABLESELECTIONWIDGET_H

#include "property.h"

#include <QFrame>
#include <QMenu>

class QPushButton;
class QLabel;

class DBContentManager;

namespace dbContent
{

class Variable;
class MetaVariable;

class VariableSelectionWidget : public QFrame
{
    Q_OBJECT

  protected slots:
    void triggerSlot(QAction* action);
    void showMenuSlot();

  signals:
    void selectionChanged();

  public:
    VariableSelectionWidget(bool h_box = true, QWidget* parent = nullptr);
    ~VariableSelectionWidget();

    bool hasVariable() const { return variable_selected_; }
    Variable& selectedVariable() const;
    void selectedVariable(Variable& variable);
    void selectEmptyVariable();

    bool hasMetaVariable() const { return meta_variable_selected_; }
    MetaVariable& selectedMetaVariable() const;
    void selectedMetaVariable(MetaVariable& variable);

    bool showMetaVariables() const;
    void showMetaVariables(bool show_meta_variables);

    bool showMetaVariablesOnly() const;
    void showMetaVariablesOnly(bool show_meta_variables_only);

    void showDBOOnly(const std::string& only_dbo_name);
    void disableShowDBOOnly();

    std::string onlyDBOName() const;

    bool showEmptyVariable() const;
    void showEmptyVariable(bool show_empty_variable);

    bool showExistingInDBOnly() const;
    void showExistingInDBOnly(bool show);

    void showDataTypesOnly(const std::vector<PropertyDataType>& only_data_types);
    void disableShowDataTypesOnly();

    void setReadOnly(bool read_only);
    void updateMenuEntries();

  private:
    DBContentManager& dbo_man_;

    QLabel* object_label_{nullptr};
    QLabel* variable_label_{nullptr};
    QPushButton* sel_button_{nullptr};
    QMenu menu_;

    bool variable_selected_{false};
    bool meta_variable_selected_{false};

    bool show_empty_variable_{true};

    bool show_meta_variables_{false};
    bool show_meta_variables_only_{false};

    bool show_dbo_only_{false};
    std::string only_dbo_name_;

    bool show_data_types_only_{false};
    std::vector<PropertyDataType> only_data_types_;

    bool showDataType(PropertyDataType type);
    void updateToolTip();
};

}

#endif  // DBCONTENT_VARIABLESELECTIONWIDGET_H
