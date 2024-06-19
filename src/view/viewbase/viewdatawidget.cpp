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

#include "viewdatawidget.h"
#include "viewwidget.h"
#include "view.h"
#include "viewtoolswitcher.h"
#include "logger.h"
#include "buffer.h"
#include "variable.h"
#include "property.h"

#include <QApplication>
#include <QPainter>

#include <iostream>

const double      ViewDataWidget::MarkerSizePx         = 8.0;
const double      ViewDataWidget::MarkerSizeSelectedPx = 6.0;

const std::string ViewDataWidget::Color_CAT001  = "#00FF00";
const std::string ViewDataWidget::Color_CAT010  = "#AAAA66";
const std::string ViewDataWidget::Color_CAT020  = "#FF0000";
const std::string ViewDataWidget::Color_CAT021  = "#6666FF";
const std::string ViewDataWidget::Color_CAT048  = "#00FF00";
const std::string ViewDataWidget::Color_RefTraj = "#FFA500";
const std::string ViewDataWidget::Color_CAT062  = "#CCCCCC";

const QColor ViewDataWidget::ColorSelected = Qt::yellow; // darker than yellow #808000

/**
 */
ViewDataWidget::ViewDataWidget(ViewWidget* view_widget, QWidget* parent, Qt::WindowFlags f)
:   QWidget     (parent, f)
,   view_widget_(view_widget)
{
    assert(view_widget_);

    setObjectName("datawidget");

    dbc_colors_["CAT001" ] = QColor(QString::fromStdString(Color_CAT001 ));
    dbc_colors_["CAT010" ] = QColor(QString::fromStdString(Color_CAT010 ));
    dbc_colors_["CAT020" ] = QColor(QString::fromStdString(Color_CAT020 ));
    dbc_colors_["CAT021" ] = QColor(QString::fromStdString(Color_CAT021 ));
    dbc_colors_["CAT048" ] = QColor(QString::fromStdString(Color_CAT048 ));
    dbc_colors_["RefTraj"] = QColor(QString::fromStdString(Color_RefTraj));
    dbc_colors_["CAT062" ] = QColor(QString::fromStdString(Color_CAT062 ));
}

/**
 * Checks if the view stores any data (usually set during a redraw or live update via updateData()).
 */
bool ViewDataWidget::hasData() const
{
    return !data_.empty();
}

unsigned int ViewDataWidget::loadedDataCount()
{
    unsigned int count = 0;

    for (auto& buf_it : data_)
        count += (*buf_it.second).size();

    return count;
}

/**
 * Checks if the view currently shows any data.
 */
bool ViewDataWidget::showsData() const
{
    //the view needs to obtain data, and the last redraw needs to be a valid one.
    return drawn_;
}

/**
*/
QColor ViewDataWidget::colorForDBContent(const std::string& dbcontent_name) const
{
    return dbc_colors_.at(dbcontent_name);
}

/**
*/
const std::map<std::string, QColor>& ViewDataWidget::dbContentColors() const
{
    return dbc_colors_;
}

/**
 * Sets the tool switcher, which handles switching of view tools.
 * (Note: The selected view tool might have an impact on how clicks are handled in the data widget,
 * which cursor is displayed, etc.)
 */
void ViewDataWidget::setToolSwitcher(ViewToolSwitcher* tool_switcher)
{
    if (!tool_switcher)
        throw std::runtime_error("ViewDataWidget::setToolSwitcher: nullptr passed");
    if (tool_switcher_)
        throw std::runtime_error("ViewDataWidget::setToolSwitcher: called twice");

    tool_switcher_ = tool_switcher;
    tool_switcher_->setDataWidget(this);

    connect(tool_switcher_, &ViewToolSwitcher::toolChanged, this, &ViewDataWidget::toolChanged);

    tool_switcher_->update();
}

/**
 * Reacts on view tool changes.
 * (Note: The selected view tool might have an impact on how clicks are handled in the data widget,
 * which cursor is displayed, etc.)
 */
namespace
{
    void setCursorRecursive(QWidget* w, const QCursor& cursor)
    {
        w->setCursor(cursor);

        for(auto c : w->findChildren<QWidget*>())
            setCursorRecursive(c, cursor);
    }
}
void ViewDataWidget::toolChanged(int mode, const QCursor& cursor)
{
    //update active view cursor to the tool's cursor
    setCursorRecursive(this, cursor);

    //invoke derived
    toolChanged_impl(mode);
}

/**
 * Ends the currently set tool.
 * (Note: should be called if a tool handled by the data widget has been ended)
 */
