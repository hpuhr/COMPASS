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

#include "ui_test_set.h"
#include "ui_test_set_native.h"
#include "ui_test_find.h"
#include "ui_test_adapters.h"

#include <QWidget>

/**
 * Tries to invoke the given adapter and returns true if the adapter matched the passed widget's type.
 */
#define TRY_INVOKE_ADAPTER(WidgetType, widget, value, delay)                 \
    {                                                                        \
        if (UITestAdapter::setUIElement<WidgetType>(widget, value, delay))   \
            return true;                                                     \
    }                                                  

namespace ui_test
{

/**
 * Widget type agnostic value setter.
 * Includes checks on adapters.
 */
bool setUIElement(QWidget* parent, 
                  const QString& obj_name, 
                  const QString& value, 
                  int delay)
{
    auto w = findObjectAs<QWidget>(parent, obj_name);
    if (w.first != FindObjectErrCode::NoError)
        return false;
    
    //here we can add adapters for special widgets defined in ui_test_adapters.h
    TRY_INVOKE_ADAPTER(dbContent::VariableSelectionWidget, w.second, value, delay)

    //type of widget could not be processed, try native Qt types
    return setUIElementNative(parent, obj_name, value, delay);
}

} // namespace ui_test
