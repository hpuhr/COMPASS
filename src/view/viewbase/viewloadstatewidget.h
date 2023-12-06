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

#include <string>

#include <QWidget>
#include <QColor>

class ViewWidget;

class QLabel;
class QPushButton;
class QToolButton;
class QCheckBox;

/**
 * Widget keeping/displaying the current data state and handling manual updates like reloading and redrawing.
*/
class ViewLoadStateWidget : public QWidget
{
public:
    enum class State
    {
        None = 0,
        NoData,
        Loading,
        Drawing,
        Loaded,
        ReloadRequired,
        RedrawRequired
    };

    ViewLoadStateWidget(ViewWidget* view_widget, QWidget* parent = nullptr);
    virtual ~ViewLoadStateWidget() = default;

    void setState(State state);
    void updateState();
    
    void loadingStarted();
    void loadingDone();
    void redrawStarted();
    void redrawDone();

    void appModeSwitch(AppMode app_mode);

    virtual nlohmann::json viewInfo(const std::string& what) const { return {}; }

    static const int DefaultMargin = 4;

private:
    static std::string messageFromState(State state);
    static QColor colorFromState(State state);
    static std::string buttonTextFromState(State state);

    void updateData();
    void setAutoReload(bool enabled);

    State        state_              = State::None;

    QLabel*      status_label_       = nullptr;
    QPushButton* refresh_button_     = nullptr;
    QToolButton* auto_refresh_button_ = nullptr;
    QCheckBox*   auto_refresh_box_    = nullptr;

    ViewWidget*  view_widget_        = nullptr;
};
