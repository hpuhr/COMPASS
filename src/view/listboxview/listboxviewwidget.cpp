/*
 * ListBoxViewWidget.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */


#include <QTabWidget>
#include <QHBoxLayout>


#include "listboxview.h"
#include "listboxviewdatawidget.h"
#include "listboxviewwidget.h"
#include "listboxviewconfigwidget.h"
//#include "config.h"

/*
 */
ListBoxViewWidget::ListBoxViewWidget( const std::string& class_id, const std::string& instance_id, Configurable* config_parent, ListBoxView* view, QWidget* parent )
:   ViewWidget( class_id, instance_id, config_parent, view, parent ), data_widget_(nullptr), config_widget_(nullptr)
{
  QHBoxLayout *hlayout = new QHBoxLayout;

  data_widget_ = new ListBoxViewDataWidget (view->getDataSource());
  hlayout->addWidget( data_widget_ );

  //unsigned int tab_config_width;
  //Config::getInstance().getValue( "tab_config_width", &tab_config_width );

  tab_widget_ = new QTabWidget( );
  //tab_widget_->setFixedWidth ( tab_config_width );
  //tab_widget_->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );

  config_widget_ = new ListBoxViewConfigWidget( getView(), tab_widget_ );

  tab_widget_->addTab ( config_widget_ , tr("Config"));

  hlayout->addWidget( tab_widget_ );

  setLayout( hlayout );

  setFocusPolicy( Qt::StrongFocus );

  //connect stuff here
  //connect( config_widget_, SIGNAL(variableChanged()), this, SLOT(variableChangedSlot()) );
}

/*
 */
ListBoxViewWidget::~ListBoxViewWidget()
{
}

/*
 */
void ListBoxViewWidget::updateView()
{
}

/*
 */
void ListBoxViewWidget::toggleConfigWidget()
{
  if( !tab_widget_ )
    return;
  bool vis = tab_widget_->isVisible();
  tab_widget_->setVisible( !vis );
}

/*
 */
ListBoxViewConfigWidget* ListBoxViewWidget::configWidget()
{
  return config_widget_;
}

