/*
 * ListBoxViewDataWidget.cpp
 *
 *  Created on: Nov 11, 2012
 *      Author: sk
 */

#include <QTabWidget>
#include <QHBoxLayout>

#include "BufferTableWidget.h"
#include "DBObject.h"
#include "DBObjectManager.h"
#include "ListBoxViewDataWidget.h"
#include "ListBoxViewDataSource.h"
#include "Buffer.h"
#include "Logger.h"

ListBoxViewDataWidget::ListBoxViewDataWidget(ListBoxViewDataSource *data_source, QWidget * parent, Qt::WindowFlags f)
 : QWidget (parent, f), data_source_ (data_source)
{
  assert (data_source_);

  QHBoxLayout *layout = new QHBoxLayout ();

  tab_widget_ = new QTabWidget();
  layout->addWidget (tab_widget_);

  std::map <DB_OBJECT_TYPE, Buffer*> &data = data_source_->getData ();
  std::map <DB_OBJECT_TYPE, Buffer*>::iterator it;

  for (it = data.begin(); it != data.end(); it++)
  {
    assert (DBObjectManager::getInstance().existsDBObject(it->first));
    BufferTableWidget *buffer_table = new BufferTableWidget ();
    tab_widget_->addTab ( buffer_table , tr(DBObjectManager::getInstance().getDBObject(it->first)->getName().c_str()));
    buffer_tables_[it->first] = buffer_table;
  }

  setLayout (layout);
}

ListBoxViewDataWidget::~ListBoxViewDataWidget()
{
  buffer_tables_.clear();
}

void ListBoxViewDataWidget::clearTables ()
{
  logdbg  << "ListBoxViewDataWidget: updateTables: start";

  std::map <DB_OBJECT_TYPE, BufferTableWidget*>::iterator it;

  for (it = buffer_tables_.begin(); it != buffer_tables_.end(); it++)
  {
    it->second->show (0, 0, false);
  }

  logdbg  << "ListBoxViewDataWidget: updateTables: end";
}

void ListBoxViewDataWidget::updateData (unsigned int dbo_type, Buffer *buffer)
{
  loginf  << "ListBoxViewDataWidget: updateTables: start";

  DB_OBJECT_TYPE type = (DB_OBJECT_TYPE)dbo_type;
  assert (buffer_tables_.find (type) != buffer_tables_.end());
  buffer_tables_ [type]->show(buffer, data_source_->getSet()->getFor(type), data_source_->getDatabaseView());

  logdbg  << "ListBoxViewDataWidget: updateTables: end";
}
