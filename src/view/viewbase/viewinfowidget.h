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

#include "viewinfo.h"
#include "viewcomponent.h"
#include "appmode.h"

#include <QWidget>

class View;

class QGridLayout;
class QVBoxLayout;
class QLabel;

/**
 */
class ViewInfoWidget : public QWidget, public ViewComponent
{
public:
    typedef ViewInfos::Style Style;

    ViewInfoWidget(View* view, QWidget* parent = nullptr);
    virtual ~ViewInfoWidget() = default;

    bool hasItems() const;

    void clear();

    void addInfo(const std::string& id,
                 const std::string& name, 
                 const std::string& value,
                 bool value_italic);
    void addSection(const std::string& name);
    void addSpace();
    void addInfos(const ViewInfos& infos);

    void updateInfos();

    void loadingStarted();
    void loadingDone();
    void redrawStarted();
    void redrawDone();
    void appModeSwitch(AppMode app_mode);
    void configChanged();
    void onDisplayChange();

    nlohmann::json viewInfoJSON() const override final;

private:
    struct DisplayedInfo
    {
        std::string id;
        QLabel*     label_name  = nullptr;
        QLabel*     label_value = nullptr;
    };

    void reinit();
    void addLine(const std::string& id,
                 const std::string& name, 
                 const std::string& value,
                 bool is_section,
                 Style name_style = Style::None,
                 Style value_style = Style::None);

    View* view_ = nullptr;

    QWidget*     widget_      = nullptr;
    QVBoxLayout* layout_      = nullptr;
    QGridLayout* info_layout_ = nullptr;

    std::vector<DisplayedInfo> displayed_infos_;

    size_t num_items_ = 0;
};
