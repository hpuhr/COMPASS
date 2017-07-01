/*
 * ViewContainerGUI.cpp
 *
 *  Created on: Jan 15, 2012
 *      Author: sk
 */


#include "viewcontainerwidget.h"
#include "logger.h"

using namespace Utils;


ViewContainerWidget::ViewContainerWidget(const std::string &class_id, const std::string &instance_id, ViewManager *parent)
:   QWidget( 0 ), ViewContainer (class_id, instance_id, parent)
{
  logdbg  << "ViewContainerWidget: constructor: instance " << instance_id_;

  setAttribute( Qt::WA_DeleteOnClose, true );

  QHBoxLayout *layout = new QHBoxLayout ();
  layout->setSpacing(0);
  layout->setMargin(0);


  target_ = new QWidget();
  layout->addWidget(target_);
  registerParameter ("pos_x", &pos_x_, 0);
  registerParameter ("pos_y", &pos_y_, 0);
  registerParameter ("width", &width_, 1000);
  registerParameter ("height", &height_, 700);
  registerParameter ("min_width", &min_width_, 1000);
  registerParameter ("min_height", &min_height_, 700);

  setLayout (layout);
  setMinimumSize(QSize(min_width_, min_height_));
  setGeometry(pos_x_, pos_y_, width_, height_);

  show();

  logdbg  << "ViewContainerGUI: constructor: end";
}

ViewContainerWidget::~ViewContainerWidget()
{
    logdbg  << "ViewContainerWidget: destructor";

    //TODO
//    while( !views_.empty() )
//    {
//        View* view = views_.back();
//        delete view;
//    }
}

std::string ViewContainerWidget::getName ()
{
  return "Window"+String::intToString(String::getAppendedInt (instance_id_));
}


void ViewContainerWidget::closeEvent ( QCloseEvent * event )
{
  loginf  << "ViewContainerWidget: closeEvent: instance " << instance_id_;
  view_manager_.removeContainer( instance_id_ );
  QWidget::closeEvent(event);
}

void ViewContainerWidget::moveEvent (QMoveEvent *event)
{
  logdbg  << "ViewContainerWidget " << instance_id_ << ": moveEvent";
  pos_x_ = event->pos().x();
  pos_y_ = event->pos().y();
}

void ViewContainerWidget::resizeEvent (QResizeEvent *event)
{
  logdbg  << "ViewContainerWidget " << instance_id_ << ": resizeEvent";
  width_ = event->size().width();
  height_ = event->size().height();
}



