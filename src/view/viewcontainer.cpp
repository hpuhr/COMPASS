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

#include "viewcontainer.h"

//#include "config.h"
#include "files.h"
#include "global.h"
#include "tableview.h"
#include "histogramview.h"
#include "scatterplotview.h"
#include "logger.h"
//#include "mainloadwidget.h"
//#include "stringconv.h"
#include "view.h"
#include "viewcontainer.h"
#include "viewmanager.h"
#include "ui_test_common.h"
#include "compass.h"
#include "rtcommand_helpers.h"
#include "viewwidget.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "geographicview.h"
#endif

#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QPushButton>
#include <QTabBar>
#include <QTabWidget>

//unsigned int ViewContainer::view_count_ = 0;

using namespace Utils;
using namespace std;

ViewContainer::ViewContainer(const std::string& class_id, 
                             const std::string& instance_id,
                             Configurable* parent, 
                             ViewManager* view_manager,
                             QTabWidget* tab_widget, 
                             int window_cnt)
    : QObject(),
      Configurable(class_id, instance_id, parent),
      view_manager_(*view_manager),
      tab_widget_(tab_widget),
      window_cnt_(window_cnt)
{
    logdbg << "ViewContainer: ctor: window " << window_cnt_;
    assert(tab_widget_);

    creation_time_ = boost::posix_time::to_time_t(boost::posix_time::microsec_clock::local_time());

    disable_add_remove_views_ = COMPASS::instance().disableAddRemoveViews();

    if (window_cnt != 0)
    {
        const QString tool_tip = "Add view";

        QPushButton* add_button = new QPushButton(tab_widget_);
        UI_TEST_OBJ_NAME(add_button, tool_tip);
        add_button->setIcon(QIcon(Files::getIconFilepath("crosshair_fat.png").c_str()));
        add_button->setFixedSize(UI_ICON_SIZE);
        add_button->setFlat(UI_ICON_BUTTON_FLAT);
        add_button->setToolTip(tr(tool_tip.toStdString().c_str()));
        add_button->setDisabled(disable_add_remove_views_);

        connect(add_button, &QPushButton::clicked, this, &ViewContainer::showAddViewMenuSlot);
        tab_widget_->setCornerWidget(add_button);
    }

    createSubConfigurables();
}

ViewContainer::~ViewContainer()
{
    logdbg << "ViewContainer: dtor";

    view_manager_.removeContainer(instanceId());

    logdbg << "ViewContainer: dtor: views list";
    for (auto& view : views_)
        logdbg << "ViewContainer: dtor: view " << view->instanceId();

    views_.clear();

    logdbg << "ViewContainer: dtor: done";
}

void ViewContainer::addView(const std::string& class_id)
{
    assert (!disable_add_remove_views_);

    auto config = Configuration::create(class_id, view_manager_.newViewInstanceId(class_id),
                          view_manager_.newViewName(class_id));

    generateSubConfigurableFromConfig(std::move(config));
}

void ViewContainer::enableViewTab(QWidget* widget, bool value)
{
    assert (widget);

    int index = tab_widget_->indexOf(widget);

    assert (index >= 0); // check if has widget
    tab_widget_->setTabEnabled(index, value);;
}

void ViewContainer::showView(QWidget* widget)
{
    assert (widget);
    assert (tab_widget_->indexOf(widget) >= 0); // check if has widget
    tab_widget_->setCurrentWidget(widget);
}

// void ViewContainer::addTemplateView (std::string template_name)
//{
//    std::string view_name = template_name+intToString(ViewContainerWidget::getViewCount());
//    std::map<std::string, Configuration> &templates =
//    ViewManager::getInstance().getConfiguration()
//            .getConfigurationTemplates ();
//    assert (templates.find (template_name) != templates.end());
//    Configuration view_config = templates [template_name];
//    view_config.setInstanceId(view_name);
//    view_config.setTemplate(false, "");

