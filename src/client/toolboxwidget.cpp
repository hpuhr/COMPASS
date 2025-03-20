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

#include "toolboxwidget.h"

#include <QHBoxLayout>

/**
*/
ToolBoxWidget::ToolBoxWidget(QWidget* parent)
:   QWidget(parent)
{
}

/**
*/
ToolBoxWidget::~ToolBoxWidget() = default;

/**
*/
WrappedToolBoxWidget::WrappedToolBoxWidget(QWidget* w, 
                                           const std::string& name,
                                           const std::string& info,
                                           const std::vector<std::string>& labels,
                                           const QIcon& icon,
                                           QWidget* parent)
:   ToolBoxWidget(parent)
,   name_  (name  )
,   info_  (info  )
,   labels_(labels)
,   icon_  (icon  )
{
    auto layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);

    layout->addWidget(w);
}

/**
*/
WrappedToolBoxWidget::~WrappedToolBoxWidget() = default;

/**
*/
QIcon WrappedToolBoxWidget::toolIcon() const 
{
    return icon_;
}

/**
*/
std::string WrappedToolBoxWidget::toolName() const 
{
    return name_;
}

/**
*/
std::string WrappedToolBoxWidget::toolInfo() const 
{
    return info_;
}

/**
*/
const std::vector<std::string>& WrappedToolBoxWidget::toolLabels() const
{
    return labels_;
}
