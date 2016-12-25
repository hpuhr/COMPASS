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
 * MetaDBObjectEditWidget.cpp
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
#include <QCheckBox>

#include "Configuration.h"
#include "ConfigurationManager.h"
#include "DBObject.h"
#include "MetaDBObjectEditWidget.h"
#include "DBOVariable.h"
#include "DBTableColumn.h"
#include "DBOTypeComboBox.h"
#include "DBObjectManager.h"
#include "MetaDBTable.h"
#include "Logger.h"
#include "DBOVariableDataTypeComboBox.h"
//#include "StringRepresentationComboBox.h"
#include "DBOVariableComboBox.h"
#include "DBSchemaManager.h"

MetaDBObjectEditWidget::MetaDBObjectEditWidget(DBObject *object, QWidget * parent, Qt::WindowFlags f)
 : QWidget (parent, f), object_(object), name_edit_(0), info_edit_(0), dbovars_grid_(0), new_var_name_edit_(0)
{
  assert (object_);
  assert (object_->isMeta());

  //TODO
  assert (false);
  //assert (object_->getType() == DBO_UNDEFINED);
  assert (!object_->isLoadable());

  setMinimumSize(QSize(1000, 800));

  createElements ();

  show();
}

MetaDBObjectEditWidget::~MetaDBObjectEditWidget()
{
}

void MetaDBObjectEditWidget::createElements ()
{
  QFont font_bold;
  font_bold.setBold(true);

  QFont font_big;
  font_big.setPointSize(18);

  int frame_width_small = 1;

  QVBoxLayout *main_layout = new QVBoxLayout ();

  QLabel *main_label = new QLabel ("Edit meta DB object");
  main_label->setFont (font_big);
  main_layout->addWidget (main_label);


  // object parameters

  QGridLayout *properties_layout = new QGridLayout ();

  QLabel *name_label = new QLabel ("Table name");
  properties_layout->addWidget (name_label, 0, 0);

  name_edit_ = new QLineEdit (object_->getName().c_str());
  connect(name_edit_, SIGNAL( returnPressed() ), this, SLOT( editName() ));
  properties_layout->addWidget (name_edit_, 0, 1);

  QLabel *info_label = new QLabel ("Description");
  properties_layout->addWidget (info_label, 1, 0);

  info_edit_ = new QLineEdit (object_->getInfo().c_str());
  connect(info_edit_, SIGNAL( returnPressed() ), this, SLOT( editInfo() ));
  properties_layout->addWidget (info_edit_, 1, 1);

  main_layout->addLayout (properties_layout);

  // dobvars

  QLabel *dbo_label = new QLabel ("Variables");
  dbo_label->setFont (font_big);
  main_layout->addWidget (dbo_label);

  QFrame *dbo_frame = new QFrame ();
  dbo_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
  dbo_frame->setLineWidth(frame_width_small);

  dbovars_grid_ = new QGridLayout ();
  updateDBOVarsGrid();

  dbo_frame->setLayout (dbovars_grid_);

  QScrollArea *dbo_scroll_ = new QScrollArea ();
  dbo_scroll_->setWidgetResizable (true);
  dbo_scroll_->setWidget(dbo_frame);

  main_layout->addWidget (dbo_scroll_);

  // new dobvar

  QHBoxLayout *new_var_layout = new QHBoxLayout ();

  QLabel *new_var_label = new QLabel ("New variable");
  new_var_label->setFont (font_bold);
  new_var_layout->addWidget (new_var_label);

  QLabel *new_var_name_label = new QLabel ("Name");
  new_var_layout->addWidget (new_var_name_label);

  new_var_name_edit_ = new QLineEdit ("Undefined");
  new_var_layout->addWidget (new_var_name_edit_);

  QPushButton *new_var_add = new QPushButton ("Add");
  connect(new_var_add, SIGNAL( clicked() ), this, SLOT( addVariable() ));
  new_var_layout->addWidget (new_var_add);

  main_layout->addLayout (new_var_layout);

  setLayout (main_layout);
}



void MetaDBObjectEditWidget::editName ()
{
  assert (name_edit_);
  assert (object_);

  std::string text = name_edit_->text().toStdString();
  assert (text.size()>0);
  object_->setName (text);
  emit changedDBO();
}
void MetaDBObjectEditWidget::editInfo ()
{
  assert (info_edit_);
  assert (object_);

  std::string text = info_edit_->text().toStdString();
  assert (text.size()>0);
  object_->setInfo (text);
  emit changedDBO();
}

void MetaDBObjectEditWidget::addVariable()
{
  assert (new_var_name_edit_);
  assert (object_);

  std::string name = new_var_name_edit_->text().toStdString();

  std::string instance = "DBOVariable"+object_->getName()+name+"0";

  Configuration &config = object_->addNewSubConfiguration ("DBOVariable", instance);
  config.addParameterString ("id", name);

  object_->generateSubConfigurable("DBOVariable", instance);
  updateDBOVarsGrid();
}

