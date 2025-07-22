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

#include "viewmanager.h"
#include "compass.h"
#include "buffer.h"
#include "logger.h"
#include "stringconv.h"
#include "view.h"
#include "viewcontainer.h"
#include "viewcontainerwidget.h"
#include "viewpoint.h"
#include "dbinterface.h"
#include "viewpointswidget.h"
#include "filtermanager.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"
#include "util/timeconv.h"
#include "viewpoint_commands.h"
#include "global.h"
#include "files.h"

#include "json.hpp"

#include <QMessageBox>
#include <QWidget>
#include <QMetaType>
#include <QApplication>
#include <QTabWidget>

#include <cassert>

#define SCAN_PRESETS

using namespace Utils;
using namespace nlohmann;
using namespace std;

ViewManager::ViewManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "views.json"), compass_(*compass)
{
    logdbg << "ViewManager: constructor";

    qRegisterMetaType<ViewPoint*>("ViewPoint*");

    init_view_point_commands();

    registerParameter("automatic_reload", &config_.automatic_reload, Config().automatic_reload);
    registerParameter("automatic_redraw", &config_.automatic_redraw, Config().automatic_redraw);
}

void ViewManager::init(QTabWidget* main_tab_widget)
{
    logdbg << "ViewManager: init";

    assert(main_tab_widget);
    assert(!main_tab_widget_);
    assert(!initialized_);

    main_tab_widget_ = main_tab_widget;

    connect (&COMPASS::instance(), &COMPASS::appModeSwitchSignal, this, &ViewManager::appModeSwitchSlot);

    // view point stuff

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    view_points_widget_ = new ViewPointsWidget(*this);

    QApplication::restoreOverrideCursor();

    assert(view_points_widget_);

    FilterManager& filter_man = COMPASS::instance().filterManager();

    connect (this, &ViewManager::showViewPointSignal, &filter_man, &FilterManager::showViewPointSlot);
    connect (this, &ViewManager::unshowViewPointSignal, &filter_man, &FilterManager::unshowViewPointSlot);

    view_class_list_.insert({"HistogramView", "Histogram View"});
    view_class_list_.insert({"TableView", "Table View"});

#if USE_EXPERIMENTAL_SOURCE == true
    view_class_list_.insert({"GeographicView", "Geographic View"});
#endif

    view_class_list_.insert({"ScatterPlotView", "Scatterplot View"});
    view_class_list_.insert({"GridView", "Grid View"});

#ifdef SCAN_PRESETS
    //scan view presets
    if (!presets_.scanForPresets())
        logwrn << "ViewManager: init: view presets could not be loaded";
#endif

    connect(&presets_, &ViewPresets::presetEdited, this, &ViewManager::presetEdited);

    initialized_ = true;

    createSubConfigurables();
    updateFeatures();
}

void ViewManager::loadViewPoints()
{
    assert (view_points_widget_);
    view_points_widget_->loadViewPoints();
}

void ViewManager::close()
{
    loginf << "ViewManager: close";
    initialized_ = false;

    logdbg << "ViewManager: close: deleting container widgets";
    while (container_widgets_.size())
    {
        auto first_it = container_widgets_.begin();
        logdbg << "ViewManager: close: deleting container widget " << first_it->first;
        delete first_it->second; // deletes the respective view container, which removes itself from this
        container_widgets_.erase(first_it);
    }

    logdbg << "ViewManager: close: deleting containers size " << containers_.size();
    while (containers_.size())
    {
        auto first_it = containers_.begin();
        logdbg << "ViewManager: close: deleting container " << first_it->first;
        delete first_it->second;
        //containers_.erase(first_it);  // TODO CAUSES SEGFAULT, FIX THIS
    }

    if (view_points_widget_)
    {
        //view_points_widget_->tableModel()->saveViewPoints();
        delete view_points_widget_;
        view_points_widget_ = nullptr;
    }

    view_points_report_gen_ = nullptr;

    loginf << "ViewManager: close: done";
}

