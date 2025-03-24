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

#include "stringconv.h"
#include "files.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QPushButton>
#include <QStackedWidget>
#include <QLabel>
#include <QFontMetrics>
#include <QMenu>
#include <QToolBar>

const int ToolBox::ToolIconSize      = 50;
const int ToolBox::ToolNameFontSize  = 12;
const int ToolBox::ToolLabelFontSize = 8;

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
    top_layout->setContentsMargins(10, 5, 10, 0);

    tool_name_label_ = new QLabel;
    auto name_font = tool_name_label_->font();
    name_font.setBold(true);
    //name_font.setPointSize(ToolNameFontSize);
    tool_name_label_->setFont(name_font);

    tool_bar_ = new QToolBar;

    config_button_ = new QPushButton;
    config_button_->setStyleSheet("QPushButton::menu-indicator { image: none; }");
    config_button_->setIcon(QIcon(Utils::Files::getIconFilepath("edit.png").c_str()));
    config_button_->setFixedSize(UI_ICON_SIZE); 
    config_button_->setFlat(UI_ICON_BUTTON_FLAT);

    top_layout->addWidget(tool_name_label_);
    top_layout->addStretch(1);
    top_layout->addWidget(tool_bar_);
    top_layout->addStretch(1);
    top_layout->addWidget(config_button_);

    widget_stack_ = new QStackedWidget;

    right_layout->addLayout(top_layout);
    right_layout->addWidget(widget_stack_);

    main_layout->addWidget(right_widget_);

    updateMenu();
    updateToolBar();
}

/**
 */
void ToolBox::updateMenu()
{
    config_button_->setMenu(nullptr);

    config_menu_.reset(new QMenu);

    if (active_tool_idx_ < 0)
        return;

    // tool specific custom part
    tools_.at(active_tool_idx_).widget->addToConfigMenu(config_menu_.get());

    if (config_menu_->actions().size() > 0)
        config_menu_->addSeparator();

    //screen ratio selection
    {
        auto sr_menu = config_menu_->addMenu("Screen Ratio");

        QActionGroup* screen_ratio_group = new QActionGroup(config_menu_.get());
        screen_ratio_group->setExclusive(true);

        auto sr = currentScreenRatio();

        auto addSRAction = [ & ] (const QString& label, toolbox::ScreenRatio screen_ratio)
        {
            auto action = sr_menu->addAction(label);
            screen_ratio_group->addAction(action);

            action->setCheckable(true);
            action->setChecked(sr.has_value() && sr.value() == screen_ratio);
            connect(action, &QAction::triggered, [ this, screen_ratio] () { this->screenRatioChanged(screen_ratio); });
        };

        addSRAction("1:3", toolbox::ScreenRatio::Ratio_Quarter);
        addSRAction("1:2", toolbox::ScreenRatio::Ratio_Third);
        addSRAction("1:1", toolbox::ScreenRatio::Ratio_Half);
        addSRAction("2:1", toolbox::ScreenRatio::Ratio_TwoThirds);
        addSRAction("3:1", toolbox::ScreenRatio::Ratio_ThreeQuarter);
    }

    config_button_->setMenu(config_menu_.get());
}

/**
 */
void ToolBox::updateToolBar()
{
    tool_bar_->clear();

    if (active_tool_idx_ < 0)
        return;

    // tool specific custom part
    tools_.at(active_tool_idx_).widget->addToToolBar(tool_bar_);

    //if (tool_bar_->actions().size() > 0)
    //    tool_bar_->addSeparator();

    //add general actions
}

/**
 */
