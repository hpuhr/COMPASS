/*
 * OSGViewWidget.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */


#include <QTabWidget>
#include <QHBoxLayout>


#include "osgview.h"
#include "osgviewdatawidget.h"
#include "osgviewwidget.h"
#include "osgviewconfigwidget.h"

/*
 */
OSGViewWidget::OSGViewWidget( const std::string& class_id, const std::string& instance_id, Configurable* config_parent, OSGView* view, QWidget* parent )
    :   ViewWidget( class_id, instance_id, config_parent, view, parent ), data_widget_(nullptr), config_widget_(nullptr)
{
    setAutoFillBackground(true);

    QHBoxLayout *hlayout = new QHBoxLayout;

    data_widget_ = new OSGViewDataWidget (view->getDataSource(), 1, 1);
    data_widget_->setAutoFillBackground(true);
    QSizePolicy sp_left(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp_left.setHorizontalStretch(3);
    data_widget_->setSizePolicy(sp_left);
    hlayout->addWidget( data_widget_ );


    config_widget_ = new OSGViewConfigWidget (getView());
    config_widget_->setAutoFillBackground(true);
    QSizePolicy sp_right(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp_right.setHorizontalStretch(1);
    config_widget_->setSizePolicy(sp_right);

    hlayout->addWidget( config_widget_ );

    setLayout( hlayout );

    setFocusPolicy( Qt::StrongFocus );

    //connect stuff here
    //connect( config_widget_, SIGNAL(variableChanged()), this, SLOT(variableChangedSlot()) );
}

/*
 */
OSGViewWidget::~OSGViewWidget()
{
}

/*
 */
void OSGViewWidget::updateView()
{
}

/*
 */
void OSGViewWidget::toggleConfigWidget()
{
    assert (config_widget_);
    bool vis = config_widget_->isVisible();
    config_widget_->setVisible( !vis );
}

/*
 */
OSGViewConfigWidget* OSGViewWidget::configWidget()
{
    return config_widget_;
}

