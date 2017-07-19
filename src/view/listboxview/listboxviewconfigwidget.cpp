/*
 * ListBoxViewConfigWidget.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QLabel>

#include "dbobjectmanager.h"
#include "dbovariableorderedsetwidget.h"
#include "listboxview.h"
#include "listboxviewconfigwidget.h"
#include "listboxviewdatasource.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

ListBoxViewConfigWidget::ListBoxViewConfigWidget( ListBoxView* view, QWidget* parent )
    :   QWidget( parent ), view_( view ), variable_set_widget_ (nullptr), presentation_check_ (nullptr), overwrite_check_(nullptr)
{
    QVBoxLayout *vlayout = new QVBoxLayout;

    assert (view_);

    variable_set_widget_ = view_->getDataSource()->getSet()->widget();
    vlayout->addWidget (variable_set_widget_);

    presentation_check_ = new QCheckBox("Use Presentation");
    presentation_check_->setChecked(view_->getDataSource()->usePresentation());
    //connect(use_selection, SIGNAL( clicked() ), this, SLOT( toggleUseSelection() ));
    vlayout->addWidget(presentation_check_);


//    QCheckBox *db_view = new QCheckBox("Database view");
//    db_view->setChecked(view_->getDataSource()->getDatabaseView());
//    connect(db_view, SIGNAL( clicked() ), this, SLOT( toggleDatabaseView() ));
//    vlayout->addWidget(db_view);

    vlayout->addStretch();

    QPushButton *export_button = new QPushButton ("Export");
    connect(export_button, SIGNAL(clicked(bool)), this, SLOT(exportSlot()));
    vlayout->addWidget(export_button);

    setLayout( vlayout );
}

ListBoxViewConfigWidget::~ListBoxViewConfigWidget()
{
}

//void ListBoxViewConfigWidget::toggleUseSelection()
//{
//  QCheckBox *send = ((QCheckBox*)sender());
//  bool checked = send->checkState() == Qt::Checked;
//  logdbg  << "ListBoxViewConfigWidget: toggleUseSelection: setting use filters to " << checked;
//  view_->getDataSource()->setUseSelection (checked);
//}

//void ListBoxViewConfigWidget::toggleDatabaseView ()
//{
//    QCheckBox *send = ((QCheckBox*)sender());
//    bool checked = send->checkState() == Qt::Checked;
//    logdbg  << "ListBoxViewConfigWidget: toggleDatabaseView: setting database view to " << checked;
//    view_->getDataSource()->setDatabaseView (checked);

//}

void ListBoxViewConfigWidget::exportSlot()
{
    logdbg << "ListBoxViewConfigWidget: exportSlot";
    emit exportSignal();
}
