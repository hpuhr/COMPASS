/*
 * ViewContainerGUI.cpp
 *
 *  Created on: Jan 15, 2012
 *      Author: sk
 */


//#include <QTabWidget>
#include <QHBoxLayout>
#include <QMoveEvent>
#include <QPushButton>
#include <QMenu>
#include <QTabBar>
#include <QInputDialog>

#include "viewcontainerwidget.h"
#include "viewcontainertabwidget.h"
#include "config.h"
#include "logger.h"
//#include "GeographicView.h"
#include "managementwidget.h"
#include "viewmanager.h"
//#include "HistogramView.h"
//#include "ScatterPlotView.h"
//#include "MosaicView.h"
//#include "ListBoxView.h"
//#include "view.h"
//#include "DBViewModel.h"
#include "stringconv.h"

using namespace Utils;

unsigned int ViewContainerWidget::view_count_=0;


ViewContainerWidget::ViewContainerWidget(std::string class_id, std::string instance_id, Configurable *parent)
:   QWidget( 0 ), Configurable( class_id, instance_id, parent ), tab_widget_( NULL ),
    last_active_manage_button_ (0)
{
  logdbg  << "ViewContainerWidget: constructor: instance " << instance_id_;

  setAttribute( Qt::WA_DeleteOnClose, true );

  registerParameter ("seperate_window", &seperate_window_, true);

  manager_=0;

  if (seperate_window_)
  {
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
  }
  else
  {
    target_ = ViewManager::getInstance().getCentralWidget();
  }

  logdbg  << "ViewContainerWidget: constructor: creating gui elements";
  createGUIElements();
  createSubConfigurables ();

  logdbg  << "ViewContainerGUI: constructor: end";
}

ViewContainerWidget::~ViewContainerWidget()
{
    logdbg  << "ViewContainerWidget: destructor";

    while( !views_.empty() )
    {
        View* view = views_.back();
        delete view;
    }
}

void ViewContainerWidget::createGUIElements ()
{
  logdbg  << "ViewContainerWidget: createGUIElements: start";
  assert (target_);

  target_layout_ = new QHBoxLayout(target_);
  target_layout_->setSpacing(1);
  target_layout_->setMargin(1);

  logdbg  << "ViewContainerWidget: createGUIElements: creating widgets and layouts";

  tab_widget_ = new ViewContainerTabWidget( target_ );
  //tab_widget_->setTabsClosable (true);

  logdbg  << "ViewContainerWidget: createGUIElements: adding widget to layout";
  target_layout_->addWidget(tab_widget_);

  if (!seperate_window_)
  {
    logdbg  << "ViewContainerWidget: createGUIElements: adding management tab";
    manager_ = new ManagementWidget ();
    tab_widget_->addTab (manager_, tr("Management"));
  }

  QAction *template_action = menu_.addAction(tr("Save As Template"));
  connect(template_action, SIGNAL(triggered()), this, SLOT(saveViewTemplate()));

  QAction *delete_action = menu_.addAction(tr("Close"));
  connect(delete_action, SIGNAL(triggered()), this, SLOT(deleteView()));
}

void ViewContainerWidget::addGeographicView()
{
  generateSubConfigurable ("GeographicView", "GeographicView"+intToString(view_count_));
}

void ViewContainerWidget::addHistogramView()
{
  generateSubConfigurable ("HistogramView", "HistogramView"+intToString(view_count_));
}

void ViewContainerWidget::addListBoxView()
{
  generateSubConfigurable ("ListBoxView", "ListBoxView"+intToString(view_count_));
}

void ViewContainerWidget::addMosaicView()
{
  generateSubConfigurable ("MosaicView", "MosaicView"+intToString(view_count_));
}

void ViewContainerWidget::addScatterPlotView()
{
  generateSubConfigurable ("ScatterPlotView", "ScatterPlotView"+intToString(view_count_));
}

void ViewContainerWidget::addTemplateView (std::string template_name)
{
    std::string view_name = template_name+intToString(ViewContainerWidget::getViewCount());
    std::map<std::string, Configuration> &templates = ViewManager::getInstance().getConfiguration()
            .getConfigurationTemplates ();
    assert (templates.find (template_name) != templates.end());
    Configuration view_config = templates [template_name];
    view_config.setInstanceId(view_name);
    view_config.setTemplate(false, "");

    configuration_.addNewSubConfiguration(view_config);

    generateSubConfigurable (view_config.getClassId(), view_config.getInstanceId());
}

ViewContainerTabWidget *ViewContainerWidget::getTabWidget ()
{
  assert(tab_widget_);
  return tab_widget_;
}

void ViewContainerWidget::closeEvent ( QCloseEvent * event )
{
  loginf  << "ViewContainerWidget: closeEvent: instance " << instance_id_;
  ViewManager::getInstance().removeContainer( instance_id_ );
  QWidget::closeEvent(event);
}

