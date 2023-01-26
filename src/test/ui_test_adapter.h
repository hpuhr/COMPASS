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

#include "ui_test_set.h"

#include <QRect>
#include <QString>
#include <QWidget>

#include <boost/optional.hpp>

class QWidget;
class QString;

#define UI_TEST_DEFINE_ADAPTER(WidgetType, var_name) \
    template<>                                       \
    boost::optional<UITestAdapter::AdapterRerouteResult> UITestAdapter::rerouteWidget(WidgetType* var_name)

namespace ui_test
{

class UITestAdapter
{
public:
    /**
     * Hint describing where in the rerouted widget the setter should be applied.
     * This is a way to apply a setter for a native Qt widget to a complex (non-native) widget.
     */
    struct AdapterRerouteResult
    {
        QWidget* widget = nullptr; //a new root widget to be processed, most likely a subwidget of the rerouted widget
        QString  obj_name;         //a new object name, most likely the object name of a subwidget of the rerouted widget
        QRect    roi;              //a hint of where in the rerouted widget to execute the setter after rerouting
    };

    template<class T>
    static bool setUIElement(QWidget* w, const QString& value, int delay = -1)
    {
        //widget matches template?
        T* w_cast = dynamic_cast<T*>(w);
        if (!w_cast)
            return {};

        //reroute widget via adapter to obtain a child which is a native Qt widget
        auto res = rerouteWidget<T>(w_cast);
        if (!res.has_value())
            return false;

        //generate hint if roi was create by rerouting
        SetUIHint hint;
        if (!res.value().roi.isEmpty())
        {
            hint.x = res.value().roi.width()  / 2;
            hint.y = res.value().roi.height() / 2;
        }
        
        //try to reinterpret the reroute result as native Qt widget
        return setUIElementNative(res.value().widget ? res.value().widget : w, res.value().obj_name, value, delay, hint);
    } 

    /**
     * Specialize for widget specific behavior.
     * Reroutes a complex widget to a certain child widget which is a native
     */
    template<class T>
    static boost::optional<AdapterRerouteResult> rerouteWidget(T* w)
    {
        //default behavior
        return {};
    }
};

} // namespace ui_test
