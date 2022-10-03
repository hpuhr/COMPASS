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

#include "histogramviewwidget.h"
#include "histogramviewdatatoolwidget.h"
#include "files.h"
#include "viewtoolswitcher.h"
#include "viewtoolwidget.h"

#include <QHBoxLayout>
#include <QSettings>
#include <QSplitter>
#include <QTabWidget>

#include "histogramview.h"

/*
 */
HistogramViewWidget::HistogramViewWidget(const std::string& class_id, const std::string& instance_id,
                                     Configurable* config_parent, HistogramView* view,
                                     QWidget* parent)
    : ViewWidget(class_id, instance_id, config_parent, view, parent)
{
    createStandardLayout();

    auto data_widget = new HistogramViewDataWidget(getView(), view->getDataSource());
    setDataWidget(data_widget);

    auto config_widget = new HistogramViewConfigWidget(getView());
    setConfigWidget(config_widget);

    typedef HistogramViewDataTool Tool;

    auto icon = [ = ] (const std::string& fn) 
    {
        return QIcon(Utils::Files::getIconFilepath(fn).c_str());
    };

    getViewToolSwitcher()->addTool(Tool::HG_DEFAULT_TOOL, "", "", QIcon(), Qt::ArrowCursor);
    getViewToolSwitcher()->addTool(Tool::HG_SELECT_TOOL, "Select", "S", icon("select_action.png"), Qt::CrossCursor);
    getViewToolSwitcher()->setDefaultTool(Tool::HG_DEFAULT_TOOL);

    //we could add the default action if we wanted
    getViewToolWidget()->addTool(Tool::HG_SELECT_TOOL);

    getViewToolWidget()->addSeparator();

    getViewToolWidget()->addActionCallback("Invert Selection", [=] () { data_widget->invertSelectionSlot(); }, icon("select_invert.png"));
    getViewToolWidget()->addActionCallback("Delete Selection", [=] () { data_widget->clearSelectionSlot(); }, icon("select_delete.png"));

    getViewToolWidget()->addSeparator();

    getViewToolWidget()->addActionCallback("Zoom to Home", [=] () { data_widget->resetZoomSlot(); }, icon("zoom_home.png"), "Space");
}

/*
 */
HistogramViewWidget::~HistogramViewWidget()
{
}

/**
 */
HistogramViewDataWidget* HistogramViewWidget::getViewDataWidget()
{
    return dynamic_cast<HistogramViewDataWidget*>(ViewWidget::getViewDataWidget());
}

/**
 */
HistogramViewConfigWidget* HistogramViewWidget::getViewConfigWidget()
{
    return dynamic_cast<HistogramViewConfigWidget*>(ViewWidget::getViewConfigWidget());
}
