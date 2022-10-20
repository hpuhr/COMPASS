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

#include "scatterplotviewwidget.h"

#include <QHBoxLayout>
#include <QSettings>
#include <QSplitter>
#include <QTabWidget>

#include "scatterplotview.h"
#include "scatterplotviewdatatoolwidget.h"
#include "viewtoolwidget.h"
#include "viewtoolswitcher.h"
#include "files.h"

/*
 */
ScatterPlotViewWidget::ScatterPlotViewWidget(const std::string& class_id, const std::string& instance_id,
                                     Configurable* config_parent, ScatterPlotView* view,
                                     QWidget* parent)
    : ViewWidget(class_id, instance_id, config_parent, view, parent)
{
    createStandardLayout();

    auto data_widget = new ScatterPlotViewDataWidget(getView(), view->getDataSource());
    setDataWidget(data_widget);

    auto config_widget = new ScatterPlotViewConfigWidget(getView());
    setConfigWidget(config_widget);

    typedef ScatterPlotViewDataTool Tool;

    auto icon = [ = ] (const std::string& fn) 
    {
        return QIcon(Utils::Files::getIconFilepath(fn).c_str());
    };

    getViewToolSwitcher()->addTool(Tool::SP_NAVIGATE_TOOL, "Navigate", {}, QIcon(), Qt::OpenHandCursor);
    getViewToolSwitcher()->addTool(Tool::SP_SELECT_TOOL, "Select", Qt::Key_S, icon("select_action.png"), Qt::CrossCursor);
    getViewToolSwitcher()->addTool(Tool::SP_ZOOM_RECT_TOOL, "Zoom to Rectangle", Qt::Key_R, icon("zoom_select_action.png"), Qt::CrossCursor);

    getViewToolSwitcher()->setDefaultTool(Tool::SP_NAVIGATE_TOOL);
    
    getViewToolWidget()->addTool(Tool::SP_SELECT_TOOL);
    getViewToolWidget()->addTool(Tool::SP_ZOOM_RECT_TOOL);

    getViewToolWidget()->addSeparator();

    getViewToolWidget()->addActionCallback("Invert Selection", [=] () { data_widget->invertSelectionSlot(); }, icon("select_invert.png"));
    getViewToolWidget()->addActionCallback("Delete Selection", [=] () { data_widget->clearSelectionSlot(); }, icon("select_delete.png"));

    getViewToolWidget()->addSeparator();

    getViewToolWidget()->addActionCallback("Zoom to Home", [=] () { data_widget->resetZoomSlot(); }, icon("zoom_home.png"), Qt::Key_Space);
}

/**
 */
ScatterPlotViewWidget::~ScatterPlotViewWidget()
{
}

/**
 */
ScatterPlotViewDataWidget* ScatterPlotViewWidget::getViewDataWidget()
{
    return dynamic_cast<ScatterPlotViewDataWidget*>(ViewWidget::getViewDataWidget());
}

/**
 */
ScatterPlotViewConfigWidget* ScatterPlotViewWidget::getViewConfigWidget()
{
    return dynamic_cast<ScatterPlotViewConfigWidget*>(ViewWidget::getViewConfigWidget());
}