void ToolBox::addTool(ToolBoxWidget* tool)
{
    assert(tool);

    auto icon   = tool->toolIcon();
    auto name   = tool->toolName();
    auto info   = tool->toolInfo();
    auto labels = tool->toolLabels();

    std::string label = Utils::String::compress(labels, '\n');

    auto button = new QToolButton;

    auto font = button->font();
    font.setPointSize(ToolLabelFontSize);

    button->setIcon(icon);
    button->setText(QString::fromStdString(label));
    button->setToolTip(QString::fromStdString(info));
    button->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    button->setIconSize(QSize(ToolIconSize, ToolIconSize));
    button->setCheckable(true);
    button->setFont(font); 

    tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    int toolIdx = (int)tools_.size();

    connect(button, &QToolButton::pressed, [ this, toolIdx ] () { this->toolActivated(toolIdx); });

    Tool t;
    t.idx    = toolIdx;
    t.widget = tool;
    t.button = button;

    tools_.push_back(t);
    widget_stack_->addWidget(tool);
    icon_layout_->addWidget(button);
}

/**
 */
size_t ToolBox::numTools() const
{
    return tools_.size();
}

/**
 */
void ToolBox::selectTool(size_t idx)
{
    assert(idx >= 0);
    assert(idx < numTools());

    //deactivate any active tool
    toolActivated(-1);

    //activate new tool
    tools_.at(idx).button->blockSignals(true);
    tools_.at(idx).button->setChecked(true);
    tools_.at(idx).button->blockSignals(false);

    toolActivated((int)idx);
}

/**
 */
bool ToolBox::selectTool(const std::string& name)
{
    //find tool of the given name
    auto it = std::find_if(tools_.begin(), tools_.end(), [ & ] (const Tool& t) { return t.widget->toolName() == name; });
    if (it == tools_.end())
        return false;

    //select
    int idx = std::distance(tools_.begin(), it);
    selectTool(idx);

    return true;
}

/**
 */
void ToolBox::toolActivated(int idx)
{
    assert(idx < (int)tools_.size());

    if (idx < 0 && active_tool_idx_ >= 0)
        idx = active_tool_idx_;

    if (idx == active_tool_idx_)
    {
        //close active tool
        right_widget_->setVisible(false);
        active_tool_idx_ = -1;
    }
    else
    {
        assert(idx >= 0);

        const auto& tool = tools_.at(idx);

        tool_name_label_->setText(QString::fromStdString(tool.widget->toolName()));

        active_tool_idx_ = idx;
        widget_stack_->setCurrentIndex(idx);
        right_widget_->setVisible(true);

        for (auto& t : tools_)
        {
            if (t.idx == active_tool_idx_)
                continue;

            t.button->blockSignals(true);
            t.button->setChecked(false);
            t.button->blockSignals(false);
        }
    }

    updateMenu();
    updateToolBar();

    emit toolChanged();
}

/**
 */
void ToolBox::adjustSizings()
{
    size_t maxrows = 0;
    int    maxw    = 0;
    int    maxh    = 0;
    for (const auto& t : tools_)
    {
        auto labels = t.widget->toolLabels();

        if (labels.size() > maxrows)
            maxrows = labels.size();

        for (const auto& l : labels)
        {
            QFontMetrics fm(t.button->font());
            auto r = fm.boundingRect(QString::fromStdString(l));
            if (r.height() > maxh) maxh = r.height();
            if (r.width()  > maxw) maxw = r.width();
        }
    }

    int textw = maxw * 1.1;
    int texth = maxrows * maxh * 1.3;

    int w = std::max(ToolIconSize + 2, textw);
    int h = ToolIconSize + 2 + texth;

    for (auto& t : tools_)
        t.button->setFixedSize(w, h);
}

/**
 */
boost::optional<toolbox::ScreenRatio> ToolBox::currentScreenRatio() const
{
    if (active_tool_idx_ < 0)
        return boost::optional<toolbox::ScreenRatio>();

    return tools_.at(active_tool_idx_).widget->screenRatio();
}

/**
 */
void ToolBox::screenRatioChanged(toolbox::ScreenRatio screen_ratio)
{
    if (active_tool_idx_ < 0)
        return;
    
    tools_.at(active_tool_idx_).widget->setScreenRatio(screen_ratio);

    emit toolChanged();
}
