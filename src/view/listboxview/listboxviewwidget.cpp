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

/*
 */
ListBoxViewWidget::ListBoxViewWidget( const std::string& class_id, const std::string& instance_id, Configurable* config_parent, ListBoxView* view, QWidget* parent )
    :   ViewWidget( class_id, instance_id, config_parent, view, parent ), data_widget_(nullptr), config_widget_(nullptr)
{
    setAutoFillBackground(true);

    QHBoxLayout *hlayout = new QHBoxLayout;

    data_widget_ = new ListBoxViewDataWidget (view->getDataSource());
    data_widget_->setAutoFillBackground(true);
    QSizePolicy sp_left(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp_left.setHorizontalStretch(3);
    data_widget_->setSizePolicy(sp_left);
    hlayout->addWidget( data_widget_ );


    config_widget_ = new ListBoxViewConfigWidget (getView());
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
    assert (config_widget_);
    bool vis = config_widget_->isVisible();
    config_widget_->setVisible( !vis );
}

/*
 */
ListBoxViewConfigWidget* ListBoxViewWidget::configWidget()
{
    return config_widget_;
}

