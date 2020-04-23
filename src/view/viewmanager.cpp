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

#include "json.hpp"

#include <QMessageBox>
#include <QWidget>
#include <QTabWidget>
#include <QMetaType>

#include <cassert>
#include <fstream>

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

    if (ATSDB::instance().interface().existsViewPointsTable())
    {
        for (const auto& vp_it : ATSDB::instance().interface().viewPoints())
        {
            assert (!view_points_.count(vp_it.first));
            view_points_.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(vp_it.first),   // args for key
                                 std::forward_as_tuple(vp_it.first, vp_it.second, *this));  // args for mapped value
        }
    }

    view_points_widget_ = new ViewPointsWidget(*this);
    view_points_widget_->setAutoFillBackground(true);

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

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    saveViewPoints();
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

    if (class_id.compare("ViewContainer") == 0)
    {
        ViewContainer* container =
                new ViewContainer(class_id, instance_id, this, this, main_tab_widget_, 0);
        assert(containers_.count(instance_id) == 0);
        containers_.insert(std::pair<std::string, ViewContainer*>(instance_id, container));

        unsigned int number = String::getAppendedInt(instance_id);
        if (number >= container_count_)
            container_count_ = number;
    }
    else if (class_id.compare("ViewContainerWidget") == 0)
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

unsigned int ViewManager::saveNewViewPoint(bool update)
{
    unsigned int new_id {0};

    if (view_points_.size())
        new_id = view_points_.rbegin()->first + 1;

    assert (!existsViewPoint(new_id));

    saveNewViewPoint(new_id, update);

    return new_id;
}

ViewPoint& ViewManager::saveNewViewPoint(unsigned int id, bool update)
{
    if (view_points_.count(id))
        throw std::runtime_error ("ViewManager: addNewViewPoint: id "+std::to_string(id)+" already exists");

    view_points_.emplace(std::piecewise_construct,
                         std::forward_as_tuple(id),   // args for key
                         std::forward_as_tuple(id, *this));  // args for mapped value

    assert (existsViewPoint(id));

    ATSDB::instance().filterManager().setConfigInViewPoint(view_points_.at(id));
    view_points_.at(id).dirty(true);

    if (update && view_points_widget_)
        view_points_widget_->update();

    return view_points_.at(id);
}

bool ViewManager::existsViewPoint(unsigned int id)
{
    return view_points_.count(id) == 1;
}

ViewPoint& ViewManager::viewPoint(unsigned int id)
{
    assert (existsViewPoint(id));
    return view_points_.at(id);
}

void ViewManager::removeViewPoint(unsigned int id)
{
    assert (existsViewPoint(id));
    view_points_.erase(id);
    ATSDB::instance().interface().deleteViewPoint(id);
}

void ViewManager::deleteAllViewPoints ()
{
    unsetCurrentViewPoint();
    view_points_.clear();
    ATSDB::instance().interface().deleteAllViewPoints();
}

void ViewManager::printViewPoints()
{
    for (auto& vp_it : view_points_)
        vp_it.second.print();
}

void ViewManager::saveViewPoints()
{
    loginf << "ViewManager: saveViewPoints";

    DBInterface& db_interface = ATSDB::instance().interface();

    for (auto& vp_it : view_points_)
    {
        if (vp_it.second.dirty())
        {
            db_interface.setViewPoint(vp_it.first, vp_it.second.data().dump());
            vp_it.second.dirty(false);
        }
    }
}

ViewPointsWidget* ViewManager::viewPointsWidget() const
{
    return view_points_widget_;
}

void ViewManager::importViewPoints (const std::string& filename)
{
    loginf << "ViewManager: importViewPoints: filename '" << filename << "'";

    try
    {
        if (!Files::fileExists(filename))
            throw std::runtime_error ("File '"+filename+"' not found.");

        std::ifstream ifs(filename);
        json j = json::parse(ifs);

        if (j.contains("view_point_context"))
            loginf << "ViewManager: importViewPoints: context '" << j.at("view_point_context").dump(4) << "'";

        if (!j.contains("view_points"))
            throw std::runtime_error ("File '"+filename+"' does not contain view points.");

        json& view_points = j.at("view_points");

        if (!view_points.is_array())
            throw std::runtime_error ("View points are not in an array.");

        unsigned int id;
        for (auto& vp_it : view_points.get<json::array_t>())
        {
            if (!vp_it.contains("id"))
                throw std::runtime_error ("View point does not contain id");

            id = vp_it.at("id");

            if (!vp_it.contains("status"))
                vp_it["status"] = "open";

            ViewPoint& vp = saveNewViewPoint(id, false);
            vp.data() = vp_it;
        }

        if (view_points_widget_)
            view_points_widget_->update();

        QMessageBox m_info(QMessageBox::Information, "View Points Import File",
                           "File import: '"+QString(filename.c_str())+"' done.\n"
                           +QString::number(view_points.size())+" View Points added.", QMessageBox::Ok);
        m_info.exec();
    }
    catch (std::exception& e)
    {
        QMessageBox m_warning(QMessageBox::Warning, "View Points Import File",
                              "File import error: '"+QString(e.what())+"'.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }
}

void ViewManager::exportViewPoints (const std::string& filename)
{
    loginf << "ViewManager: exportViewPoints: filename '" << filename << "'";

    json data;

    data["view_points"] = json::array();
    json& view_points = data.at("view_points");

    unsigned int cnt = 0;
    for (auto& vp_it : view_points_)
    {
        view_points[cnt] = vp_it.second.data();
        ++cnt;
    }

    std::ofstream file(filename);
    file << data.dump(4);

    QMessageBox m_info(QMessageBox::Information, "View Points Export File",
                       "File export: '"+QString(filename.c_str())+"' done.\n"
                       +QString::number(view_points.size())+" View Points saved.", QMessageBox::Ok);
    m_info.exec();
}


void ViewManager::setCurrentViewPoint (unsigned int id)
{
    if (current_view_point_set_)
        unsetCurrentViewPoint();

    assert (existsViewPoint(id));
    current_view_point_set_ = true;
    current_view_point_ = id;
    emit showViewPointSignal(&view_points_.at(current_view_point_));

    ATSDB::instance().objectManager().loadSlot();
}


void ViewManager::unsetCurrentViewPoint ()
{
    if (current_view_point_set_)
    {
        assert (existsViewPoint(current_view_point_));
        emit unshowViewPointSignal(&view_points_.at(current_view_point_));
        current_view_point_set_ = false;
        current_view_point_ = 0;
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
