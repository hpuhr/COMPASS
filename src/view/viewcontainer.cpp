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

#include "config.h"
#include "files.h"
#include "global.h"
#include "listboxview.h"
#include "logger.h"
#include "managementwidget.h"
#include "stringconv.h"
#include "view.h"
#include "viewcontainer.h"
#include "viewcontainerconfigwidget.h"
#include "viewmanager.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "osgview.h"
#endif

#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QPushButton>
#include <QTabBar>
#include <QTabWidget>

unsigned int ViewContainer::view_count_ = 0;

using namespace Utils;

ViewContainer::ViewContainer(const std::string& class_id, const std::string& instance_id,
                             Configurable* parent, ViewManager* view_manager,
                             QTabWidget* tab_widget, int window_cnt)
    : QObject(),
      Configurable(class_id, instance_id, parent),
      view_manager_(*view_manager),
      tab_widget_(tab_widget),
      window_cnt_(window_cnt)
{
    logdbg << "ViewContainer: constructor: creating gui elements";
    assert(tab_widget_);

    //    QAction *template_action = menu_.addAction(tr("Save As Template"));
    //    connect(template_action, SIGNAL(triggered()), this, SLOT(saveViewTemplate()));

    QAction* delete_action = menu_.addAction(tr("Close"));
    connect(delete_action, SIGNAL(triggered()), this, SLOT(deleteView()));

    createSubConfigurables();
}

ViewContainer::~ViewContainer()
{
    view_manager_.removeContainer(instanceId());

    if (config_widget_)
    {
        delete config_widget_;
        config_widget_ = nullptr;
    }

    for (auto view : views_)
    {
        delete view;
    }
    views_.clear();
}

void ViewContainer::addView(const std::string& class_name)
{
    generateSubConfigurable(class_name, class_name + std::to_string(view_count_));

    if (config_widget_)
        config_widget_->updateSlot();
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

    views_.push_back(view);
    int index = tab_widget_->addTab(w, QString::fromStdString(view->getName()));

    QPushButton* manage_button = new QPushButton();
    manage_button->setIcon(QIcon(Files::getIconFilepath("edit.png").c_str()));
    manage_button->setFixedSize(UI_ICON_SIZE);
    manage_button->setFlat(UI_ICON_BUTTON_FLAT);
    manage_button->setToolTip(tr("Manage view"));
    connect(manage_button, SIGNAL(clicked()), this, SLOT(showMenuSlot()));
    tab_widget_->tabBar()->setTabButton(index, QTabBar::RightSide, manage_button);

    assert(view_manage_buttons_.find(manage_button) == view_manage_buttons_.end());
    view_manage_buttons_[manage_button] = view;
    loginf << "ViewContainer: addView: view " << view->getName() << " added";
}

void ViewContainer::removeView(View* view)
{
    assert(view);
    QWidget* w = view->getCentralWidget();
    assert(w);

    int id = tab_widget_->indexOf(view->getCentralWidget());
    std::vector<View*>::iterator it = std::find(views_.begin(), views_.end(), view);

    if (id != -1)
        tab_widget_->removeTab(id);

    if (it != views_.end())
        views_.erase(it);

    bool found = false;
    std::map<QPushButton*, View*>::iterator it2;
    for (it2 = view_manage_buttons_.begin(); it2 != view_manage_buttons_.end(); it2++)
    {
        if (it2->second == view)
        {
            found = true;
            view_manage_buttons_.erase(it2);
            break;
        }
    }
    assert(found);

    return;
}

void ViewContainer::deleteView()
{
    assert(last_active_manage_button_);

    assert(view_manage_buttons_.find(last_active_manage_button_) != view_manage_buttons_.end());
    View* view = view_manage_buttons_[last_active_manage_button_];

    loginf << "ViewContainer: deleteView: for view " << view->instanceId();
    delete view;

    last_active_manage_button_ = nullptr;

    if (config_widget_)
        config_widget_->updateSlot();
}

const std::vector<View*>& ViewContainer::getViews() const { return views_; }

void ViewContainer::generateSubConfigurable(const std::string& class_id,
                                            const std::string& instance_id)
{
    if (class_id.compare("ListBoxView") == 0)
    {
        ListBoxView* view = new ListBoxView(class_id, instance_id, this, view_manager_);
        unsigned int number = String::getAppendedInt(instance_id);

        if (number >= view_count_)
            view_count_ = number + 1;

        assert(view);
        view->init();
    }
#if USE_EXPERIMENTAL_SOURCE == true
    else if (class_id.compare("OSGView") == 0)
    {
        OSGView* view = new OSGView(class_id, instance_id, this, view_manager_);
        unsigned int number = String::getAppendedInt(instance_id);
        if (number >= view_count_)
            view_count_ = number + 1;

        assert(view);
        view->init();
    }
#endif
    //  else if (class_id.compare ("HistogramView") == 0)
    //  {
    //    HistogramView* view = new HistogramView ( class_id, instance_id, this );
    //    unsigned int number = getAppendedInt (instance_id);
    //    if (number >= view_count_)
    //      view_count_ = number+1;

    //    assert( view );
    //    view->init();
    //  }
    //  else if (class_id.compare ("ScatterPlotView") == 0)
    //  {
    //    ScatterPlotView* view = new ScatterPlotView ( class_id, instance_id, this );
    //    unsigned int number = getAppendedInt (instance_id);
    //    if (number >= view_count_)
    //      view_count_ = number+1;

    //    assert( view );
    //    view->init();
    //  }
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

ViewContainerConfigWidget* ViewContainer::configWidget()
{
    if (!config_widget_)
    {
        config_widget_ = new ViewContainerConfigWidget(this);
    }

    assert(config_widget_);
    return config_widget_;
}

void ViewContainer::showMenuSlot()
{
    last_active_manage_button_ = (QPushButton*)sender();
    menu_.exec(QCursor::pos());
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
