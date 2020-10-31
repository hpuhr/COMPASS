/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "viewmanager.h"
#include "atsdb.h"
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
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "metadbovariable.h"
#include "dbovariable.h"
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

ViewManager::ViewManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb)
    : Configurable(class_id, instance_id, atsdb, "views.json"), atsdb_(*atsdb)
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

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    view_points_widget_ = new ViewPointsWidget(*this);
    //view_points_widget_->setAutoFillBackground(true);

    QApplication::restoreOverrideCursor();

    assert(view_points_widget_);
    tab_widget->addTab(view_points_widget_, "View Points");

    FilterManager& filter_man = ATSDB::instance().filterManager();

    connect (this, &ViewManager::showViewPointSignal, &filter_man, &FilterManager::showViewPointSlot);
    connect (this, &ViewManager::unshowViewPointSignal, &filter_man, &FilterManager::unshowViewPointSlot);

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
        delete first_it->second;
        container_widgets_.erase(first_it);
    }

    logdbg << "ViewManager: close: deleting containers size " << containers_.size();
    while (containers_.size())
    {
        auto first_it = containers_.begin();
        logdbg << "ViewManager: close: deleting container " << first_it->first;
        delete first_it->second;
    }

    if (view_points_widget_)
    {
        //view_points_widget_->tableModel()->saveViewPoints();
        delete view_points_widget_;
        view_points_widget_ = nullptr;
    }

    view_points_report_gen_ = nullptr;

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

ViewManager::~ViewManager()
{
    logdbg << "ViewManager: destructor";

    assert(!container_widgets_.size());
    assert(!containers_.size());
    assert(!widget_);
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

    if (widget_)
        widget_->update();
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

DBOVariableSet ViewManager::getReadSet(const std::string& dbo_name)
{
    DBOVariableSet read_set;
    DBOVariableSet read_set_tmp;

    for (auto view_it : views_)
    {
        read_set_tmp = view_it.second->getSet(dbo_name);
        read_set.add(read_set_tmp);
    }
    return read_set;
}

ViewManagerWidget* ViewManager::widget()
{
    if (!widget_)
    {
        widget_ = new ViewManagerWidget(*this);
    }

    assert(widget_);
    return widget_;
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

    ATSDB::instance().objectManager().loadSlot();
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

    DBObjectManager& object_manager = ATSDB::instance().objectManager();

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

        const DBOVariable& tod_var = object_manager.metaVariable("tod").getFor(dbo_name);
        const DBOVariable& latitude_var =
                object_manager.metaVariable("pos_lat_deg").getFor(dbo_name);
        const DBOVariable& longitude_var =
                object_manager.metaVariable("pos_long_deg").getFor(dbo_name);

        if (!tod_var.existsInDB() || !latitude_var.existsInDB() || !longitude_var.existsInDB())
        {
            logdbg << "ViewManager: doViewPointAfterLoad: required variables not in db for " << dbo_name;
            continue;
        }

        std::shared_ptr<Buffer> buffer = dbo_it.second->data();

        if (buffer)
        {
            assert(buffer->has<bool>("selected"));
            NullableVector<bool>& selected_vec = buffer->get<bool>("selected");

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

    DBObjectManager& object_manager = ATSDB::instance().objectManager();

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

        const DBOVariable& tod_var = object_manager.metaVariable("tod").getFor(dbo_name);

        if (!tod_var.existsInDB())
        {
            logdbg << "ViewManager: selectTimeWindow: required variables not in db for " << dbo_name;
            continue;
        }

        std::shared_ptr<Buffer> buffer = dbo_it.second->data();

        if (buffer)
        {
            assert(buffer->has<bool>("selected"));
            NullableVector<bool>& selected_vec = buffer->get<bool>("selected");

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

// void ViewManager::deleteContainer (std::string instance_id)
//{
//    logdbg  << "ViewManager: removeContainer: instance " << instance_id;

//    std::map <std::string, ViewContainer*>::iterator it=containers_.find(instance_id);

//    if (it != containers_.end())
//    {
//        //it->second->close(); // TODO for widgets
//        it->second->deleteLater();

//        containers_.erase(it);

//        if (widget_)
//            widget_->update();

//        return;
//    }

//    throw std::runtime_error ("ViewManager: removeContainer:  key not found");
//}

void ViewManager::removeContainer(std::string instance_id)
{
    std::map<std::string, ViewContainer*>::iterator it;

    logdbg << "ViewManager: removeContainer: instance " << instance_id;

    it = containers_.find(instance_id);

    if (it != containers_.end())
    {
        containers_.erase(it);

        if (initialized_ && widget_)  // not during destructor
            widget_->update();

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

        if (initialized_ && widget_)  // not during destructor
            widget_->update();

        return;
    }

    throw std::runtime_error("ViewManager: removeContainer:  key not found");
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

        if (initialized_ && widget_)  // not during destructor
            widget_->update();

        return;
    }

    throw std::runtime_error("ViewManager: deleteContainerWidget:  key not found");
}

// void ViewManager::updateReadSet ()
//{
//    logdbg  << "ViewManager: updateReadSet";
//    //
//    DBOVariableSet new_set;

//    std::map<std::string, View*>::iterator it;

//    for (it = views_.begin(); it != views_.end(); it++)
//    {
//        if( it->second->viewType() != "DBView" )
//            continue;

//        DBView* dbview = (DBView*)it->second;
//        new_set.add (dbview->getReadList());
//    }

//    read_set_.add (new_set);
//    read_set_.intersect (new_set);

//    if (DBObjectManager::getInstance().hasObjects() &&
//    DBObjectManager::getInstance().existsDBOVariable (DBO_UNDEFINED, "id"))
//    {
//        logdbg  << "DBResultSetManager: constructor: adding id";
//        read_set_.add (DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id"));
//    }

//    if (read_set_.getChanged())
//    {
//        if (DBResultSetManager::getInstance().getAutoUpdate ())
//            DBResultSetManager::getInstance().startLoadingData ();
//    }
//}

void ViewManager::viewShutdown(View* view, const std::string& err)
{
    delete view;

    // TODO
    //    if( views_widget_ )
    //        views_widget_->update();

    if (err.size())
        QMessageBox::critical(NULL, "View Shutdown", QString::fromStdString(err));
}

void ViewManager::selectionChangedSlot()
{
    loginf << "ViewManager: selectionChangedSlot";
    emit selectionChangedSignal();
}

// void ViewManager::saveViewAsTemplate (View *view, std::string template_name)
//{
//    //view->saveConfigurationAsTemplate("Template"+view->getInstanceId());
//    saveTemplateConfiguration (view, template_name);
//}