ViewManager::~ViewManager()
{
    logdbg << "ViewManager: destructor";

    assert(!container_widgets_.size());
    assert(!containers_.size());
    assert(!initialized_);
}

void ViewManager::generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id)
{
    logdbg << "ViewManager: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;

    assert(initialized_);

    if (class_id == "ViewContainer")
    {
        ViewContainer* container =
                new ViewContainer(class_id, instance_id, this, this, main_tab_widget_, 0);
        assert(containers_.count(instance_id) == 0);
        containers_.insert(std::pair<std::string, ViewContainer*>(instance_id, container));

        unsigned int number = String::getAppendedInt(instance_id);
        if (number >= container_count_)
            container_count_ = number;
    }
    else if (class_id == "ViewContainerWidget")
    {
        ViewContainerWidget* container_widget =
                new ViewContainerWidget(class_id, instance_id, this);
        assert(containers_.count(container_widget->viewContainer().instanceId()) == 0);
        containers_.insert(std::pair<std::string, ViewContainer*>(
                               container_widget->viewContainer().instanceId(), &container_widget->viewContainer()));
        assert(container_widgets_.count(instance_id) == 0);
        container_widgets_.insert(
                    std::pair<std::string, ViewContainerWidget*>(instance_id, container_widget));

        unsigned int number = String::getAppendedInt(instance_id);
        if (number >= container_count_)
            container_count_ = number;
    }
    else if (class_id == "ViewPointsReportGenerator")
    {
        assert (!view_points_report_gen_);

        view_points_report_gen_.reset(new ViewPointsReportGenerator(class_id, instance_id, *this));
        assert (view_points_report_gen_);
    }
    else
        throw std::runtime_error("ViewManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
//    if (widget_)
//        widget_->update();
}

void ViewManager::checkSubConfigurables()
{
    if (containers_.size() == 0)
    {
        generateSubConfigurableFromConfig("ViewContainer", "ViewContainer0");
    }

    if (!view_points_report_gen_)
    {
        generateSubConfigurableFromConfig("ViewPointsReportGenerator", "ViewPointsReportGenerator0");
    }
}

void ViewManager::enableStoredReadSets()
{
    loginf << "ViewManager: enableStoredReadSets";

    for (const auto& cont_it : COMPASS::instance().dbContentManager())
    {
        logdbg << "ViewManager: enableStoredReadSets: stored readset for '" << cont_it.first << "'";
        tmp_stored_readset_[cont_it.first] = getReadSet(cont_it.first);
    }

    use_tmp_stored_readset_ = true;
}
void ViewManager::disableStoredReadSets()
{
    loginf << "ViewManager: disableStoredReadSets";

    use_tmp_stored_readset_ = false;
    tmp_stored_readset_.clear();
}

dbContent::VariableSet ViewManager::getReadSet(const std::string& dbcontent_name)
{
    if (use_tmp_stored_readset_)
    {
        logdbg << "ViewManager: getReadSet: stored readset for '" << dbcontent_name << "'";
        assert (tmp_stored_readset_.count(dbcontent_name));
        return tmp_stored_readset_.at(dbcontent_name);
    }

    dbContent::VariableSet read_set;
    dbContent::VariableSet read_set_tmp;

    for (auto view_it : views_)
    {
        read_set_tmp = view_it.second->getSet(dbcontent_name);
        read_set.add(read_set_tmp);
    }
    return read_set;
}

ViewPointsWidget* ViewManager::viewPointsWidget() const
{
    assert (view_points_widget_);
    return view_points_widget_;
}

ViewPointsReportGenerator& ViewManager::viewPointsGenerator()
{
    assert (view_points_report_gen_);
    return *view_points_report_gen_;
}

std::pair<bool, std::string> ViewManager::loadViewPoints(nlohmann::json json_obj)
{
    try
    {
        //check if valid JSON
        std::string err;
        bool json_ok = ViewPoint::isValidJSON(json_obj, "", &err, true);
        if (!json_ok)
            return std::make_pair(false, err);

        DBInterface& db_interface = COMPASS::instance().dbInterface();

        //delete existing viewpoints
        if (db_interface.existsViewPointsTable() && db_interface.viewPoints().size())
            db_interface.deleteAllViewPoints();

        assert (json_obj.contains(ViewPoint::VP_COLLECTION_ARRAY_KEY));
        
        //add new ones
        json& view_points = json_obj.at(ViewPoint::VP_COLLECTION_ARRAY_KEY);
        assert (view_points.size());

        unsigned int id;
        for (auto& vp_it : view_points.get<json::array_t>())
        {
            assert (vp_it.contains(ViewPoint::VP_ID_KEY));

            id = vp_it.at(ViewPoint::VP_ID_KEY);

            if (!vp_it.contains(ViewPoint::VP_STATUS_KEY))
                vp_it[ViewPoint::VP_STATUS_KEY] = "open";

            db_interface.setViewPoint(id, vp_it.dump());
        }

        //reload viewpoints
        loadViewPoints();

        loginf << "ViewManager::loadViewPoints: imported " << std::to_string(view_points.size()) << " view points";
    }
    catch (const std::exception& ex)
    {
        return std::make_pair(false, ex.what());
    }
    catch (...)
    {
        return std::make_pair(false, "unknown error");
    }
    
    return std::make_pair(true, "");  
}

void ViewManager::clearViewPoints()
{
    DBInterface& db_interface = COMPASS::instance().dbInterface();

            //delete existing viewpoints
    if (db_interface.existsViewPointsTable() && db_interface.viewPoints().size())
        db_interface.deleteAllViewPoints();

    viewPointsWidget()->clearViewPoints();

}
void ViewManager::addViewPoints(const std::vector <nlohmann::json>& viewpoints)
{
    viewPointsWidget()->addViewPoints(viewpoints);
}

void ViewManager::setCurrentViewPoint (const ViewableDataConfig* viewable,
                                       bool load_blocking)
{
    if (current_viewable_)
        unsetCurrentViewPoint();

    current_viewable_ = viewable;

    view_point_data_selected_ = false;

    logdbg << "ViewManager: setCurrentViewPoint: setting current view point data: '"
    << viewable->data().dump(4) << "'";

    emit showViewPointSignal(current_viewable_);

    if (load_blocking)
        COMPASS::instance().dbContentManager().loadBlocking();
    else
        COMPASS::instance().dbContentManager().load();
}


void ViewManager::unsetCurrentViewPoint ()
{
    if (current_viewable_)
    {
        emit unshowViewPointSignal(current_viewable_);

        current_viewable_ = nullptr;

        view_point_data_selected_ = false;
    }
}

void ViewManager::doViewPointAfterLoad ()
{
    logdbg << "ViewManager: doViewPointAfterLoad";

    if (!current_viewable_)
    {
        logdbg << "ViewManager: doViewPointAfterLoad: no viewable";
        return; // nothing to do
    }

    if (view_point_data_selected_)
    {
        logdbg << "ViewManager: doViewPointAfterLoad: data already selected";
        return; // already done, this is a re-load
    }

    assert (view_points_widget_);

    const json& data = current_viewable_->data();

    logdbg << "ViewManager: doViewPointAfterLoad: data '" << data.dump(4) << "'";

    bool vp_contains_timestamp = data.contains(ViewPoint::VP_TIMESTAMP_KEY);
    boost::posix_time::ptime vp_timestamp;
    bool vp_contains_time_window = data.contains(ViewPoint::VP_TIME_WIN_KEY);
    float vp_time_window;
    boost::posix_time::ptime vp_ts_min, vp_ts_max;

    if (!vp_contains_timestamp)
    {
        loginf << "ViewManager: doViewPointAfterLoad: no time given";
        return; // nothing to do
    }
    else
    {
        assert (data.at(ViewPoint::VP_TIMESTAMP_KEY).is_string());
        vp_timestamp = Time::fromString(data.at(ViewPoint::VP_TIMESTAMP_KEY));

        loginf << "ViewManager: doViewPointAfterLoad: time " << Time::toString(vp_timestamp);
    }

    if (vp_contains_time_window)
    {
        assert (data.at(ViewPoint::VP_TIME_WIN_KEY).is_number());
        vp_time_window = data.at(ViewPoint::VP_TIME_WIN_KEY);
        vp_ts_min = vp_timestamp - Time::partialSeconds(vp_time_window / 2.0);
        vp_ts_max = vp_timestamp + Time::partialSeconds(vp_time_window / 2.0);

        loginf << "ViewManager: doViewPointAfterLoad: time window min " << Time::toString(vp_ts_min)
               << " max " << Time::toString(vp_ts_max);
    }

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    bool selection_changed = false;
    for (auto& dbo_it : dbcont_man)
    {
        std::string dbcontent_name = dbo_it.first;

        if (!dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_timestamp_))
        {
            logerr << "ViewManager: doViewPointAfterLoad: required variables missing in " << dbcontent_name;
            continue;
        }

        const dbContent::Variable& ts_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_);

        if (dbcont_man.data().count(dbo_it.first))
        {
            std::shared_ptr<Buffer> buffer = dbcont_man.data().at(dbo_it.first);

            assert(buffer->has<bool>(DBContent::selected_var.name()));
            NullableVector<bool>& selected_vec = buffer->get<bool>(DBContent::selected_var.name());

            assert(buffer->has<boost::posix_time::ptime>(ts_var.name()));
            NullableVector<boost::posix_time::ptime>& tods = buffer->get<boost::posix_time::ptime>(ts_var.name());

            unsigned int buffer_size = buffer->size();

            bool ts_null;
            boost::posix_time::ptime timestamp;

            for (unsigned int cnt =0; cnt < buffer_size; ++cnt)
            {
                ts_null = tods.isNull(cnt);

                if (ts_null)
                    continue; // nothing to do

                timestamp = tods.get(cnt);

                if (vp_contains_time_window)
                {
                    if (timestamp >= vp_ts_min && timestamp <= vp_ts_max)
                    {
                        selected_vec.set(cnt, true);
                        selection_changed = true;

                        logdbg << "ViewManager: doViewPointAfterLoad: time " << timestamp << " selected ";
                    }
                }
                else if (vp_contains_timestamp && timestamp == vp_timestamp)
                {
                    selected_vec.set(cnt, true);
                    selection_changed = true;
                }
            }
        }
    }

    view_point_data_selected_ = true;

    if (selection_changed)
    {
        loginf << "ViewManager: doViewPointAfterLoad: selection changed";
        emit selectionChangedSignal();
    }
}

