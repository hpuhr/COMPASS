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

#include <vector>

#include <QWidget>

class ToolBoxWidget;

class QToolButton;
class QVBoxLayout;
class QStackedWidget;
class QLabel;
 
/**
 */
class ToolBox : public QWidget
{
    Q_OBJECT
public:
    ToolBox(QWidget* parent = nullptr);
    virtual ~ToolBox();

    void addTool(ToolBoxWidget* tool);
    size_t numTools() const;

    void adjustSizings();

    void selectTool(size_t idx);
    bool selectTool(const std::string& name);

    static const int ToolIconSize;
    static const int ToolNameFontSize;
    static const int ToolLabelFontSize;

    static const int StretchToolBox;
    static const int StretchView;

signals:
    void toolToggled(int toolIdx);

private:
    struct Tool
    {
        int            idx    = -1;
        ToolBoxWidget* widget = nullptr;
        QToolButton*   button = nullptr;
        bool           hidden = false;
    };

    void createUI();
    void toolActivated(int idx);

    std::vector<Tool> tools_;

    QVBoxLayout*    icon_layout_     = nullptr;
    QStackedWidget* widget_stack_    = nullptr;
    QWidget*        right_widget_    = nullptr;
    QLabel*         tool_name_label_ = nullptr;

    int active_tool_idx_ = -1;
};
