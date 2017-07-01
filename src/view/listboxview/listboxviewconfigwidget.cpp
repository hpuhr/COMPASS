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

#include "DBObjectManager.h"
#include "DBOVariableOrderedSetWidget.h"
#include "DBOVariableSelectionWidget.h"
#include "ListBoxView.h"
#include "ListBoxViewConfigWidget.h"
#include "ListBoxViewDataSource.h"
#include "Logger.h"
#include "String.h"

using namespace Utils::String;

ListBoxViewConfigWidget::ListBoxViewConfigWidget( ListBoxView* view, QWidget* parent )
:   QWidget( parent ), view_( view ), variable_set_widget_ (0), order_variable_widget_(0), limit_min_edit_ (0), limit_max_edit_(0)
{
  createElements ();
}

ListBoxViewConfigWidget::~ListBoxViewConfigWidget()
{
}

void ListBoxViewConfigWidget::createElements ()
{
  QVBoxLayout *vlayout = new QVBoxLayout;

  assert (view_);

  variable_set_widget_ = new DBOVariableOrderedSetWidget (view_->getDataSource()->getSet());
  connect( variable_set_widget_, SIGNAL(setChanged()), this, SLOT(variableSetChanged()) );
  vlayout->addWidget (variable_set_widget_);

  QCheckBox *use_filters = new QCheckBox("Use filters");
  use_filters->setChecked(view_->getDataSource()->getUseFilters());
  connect(use_filters, SIGNAL( clicked() ), this, SLOT( toggleUseFilters() ));
  vlayout->addWidget(use_filters);

  QCheckBox *use_order = new QCheckBox("Use order");
  use_order->setChecked(view_->getDataSource()->getUseOrder());
  connect(use_order, SIGNAL( clicked() ), this, SLOT( toggleUseOrder() ));
  vlayout->addWidget(use_order);

  QFrame *order_frame = new QFrame ();
  order_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
  QVBoxLayout *order_layout = new QVBoxLayout ();

//  QLabel *order_label = new QLabel ("Order");
//  order_layout->addWidget(order_label);

  QCheckBox *order_ascending = new QCheckBox("Ascending");
  order_ascending->setChecked(view_->getDataSource()->getOrderAscending());
  connect(order_ascending, SIGNAL( clicked() ), this, SLOT( toggleOrderAscending() ));
  order_layout->addWidget(order_ascending);

  order_variable_widget_ = new DBOVariableSelectionWidget ();
  if (DBObjectManager::getInstance().existsDBOVariable(view_->getDataSource()->getOrderVariableType(),
      view_->getDataSource()->getOrderVariableName()))
  {
    order_variable_widget_->setSelectedVariable(DBObjectManager::getInstance().getDBOVariable(view_->getDataSource()->getOrderVariableType(),
        view_->getDataSource()->getOrderVariableName()));
  }
  connect (order_variable_widget_, SIGNAL (selectionChanged()), this, SLOT(orderVariableChanged()));
  order_layout->addWidget (order_variable_widget_);

  order_frame->setLayout (order_layout);
  vlayout->addWidget (order_frame);

  QHBoxLayout *limit_labels_layout = new QHBoxLayout ();

  QLabel *limit_min_label = new QLabel ("Limit: Starting position");
  limit_labels_layout->addWidget(limit_min_label);

  QLabel *limit_max_label = new QLabel ("Row number");
  limit_labels_layout->addWidget(limit_max_label);

  vlayout->addLayout(limit_labels_layout);

  QHBoxLayout *limit_edits_layout = new QHBoxLayout ();

  limit_min_edit_ = new QLineEdit ();
  limit_min_edit_->setText (intToString(view_->getDataSource()->getLimitMin()).c_str());
  connect( limit_min_edit_, SIGNAL(returnPressed()), this, SLOT(limitMinChanged()) );
  limit_edits_layout->addWidget(limit_min_edit_);

  limit_max_edit_ = new QLineEdit ();
  limit_max_edit_->setText (intToString(view_->getDataSource()->getLimitMax()).c_str());
  connect( limit_max_edit_, SIGNAL(returnPressed()), this, SLOT(limitMaxChanged()) );
  limit_edits_layout->addWidget(limit_max_edit_);

  vlayout->addLayout(limit_edits_layout);

  QCheckBox *use_selection = new QCheckBox("Use Selection");
  use_selection->setChecked(view_->getDataSource()->getUseSelection());
  connect(use_selection, SIGNAL( clicked() ), this, SLOT( toggleUseSelection() ));
  vlayout->addWidget(use_selection);


  QCheckBox *db_view = new QCheckBox("Database view");
  db_view->setChecked(view_->getDataSource()->getDatabaseView());
  connect(db_view, SIGNAL( clicked() ), this, SLOT( toggleDatabaseView() ));
  vlayout->addWidget(db_view);

  vlayout->addStretch();

  QPushButton *update = new QPushButton ("Update");
  connect( update, SIGNAL(clicked()), view_, SLOT(updateData()) );
  vlayout->addWidget (update);


  setLayout( vlayout );
}

void ListBoxViewConfigWidget::variableSetChanged ()
{
  logdbg  << "ListBoxViewConfigWidget: variableSetChanged";
//  assert (view_);
//  view_->getDataSource()->setSet (variable_set_widget_->getSet());
}

void ListBoxViewConfigWidget::toggleUseFilters()
{
  QCheckBox *send = ((QCheckBox*)sender());
  bool checked = send->checkState() == Qt::Checked;
  logdbg  << "ListBoxViewConfigWidget: toggleUseFilters: setting use filters to " << checked;
  view_->getDataSource()->setUseFilters (checked);
}

void ListBoxViewConfigWidget::toggleUseOrder ()
{
  QCheckBox *send = ((QCheckBox*)sender());
  bool checked = send->checkState() == Qt::Checked;
  view_->getDataSource()->setUseOrder (checked);
}

void ListBoxViewConfigWidget::toggleOrderAscending ()
{
  QCheckBox *send = ((QCheckBox*)sender());
  bool checked = send->checkState() == Qt::Checked;
  view_->getDataSource()->setOrderAscending (checked);
}

void ListBoxViewConfigWidget::orderVariableChanged ()
{
  assert (order_variable_widget_);
  DBOVariable *var = order_variable_widget_->getSelectedVariable();
  view_->getDataSource()->setOrderVariableName (var->getName());
  view_->getDataSource()->setOrderVariableType (var->getDBOType());
}

void ListBoxViewConfigWidget::limitMinChanged()
{
  assert (limit_min_edit_);
  unsigned int min = intFromString (limit_min_edit_->text().toStdString());
  view_->getDataSource()->setLimitMin (min);
}
void ListBoxViewConfigWidget::limitMaxChanged()
{
  assert (limit_max_edit_);
  unsigned int max = intFromString (limit_max_edit_->text().toStdString());
  view_->getDataSource()->setLimitMax (max);

}

void ListBoxViewConfigWidget::toggleUseSelection()
{
  QCheckBox *send = ((QCheckBox*)sender());
  bool checked = send->checkState() == Qt::Checked;
  logdbg  << "ListBoxViewConfigWidget: toggleUseSelection: setting use filters to " << checked;
  view_->getDataSource()->setUseSelection (checked);
}

void ListBoxViewConfigWidget::toggleDatabaseView ()
{
    QCheckBox *send = ((QCheckBox*)sender());
    bool checked = send->checkState() == Qt::Checked;
    logdbg  << "ListBoxViewConfigWidget: toggleDatabaseView: setting database view to " << checked;
    view_->getDataSource()->setDatabaseView (checked);

}