void ViewManager::selectTimeWindow(boost::posix_time::ptime ts_min, boost::posix_time::ptime ts_max)
{
    loginf << "ViewManager: selectTimeWindow: ts_min " << ts_min << " ts_max " << ts_max;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    bool selection_changed = false;
    for (auto& dbo_it : dbcont_man)
    {
        std::string dbcontent_name = dbo_it.first;

        if (!dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_timestamp_))
        {
            logerr << "ViewManager: selectTimeWindow: required variables missing, quitting";
            continue;
        }

        const dbContent::Variable& ts_var = dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_);

        if (dbcont_man.data().count(dbo_it.first))
        {
            std::shared_ptr<Buffer> buffer = dbcont_man.data().at(dbo_it.first);

            assert(buffer->has<bool>(DBContent::selected_var.name()));
            NullableVector<bool>& selected_vec = buffer->get<bool>(DBContent::selected_var.name());

            assert(buffer->has<boost::posix_time::ptime>(ts_var.name()));
            NullableVector<boost::posix_time::ptime>& ts_vec = buffer->get<boost::posix_time::ptime>(ts_var.name());

            unsigned int buffer_size = buffer->size();

            bool tod_null;
            boost::posix_time::ptime timestamp;

            for (unsigned int cnt =0; cnt < buffer_size; ++cnt)
            {
                tod_null = ts_vec.isNull(cnt);

                if (tod_null)
                    continue; // nothing to do

                timestamp = ts_vec.get(cnt);

                if (timestamp >= ts_min && timestamp <= ts_max)
                {
                    selected_vec.set(cnt, true);
                    selection_changed = true;

                    logdbg << "ViewManager: selectTimeWindow: time " << timestamp << " selected ";

                }
            }
        }
    }

    view_point_data_selected_ = true;

    if (selection_changed)
    {
        loginf << "ViewManager: selectTimeWindow: selection changed";
        emit selectionChangedSignal();
    }
}

