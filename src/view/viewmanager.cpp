/*
 * ViewManager.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: sk
 */

#include <QWidget>
#include <QMessageBox>

#include <cassert>

#include "atsdb.h"
#include "buffer.h"
#include "viewmanager.h"
#include "viewmanagerwidget.h"
#include "logger.h"
//#include "DBObjectManager.h"
//#include "DBResultSetManager.h"
//#include "ConfigurationManager.h"
//#include "BufferSet.h"
#include "viewcontainer.h"
#include "view.h"
//#include "DBView.h"
#include "stringconv.h"

using namespace Utils;

ViewManager::ViewManager(const std::string &class_id, const std::string &instance_id, ATSDB *atsdb)
    : Configurable (class_id, instance_id, atsdb, "conf/config_views.xml"), atsdb_(*atsdb), widget_(nullptr), initialized_(false), container_count_(0)
{
    logdbg  << "ViewManager: constructor";

    //  central_widget_=0;

    //  if (DBObjectManager::getInstance().hasObjects() && DBObjectManager::getInstance().existsDBOVariable (DBO_UNDEFINED, "id"))
    //  {
    //      logdbg  << "DBResultSetManager: constructor: adding id";
    //      read_set_.add (DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id"));
    //  }

}

void ViewManager::init (QTabWidget *tab_widget)
{
  logdbg  << "ViewManager: init";
  assert (tab_widget);
  assert (!tab_widget_);
  assert (!initialized_);
  tab_widget_=tab_widget;

  initialized_=true;

  createSubConfigurables ();
}

void ViewManager::close ()
{
  logdbg  << "ViewManager: close";

  for (auto it = containers_.begin(); it != containers_.end(); it++)
  {
    //it->second->deleteLater ();
    delete it->second;
  }
  containers_.clear();

  if (widget_)
  {
      delete widget_;
      widget_ = nullptr;
  }

  initialized_=false;
}

ViewManager::~ViewManager()
{
    logdbg  << "ViewManager: destructor";

    assert (!widget_);
    assert (!initialized_);
}

void ViewManager::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "ViewManager: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
    if (class_id.compare ("ViewContainerWidget") == 0)
    {
        assert (false);
        //    ViewContainerWidget *container = new ViewContainerWidget (class_id, instance_id, this);
        //    containers_.insert (std::pair <std::string, ViewContainerWidget *> (instance_id, container));

        //    unsigned int number = getAppendedInt (instance_id);
        //    if (number >= container_count_)
        //      container_count_ = number;
    }
    else
        throw std::runtime_error ("ViewManager: generateSubConfigurable: unknown class_id "+class_id );
}

void ViewManager::checkSubConfigurables ()
{
//    if (containers_.size() == 0)
//    {
//        Configuration &main_view_container_config = addNewSubConfiguration ("ViewContainerWidget", "ViewContainerWidget0");
//        main_view_container_config.addParameterBool ("seperate_window", false);
//        generateSubConfigurable ("ViewContainerWidget", "ViewContainerWidget0");
//    }

//    if (views_widget_)
//        views_widget_->update();
}

ViewManagerWidget *ViewManager::widget()
{
    if (!widget_)
    {
        widget_ = new ViewManagerWidget(*this);
    }

    assert (widget_);
    return widget_;
}

//void ViewManager::addContainerWithGeographicView ()
//{
//    logdbg  << "ViewManager: addContainerWithGeographicView";

//    container_count_++;
//    std::string container_name = "ViewContainerWidget"+intToString(container_count_);
//    std::string view_name = "GeographicView"+intToString(ViewContainerWidget::getViewCount());

//    Configuration &container_config = addNewSubConfiguration ("ViewContainerWidget", container_name);
//    container_config.addNewSubConfiguration ("GeographicView", view_name);
//    generateSubConfigurable ("ViewContainerWidget", container_name);

//    if (views_widget_)
//        views_widget_->update();
//}

//void ViewManager::addContainerWithHistogramView ()
//{
//    logdbg  << "ViewManager: addContainerWithHistogramView";

//    container_count_++;
//    std::string container_name = "ViewContainerWidget"+intToString(container_count_);
//    std::string view_name = "HistogramView"+intToString(ViewContainerWidget::getViewCount());

//    Configuration &container_config = addNewSubConfiguration ("ViewContainerWidget", container_name);
//    container_config.addNewSubConfiguration ("HistogramView", view_name);
//    generateSubConfigurable ("ViewContainerWidget", container_name);

//    if (views_widget_)
//        views_widget_->update();
//}

//void ViewManager::addContainerWithListBoxView ()
//{
//    logdbg  << "ViewManager: addContainerWithHistogramView";

//    container_count_++;
//    std::string container_name = "ViewContainerWidget"+intToString(container_count_);
//    std::string view_name = "ListBoxView"+intToString(ViewContainerWidget::getViewCount());

//    Configuration &container_config = addNewSubConfiguration ("ViewContainerWidget", container_name);
//    container_config.addNewSubConfiguration ("ListBoxView", view_name);
//    generateSubConfigurable ("ViewContainerWidget", container_name);

//    if (views_widget_)
//        views_widget_->update();
//}

//void ViewManager::addContainerWithMosaicView ()
//{
//    logdbg  << "ViewManager: addContainerWithMosaicView";

//    container_count_++;
//    std::string container_name = "ViewContainerWidget"+intToString(container_count_);
//    std::string view_name = "MosaicView"+intToString(ViewContainerWidget::getViewCount());

