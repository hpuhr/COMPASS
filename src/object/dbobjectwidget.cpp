/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * DBObjectWidget.cpp
 *
 *  Created on: Aug 27, 2012
 *      Author: sk
 */

#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>

#include "Configuration.h"
#include "ConfigurationManager.h"
#include "DBOTypeComboBox.h"
#include "DBObject.h"
#include "DBObjectWidget.h"
#include "DBObjectManager.h"
#include "DBObjectEditWidget.h"
#include "DBSchema.h"
#include "DBSchemaManager.h"
#include "MetaDBTable.h"
#include "MetaDBObjectEditWidget.h"

DBObjectWidget::DBObjectWidget()
 : grid_ (0), unlocked_(false), new_edit_(0), new_type_(0), new_meta_table_(0), new_button_(0),
    new_meta_edit_ (0), new_meta_button_ (0)

{
  createElements ();
}

DBObjectWidget::~DBObjectWidget()
{
  std::map <DBObject *, DBObjectEditWidget*>::iterator it;
  for (it = edit_dbo_widgets_.begin(); it != edit_dbo_widgets_.end(); it++)
  {
    delete it->second;
  }
  edit_dbo_widgets_.clear();

  std::map <DBObject *, MetaDBObjectEditWidget*>::iterator it2;
  for (it2 = edit_metadbo_widgets_.begin(); it2 != edit_metadbo_widgets_.end(); it2++)
  {
    delete it2->second;
  }
  edit_metadbo_widgets_.clear();
}

void DBObjectWidget::unlock ()
{
  unlocked_=true;

  std::map <QPushButton *, DBObject *>::iterator it;
  for ( it=edit_dbo_buttons_.begin(); it !=  edit_dbo_buttons_.end(); it++)
    it->first->setDisabled (false);

  if (new_meta_button_)
    new_meta_button_->setDisabled (false);

  if (new_edit_)
    new_edit_->setDisabled (false);

  if (new_button_)
    new_button_->setDisabled (false);

  if (new_type_)
    new_type_->setDisabled (false);

  if (new_meta_edit_)
    new_meta_edit_->setDisabled (false);

  if (new_meta_table_)
    new_meta_table_->setDisabled(false);
}

void DBObjectWidget::createElements()
{
  unsigned int frame_width = 2;
  unsigned int frame_width_small = 1;

  setFrameStyle(QFrame::Panel | QFrame::Raised);
  setLineWidth(frame_width);

  QFont font_bold;
  font_bold.setBold(true);

  QFont font_big;
  font_big.setPointSize(18);

  QVBoxLayout *main_layout = new QVBoxLayout ();

  QLabel *main_label = new QLabel ("Database objects");
  main_label->setFont (font_big);
  main_layout->addWidget (main_label);

  QFrame *dob_frame = new QFrame ();
  dob_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
  dob_frame->setLineWidth(frame_width_small);

  grid_ = new QGridLayout ();
  updateDBOs ();

  dob_frame->setLayout (grid_);

  QScrollArea *scroll = new QScrollArea ();
  scroll->setWidgetResizable (true);
  scroll->setWidget(dob_frame);

  main_layout->addWidget (scroll);

  // new object

  QHBoxLayout *new_layout = new QHBoxLayout ();

  QLabel *new_label = new QLabel ("New object");
  new_label->setFont (font_bold);
  new_layout->addWidget (new_label);

  new_edit_ = new QLineEdit ("Undefined");
  new_edit_->setDisabled (true);
  new_layout->addWidget (new_edit_);

  new_type_ = new DBOTypeComboBox ();
  new_type_->setType ("");
  new_type_->setDisabled (true);
  new_layout->addWidget (new_type_);
  //connect(type_box_, SIGNAL( changedType( ) ), this, SLOT( changedType() ));
  //properties_layout->addWidget (type_box_, 2, 1);

  QLabel *new_meta_label = new QLabel ("Meta table");
  new_layout->addWidget (new_meta_label);

  new_meta_table_ = new QComboBox ();
  updateMetaTables();
  new_meta_table_->setDisabled (true);
  new_layout->addWidget (new_meta_table_);

  new_button_ = new QPushButton("Add");
  connect(new_button_, SIGNAL( clicked() ), this, SLOT( addDBO() ));
  new_button_->setDisabled (true);
  new_layout->addWidget (new_button_);

  main_layout->addLayout (new_layout);

  // meta object
  QHBoxLayout *new_meta_layout = new QHBoxLayout ();

  QLabel *new_meta_name_label = new QLabel ("New meta object");
  new_meta_name_label->setFont (font_bold);
  new_meta_layout->addWidget (new_meta_name_label);

  new_meta_edit_ = new QLineEdit ("Undefined");
  new_meta_edit_->setDisabled (true);
  new_meta_layout->addWidget (new_meta_edit_);

  new_meta_button_ = new QPushButton("Add");
  connect(new_meta_button_, SIGNAL( clicked() ), this, SLOT( addMetaDBO() ));
  new_meta_button_->setDisabled (true);
  new_meta_layout->addWidget (new_meta_button_);

  main_layout->addLayout (new_meta_layout);

  setLayout (main_layout);
}

