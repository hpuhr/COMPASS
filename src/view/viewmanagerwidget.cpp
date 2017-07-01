/*
 * ViewManagerWidget.cpp
 *
 *  Created on: Apr 12, 2012
 *      Author: sk
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QVariant>

#include "viewmanagerwidget.h"
#include "viewmanager.h"
#include "logger.h"
#include "jobmanager.h"
#include "viewcontainer.h"
#include "viewcontainerconfigwidget.h"

ViewManagerWidget::ViewManagerWidget(ViewManager &view_manager)
    : view_manager_(view_manager), layout_(nullptr), cont_layout_(nullptr), add_button_(nullptr)
{
    logdbg  << "ViewManagerWidget: constructor: start";

    QFont font_bold;
    font_bold.setBold(true);

    layout_ = new QVBoxLayout();

    QLabel *head = new QLabel (tr("Views"));
    head->setFont (font_bold);
    layout_->addWidget(head);

    add_button_ = new QPushButton(tr("Add View"));
    connect(add_button_, SIGNAL( clicked() ), this, SLOT( addViewSlot() ));
    layout_->addWidget(add_button_);

    cont_layout_ = new QVBoxLayout ();
    cont_layout_->setSpacing (0);
    cont_layout_->setMargin (0);
    layout_->addLayout (cont_layout_);

    layout_->addStretch ();
    setLayout (layout_);

    connect (&JobManager::instance(), SIGNAL(databaseBusy()), this, SLOT(databaseBusy()));
    connect (&JobManager::instance(), SIGNAL(databaseIdle()), this, SLOT(databaseIdle()));

    //view_manager_.setViewManagerWidget(this);
    update ();

    logdbg  << "ViewManagerWidget: constructor: end";
}

ViewManagerWidget::~ViewManagerWidget()
{
}

void ViewManagerWidget::databaseBusy ()
{
    assert (add_button_);
    add_button_->setDisabled(true);
}

void ViewManagerWidget::databaseIdle ()
{
    assert (add_button_);
    add_button_->setDisabled(false);
}


void ViewManagerWidget::addViewSlot()
{
    add_template_actions_.clear();

    QMenu menu;
    QMenu* submenu;
    QString name;
    unsigned int i, n = cont_widgets_.size();

    //  //scatter plot view
    //  submenu = menu.addMenu( "Geographic View" );
    //  submenu->addAction( "New Window", this, SLOT(addGeographicViewNewWindowSlot()) );
    //  for( i=0; i<n; ++i )
    //  {
    //    name = cont_widgets_[ i ]->name();
    //    QAction* action = submenu->addAction( name, this, SLOT(addGeographicViewSlot()) );
    //    action->setData( QVariant( i ) );
    //  }

    //  //scatter plot 2d view
    //  submenu = menu.addMenu( "ScatterPlot View" );
    //  submenu->addAction( "New Window", this, SLOT(addScatterPlotViewNewWindowSlot()) );
    //  for( i=0; i<n; ++i )
    //  {
    //    name = cont_widgets_[ i ]->name();
    //    QAction* action = submenu->addAction( name, this, SLOT(addScatterPlotViewSlot()) );
    //    action->setData( QVariant( i ) );
    //  }

    //  //histogram view
    //  submenu = menu.addMenu( "Histogram View" );
    //  submenu->addAction( "New Window", this, SLOT(addHistogramViewNewWindowSlot()) );
    //  for( i=0; i<n; ++i )
    //  {
    //    name = cont_widgets_[ i ]->name();
    //    QAction* action = submenu->addAction( name, this, SLOT(addHistogramViewSlot()) );
    //    action->setData( QVariant( i ) );
    //  }

    // listbox view
    submenu = menu.addMenu( "ListBox View" );
    //  submenu->addAction( "New Window", this, SLOT(addListBoxViewNewWindowSlot()) );
    for( i=0; i<n; ++i )
    {
        name = cont_widgets_[ i ]->name();
        QAction* action = submenu->addAction( name, this, SLOT(addListBoxViewSlot()) );
        action->setData( QVariant( i ) );
    }

    //  //mosaic view
    //  /*submenu = menu.addMenu( "Mosaic View" );
    //  submenu->addAction( "New Window", this, SLOT(addMosaicViewNewWindowSlot()) );
    //  for( i=0; i<n; ++i )
    //  {
    //    name = cont_widgets_[ i ]->name();
    //    QAction* action = submenu->addAction( name, this, SLOT(addMosaicViewSlot()) );
    //    action->setData( QVariant( i ) );
    //  }*/

    //  std::map<std::string, Configuration> &templates = ViewManager::getInstance().getConfiguration()
    //          .getConfigurationTemplates ();

    //  if (templates.size() > 0)
    //  {
    //      QMenu *templatesubmenu = menu.addMenu ("Templates");

    //      std::map<std::string, Configuration>::const_iterator it;
    //      for (it = templates.begin(); it != templates.end(); it++)
    //      {
    //          submenu = templatesubmenu->addMenu( it->first.c_str());

    //          QAction* newaction = submenu->addAction( "New Window", this, SLOT(addTemplateNewWindowSlot()) );
    //          newaction->setData( QVariant( tr(it->first.c_str()) ) );

    //          for( i=0; i<n; ++i )
    //          {
    //            name = cont_widgets_[ i ]->name();
    //            QAction* action = submenu->addAction( name, this, SLOT(addTemplateSlot()) );

    //            assert (add_template_actions_.find (action) == add_template_actions_.end());
    //            add_template_actions_ [action] = std::pair <std::string, int> (it->first.c_str(), i);
    //          }
    //      }
    //  }

    menu.exec(QCursor::pos());
}