void ViewManager::showMainViewContainerAddView()
{
    assert (containers_.count("ViewContainer0"));
    containers_.at("ViewContainer0")->showAddViewMenuSlot();
}

std::map<std::string, std::string> ViewManager::viewClassList() const
{
    return view_class_list_;
}

unsigned int ViewManager::newViewNumber(const std::string& class_id)
{
    int max_number = -1;
    int tmp;

    for (auto& view_it : views_)
    {
        if (view_it.second->classId() != class_id)
            continue;

        tmp = String::getAppendedInt(view_it.second->instanceId());

        if (tmp > max_number)
            max_number = tmp;
    }

    return max_number + 1;
}

std::string ViewManager::newViewInstanceId(const std::string& class_id)
{
    return class_id + to_string(newViewNumber(class_id));
}

std::string ViewManager::newViewName(const std::string& class_id)
{
    assert (view_class_list_.count(class_id));
    return view_class_list_.at(class_id) + " " + to_string(newViewNumber(class_id));
}

void ViewManager::disableDataDistribution(bool value)
{
    loginf << "ViewManager: disableDataDistribution: value " << value;

    disable_data_distribution_ = value;
}

bool ViewManager::isProcessingData() const
{
    return processing_data_;
}

void ViewManager::resetToStartupConfiguration()
{
    loginf << "ViewManager: resetToStartupConfiguration";

    enableStoredReadSets();

    // logdbg << "ViewManager: resetToStartupConfiguration: deleting container widgets";
    // while (container_widgets_.size())
    // {
    //     auto first_it = container_widgets_.begin();
    //     logdbg << "ViewManager: resetToStartupConfiguration: deleting container widget " << first_it->first;

    //     first_it->second->setTmpDisableRemoveConfigOnDelete(true);
    //     delete first_it->second; // deletes the respective view container, which removes itself from this

    //     container_widgets_.erase(first_it);
    // }

    // logdbg << "ViewManager: resetToStartupConfiguration: deleting containers size " << containers_.size();
    // while (containers_.size())
    // {
    //     auto first_it = containers_.begin();
    //     logdbg << "ViewManager: resetToStartupConfiguration: deleting container " << first_it->first;

    //     first_it->second->setTmpDisableRemoveConfigOnDelete(true);
    //     delete first_it->second;
    //     //containers_.erase(first_it);  // TODO CAUSES SEGFAULT, FIX THIS
    // }

    logdbg << "ViewManager: resetToStartupConfiguration: resettings containers";

    for (auto& cw : container_widgets_)
        cw.second->setVisible(false);

    for (auto& c : containers_)
        c.second->resetToStartupConfiguration();

    for (auto& cw : container_widgets_)
        cw.second->setVisible(true);

    //logdbg << "ViewManager: resetToStartupConfiguration: view points generator";
    //view_points_report_gen_->setTmpDisableRemoveConfigOnDelete(true);
    //view_points_report_gen_ = nullptr;

    //createSubConfigurables();

    disableStoredReadSets();
}

