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

#ifndef LISTBOXVIEWCONFIGWIDGET_H_
#define LISTBOXVIEWCONFIGWIDGET_H_

#include "viewconfigwidget.h"
//#include "dbcontent/variable/variable.h"
//#include "appmode.h"

class VariableOrderedSetWidget;
class ListBoxView;
class ListBoxViewWidget;

class QComboBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QStackedWidget;

/**
 * @brief Widget with configuration elements for a ListBoxView
 *
 */
class ListBoxViewConfigWidget : public ViewConfigWidget
{
    Q_OBJECT
public:
    ListBoxViewConfigWidget(ListBoxViewWidget* view_widget, QWidget* parent = nullptr);
    virtual ~ListBoxViewConfigWidget();

public slots:
    void selectedSetSlot(const QString& text);
    void addSetSlot();
    void copySetSlot();
    void renameSetSlot();
    void removeSetSlot();

    void toggleShowOnlySeletedSlot();
    void toggleUsePresentation();
    void toggleUseOverwrite();
    void exportSlot();
    void exportDoneSlot(bool cancelled);

signals:
    void exportSignal(bool overwrite);

protected:
    void updateUpdateButton();
    void updateSetBox();
    void updateSetButtons();
    void updateSetWidget();

    ListBoxView*    view_ = nullptr;

    QComboBox*      set_box_{nullptr};
    QPushButton*    add_set_button_{nullptr};
    QPushButton*    copy_set_button_{nullptr};
    QPushButton*    rename_set_button_{nullptr};
    QPushButton*    remove_set_button_{nullptr};

    QStackedWidget* set_stack_{nullptr};

    QCheckBox*      only_selected_check_{nullptr};
    QCheckBox*      presentation_check_{nullptr};

    QCheckBox*      overwrite_check_{nullptr};

    QPushButton*    export_button_{nullptr};
};

#endif /* LISTBOXVIEWCONFIGWIDGET_H_ */