void DBObjectWidget::addDBO ()
{
  assert (new_edit_);
  assert (new_meta_table_);
  assert (new_type_);

  std::string name = new_edit_->text().toStdString();
  std::string meta_table_name = new_meta_table_->currentText().toStdString();

  std::string instance = "DBObject"+name+"0";

  const std::string &type = new_type_->getType ();

  Configuration &config = DBObjectManager::getInstance().addNewSubConfiguration ("DBObject", instance);
  config.addParameterString ("name", name);
  config.addParameterString ("meta_table", meta_table_name);
  config.addParameterString ("dbo_type", type);

  DBObjectManager::getInstance().generateSubConfigurable("DBObject", instance);

  updateDBOs();
}

void DBObjectWidget::addMetaDBO ()
{
  assert (new_meta_edit_);

  std::string name = new_meta_edit_->text().toStdString();

  std::string instance = "DBObject"+name+"0";


  Configuration &config = DBObjectManager::getInstance().addNewSubConfiguration ("DBObject", instance);
  config.addParameterString ("name", name);
  config.addParameterBool ("is_meta", true);

  DBObjectManager::getInstance().generateSubConfigurable("DBObject", instance);
  updateDBOs();
}

void DBObjectWidget::changedDBO ()
{
  updateDBOs ();
}

void DBObjectWidget::editDBO ()
{
  assert (edit_dbo_buttons_.find((QPushButton*)sender()) != edit_dbo_buttons_.end());

  DBObject *object = edit_dbo_buttons_ [(QPushButton*)sender()];

  if (object->isMeta())
  {
    if (edit_metadbo_widgets_.find (object) == edit_metadbo_widgets_.end())
    {
      MetaDBObjectEditWidget *widget = new MetaDBObjectEditWidget (object);
      connect(widget, SIGNAL( changedDBO() ), this, SLOT( changedDBO() ));
      edit_metadbo_widgets_[object] = widget;
    }
    else
      edit_metadbo_widgets_[object]->show();

  }
  else
  {
    if (edit_dbo_widgets_.find (object) == edit_dbo_widgets_.end())
    {
      DBObjectEditWidget *widget = new DBObjectEditWidget (object);
      connect(widget, SIGNAL( changedDBO() ), this, SLOT( changedDBO() ));
      edit_dbo_widgets_[object] = widget;
    }
    else
      edit_dbo_widgets_[object]->show();
  }
}


void DBObjectWidget::updateDBOs ()
{
  QLayoutItem *child;
  while ((child = grid_->takeAt(0)) != 0)
  {
    if (child->widget())
        delete child->widget();
    delete child;
  }
  edit_dbo_buttons_.clear();

  QFont font_bold;
  font_bold.setBold(true);

  QLabel *name_label = new QLabel ("Name");
  name_label->setFont (font_bold);
  grid_->addWidget (name_label, 0, 0);

  QLabel *info_label = new QLabel ("Description");
  info_label->setFont (font_bold);
  grid_->addWidget (info_label, 0, 1);

  unsigned int row=1;

  const std::map <std::string, DBObject*> &objects = DBObjectManager::getInstance().getDBObjects ();
  std::map <std::string, DBObject*>::const_iterator it;

  for (it = objects.begin(); it != objects.end(); it++)
  {
    QLabel *name = new QLabel (it->second->getName().c_str());
    grid_->addWidget (name, row, 0);

    QLabel *info = new QLabel (it->second->getInfo().c_str());
    grid_->addWidget (info, row, 1);

    QPushButton *edit = new QPushButton ("Edit");
    connect(edit, SIGNAL( clicked() ), this, SLOT( editDBO() ));
    grid_->addWidget (edit, row, 5);
    edit->setDisabled (!unlocked_);
    edit_dbo_buttons_ [edit] = it->second;

    row++;
  }
}

void DBObjectWidget::updateMetaTables ()
{
  assert (new_meta_table_);

  if (!DBSchemaManager::getInstance().hasCurrentSchema())
    return;

  std::string selection;
  if (new_meta_table_->count() > 0)
    selection = new_meta_table_->currentText().toStdString();
  while (new_meta_table_->count() > 0)
    new_meta_table_->removeItem (0);

  std::map <std::string, MetaDBTable*> &metas = DBSchemaManager::getInstance().getCurrentSchema()->getMetaTables ();
  std::map <std::string, MetaDBTable*>::iterator it;

  int index_cnt=-1;
  unsigned int cnt=0;
  for (it = metas.begin(); it != metas.end(); it++)
  {
    if (selection.size()>0 && selection.compare(it->second->getName()) == 0)
      index_cnt=cnt;

    new_meta_table_->addItem (it->second->getName().c_str());

    cnt++;
  }

  if (index_cnt != -1)
  {
    new_meta_table_->setCurrentIndex (index_cnt);
  }
}