bool ViewManager::isInitialized() const
{
    return initialized_;
}

ViewContainerWidget* ViewManager::addNewContainerWidget()
{
    logdbg << "ViewManager: addNewContainerWidget";
    
    container_count_++;
    std::string container_widget_name = "ViewWindow" + std::to_string(container_count_);

    generateSubConfigurableFromConfig("ViewContainerWidget", container_widget_name);

    assert(container_widgets_.count(container_widget_name) == 1);

    return container_widgets_.at(container_widget_name);
}

void ViewManager::clearDataInViews()
{
    for (auto& view_it : views_)
    {
        view_it.second->clearData();
    }
}

void ViewManager::registerView(View* view)
{
    logdbg << "ViewManager: registerView";
    assert(view);
    assert(!isRegistered(view));
    views_[view->instanceId()] = view;
}

void ViewManager::unregisterView(View* view)
{
    logdbg << "ViewManager: unregisterView " << view->getName().c_str();
    assert(view);
    assert(isRegistered(view));

    std::map<std::string, View*>::iterator it;

    it = views_.find(view->instanceId());
    views_.erase(it);
}

bool ViewManager::isRegistered(View* view)
{
    logdbg << "ViewManager: isRegistered";
    assert(view);

    std::map<std::string, View*>::iterator it;

    it = views_.find(view->instanceId());

    return !(it == views_.end());
}