//    configuration_.addNewSubConfiguration(view_config);

//    generateSubConfigurable (view_config.getClassId(), view_config.getInstanceId());
//}

void ViewContainer::addView(View* view)
{
    assert(view);
    QWidget* w = view->getCentralWidget();
    assert(w);

    const QString view_name = QString::fromStdString(view->getName());

    int index = tab_widget_->addTab(w, view_name);

    QPushButton* manage_button = new QPushButton();
    UI_TEST_OBJ_NAME(manage_button, view_name + " Manager"); //manage buttons can be reached via e.g. window1.geographicview2_manager

    manage_button->setIcon(QIcon(Files::getIconFilepath("edit.png").c_str()));
    manage_button->setFixedSize(UI_ICON_SIZE);
    manage_button->setFlat(UI_ICON_BUTTON_FLAT);
    manage_button->setToolTip(tr("Manage view"));
    manage_button->setProperty("view_instance_id", view->instanceId().c_str());
    manage_button->setDisabled(disable_add_remove_views_);
    connect(manage_button, SIGNAL(clicked()), this, SLOT(showViewMenuSlot()));
    tab_widget_->tabBar()->setTabButton(index, QTabBar::RightSide, manage_button);

    //in localbuild we show some info about how the view is reachable via rtcommands
    if (!COMPASS::instance().isAppImage())
    {
        QString tt = rtcommand::getTooltip(view->getViewWidget(), view);
        tab_widget_->setTabToolTip(index, tt);
    }
}

void ViewContainer::deleteViewSlot()
{
    assert (!disable_add_remove_views_);

    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    QVariant instance_id_var = action->property("view_instance_id");
    assert (instance_id_var.isValid());

    string instance_id = instance_id_var.toString().toStdString();

    auto iter = std::find_if(views_.begin(), views_.end(),
                             [&instance_id](const unique_ptr<View>& x) { return x->instanceId() == instance_id;});

    assert (iter != views_.end());

    views_.erase(iter);
}

void ViewContainer::addNewViewSlot()
{
    assert (!disable_add_remove_views_);

    QAction* action = dynamic_cast<QAction*>(sender());
    assert (action);

    QVariant location_var = action->property("location");
    assert (location_var.isValid());

    string location = location_var.toString().toStdString();

    QVariant class_id_var = action->property("class_id");
    assert (class_id_var.isValid());

    string class_id = class_id_var.toString().toStdString();

    loginf << "ViewContainer: addNewViewSlot: location " << location << " class_id " << class_id;

    if (location == "here")
        addView(class_id);
    else if (location == "new")
    {
        ViewContainerWidget* container_widget = view_manager_.addNewContainerWidget();
        container_widget->viewContainer().addView(class_id);
    }
    else
        logerr << "ViewContainer: addNewViewSlot: unknown location '" << location << "'";

}

const std::vector<std::unique_ptr<View>>& ViewContainer::getViews() const { return views_; }

