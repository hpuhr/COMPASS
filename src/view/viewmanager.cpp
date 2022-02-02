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
#include "viewmanagerwidget.h"
#include "viewpoint.h"
#include "dbinterface.h"
#include "viewpointswidget.h"
#include "files.h"
#include "filtermanager.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/variable/variable.h"
#include "viewpointstablemodel.h"
#include "viewpointsreportgenerator.h"
#include "viewpointsreportgeneratordialog.h"

#include "json.hpp"

#include <QMessageBox>
#include <QWidget>
#include <QTabWidget>
#include <QMetaType>
#include <QApplication>

#include <cassert>

using namespace Utils;
using namespace nlohmann;

ViewManager::ViewManager(const std::string& class_id, const std::string& instance_id, COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "views.json"), compass_(*compass)
{
    logdbg << "ViewManager: constructor";

    qRegisterMetaType<ViewPoint*>("ViewPoint*");
}

void ViewManager::init(QTabWidget* tab_widget)
{
    logdbg << "ViewManager: init";
    assert(tab_widget);
    assert(!main_tab_widget_);
    assert(!initialized_);

    main_tab_widget_ = tab_widget;

    connect (&COMPASS::instance(), &COMPASS::appModeSwitchSignal,
             this, &ViewManager::appModeSwitchSlot);

    // view point stuff

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    view_points_widget_ = new ViewPointsWidget(*this);

    QApplication::restoreOverrideCursor();

    assert(view_points_widget_);
    tab_widget->addTab(view_points_widget_, "View Points");

    FilterManager& filter_man = COMPASS::instance().filterManager();

    connect (this, &ViewManager::showViewPointSignal, &filter_man, &FilterManager::showViewPointSlot);
    connect (this, &ViewManager::unshowViewPointSignal, &filter_man, &FilterManager::unshowViewPointSlot);

    view_class_list_.append("HistogramView");
    view_class_list_.append("ListBoxView");

#if USE_EXPERIMENTAL_SOURCE == true
    view_class_list_.append("OSGView");
#endif

    view_class_list_.append("ScatterPlotView");


    initialized_ = true;

    createSubConfigurables();
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
        addNewSubConfiguration("ViewContainer", "ViewContainer0");
        generateSubConfigurable("ViewContainer", "ViewContainer0");
    }

    if (!view_points_report_gen_)
    {
        addNewSubConfiguration("ViewPointsReportGenerator", "ViewPointsReportGenerator0");
        generateSubConfigurable("ViewPointsReportGenerator", "ViewPointsReportGenerator0");
    }
}

dbContent::VariableSet ViewManager::getReadSet(const std::string& dbo_name)
{
    dbContent::VariableSet read_set;
    dbContent::VariableSet read_set_tmp;

    for (auto view_it : views_)
    {
        read_set_tmp = view_it.second->getSet(dbo_name);
        read_set.add(read_set_tmp);
    }
    return read_set;
}

ViewPointsWidget* ViewManager::viewPointsWidget() const
{
    return view_points_widget_;
}

ViewPointsReportGenerator& ViewManager::viewPointsGenerator()
{
    assert (view_points_report_gen_);
    return *view_points_report_gen_;
}

