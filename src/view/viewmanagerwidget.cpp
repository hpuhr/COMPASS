/*
 * ViewsWidget.cpp
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
//#include "DBResultSetManager.h"
#include "viewcontainerconfigwidget.h"

ViewManagerWidget::ViewManagerWidget(ViewManager &view_manager)
    : view_manager_(view_manager)
{
  logdbg  << "ViewsWidget: constructor: start";
  cont_layout_=0;

  QFont font_bold;
  font_bold.setBold(true);

  layout_ = new QVBoxLayout();
//  layout_->setSpacing( 0 );
//  layout_->setMargin( 0 );

  QLabel *head = new QLabel (tr("Views"));
  head->setFont (font_bold);
  layout_->addWidget(head);

  QPushButton *add = new QPushButton(tr("Add View"));
  connect(add, SIGNAL( clicked() ), this, SLOT( addViewSlot() ));
  layout_->addWidget(add);

  cont_layout_ = new QVBoxLayout ();
//  cont_layout_->setSpacing( 0 );
//  cont_layout_->setMargin( 0 );
  layout_->addLayout (cont_layout_);

  layout_->addStretch ();
  setLayout (layout_);

  //view_manager_.setViewsWidget(this);
  logdbg  << "ViewsWidget: constructor: end";
}

ViewManagerWidget::~ViewManagerWidget()
{
}

void ViewManagerWidget::addViewSlot()
{
  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  add_template_actions_.clear();

//  QMenu menu;
//  QMenu* submenu;
//  QString name;
//  unsigned int i, n = cont_widgets_.size();

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

//  // listbox view
//  submenu = menu.addMenu( "ListBox View" );
//  submenu->addAction( "New Window", this, SLOT(addListBoxViewNewWindowSlot()) );
//  for( i=0; i<n; ++i )
//  {
//    name = cont_widgets_[ i ]->name();
//    QAction* action = submenu->addAction( name, this, SLOT(addListBoxViewSlot()) );
//    action->setData( QVariant( i ) );
//  }

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

//  menu.exec( QCursor::pos() );
}

//void ViewsWidget::addGeographicViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithGeographicView();
//}

//void ViewsWidget::addHistogramViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithHistogramView();
//}

//void ViewsWidget::addListBoxViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithListBoxView();
//}


//void ViewsWidget::addMosaicViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithMosaicView();
//}

//void ViewsWidget::addScatterPlotViewNewWindowSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  ViewManager::getInstance().addContainerWithScatterPlotView();
//}

//void ViewsWidget::addGeographicViewSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  QAction* action = (QAction*)(QObject::sender());
//  unsigned int containter_id = action->data().toUInt();

//  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//    throw( std::runtime_error( "ViewsWidget :addGeographicViewSlot: container out of bounds" ) );
//  cont_widgets_[ containter_id ]->addGeographicView();
//}

//void ViewsWidget::addHistogramViewSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  QAction* action = (QAction*)(QObject::sender());
//  unsigned int containter_id = action->data().toUInt();

//  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//    throw( std::runtime_error( "ViewsWidget: addHistogramViewSlot: container out of bounds" ) );
//  cont_widgets_[ containter_id ]->addHistogramView();
//}

//void ViewsWidget::addListBoxViewSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  QAction* action = (QAction*)(QObject::sender());
//  unsigned int containter_id = action->data().toUInt();

//  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//    throw( std::runtime_error( "ViewsWidget: addListBoxViewSlot: container out of bounds" ) );
//  cont_widgets_[ containter_id ]->addListBoxView();
//}

//void ViewsWidget::addMosaicViewSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  QAction* action = (QAction*)(QObject::sender());
//  unsigned int containter_id = action->data().toUInt();

//  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//    throw( std::runtime_error( "ViewsWidget: addMosaicViewSlot: container out of bounds" ) );
//  cont_widgets_[ containter_id ]->addMosaicView();
//}

//void ViewsWidget::addScatterPlotViewSlot()
//{
//  //TODO
//  if( DBResultSetManager::getInstance().isCurrentlyLoadingData() )
//    return;

//  QAction* action = (QAction*)(QObject::sender());
//  unsigned int containter_id = action->data().toUInt();

//  if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//    throw( std::runtime_error( "ViewsWidget: addScatterPlotViewSlot: container out of bounds" ) );

//  cont_widgets_[ containter_id ]->addScatterPlotView();
//}

void ViewManagerWidget::update ()
{
  logdbg  << "ViewsWidget: update";

  // TODO
//  for (unsigned int cnt=0; cnt < cont_widgets_.size(); cnt++)
//    delete cont_widgets_.at(cnt);
//  cont_widgets_.clear();

//  std::map <std::string, ViewContainer*> containers = view_manager_.getContainers ();

//  //loginf  << "ViewsWidget: update size containers " << containers.size();

//  std::map<std::string, ViewContainerWidget*>::iterator it;
//  for (it = containers.begin(); it != containers.end(); it++)
//  {
//    ViewContainerConfigWidget* config_widget = new ViewContainerConfigWidget( it->second );
//    cont_widgets_.push_back( config_widget );
//    cont_layout_->addWidget( config_widget );
//  }
}

//void ViewManagerWidget::addTemplateSlot ()
//{
//    QAction *action = (QAction*) sender();

//    assert (add_template_actions_.find (action) != add_template_actions_.end());
//    std::pair <std::string, int> data = add_template_actions_ [action];

//    loginf << "ViewsWidget: addTemplateSlot: " << data.first << " in window " << data.second;
//    int containter_id = data.second;

//    if( containter_id < 0 || containter_id >= cont_widgets_.size() )
//      throw( std::runtime_error( "ViewsWidget: addTemplateSlot: container out of bounds" ) );

//    cont_widgets_[ containter_id ]->addTemplateView (data.first);

//}

//void ViewManagerWidget::addTemplateNewWindowSlot ()
//{
//    QAction *action = (QAction*) sender();
//    QVariant variant = action->data();
//    loginf << "ViewsWidget: addTemplateNewWindowSlot: " << variant.toString().toStdString();
//    ViewManager::getInstance().addContainerWithTemplateView(variant.toString().toStdString());
//}

