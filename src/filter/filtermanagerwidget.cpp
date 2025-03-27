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

#include "filtermanagerwidget.h"

#include "dbfilter.h"
#include "dbfilterwidget.h"
#include "filtergeneratorwidget.h"
#include "filtermanager.h"
#include "global.h"
#include "logger.h"
#include "util/files.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QScrollArea>

using namespace Utils;

/**
 */
FilterManagerWidget::FilterManagerWidget(FilterManager& filter_manager, 
                                         QWidget* parent,
                                         Qt::WindowFlags f)
:   ToolBoxWidget(parent)
,   filter_manager_         (filter_manager)
,   filter_generator_widget_(nullptr)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* layout = new QVBoxLayout();

    // top
    QHBoxLayout* top_layout = new QHBoxLayout();

    // use filters stuff
    filters_check_ = new QCheckBox();
    filters_check_->setChecked(filter_manager_.useFilters());
    connect(filters_check_, &QCheckBox::clicked, this, &FilterManagerWidget::toggleUseFilters);

    top_layout->addWidget(filters_check_);
    top_layout->addStretch();

    layout->addLayout(top_layout);

    scroll_area_ = new QScrollArea;
    scroll_area_->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    scroll_area_->setWidgetResizable(true);

    QWidget* scroll_widget = new QWidget;

    QVBoxLayout* scroll_layout = new QVBoxLayout;
    scroll_layout->setContentsMargins(0, 0, 0, 0);
    scroll_layout->setSpacing(0);

    ds_filter_layout_ = new QVBoxLayout;
    ds_filter_layout_->setContentsMargins(0, 0, 0, 0);
    ds_filter_layout_->setSpacing(0);

    scroll_layout->addLayout(ds_filter_layout_);
    scroll_layout->addStretch(1);

    scroll_widget->setLayout(scroll_layout);

    scroll_area_->setWidget(scroll_widget);

    layout->addWidget(scroll_area_);

    setLayout(layout);

    updateFilters();

    setDisabled(true);
}

/**
 */
FilterManagerWidget::~FilterManagerWidget()
{
    if (filter_generator_widget_)
    {
        delete filter_generator_widget_;
        filter_generator_widget_ = nullptr;
    }
}

/**
 */
QIcon FilterManagerWidget::toolIcon() const 
{
    return QIcon(Utils::Files::getIconFilepath("db_filters.png").c_str());
}

/**
 */
std::string FilterManagerWidget::toolName() const 
{
    return "Filters";
}

/**
 */
std::string FilterManagerWidget::toolInfo() const 
{
    return "Filters";
}

/**
 */
std::vector<std::string> FilterManagerWidget::toolLabels() const 
{
    return { "Filters" };
}

/**
 */
toolbox::ScreenRatio FilterManagerWidget::defaultScreenRatio() const 
{
    return ToolBoxWidget::defaultScreenRatio();
}

/**
 */
void FilterManagerWidget::addToConfigMenu(QMenu* menu) 
{
    QAction* new_filter_action = menu->addAction("Add New Filter");
    connect(new_filter_action, &QAction::triggered, this, &FilterManagerWidget::addFilter);

    menu->addSeparator();

    auto expand_all_action = menu->addAction("Expand All");
    connect(expand_all_action, &QAction::triggered, this, &FilterManagerWidget::expandAll);

    auto collapse_all_action = menu->addAction("Collapse All");
    connect(collapse_all_action, &QAction::triggered, this, &FilterManagerWidget::collapseAll);

    auto collapse_unused_action = menu->addAction("Collapse Unused");
    connect(collapse_unused_action, &QAction::triggered, this, &FilterManagerWidget::collapseUnused);
}

/**
 */
void FilterManagerWidget::addToToolBar(QToolBar* tool_bar)
{
}

/**
 */
void FilterManagerWidget::loadingStarted()
{
    scroll_area_->setEnabled(false);
}

/**
 */
void FilterManagerWidget::loadingDone()
{
    scroll_area_->setEnabled(true);
}

/**
 */
QCheckBox *FilterManagerWidget::filtersCheckBox() const
{
    assert (filters_check_);
    return filters_check_;
}

/**
 */
void FilterManagerWidget::toggleUseFilters()
{
    assert(filters_check_);

    bool checked = filters_check_->checkState() == Qt::Checked;
    logdbg << "FilterManagerWidget: toggleUseFilters: setting use limit to " << checked;
    filter_manager_.useFilters(checked);
}

/**
 */
void FilterManagerWidget::updateUseFilters ()
{
    assert (filters_check_);
    filters_check_->setChecked(filter_manager_.useFilters());
}

/**
 */
void FilterManagerWidget::addFilter()
{
    assert(!filter_generator_widget_);

    filter_generator_widget_ = new FilterGeneratorWidget();
    connect(filter_generator_widget_, SIGNAL(filterWidgetAction(bool)), this,
            SLOT(filterWidgetActionSlot(bool)));
    filter_generator_widget_->show();
}

/**
 */
void FilterManagerWidget::updateFilters()
{
    QLayoutItem* child;
    while (!ds_filter_layout_->isEmpty() && (child = ds_filter_layout_->takeAt(0)))
    
    while (!ds_filter_layout_->isEmpty())
    {
        auto item = ds_filter_layout_->takeAt(0);
        delete item;
    }

    auto& filters = filter_manager_.filters();

    for (auto& it : filters)
    {
        logdbg << "FilterManagerWidget: updateFilters: filter " << it->getName();

        if (!it->getActive())
            it->widget()->collapse();

        connect(it->widget(), &DBFilterWidget::filterContentChanged, this, &FilterManagerWidget::syncFilterLayouts, Qt::UniqueConnection);

        ds_filter_layout_->addWidget(it->widget());
    }

    syncFilterLayouts();
}

/**
 */
void FilterManagerWidget::filterWidgetAction(bool result)
{
    assert(filter_generator_widget_);

    if (result)
        updateFilters();

    delete filter_generator_widget_;
    filter_generator_widget_ = nullptr;
}

/**
 */
void FilterManagerWidget::databaseOpened() 
{ 
    setDisabled(false); 
}

/**
 */
void FilterManagerWidget::expandAll()
{
    auto& filters = filter_manager_.filters();

    for (auto& it : filters)
        it->widget()->expand();
}

/**
 */
void FilterManagerWidget::collapseAll()
{
    auto& filters = filter_manager_.filters();

    for (auto& it : filters)
        it->widget()->collapse();
}

/**
 */
void FilterManagerWidget::collapseUnused()
{
    auto& filters = filter_manager_.filters();

    for (auto& it : filters)
    {
        if (it->getActive())
            it->widget()->expand();
        else
            it->widget()->collapse();
    }
}

/**
 */
void FilterManagerWidget::syncFilterLayouts()
{
    auto& filters = filter_manager_.filters();

    int max_width = 0;
    for (auto& it : filters)
    {
        int col_width = it->widget()->columnWidth(0);
        if (col_width > max_width)
            max_width = col_width;
    }

    max_width *= 1.3;

    for (auto& it : filters)
        it->widget()->setFixedColumnWidth(0, max_width);
}