void MetaDBObjectEditWidget::deleteVariable()
{
  QPushButton *button = (QPushButton*)sender();
  assert (dbo_vars_grid_delete_buttons_.find(button) != dbo_vars_grid_delete_buttons_.end());
  object_->deleteVariable (dbo_vars_grid_delete_buttons_[button]->getName());
  updateDBOVarsGrid();
}

void MetaDBObjectEditWidget::updateDBOVarsGrid ()
{
  assert (object_);
  assert (dbovars_grid_);

  QLayoutItem *child;
  while ((child = dbovars_grid_->takeAt(0)) != 0)
  {
    if (child->widget())
        delete child->widget();
    delete child;
  }
  dbo_vars_grid_delete_buttons_.clear();
  dbo_vars_grid_data_type_boxes_.clear();
  dbo_vars_grid_representation_boxes_.clear();

  QFont font_bold;
  font_bold.setBold(true);

  QLabel *name_label = new QLabel ("Name");
  name_label->setFont (font_bold);
  dbovars_grid_->addWidget (name_label, 0, 1);

  QLabel *info_label = new QLabel ("Description");
  info_label->setFont (font_bold);
  dbovars_grid_->addWidget (info_label, 0, 2);

  QLabel *type_label = new QLabel ("Data type");
  type_label->setFont (font_bold);
  dbovars_grid_->addWidget (type_label, 0, 3);

  QLabel *repr_label = new QLabel ("Representation");
  repr_label->setFont (font_bold);
  dbovars_grid_->addWidget (repr_label, 0, 4);

  const std::map <std::string, DBObject*> &objects = DBObjectManager::getInstance().getDBObjects ();
  std::map <std::string, DBObject*>::const_iterator it;

  unsigned col=5;
  for (it = objects.begin(); it != objects.end(); it++)
  {
    if (it->second->isMeta() || !it->second->isLoadable())
      continue;

    QLabel *label = new QLabel ((it->second->getName()+" Variable").c_str());
    label->setFont (font_bold);
    dbovars_grid_->addWidget (label, 0, col);
    col++;
  }

  std::string schema = DBSchemaManager::getInstance().getCurrentSchemaName();

  std::map<std::string, DBOVariable*> &vars = object_->getVariables ();
  std::map <std::string, DBOVariable*>::iterator it2;

  QPixmap* pixmapmanage = new QPixmap("./Data/icons/close_icon.png");

  unsigned int row=1;
  for (it2 = vars.begin(); it2 != vars.end(); it2++)
  {
    QPushButton *del = new QPushButton ();
    del->setIcon(QIcon(*pixmapmanage));
    del->setFixedSize ( 20, 20 );
    del->setFlat(true);
    connect(del, SIGNAL( clicked() ), this, SLOT( deleteVariable() ));
    dbo_vars_grid_delete_buttons_ [del] = it2->second;
    dbovars_grid_->addWidget (del, row, 0);

    QLabel *name_label = new QLabel (it2->second->getName().c_str());
    dbovars_grid_->addWidget (name_label, row, 1);

    QLineEdit *info = new QLineEdit (it2->second->getInfo().c_str());
    dbovars_grid_->addWidget (info, row, 2);

    DBOVariableDataTypeComboBox *type = new DBOVariableDataTypeComboBox (it2->second);
//    type->setType (it2->second->getDataType());
    dbo_vars_grid_data_type_boxes_[type] = it2->second;
    dbovars_grid_->addWidget (type, row, 3);

    assert (false);
    // TODO

//    StringRepresentationComboBox *repr = new StringRepresentationComboBox (it2->second);
//    //repr->setRepresentation (it2->second->getRepresentation());
//    dbo_vars_grid_representation_boxes_[repr] = it2->second;
//    dbovars_grid_->addWidget (repr, row, 4);




//    unsigned col=5;
//    for (sit = schemas.begin(); sit != schemas.end(); sit++)
//    {
//      if (metas.find (sit->second->getName()) == metas.end())
//        continue;
//
//      DBTableColumnComboBox *box = new DBTableColumnComboBox (sit->second->getName(), metas[sit->second->getName()], it->second);
//      dbovars_grid_->addWidget (box, row, col);
//      col++;
//    }

    unsigned col=5;
    for (it = objects.begin(); it != objects.end(); it++)
    {
      if (it->second->isMeta() || !it->second->isLoadable())
        continue;

      DBOVariableComboBox *box = new DBOVariableComboBox (it->first, it2->second);
      dbovars_grid_->addWidget (box, row, col);
      col++;
    }


    row++;
  }
}
