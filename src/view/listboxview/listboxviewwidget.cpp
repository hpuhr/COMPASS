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

#include "listboxviewwidget.h"

#include <QHBoxLayout>
#include <QSettings>
#include <QSplitter>
#include <QTabWidget>

#include "listboxview.h"
#include "listboxviewconfigwidget.h"
#include "listboxviewdatawidget.h"

/*
 */
ListBoxViewWidget::ListBoxViewWidget(const std::string& class_id, 
                                     const std::string& instance_id,
                                     Configurable* config_parent, 
                                     ListBoxView* view,
                                     QWidget* parent)
:   ViewWidget(class_id, instance_id, config_parent, view, parent)
{
    auto data_widget = new ListBoxViewDataWidget(getView(), view->getDataSource());
    setDataWidget(data_widget);

    auto config_widget = new ListBoxViewConfigWidget(getView());
    setConfigWidget(config_widget);
}

/*
 */
ListBoxViewWidget::~ListBoxViewWidget() = default;

/**
 */
ListBoxViewDataWidget* ListBoxViewWidget::getViewDataWidget()
{
    return dynamic_cast<ListBoxViewDataWidget*>(ViewWidget::getViewDataWidget());
}

/**
 */
const ListBoxViewDataWidget* ListBoxViewWidget::getViewDataWidget() const
{
    return dynamic_cast<const ListBoxViewDataWidget*>(ViewWidget::getViewDataWidget());
}

/**
 */
ListBoxViewConfigWidget* ListBoxViewWidget::getViewConfigWidget()
{
    return dynamic_cast<ListBoxViewConfigWidget*>(ViewWidget::getViewConfigWidget());
}

/**
 */
const ListBoxViewConfigWidget* ListBoxViewWidget::getViewConfigWidget() const
{
    return dynamic_cast<const ListBoxViewConfigWidget*>(ViewWidget::getViewConfigWidget());
}

/**
 */
ListBoxView* ListBoxViewWidget::getView() 
{ 
    return dynamic_cast<ListBoxView*>(ViewWidget::getView()); 
}
