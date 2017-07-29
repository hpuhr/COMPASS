#include "viewcontainerwidget.h"
#include "viewcontainer.h"
#include "viewmanager.h"
#include "logger.h"
#include "stringconv.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QMoveEvent>
#include <QResizeEvent>

using namespace Utils;


ViewContainerWidget::ViewContainerWidget(const std::string &class_id, const std::string &instance_id, ViewManager *view_manager)
:   QWidget(nullptr), Configurable(class_id, instance_id, view_manager), view_manager_(*view_manager)
{
  logdbg  << "ViewContainerWidget: constructor: instance " << instance_id_;

  registerParameter ("pos_x", &pos_x_, 0);
  registerParameter ("pos_y", &pos_y_, 0);
  registerParameter ("width", &width_, 1000);
  registerParameter ("height", &height_, 700);
  registerParameter ("min_width", &min_width_, 1000);
  registerParameter ("min_height", &min_height_, 700);

  name_ = "Window"+String::intToString(String::getAppendedInt (instance_id_));

  QHBoxLayout *layout = new QHBoxLayout ();
  layout->setSpacing(0);
  layout->setMargin(0);

  tab_widget_ = new QTabWidget();
  assert (tab_widget_);
  layout->addWidget(tab_widget_);

  setLayout (layout);
  setMinimumSize(QSize(min_width_, min_height_));
  setGeometry(pos_x_, pos_y_, width_, height_);

  setAttribute( Qt::WA_DeleteOnClose, true );

  createSubConfigurables();

  show();

  logdbg  << "ViewContainerGUI: constructor: end";
}

ViewContainerWidget::~ViewContainerWidget()
{
    logdbg  << "ViewContainerWidget: destructor";

    assert (view_container_);
    delete view_container_;
    view_container_ = nullptr;
}

void ViewContainerWidget::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id.compare ("ViewContainer") == 0)
    {
        assert (tab_widget_);
        assert (!view_container_);
        view_container_ = new ViewContainer (class_id, instance_id, this, &view_manager_, tab_widget_);
        assert (view_container_);
    }
    else
        throw std::runtime_error ("ViewContainerWidget: generateSubConfigurable: unknown class_id "+class_id );
}

void ViewContainerWidget::checkSubConfigurables ()
{
    if (!view_container_)
    {
        generateSubConfigurable ("ViewContainer", instance_id_+"ViewContainer0");
        assert (view_container_);
    }
}

ViewContainer &ViewContainerWidget::viewContainer() const
{
    return *view_container_;
}

void ViewContainerWidget::closeEvent ( QCloseEvent * event )
{
  loginf  << "ViewContainerWidget: closeEvent: instance " << instance_id_;
  view_manager_.deleteContainerWidget (instance_id_ );
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



