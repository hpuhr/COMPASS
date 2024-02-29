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
#include "listboxview.h"
#include "listboxviewconfigwidget.h"
#include "listboxviewdatawidget.h"

#include <QHBoxLayout>
#include <QSettings>
#include <QSplitter>
#include <QTabWidget>

/*
 */
ListBoxViewWidget::ListBoxViewWidget(const std::string& class_id, 
                                     const std::string& instance_id,
                                     Configurable* config_parent, 
                                     ListBoxView* view,
                                     QWidget* parent)
:   ViewWidget(class_id, instance_id, config_parent, view, parent)
{
    auto data_widget = new ListBoxViewDataWidget(this);
    setDataWidget(data_widget);

    auto config_widget = new ListBoxViewConfigWidget(this);
    setConfigWidget(config_widget);
}

/*
 */
ListBoxViewWidget::~ListBoxViewWidget() = default;

/**
 */
ListBoxViewDataWidget* ListBoxViewWidget::getViewDataWidget()
{
    auto w = dynamic_cast<ListBoxViewDataWidget*>(ViewWidget::getViewDataWidget());
    assert(w);
    return w;
}

/**
 */
const ListBoxViewDataWidget* ListBoxViewWidget::getViewDataWidget() const
{
    auto w =  dynamic_cast<const ListBoxViewDataWidget*>(ViewWidget::getViewDataWidget());
    assert(w);
    return w;
}

/**
 */
ListBoxViewConfigWidget* ListBoxViewWidget::getViewConfigWidget()
{
    auto w =  dynamic_cast<ListBoxViewConfigWidget*>(ViewWidget::getViewConfigWidget());
    assert(w);
    return w;
}

/**
 */
const ListBoxViewConfigWidget* ListBoxViewWidget::getViewConfigWidget() const
{
    auto w =  dynamic_cast<const ListBoxViewConfigWidget*>(ViewWidget::getViewConfigWidget());
    assert(w);
    return w;
}

/**
 */
ListBoxView* ListBoxViewWidget::getView() 
{ 
    auto view = dynamic_cast<ListBoxView*>(ViewWidget::getView());
    assert(view);
    return view;
}
