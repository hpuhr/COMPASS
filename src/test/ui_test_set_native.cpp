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

#include "ui_test_set_native.h"
#include "ui_test_find.h"
#include "ui_test_widget_setters.h"

#include <QWidget>

/**
 * Attempts a cast of the given widget to the given type and invokes
 * the templated value setter if the cast succeeds.
 */
#define TRY_INVOKE_UI_SETTER(WidgetType, widget, value, delay, hint)                            \
    if (dynamic_cast<WidgetType*>(widget))                                                      \
        return setUIElement<WidgetType>(dynamic_cast<WidgetType*>(widget), value, delay, hint);                                               

namespace ui_test
{

/**
 * Widget type agnostic value setter.
 * Checks on native Qt widgets.
 */
bool setUIElementNative(QWidget* parent, 
                        const QString& obj_name, 
                        const QString& value, 
                        int delay,
                        const SetUIHint& hint)
{
    auto w = findObjectAs<QWidget>(parent, obj_name);
    if (w.first != FindObjectErrCode::NoError)
        return false;

    //declare setters for each type of widget here
    //corresponding template specializations need to be defined in 
    //ui_test_widget_setters.h
    TRY_INVOKE_UI_SETTER(QMenuBar, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QComboBox, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QTabWidget, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QToolBar, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QLineEdit, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QTextEdit, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QSpinBox, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QDoubleSpinBox, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QSlider, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QCheckBox, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QPushButton, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QRadioButton, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QToolButton, w.second, value, delay, hint)
    TRY_INVOKE_UI_SETTER(QAbstractButton, w.second, value, delay, hint) //for all other buttons which were not handled before

    return false;
}

} // namespace ui_test