void ViewManager::setCurrentViewPoint (const ViewableDataConfig* viewable)
{
    if (current_viewable_)
        unsetCurrentViewPoint();

    current_viewable_ = viewable;

    view_point_data_selected_ = false;

    loginf << "ViewManager: setCurrentViewPoint: setting current view point"; // << " data: '"
    //<< view_points_widget_->tableModel()->viewPoint(current_view_point_).data().dump(4) << "'";

    emit showViewPointSignal(current_viewable_);

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
    loginf << "ViewManager: doViewPointAfterLoad";

    if (!current_viewable_)
    {
        loginf << "ViewManager: doViewPointAfterLoad: no viewable";
        return; // nothing to do
    }

    if (view_point_data_selected_)
    {
        loginf << "ViewManager: doViewPointAfterLoad: data already selected";
        return; // already done, this is a re-load
    }

    assert (view_points_widget_);

    const json& data = current_viewable_->data();

    logdbg << "ViewManager: doViewPointAfterLoad: data '" << data.dump(4) << "'";

    bool contains_time = data.contains("time");
    float time;
    bool contains_time_window = data.contains("time_window");
    float time_window, time_min, time_max;

    if (!contains_time)
    {
        loginf << "ViewManager: doViewPointAfterLoad: no time given";
        return; // nothing to do
    }
    else
    {
        assert (data.at("time").is_number());
        time = data.at("time");

        loginf << "ViewManager: doViewPointAfterLoad: time " << time;
    }

    if (contains_time_window)
    {
        assert (data.at("time_window").is_number());
        time_window = data.at("time_window");
        time_min = time-time_window/2.0;
        time_max = time+time_window/2.0;

        loginf << "ViewManager: doViewPointAfterLoad: time window min " << time_min << " max " << time_max;
    }

    DBContentManager& object_manager = COMPASS::instance().dbContentManager();

    if (!object_manager.existsMetaVariable("tod") ||
            !object_manager.existsMetaVariable("pos_lat_deg") ||
            !object_manager.existsMetaVariable("pos_long_deg"))
    {
        logerr << "ViewManager: doViewPointAfterLoad: required variables missing, quitting";
        return;
    }

    bool selection_changed = false;
    for (auto& dbo_it : object_manager)
    {
        std::string dbo_name = dbo_it.first;

        if (!object_manager.metaVariable("tod").existsIn(dbo_name) ||
                !object_manager.metaVariable("pos_lat_deg").existsIn(dbo_name) ||
                !object_manager.metaVariable("pos_long_deg").existsIn(dbo_name))
        {
            logerr << "ViewManager: doViewPointAfterLoad: required variables missing for " << dbo_name;
            continue;
        }

        const dbContent::Variable& tod_var = object_manager.metaVariable("tod").getFor(dbo_name);
//        const DBOVariable& latitude_var =
//                object_manager.metaVariable("pos_lat_deg").getFor(dbo_name);
//        const DBOVariable& longitude_var =
//                object_manager.metaVariable("pos_long_deg").getFor(dbo_name);

        if (object_manager.data().count(dbo_it.first))
        {
            std::shared_ptr<Buffer> buffer = object_manager.data().at(dbo_it.first);

            assert(buffer->has<bool>(DBContent::selected_var.name()));
            NullableVector<bool>& selected_vec = buffer->get<bool>(DBContent::selected_var.name());

            assert(buffer->has<float>(tod_var.name()));
            NullableVector<float>& tods = buffer->get<float>(tod_var.name());
//            assert(buffer->has<double>(latitude_var.name()));
//            NullableVector<double>& latitudes = buffer->get<double>(latitude_var.name());
//            assert(buffer->has<double>(longitude_var.name()));
//            NullableVector<double>& longitudes = buffer->get<double>(longitude_var.name());

            unsigned int buffer_size = buffer->size();

            bool tod_null;
            float tod;

            for (unsigned int cnt =0; cnt < buffer_size; ++cnt)
            {
                tod_null = tods.isNull(cnt);

                if (tod_null)
                    continue; // nothing to do

                tod = tods.get(cnt);

                if (contains_time_window)
                {
                    if (tod >= time_min && tod <= time_max)
                    {
                        selected_vec.set(cnt, true);
                        selection_changed = true;

                        logdbg << "ViewManager: doViewPointAfterLoad: time " << tod << " selected ";
                    }
                }
                else if (contains_time && tod == time)
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

void ViewManager::selectTimeWindow(float time_min, float time_max)
{
    loginf << "ViewManager: selectTimeWindow: time_min " << time_min << " time_max " << time_max;

    DBContentManager& object_manager = COMPASS::instance().dbContentManager();

    if (!object_manager.existsMetaVariable("tod"))
    {
        logerr << "ViewManager: selectTimeWindow: required variables missing, quitting";
        return;
    }

    bool selection_changed = false;
    for (auto& dbo_it : object_manager)
    {
        std::string dbo_name = dbo_it.first;

        if (!object_manager.metaVariable("tod").existsIn(dbo_name))
        {
            logerr << "ViewManager: selectTimeWindow: required variables missing for " << dbo_name;
            continue;
        }

        const dbContent::Variable& tod_var = object_manager.metaVariable("tod").getFor(dbo_name);

        if (object_manager.data().count(dbo_it.first))
        {
            std::shared_ptr<Buffer> buffer = object_manager.data().at(dbo_it.first);

            assert(buffer->has<bool>(DBContent::selected_var.name()));
            NullableVector<bool>& selected_vec = buffer->get<bool>(DBContent::selected_var.name());

            assert(buffer->has<float>(tod_var.name()));
            NullableVector<float>& tods = buffer->get<float>(tod_var.name());

            unsigned int buffer_size = buffer->size();

            bool tod_null;
            float tod;

            for (unsigned int cnt =0; cnt < buffer_size; ++cnt)
            {
                tod_null = tods.isNull(cnt);

                if (tod_null)
                    continue; // nothing to do

                tod = tods.get(cnt);

                if (tod >= time_min && tod <= time_max)
                {
                    selected_vec.set(cnt, true);
                    selection_changed = true;

                    logdbg << "ViewManager: selectTimeWindow: time " << tod << " selected ";

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

QStringList ViewManager::viewClassList() const
{
    return view_class_list_;
}

unsigned int ViewManager::newViewNumber()
{
    unsigned int max_number = 0;
    unsigned int tmp;

    for (auto& view_it : views_)
    {
        tmp = String::getAppendedInt(view_it.second->instanceId());

        if (tmp > max_number)
            max_number = tmp;
    }

    return max_number + 1;
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

ViewContainerWidget* ViewManager::addNewContainerWidget()
{
    logdbg << "ViewManager: addNewContainerWidget";
    
    container_count_++;
    std::string container_widget_name = "ViewWindow" + std::to_string(container_count_);

    addNewSubConfiguration("ViewContainerWidget", container_widget_name);
    generateSubConfigurable("ViewContainerWidget", container_widget_name);

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

void ViewManager::loadingStartedSlot()
{
    if (disable_data_distribution_)
        return;

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
    loginf << "ViewManager: loadedDataSlot: reset " << requires_reset;

    processing_data_ = true;

    for (auto& view_it : views_)
        view_it.second->loadedData(data, requires_reset);

    processing_data_ = false;

    loginf << "ViewManager: loadedDataSlot: done";
}

void ViewManager::loadingDoneSlot() // emitted when all dbos have finished loading
{
    if (disable_data_distribution_)
        return;

    loginf << "ViewManager: loadingDoneSlot";

    for (auto& view_it : views_)
        view_it.second->loadingDone();
}

void ViewManager::appModeSwitchSlot (AppMode app_mode)
{
    loginf << "ViewManager: appModeSwitchSlot: app_mode " << COMPASS::instance().appModeStr();

    for (auto& view_it : views_)
        view_it.second->appModeSwitch(app_mode);
}


// void ViewManager::saveViewAsTemplate (View *view, std::string template_name)
//{
//    //view->saveConfigurationAsTemplate("Template"+view->getInstanceId());
//    saveTemplateConfiguration (view, template_name);
//}
