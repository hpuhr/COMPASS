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

#pragma once

#include <QWidget>

class VariableOrderedSetWidget;
class ListBoxViewDataSource;

class QComboBox;
class QPushButton;
class QStackedWidget;

/**
 * Governs editing of the ListBoxViewDataSource.
 */
class ListBoxViewSetConfigWidget : public QWidget
{
    Q_OBJECT
public:
    ListBoxViewSetConfigWidget(ListBoxViewDataSource* data_source, QWidget* parent = nullptr);
    virtual ~ListBoxViewSetConfigWidget();

    void updateFromDataSource();

signals:
    void changed();
    
protected:
    void addSet();
    void copySet();
    void renameSet();
    void removeSet();

    void updateSetCombo();
    void updateActiveStates();
    void updateSetWidget();

    void setCurrentSet(const QString& set);

    ListBoxViewDataSource* data_source_ = nullptr;

    QComboBox*      current_set_combo_ = nullptr;
    QPushButton*    add_set_button_    = nullptr;
    QPushButton*    copy_set_button_   = nullptr;
    QPushButton*    rename_set_button_ = nullptr;
    QPushButton*    remove_set_button_ = nullptr;
    QStackedWidget* set_stack_         = nullptr;
};
