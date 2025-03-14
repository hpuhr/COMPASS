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

#include "toolbox.h"
#include "toolboxwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>

const int ToolBox::ToolIconSize     = 50;
const int ToolBox::ToolNameFontSize = 12;

/**
 */
ToolBox::ToolBox(QWidget* parent)
:   QWidget(parent)
{
    createUI();
}

/**
 */
ToolBox::~ToolBox() = default;

/**
 */
void ToolBox::createUI()
{
    QHBoxLayout* main_layout = new QHBoxLayout;
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->setSpacing(0);
    setLayout(main_layout);

    QVBoxLayout* left_layout  = new QVBoxLayout;
    left_layout->setContentsMargins(0, 0, 0, 0);
    left_layout->setSpacing(0);

    icon_layout_ = new QVBoxLayout;
    icon_layout_->setContentsMargins(0, 0, 0, 0);
    icon_layout_->setSpacing(0);

    left_layout->addLayout(icon_layout_);
    left_layout->addStretch();
    
    main_layout->addLayout(left_layout);
    
    right_widget_ = new QWidget;
    right_widget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    right_widget_->setVisible(false); //initiallly invisible

    QVBoxLayout* right_layout = new QVBoxLayout;
    right_layout->setContentsMargins(0, 0, 0, 0);
    right_layout->setSpacing(0);

    right_widget_->setLayout(right_layout);

    QHBoxLayout* top_layout = new QHBoxLayout;
    top_layout->setContentsMargins(10, 10, 10, 10);

    tool_name_label_ = new QLabel;
    auto name_font = tool_name_label_->font();
    name_font.setBold(true);
    name_font.setPointSize(ToolNameFontSize);
    tool_name_label_->setFont(name_font);

    top_layout->addWidget(tool_name_label_);

    widget_stack_ = new QStackedWidget;
    right_layout->addLayout(top_layout);
    right_layout->addWidget(widget_stack_);

    main_layout->addWidget(right_widget_);
}

/**
 */
void ToolBox::addTool(ToolBoxWidget* tool)
{
    assert(tool);

    auto icon = tool->toolIcon();
    auto name = tool->toolName();
    auto info = tool->toolInfo();

    auto button = new QToolButton;
    button->setIcon(icon);
    button->setText(QString::fromStdString(name));
    button->setToolTip(QString::fromStdString(info));
    button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
    button->setIconSize(QSize(ToolIconSize, ToolIconSize));

    tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    int toolIdx = (int)tools_.size();

    connect(button, &QToolButton::pressed, [ this, toolIdx ] () { this->toolActivated(toolIdx); });

    Tool t;
    t.widget = tool;
    t.button = button;

    tools_.push_back(t);
    widget_stack_->addWidget(tool);
    icon_layout_->addWidget(button);
}

/**
 */
void ToolBox::toolActivated(int idx)
{
    if (idx == active_tool_idx_)
    {
        right_widget_->setVisible(false);
        active_tool_idx_ = -1;
        return;
    }

    const auto& tool = tools_.at(idx);

    tool_name_label_->setText(QString::fromStdString(tool.widget->toolName()));

    active_tool_idx_ = idx;
    widget_stack_->setCurrentIndex(idx);
    right_widget_->setVisible(true);
}