void ViewDataWidget::endTool()
{
    if (tool_switcher_)
        tool_switcher_->endCurrentTool();
}

/**
 * Reacts on loading started.
 */
void ViewDataWidget::loadingStarted()
{
    logdbg << "ViewDataWidget::loadingStarted";

    //clear and update display
    clearData();
    redrawData(false, false);

    //invoke derived
    loadingStarted_impl();
}

/**
 * Reacts on loading ended.
 */
void ViewDataWidget::loadingDone()
{
    logdbg << "ViewDataWidget::loadingDone";

    //invoke derived
    loadingDone_impl();

    //signal that view data has been loaded
    emit dataLoaded();
}

/**
*/
void ViewDataWidget::loadingDone_impl()
{
    //default behavior: recompute and redraw after reload
    redrawData(true, false);
}

/**
 * UPdates the data using the given buffers.
 */
void ViewDataWidget::updateData(const BufferData& data, bool requires_reset)
{
    logdbg << "ViewDataWidget::updateData";

    //store new data
    data_ = data;

    //invoke derived
    updateData_impl(requires_reset);
}

/**
 * Clears the views data. 
 * (Note: Might require an additional redraw to take effect)
 */
void ViewDataWidget::clearData()
{
    logdbg << "ViewDataWidget::clearData";

    data_  = {};
    drawn_ = false;

    //invoke derived
    clearData_impl();
}

/**
 * Redraws the views data.
 * @param recompute If set, data needed fo a redraw will be freshly recomputed (to be enforced by derived classes).
 * @param notify If set, signals will be emitted before and after the redraw (this is only needed for manual redraws like in ViewLoadStateWidget).
 * @return True if the redraw succeeded.
*/
bool ViewDataWidget::redrawData(bool recompute, bool notify)
{
    logdbg << "ViewDataWidget::redrawData";

    if (notify)
    {
        emit redrawStarted();
        QApplication::processEvents(); //process any ui reactions on this signal before ui is blocked by redraw
    }
    
    //invoke derived: redraw and remember if data has been redrawn correctly
    drawn_ = redrawData_impl(recompute);

    if (notify)
    {
        emit redrawDone();
    }

    //signal display changed to whom it may concern
    emit displayChanged();
    
    return drawn_;
}

/**
 * Runs a live reload of data and updates the diplay.
*/
void ViewDataWidget::liveReload()
{
    //invoke derived
    liveReload_impl();

    //signal that live data has been loaded
    emit liveDataLoaded();
}

/**
 * Checks if the required read set of the view is available in the loaded data.
 */
bool ViewDataWidget::isVariableSetLoaded() const
{
    const auto& view_data = viewData();
    if (view_data.empty())
        return false;

    for (const auto& dbcontent_data : view_data)
    {
        auto var_set = view_widget_->getView()->getSet(dbcontent_data.first);
        for (auto var : var_set.getSet())
        {
            if (!dbcontent_data.second->hasAnyPropertyNamed(var->name()))
                return false;
        }
    }

    return true;
}

/**
 * Renders the data widget's contents.
 */
QImage ViewDataWidget::renderData()
{
    //per default just render the data widget's content
    //note: does not work with opengl widgets such as osg view => derive as needed
    QImage img(this->size(), QImage::Format_ARGB32);
    QPainter painter(&img);

    this->render(&painter);

    return img;
}

/**
 * Generates json view information.
 */
nlohmann::json ViewDataWidget::viewInfoJSON() const
{
    nlohmann::json info;

    //add general information
    info[ "has_data"    ] = hasData();
    info[ "num_buffers" ] = data_.size();

    nlohmann::json buffer_infos = nlohmann::json::array();
    for (const auto& dbc_data : data_)
    {
        nlohmann::json buffer_info;
        buffer_info[ "dbcontent" ] = dbc_data.first;
        buffer_info[ "size"      ] = dbc_data.second->size();

        // nlohmann::json props = nlohmann::json::array();
        // for (const auto& p : dbc_data.second->properties().properties())
        // {
        //     nlohmann::json pinfo;
        //     pinfo[ "name"      ] = p.name();
        //     pinfo[ "data_type" ] = p.dataTypeString();

        //     props.push_back(pinfo);
        // }

        // buffer_info[ "properties" ] = props;

        buffer_infos.push_back(buffer_info);
    }

    info[ "buffers" ] = buffer_infos;

    //add view-specific information
    viewInfoJSON_impl(info);

    return info;
}