void ViewManager::removeContainer(std::string instance_id)
{
    std::map<std::string, ViewContainer*>::iterator it;

    logdbg << "ViewManager: removeContainer: instance " << instance_id;

    it = containers_.find(instance_id);

    if (it != containers_.end())
    {
        containers_.erase(it);

        return;
    }

    throw std::runtime_error("ViewManager: removeContainer:  key not found");
}

void ViewManager::removeContainerWidget(std::string instance_id)
{
    std::map<std::string, ViewContainerWidget*>::iterator it;

    logdbg << "ViewManager: removeContainerWidget: instance " << instance_id;

    it = container_widgets_.find(instance_id);

    if (it != container_widgets_.end())
    {
        container_widgets_.erase(it);

        return;
    }

    throw std::runtime_error("ViewManager: removeContainer: key not found");
}

void ViewManager::deleteContainerWidget(std::string instance_id)
{
    std::map<std::string, ViewContainerWidget*>::iterator it;

    logdbg << "ViewManager: deleteContainerWidget: instance " << instance_id;

    it = container_widgets_.find(instance_id);

    if (it != container_widgets_.end())
    {
        it->second->deleteLater();
        container_widgets_.erase(it);

        return;
    }

    throw std::runtime_error("ViewManager: deleteContainerWidget: key not found");
}

void ViewManager::viewShutdown(View* view, const std::string& err)
{
    delete view;

    if (err.size())
        QMessageBox::critical(NULL, "View Shutdown", QString::fromStdString(err));
}

void ViewManager::selectionChangedSlot()
{
    loginf << "ViewManager: selectionChangedSlot";
    emit selectionChangedSignal();
}

void ViewManager::databaseOpenedSlot()
{
    loginf << "ViewManager: databaseOpenedSlot";

    for (auto& view_it : views_)
        view_it.second->databaseOpened();
}

void ViewManager::databaseClosedSlot()
{
    loginf << "ViewManager: databaseClosedSlot";

    unsetCurrentViewPoint();
    clearDataInViews();

    for (auto& view_it : views_)
        view_it.second->databaseClosed();
}

void ViewManager::loadingStartedSlot()
{
    if (disable_data_distribution_)
        return;

    //reset reload flag
    reload_needed_ = false;

    loginf << "ViewManager: loadingStartedSlot";

    for (auto& view_it : views_)
        view_it.second->loadingStarted();
}

// all data contained, also new one. requires_reset true indicates that all shown info should be re-created,
// e.g. when data in the beginning was removed, or order of previously emitted data was changed, etc.
void ViewManager::loadedDataSlot (const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    if (disable_data_distribution_)
        return;

    logdbg << "ViewManager: loadedDataSlot: reset " << requires_reset;

    processing_data_ = true;

    using namespace boost::posix_time;
    ptime tmp_time;

    for (auto& view_it : views_)
    {
        tmp_time = microsec_clock::local_time();
        view_it.second->loadedData(data, requires_reset);

        logdbg << "ViewManager: loadedDataSlot: " << view_it.first << " took "
               << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true);
    }

    processing_data_ = false;

    logdbg << "ViewManager: loadedDataSlot: done";
}

void ViewManager::loadingDoneSlot() // emitted when all dbos have finished loading
{
    if (disable_data_distribution_)
        return;

    loginf << "ViewManager: loadingDoneSlot";

    for (auto& view_it : views_)
        view_it.second->loadingDone();
}