//void ViewManagerWidget::addGeographicViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithGeographicView();
//}

//void ViewManagerWidget::addHistogramViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithHistogramView();
//}

//void ViewManagerWidget::addListBoxViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithListBoxView();
//}


//void ViewManagerWidget::addMosaicViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithMosaicView();
//}

//void ViewManagerWidget::addScatterPlotViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithScatterPlotView();
//}

//void ViewManagerWidget::addGeographicViewSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  QAction* action = (QAction*)(QObject::sender());
//  unsigned int containter_id = action->data().toUInt();

//  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//    throw( std::runtime_error( "ViewManagerWidget :addGeographicViewSlot: container out of bounds" ) );
//  cont_widgets_[ containter_id ]->addGeographicView();
//}

//void ViewManagerWidget::addHistogramViewSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  QAction* action = (QAction*)(QObject::sender());
//  unsigned int containter_id = action->data().toUInt();

//  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//    throw( std::runtime_error( "ViewManagerWidget: addHistogramViewSlot: container out of bounds" ) );
//  cont_widgets_[ containter_id ]->addHistogramView();
//}

void ViewManagerWidget::addListBoxViewSlot()
{
  QAction* action = (QAction*)(QObject::sender());
  unsigned int containter_id = action->data().toUInt();

  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
    throw( std::runtime_error( "ViewManagerWidget: addListBoxViewSlot: container out of bounds" ) );
  cont_widgets_[ containter_id ]->addListBoxView();
}

//void ViewManagerWidget::addMosaicViewSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  QAction* action = (QAction*)(QObject::sender());
//  unsigned int containter_id = action->data().toUInt();

//  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//    throw( std::runtime_error( "ViewManagerWidget: addMosaicViewSlot: container out of bounds" ) );
//  cont_widgets_[ containter_id ]->addMosaicView();
//}

//void ViewManagerWidget::addScatterPlotViewSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  QAction* action = (QAction*)(QObject::sender());
//  unsigned int containter_id = action->data().toUInt();

//  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//    throw( std::runtime_error( "ViewManagerWidget: addScatterPlotViewSlot: container out of bounds" ) );

//  cont_widgets_[ containter_id ]->addScatterPlotView();
//}

void ViewManagerWidget::update ()
{
    logdbg  << "ViewManagerWidget: update";

    cont_widgets_.clear();

    QLayoutItem *child;
    while ((child = cont_layout_->takeAt(0)) != 0)
    {
        cont_layout_->removeItem(child);
    }

    std::map <std::string, ViewContainer*> containers = view_manager_.getContainers ();

    //loginf  << "ViewManagerWidget: update size containers " << containers.size();

    std::map<std::string, ViewContainer*>::iterator it;
    for (it = containers.begin(); it != containers.end(); it++)
    {
        cont_widgets_.push_back (it->second->configWidget());
        cont_layout_->addWidget (it->second->configWidget());
    }
}

//void ViewManagerWidget::addTemplateSlot ()
//{
//    QAction *action = (QAction*) sender();

//    assert (add_template_actions_.find (action) != add_template_actions_.end());
//    std::pair <std::string, int> data = add_template_actions_ [action];

//    loginf << "ViewManagerWidget: addTemplateSlot: " << data.first << " in window " << data.second;
//    int containter_id = data.second;

//    if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//      throw( std::runtime_error( "ViewManagerWidget: addTemplateSlot: container out of bounds" ) );

//    cont_widgets_[ containter_id ]->addTemplateView (data.first);

//}

//void ViewManagerWidget::addTemplateNewWindowSlot ()
//{
//    QAction *action = (QAction*) sender();
//    QVariant variant = action->data();
//    loginf << "ViewManagerWidget: addTemplateNewWindowSlot: " << variant.toString().toStdString();
//    ViewManager::getInstance().addContainerWithTemplateView(variant.toString().toStdString());
//}