void ViewContainer::generateSubConfigurable(const std::string& class_id,
                                            const std::string& instance_id)
{
    if (class_id == "TableView")
    {
        views_.emplace_back(new TableView(class_id, instance_id, this, view_manager_));

        (*views_.rbegin())->init();
        addView(views_.rbegin()->get());
    }
    else if (class_id == "HistogramView")
    {
        views_.emplace_back(new HistogramView(class_id, instance_id, this, view_manager_));

        (*views_.rbegin())->init();
        addView(views_.rbegin()->get());
    }
    else if (class_id == "ScatterPlotView")
    {
        views_.emplace_back(new ScatterPlotView(class_id, instance_id, this, view_manager_));

        (*views_.rbegin())->init();
        addView(views_.rbegin()->get());
    }
    else if (class_id == "GeographicView")
    {
#if USE_EXPERIMENTAL_SOURCE == true

        views_.emplace_back(new GeographicView(class_id, instance_id, this, view_manager_));

        (*views_.rbegin())->init();
        addView(views_.rbegin()->get());
#else
        loginf << "ViewContainer: generateSubConfigurable: GeographicView ignored since compiled w/o experimental source";
#endif

    }
    //  else if (class_id.compare ("MosaicView") == 0)
    //  {
    //    MosaicView* view = new MosaicView ( class_id, instance_id, this );
    //    unsigned int number = getAppendedInt (instance_id);
    //    if (number >= view_count_)
    //      view_count_ = number+1;

    //    assert( view );
    //    view->init();
    //  }
    else
        throw std::runtime_error("ViewContainer: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void ViewContainer::checkSubConfigurables()
{
    // move along sir
}

std::string ViewContainer::getWindowName()
{
    if (window_cnt_ == 0)
        return "MainWindow";
    else
        return "Window" + std::to_string(window_cnt_);
}

void ViewContainer::showAddViewMenuSlot()
{
    loginf << "ViewContainer: showAddViewMenuSlot: window " << window_cnt_;
    assert (!disable_add_remove_views_);

    QMenu menu;

    QMenu* here_menu = menu.addMenu("Add Here");
    for (auto& class_it : view_manager_.viewClassList()) // class name, name
    {
        QAction* action = here_menu->addAction(class_it.second.c_str());
        action->setProperty("location", "here");
        action->setProperty("class_id", class_it.first.c_str());
        connect (action, &QAction::triggered, this, &ViewContainer::addNewViewSlot);
    }

    QMenu* new_menu = menu.addMenu("Add In New Window");
    for (auto& class_it : view_manager_.viewClassList())
    {
        QAction* action = new_menu->addAction(class_it.second.c_str());
        action->setProperty("location", "new");
        action->setProperty("class_id", class_it.first.c_str());
        connect (action, &QAction::triggered, this, &ViewContainer::addNewViewSlot);
    }

    menu.setObjectName("add_view_menu");
    menu.exec(QCursor::pos());
}

void ViewContainer::showViewMenuSlot()
{
    loginf << "ViewContainer: showViewMenuSlot: window " << window_cnt_;

    assert (!disable_add_remove_views_);

    QPushButton* button = dynamic_cast<QPushButton*>(sender());
    assert (button);

    QVariant instance_id_var = button->property("view_instance_id");
    assert (instance_id_var.isValid());

    string instance_id = instance_id_var.toString().toStdString();

    QMenu menu;

    QAction* delete_action = menu.addAction(tr("Close"));
    delete_action->setProperty("view_instance_id", instance_id.c_str());
    connect(delete_action, SIGNAL(triggered()), this, SLOT(deleteViewSlot()));

    menu.exec(QCursor::pos());
}

void ViewContainer::resetToStartupConfiguration()
{
    //remove views
    while (!views_.empty())
    {
        views_.rbegin()->get()->setTmpDisableRemoveConfigOnDelete(true);
        views_.pop_back();
    }

    //recreate views from config
    createSubConfigurables();
}

// void ViewContainerWidget::saveViewTemplate ()
//{
//    assert (last_active_manage_button_);

//    assert (view_manage_buttons_.find (last_active_manage_button_) != view_manage_buttons_.end());
//    View *view = view_manage_buttons_ [last_active_manage_button_];

//    bool ok;
//    QString text = QInputDialog::getText(this, tr("Save View Template"),
//                                         tr("Template Name"), QLineEdit::Normal,
//                                         view->getInstanceId().c_str(), &ok);
//    if (ok && !text.isEmpty())
//    {
//        loginf << "ViewContainerWidget: saveViewTemplate: for view " << view->getInstanceId() <<
//                " as template " << text.toStdString();
//        //TODO
//        //ViewManager::getInstance().saveViewAsTemplate (view, text.toStdString());
//    }

//    last_active_manage_button_=0;
//}
