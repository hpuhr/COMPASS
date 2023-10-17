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

#include "appmode.h"
#include "json.h"

#include <QWidget>

class ViewWidget;
class View;

/**
 * Base class for view configuration widgets, which are held in the configuration area of the ViewWidget.
 * Derive and reimplement as needed.
*/
class ViewConfigWidget : public QWidget
{
public:
    ViewConfigWidget(ViewWidget* view_widget, QWidget* parent = nullptr, Qt::WindowFlags f = 0);
    virtual ~ViewConfigWidget() = default;

    void onDisplayChange();

    virtual void loadingStarted();
    virtual void loadingDone();
    virtual void redrawStarted();
    virtual void redrawDone();
    virtual void appModeSwitch(AppMode app_mode);
    virtual void configChanged();

    virtual nlohmann::json viewInfo(const std::string& what) const { return {}; }

protected:
    virtual void onDisplayChange_impl() {} 

    ViewWidget* getWidget() { return view_widget_; }

private:
    ViewWidget* view_widget_ = nullptr;
};