//    Configuration &container_config = addNewSubConfiguration ("ViewContainerWidget", container_name);
//    container_config.addNewSubConfiguration ("MosaicView", view_name);
//    generateSubConfigurable ("ViewContainerWidget", container_name);

//    if (views_widget_)
//        views_widget_->update();
//}

//void ViewManager::addContainerWithScatterPlotView ()
//{
//    logdbg  << "ViewManager: addContainerWithScatterPlotView";

//    container_count_++;
//    std::string container_name = "ViewContainerWidget"+intToString(container_count_);
//    std::string view_name = "ScatterPlotView"+intToString(ViewContainerWidget::getViewCount());

//    Configuration &container_config = addNewSubConfiguration ("ViewContainerWidget", container_name);
//    container_config.addNewSubConfiguration ("ScatterPlotView", view_name);
//    generateSubConfigurable ("ViewContainerWidget", container_name);

//    if (views_widget_)
//        views_widget_->update();
//}

//void ViewManager::addContainerWithTemplateView (std::string template_name)
//{
//    logdbg  << "ViewManager: addContainerWithTemplateView";

//    container_count_++;
//    std::string container_name = "ViewContainerWidget"+intToString(container_count_);
//    std::string view_name = template_name+intToString(ViewContainerWidget::getViewCount());

//    Configuration &container_config = addNewSubConfiguration ("ViewContainerWidget", container_name);

//    std::map<std::string, Configuration> &templates = configuration_.getConfigurationTemplates ();
//    assert (templates.find (template_name) != templates.end());
//    Configuration view_config = templates [template_name];
//    view_config.setInstanceId(view_name);
//    view_config.setTemplate(false, "");

//    container_config.addNewSubConfiguration (view_config);
//    generateSubConfigurable ("ViewContainerWidget", container_name);

//    if (views_widget_)
//        views_widget_->update();
//}

void ViewManager::registerView (View *view)
{
    logdbg  << "ViewManager: registerView";
    assert (view);
    assert (!isRegistered(view));
    views_[view->getInstanceId()]=view;
    //updateReadSet();
}

void ViewManager::unregisterView (View *view)
{
    logdbg  << "ViewManager: unregisterView " << view->getName().c_str();
    assert (view);
    assert (isRegistered(view));

    std::map<std::string, View*>::iterator it;

    it=views_.find(view->getInstanceId());
    views_.erase(it);
    //updateReadSet();
}

bool ViewManager::isRegistered (View *view)
{
    logdbg  << "ViewManager: isRegistered";
    assert (view);

    std::map<std::string, View*>::iterator it;

    it=views_.find(view->getInstanceId());

    return !(it == views_.end());
}

//void ViewManager::distributeData (Buffer *buffer)
//{
//    logdbg  << "ViewManager: distributeData";
//    assert (buffer);

//    std::map<std::string, View*>::iterator it;

//    for (it = views_.begin(); it != views_.end(); it++)
//    {
//        BufferSet *copy = new BufferSet();
//        assert (copy);

//        if( it->second->viewType() != "DBView" )
//            continue;

//        DBView* dbview = (DBView*)it->second;

//        copy->addBuffer(buffer->getShallowCopy());

//        if( !dbview->addData(copy) )
//        {
//            copy->clearAndDelete();
//            delete copy;
//        }
//    }

//}

//void ViewManager::clearData ()
//{
//    logdbg  << "ViewManager: clearData";

//    std::map<std::string, View*>::iterator it;

//    for (it = views_.begin(); it != views_.end(); it++)
//    {
//        logdbg  << "ViewManager: clearData: calling on view " << it->first;
//        it->second->clearData();
//    }
//}

//void ViewManager::setViewsWidget (ViewsWidget *views_widget)
//{
//    assert (!views_widget_);
//    assert (views_widget);
//    views_widget_=views_widget;
//}

void ViewManager::deleteContainer (std::string instance_id)
{
    logdbg  << "ViewManager: removeContainer: instance " << instance_id;

    std::map <std::string, ViewContainer*>::iterator it=containers_.find(instance_id);

    if (it != containers_.end())
    {
        //it->second->close(); // TODO for widgets
        it->second->deleteLater();

        containers_.erase(it);

        if (widget_)
            widget_->update();

        return;
    }

    throw std::runtime_error ("ViewManager: removeContainer:  key not found");
}

void ViewManager::removeContainer (std::string instance_id)
{
    std::map <std::string, ViewContainer*>::iterator it;

    logdbg  << "ViewManager: removeContainer: instance " << instance_id;

    it=containers_.find(instance_id);

    if (it != containers_.end())
    {
        containers_.erase(it);

        if (widget_)
            widget_->update();

        return;
    }

    throw std::runtime_error ("ViewManager: removeContainer:  key not found");
}

//void ViewManager::updateReadSet ()
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

//    if (DBObjectManager::getInstance().hasObjects() && DBObjectManager::getInstance().existsDBOVariable (DBO_UNDEFINED, "id"))
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

void ViewManager::viewShutdown( View* view, const std::string& err )
{
    delete view;

    //TODO
//    if( views_widget_ )
//        views_widget_->update();

    if (err.size())
        QMessageBox::critical( NULL, "View Shutdown", QString::fromStdString( err ) );
}

//void ViewManager::saveViewAsTemplate (View *view, std::string template_name)
//{
//    //view->saveConfigurationAsTemplate("Template"+view->getInstanceId());
//    saveTemplateConfiguration (view, template_name);
//}