void ViewContainerWidget::addView (View *view)
{
  assert (view);
  QWidget* w = view->getCentralWidget();
  assert ( w );

  views_.push_back( view );
  int index = tab_widget_->addTab( w, QString::fromStdString( view->getName() ) );

  QPushButton *manage_button = new QPushButton();
  manage_button->setIcon( QIcon( "./Data/icons/gear.png" ) );
  manage_button->setFixedSize ( 20, 20 );
  manage_button->setFlat(true);
  manage_button->setToolTip(tr("Manage view"));
  connect (manage_button, SIGNAL(clicked()), this, SLOT(showMenuSlot()));
  tab_widget_->getTabBar()->setTabButton(index, QTabBar::RightSide, manage_button);

  assert (view_manage_buttons_.find (manage_button) == view_manage_buttons_.end());
  view_manage_buttons_ [manage_button] = view;
}

void ViewContainerWidget::removeView (View *view)
{
  assert (view);
  QWidget* w = view->getCentralWidget();
  assert ( w );

  int id = tab_widget_->indexOf( view->getCentralWidget() );
  std::vector<View*>::iterator it = std::find( views_.begin(), views_.end(), view );

  if( id != -1)
    tab_widget_->removeTab( id );

  if ( it != views_.end() )
    views_.erase( it );

  bool found=false;
  std::map <QPushButton*, View*>::iterator it2;
  for (it2 = view_manage_buttons_.begin(); it2 != view_manage_buttons_.end(); it2++)
  {
      if (it2->second == view)
      {
          found=true;
          view_manage_buttons_.erase(it2);
          break;
      }
  }
  assert (found);

  return;
}

const std::vector<View*>& ViewContainerWidget::getViews() const
{
  return views_;
}

void ViewContainerWidget::generateSubConfigurable (std::string class_id, std::string instance_id)
{
  if (class_id.compare ("GeographicView") == 0)
  {
    GeographicView* view = new GeographicView ( class_id, instance_id, this );
    unsigned int number = getAppendedInt (instance_id);
    if (number >= view_count_)
      view_count_ = number+1;

    assert( view );
    view->init();
  }
  else if (class_id.compare ("HistogramView") == 0)
  {
    HistogramView* view = new HistogramView ( class_id, instance_id, this );
    unsigned int number = getAppendedInt (instance_id);
    if (number >= view_count_)
      view_count_ = number+1;

    assert( view );
    view->init();
  }
  else if (class_id.compare ("ListBoxView") == 0)
  {
    ListBoxView* view = new ListBoxView ( class_id, instance_id, this );
    unsigned int number = getAppendedInt (instance_id);
    if (number >= view_count_)
      view_count_ = number+1;

    assert( view );
    view->init();
  }
  else if (class_id.compare ("ScatterPlotView") == 0)
  {
    ScatterPlotView* view = new ScatterPlotView ( class_id, instance_id, this );
    unsigned int number = getAppendedInt (instance_id);
    if (number >= view_count_)
      view_count_ = number+1;

    assert( view );
    view->init();
  }
  else if (class_id.compare ("MosaicView") == 0)
  {
    MosaicView* view = new MosaicView ( class_id, instance_id, this );
    unsigned int number = getAppendedInt (instance_id);
    if (number >= view_count_)
      view_count_ = number+1;

    assert( view );
    view->init();
  }
  else
    throw std::runtime_error ("ViewContainerWidget: generateSubConfigurable: unknown class_id "+class_id );
}

void ViewContainerWidget::checkSubConfigurables ()
{
    // move along sir
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

std::string ViewContainerWidget::getName ()
{
  return "Window"+intToString(getAppendedInt (instance_id_));
}

void ViewContainerWidget::showMenuSlot ()
{
    last_active_manage_button_ = (QPushButton*)sender ();
    menu_.exec( QCursor::pos() );
}

void ViewContainerWidget::saveViewTemplate ()
{
    assert (last_active_manage_button_);

    assert (view_manage_buttons_.find (last_active_manage_button_) != view_manage_buttons_.end());
    View *view = view_manage_buttons_ [last_active_manage_button_];

    bool ok;
    QString text = QInputDialog::getText(this, tr("Save View Template"),
                                         tr("Template Name"), QLineEdit::Normal,
                                         view->getInstanceId().c_str(), &ok);
    if (ok && !text.isEmpty())
    {
        loginf << "ViewContainerWidget: saveViewTemplate: for view " << view->getInstanceId() <<
                " as template " << text.toStdString();
        ViewManager::getInstance().saveViewAsTemplate (view, text.toStdString());
    }

    last_active_manage_button_=0;
}
void ViewContainerWidget::deleteView ()
{
    assert (last_active_manage_button_);

    assert (view_manage_buttons_.find (last_active_manage_button_) != view_manage_buttons_.end());
    View *view = view_manage_buttons_ [last_active_manage_button_];

    loginf << "ViewContainerWidget: saveViewTemplate: for view " << view->getInstanceId();
    ViewManager::getInstance().viewShutdown (view);

    last_active_manage_button_=0;
}