void ViewManager::appModeSwitchSlot (AppMode app_mode_previous, AppMode app_mode_current)
{
    loginf << "ViewManager: appModeSwitchSlot: app_mode " << COMPASS::instance().appModeStr();

    for (auto& view_it : views_)
    {
        view_it.second->appModeSwitch(app_mode_previous, app_mode_current);

        if (app_mode_current == AppMode::LiveRunning)
            view_it.second->enableInTabWidget(view_it.second->classId() == "GeographicView");
        else
            view_it.second->enableInTabWidget(true);
    }
}

View* ViewManager::latestView()
{
    time_t latest = std::numeric_limits<time_t>::min();
    View* latest_view = nullptr;

    for (const auto& elem : views_)
    {
        if (elem.second->created() > latest)
        {
            latest      = elem.second->created();
            latest_view = elem.second;
        }
    }

    return latest_view;
}

ViewContainerWidget* ViewManager::latestViewContainer()
{
    time_t latest = std::numeric_limits<time_t>::min();
    ViewContainerWidget* latest_container = nullptr;

    for (const auto& elem : container_widgets_)
    {
        if (elem.second->viewContainer().created() > latest)
        {
            latest           = elem.second->viewContainer().created();
            latest_container = elem.second;
        }
    }

    return latest_container;
}

bool ViewManager::viewPresetsEnabled() const
{
#ifdef SCAN_PRESETS
    return true;
#else
    return false;
#endif
}

/**
 * Notifies the view manager that the reload state in a view has changed, 
 * determines the new global reload state, and informs all views about it.
 */
void ViewManager::notifyReloadStateChanged()
{
    //query views if one of them needs to reload
    bool reload_needed = false;
    for (const auto& elem : views_)
    {
        if (!elem.second->reloadNeeded())
            continue;

        logdbg << "ViewManager::notifyReloadStateChanged: view '" << elem.first << "' needs to reload";

        reload_needed = true;
        break;
    }

    logdbg << "ViewManager::notifyReloadStateChanged: reload needed before: " << reload_needed_ << ", now: " << reload_needed;

    //reload state has not changed? => just return
    if (reload_needed_ == reload_needed)
        return;

    //update global reload flag
    reload_needed_ = reload_needed;

    logdbg << "ViewManager::notifyReloadStateChanged: emitting new reload state " << reload_needed_;

    //inform views about changed reload state
    emit reloadStateChanged();
}

/**
 * Checks if a reload is needed (has been notified by a view).
 */
bool ViewManager::reloadNeeded() const
{
    return reload_needed_;
}

/**
 * Enables/disables automatic reloading in the view manager and informs all views about it.
 */
void ViewManager::enableAutomaticReload(bool enable)
{
    if (config_.automatic_reload == enable)
        return;

    config_.automatic_reload = enable;

    //inform views about changed auto-update state
    emit automaticUpdatesChanged();
}

/**
 * Enables/disables automatic redrawing in the view manager and informs all views about it.
 */
void ViewManager::enableAutomaticRedraw(bool enable)
{
    if (config_.automatic_redraw == enable)
        return;

    config_.automatic_redraw = enable;

    //inform about changed auto-update state
    emit automaticUpdatesChanged();
}

/**
 */
bool ViewManager::automaticReloadEnabled() const
{
    return config_.automatic_reload;
}

/**
 */
bool ViewManager::automaticRedrawEnabled() const
{
    return config_.automatic_redraw;
}

/**
*/
void ViewManager::updateFeatures()
{
    for (const auto& cw : container_widgets_)
        if (cw.second)
            cw.second->updateFeatures();
    
    for (const auto& v : views_)
        if (v.second)
            v.second->updateFeatures();
}

// void ViewManager::saveViewAsTemplate (View *view, std::string template_name)
//{
//    //view->saveConfigurationAsTemplate("Template"+view->getInstanceId());
//    saveTemplateConfiguration (view, template_name);
//}
