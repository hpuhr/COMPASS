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

#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QCheckBox>

#include "configuration.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectwidget.h"
#include "dbovariable.h"
#include "dbtablecolumn.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "metadbtable.h"
#include "logger.h"
#include "dbovariabledatatypecombobox.h"
//#include "StringRepresentationComboBox.h"
#include "dbtablecolumncombobox.h"
#include "unitselectionwidget.h"

#include "stringconv.h"

using namespace Utils;

DBObjectWidget::DBObjectWidget(DBObject *object, DBSchemaManager &schema_manager, QWidget * parent, Qt::WindowFlags f)
 : QWidget (parent, f), object_(object), schema_manager_(schema_manager), name_edit_(0), info_edit_(0), loadable_check_(0),
   ds_schema_box_(0), ds_local_key_box_ (0), ds_meta_name_box_ (0), ds_foreign_key_box_(0), meta_table_grid_(0),
   new_meta_schema_box_ (0), new_meta_box_ (0), dbovars_grid_(0), new_var_name_edit_(0), all_schemas_box_(0), add_all_button_(0)
{
  assert (object_);

  setMinimumSize(QSize(1000, 800));

  QFont font_bold;
  font_bold.setBold(true);

  QFont font_big;
  font_big.setPointSize(18);

  int frame_width_small = 1;

  QVBoxLayout *main_layout = new QVBoxLayout ();

  QLabel *main_label = new QLabel ("Edit DB object");
  main_label->setFont (font_big);
  main_layout->addWidget (main_label);


  QHBoxLayout *upper_layout = new QHBoxLayout ();

  // object parameters
  QVBoxLayout *properties_main_layout = new QVBoxLayout ();

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

  QLabel *loadable_label = new QLabel ("Is loadable");
  properties_layout->addWidget (loadable_label, 2, 0);

  loadable_check_ = new QCheckBox ();
  loadable_check_->setChecked (object_->isLoadable());
  connect(loadable_check_, SIGNAL( clicked() ), this, SLOT( changedLoadable() ));
  properties_layout->addWidget (loadable_check_, 2, 1);

  properties_main_layout->addLayout (properties_layout);
  properties_main_layout->addStretch();

  upper_layout->addLayout (properties_main_layout);

  // metas
  QVBoxLayout *meta_layout = new QVBoxLayout ();

  QLabel *meta_label = new QLabel ("Meta tables");
  meta_label->setFont (font_big);
  meta_layout->addWidget (meta_label);

  QFrame *meta_frame = new QFrame ();
  meta_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
  meta_frame->setLineWidth(frame_width_small);

  meta_table_grid_ = new QGridLayout ();
  updateMetaTablesGrid ();

  meta_frame->setLayout (meta_table_grid_);

  QScrollArea *meta_scroll_ = new QScrollArea ();
  meta_scroll_->setWidgetResizable (true);
  meta_scroll_->setWidget(meta_frame);

  meta_layout->addWidget (meta_scroll_);

  // new meta
  QHBoxLayout *new_layout = new QHBoxLayout ();

  QLabel *new_meta_label = new QLabel ("New");
  new_meta_label->setFont (font_bold);
  new_layout->addWidget (new_meta_label);

  QLabel *new_meta_schema_label = new QLabel ("Schema");
  new_layout->addWidget (new_meta_schema_label);

  new_meta_schema_box_ = new QComboBox ();
  updateMetaSchemaSelection ();
  new_layout->addWidget (new_meta_schema_box_);

  QLabel *new_meta_meta_label = new QLabel ("Meta table");
  new_layout->addWidget (new_meta_meta_label);

  new_meta_box_ = new QComboBox ();
  updateMetaTableSelection ();
  new_layout->addWidget (new_meta_box_);

  QPushButton *new_add = new QPushButton ("Add");
  connect(new_add, SIGNAL( clicked() ), this, SLOT( addMetaTable() ));
  new_layout->addWidget (new_add);

  meta_layout->addLayout (new_layout);
  upper_layout->addLayout (meta_layout);

  main_layout->addLayout (upper_layout);

  //data sources
  QVBoxLayout *ds_layout = new QVBoxLayout ();

  QLabel *ds_label = new QLabel ("Data sources");
  ds_label->setFont (font_big);
  ds_layout->addWidget (ds_label);

  QFrame *ds_frame = new QFrame ();
  ds_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
  ds_frame->setLineWidth(frame_width_small);

  ds_grid_ = new QGridLayout ();
  updateDataSourcesGrid ();

  ds_frame->setLayout (ds_grid_);
  ds_layout->addWidget (ds_frame);

  main_layout->addLayout (ds_layout);

  // new ds

  QHBoxLayout *new_ds_layout = new QHBoxLayout ();

  QLabel *new_ds_label = new QLabel ("New");
  new_ds_label->setFont (font_bold);
  new_ds_layout->addWidget (new_ds_label);

  QLabel *ds_schema_label = new QLabel ("Schema");
  new_ds_layout->addWidget (ds_schema_label);

  ds_schema_box_ = new QComboBox ();
  updateDSSchemaSelection();
  connect(ds_schema_box_, SIGNAL( activated(int) ), this, SLOT( changedDSSchema() ));
  new_ds_layout->addWidget (ds_schema_box_);

  QLabel *ds_local_label = new QLabel ("Local key");
  new_ds_layout->addWidget (ds_local_label);

  ds_local_key_box_ = new QComboBox ();
  updateDSLocalKeySelection();
  new_ds_layout->addWidget (ds_local_key_box_);

  QLabel *ds_meta_label = new QLabel ("Meta table");
  new_ds_layout->addWidget (ds_meta_label);

  ds_meta_name_box_ = new QComboBox ();
  updateDSMetaTableNameSelection();
  connect(ds_meta_name_box_, SIGNAL( activated(int) ), this, SLOT( changedDSMetaTable() ));
  new_ds_layout->addWidget (ds_meta_name_box_);

  QLabel *ds_foreign_label = new QLabel ("Foreign key");
  new_ds_layout->addWidget (ds_foreign_label);

  ds_foreign_key_box_ = new QComboBox ();
  updateDSForeignKeySelection();
  new_ds_layout->addWidget (ds_foreign_key_box_);

  QLabel *ds_name_label = new QLabel ("Name");
  new_ds_layout->addWidget (ds_name_label);

  ds_foreign_name_box_ = new QComboBox ();
  updateDSNameColumnSelection ();
  new_ds_layout->addWidget (ds_foreign_name_box_);


  QPushButton *new_ds_add = new QPushButton ("Add");
  connect(new_ds_add, SIGNAL( clicked() ), this, SLOT( addDataSource() ));
  new_ds_layout->addWidget (new_ds_add);


  main_layout->addLayout (new_ds_layout);

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

  QLabel *new_var_label = new QLabel ("New");
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

  // add all

  QHBoxLayout *all_var_layout = new QHBoxLayout ();

  QLabel *all_var_label = new QLabel ("Create variables");
  all_var_label->setFont (font_bold);
  all_var_layout->addWidget (all_var_label);

  QLabel *all_var_schema_label = new QLabel ("Schema");
  all_var_layout->addWidget (all_var_schema_label);

  all_schemas_box_ = new QComboBox ();
  updateAllVarsSchemaSelection();
  all_var_layout->addWidget (all_schemas_box_);

  add_all_button_ = new QPushButton ("Add all");
  connect(add_all_button_, SIGNAL( clicked() ), this, SLOT( addAllVariables() ));
  all_var_layout->addWidget (add_all_button_);

  QPushButton *add_new_button_ = new QPushButton ("Add new");
  connect(add_new_button_, SIGNAL( clicked() ), this, SLOT( addNewVariables() ));
  all_var_layout->addWidget (add_new_button_);


  main_layout->addLayout (all_var_layout);

  setLayout (main_layout);


  show();
}

DBObjectWidget::~DBObjectWidget()
{

}

void DBObjectWidget::updateMetaSchemaSelection ()
{
  logdbg  << "DBObjectWidget: updateSchemaSelection";
  assert (new_meta_schema_box_);
  updateSchemaSelectionBox (new_meta_schema_box_);
}

void DBObjectWidget::updateMetaTableSelection ()
{
  logdbg  << "DBObjectWidget: updateMetaTableSelection";
  assert (new_meta_box_);

  std::string selection;

  if (new_meta_box_->count() > 0)
    selection = new_meta_box_->currentText().toStdString();

  while (new_meta_box_->count() > 0)
    new_meta_box_->removeItem (0);

  auto metas = schema_manager_.getCurrentSchema().metaTables ();

  int index_cnt=-1;
  unsigned int cnt=0;
  for (auto it = metas.begin(); it != metas.end(); it++)
  {
    if (selection.size()>0 && selection.compare(it->second->name()) == 0)
      index_cnt=cnt;

    new_meta_box_->addItem (it->second->name().c_str());

    cnt++;
  }

  if (index_cnt != -1)
  {
    new_meta_box_->setCurrentIndex (index_cnt);
  }
}

void DBObjectWidget::updateAllVarsSchemaSelection ()
{
  logdbg  << "DBObjectWidget: updateAllVarsSchemaSelection";
  assert (all_schemas_box_);
  updateSchemaSelectionBox (all_schemas_box_);
}

void DBObjectWidget::editName ()
{
  logdbg  << "DBObjectWidget: editName";
  assert (name_edit_);
  assert (object_);

  std::string text = name_edit_->text().toStdString();
  assert (text.size()>0);
  object_->setName (text);
  emit changedDBO();
}
void DBObjectWidget::editInfo ()
{
  logdbg  << "DBObjectWidget: editInfo";
  assert (info_edit_);
  assert (object_);

  std::string text = info_edit_->text().toStdString();
  assert (text.size()>0);
  object_->setInfo (text);
  emit changedDBO();
}

void DBObjectWidget::deleteDBOVar()
{
  logdbg  << "DBObjectWidget: deleteDBOVar";
  QPushButton *button = (QPushButton*)sender();
  assert (dbo_vars_grid_delete_buttons_.find(button) != dbo_vars_grid_delete_buttons_.end());
  object_->deleteVariable (dbo_vars_grid_delete_buttons_[button]->getName());
  updateDBOVarsGrid();
}

void DBObjectWidget::editDBOVarName()
{
  logdbg  << "DBObjectWidget: editDBOVarName";
  QLineEdit *edit = (QLineEdit*)sender();
  assert (dbo_vars_grid_name_edits_.find(edit) != dbo_vars_grid_name_edits_.end());
  dbo_vars_grid_name_edits_[edit]->setName(edit->text().toStdString());
}

void DBObjectWidget::editDBOVarInfo()
{
  logdbg  << "DBObjectWidget: editDBOVarInfo";
  QLineEdit *edit = (QLineEdit*)sender();
  assert (dbo_vars_grid_info_edits_.find(edit) != dbo_vars_grid_info_edits_.end());
  dbo_vars_grid_info_edits_[edit]->setInfo(edit->text().toStdString());
}

void DBObjectWidget::changedLoadable ()
{
  logdbg  << "DBObjectWidget: changedLoadable";
  assert (object_);
  assert (loadable_check_);
  object_->setLoadable(loadable_check_->isChecked());
}

void DBObjectWidget::updateDSSchemaSelection()
{
  logdbg  << "DBObjectWidget: updateDSSchemaSelection";
  assert (ds_schema_box_);
  updateSchemaSelectionBox (ds_schema_box_);
}


void DBObjectWidget::updateDSLocalKeySelection()
{
  logdbg  << "DBObjectWidget: updateDSLocalKeySelection";
  auto variables = object_->getVariables ();

  std::string selection;

  if (ds_local_key_box_->count() > 0)
    selection = ds_local_key_box_->currentText().toStdString();

  while (ds_local_key_box_->count() > 0)
  	ds_local_key_box_->removeItem (0);

  int index_cnt=-1;
  unsigned int cnt=0;
  for (auto it = variables.begin(); it != variables.end(); it++)
  {
    if (selection.size()>0 && selection.compare(it->second.getName()) == 0)
      index_cnt=cnt;

    ds_local_key_box_->addItem (it->second.getName().c_str());

    cnt++;
  }

  if (index_cnt != -1)
  {
  	ds_local_key_box_->setCurrentIndex (index_cnt);
  }
}
void DBObjectWidget::updateDSMetaTableNameSelection ()
{
  logdbg  << "DBObjectWidget: updateDSMetaTableNameSelection";
  assert (ds_meta_name_box_);
  assert (ds_schema_box_);

  std::string selection;

  if (ds_meta_name_box_->count() > 0)
    selection = ds_meta_name_box_->currentText().toStdString();

  while (ds_meta_name_box_->count() > 0)
    ds_meta_name_box_->removeItem (0);

  std::string schema_name = ds_schema_box_->currentText().toStdString();

  DBSchema &schema = schema_manager_.getSchema(schema_name);

  auto metatables = schema.metaTables();

  int index_cnt=-1;
  unsigned int cnt=0;
  for (auto it = metatables.begin(); it != metatables.end(); it++)
  {
    if (selection.size()>0 && selection.compare(it->second->name()) == 0)
      index_cnt=cnt;

    ds_meta_name_box_->addItem (it->second->name().c_str());

    cnt++;
  }

  if (index_cnt != -1)
  {
    ds_meta_name_box_->setCurrentIndex (index_cnt);
  }

}
void DBObjectWidget::updateDSForeignKeySelection ()
{
  logdbg  << "DBObjectWidget: updateDSForeignKeySelection";
  assert (ds_foreign_key_box_);
  assert (ds_meta_name_box_);
  assert (ds_schema_box_);

  std::string meta_table_name = ds_meta_name_box_->currentText().toStdString();
  std::string schema_name = ds_schema_box_->currentText().toStdString();

  updateVariableSelectionBox (ds_foreign_key_box_, schema_name, meta_table_name);
}

void DBObjectWidget::updateDSNameColumnSelection ()
{
  logdbg  << "DBObjectWidget: updateDSNameColumnSelection";
  assert (ds_foreign_name_box_);
  assert (ds_meta_name_box_);
  assert (ds_schema_box_);

  std::string meta_table_name = ds_meta_name_box_->currentText().toStdString();
  std::string schema_name = ds_schema_box_->currentText().toStdString();

  updateVariableSelectionBox (ds_foreign_name_box_, schema_name, meta_table_name);

}

void DBObjectWidget::updateDataSourcesGrid ()
{
  logdbg  << "DBObjectWidget: updateDataSourcesGrid";
  assert (object_);
  assert (ds_grid_);

  QLayoutItem *child;
  while ((child = ds_grid_->takeAt(0)) != 0)
  {
    if (child->widget())
        delete child->widget();
    delete child;
  }

  QFont font_bold;
  font_bold.setBold(true);

  QLabel *schema_label = new QLabel ("Schema");
  schema_label->setFont (font_bold);
  ds_grid_->addWidget (schema_label, 0, 0);

  QLabel *local_label = new QLabel ("Local key");
  local_label->setFont (font_bold);
  ds_grid_->addWidget (local_label, 0, 1);

  QLabel *meta_label = new QLabel ("Meta table");
  meta_label->setFont (font_bold);
  ds_grid_->addWidget (meta_label, 0, 2);

  QLabel *foreign_label = new QLabel ("Foreign key");
  foreign_label->setFont (font_bold);
  ds_grid_->addWidget (foreign_label, 0, 3);

  QLabel *ds_name_label = new QLabel ("Name column");
  ds_name_label->setFont (font_bold);
  ds_grid_->addWidget (ds_name_label, 0, 4);


  auto dsdefs = object_->getDataSourceDefinitions ();

  unsigned int row=1;
  for (auto it = dsdefs.begin(); it != dsdefs.end(); it++)
  {
    QLabel *schema = new QLabel (it->second.getSchema().c_str());
    ds_grid_->addWidget (schema, row, 0);

    QLabel *localkey= new QLabel (it->second.getLocalKey().c_str());
    ds_grid_->addWidget (localkey, row, 1);

    QLabel *meta= new QLabel (it->second.getMetaTableName().c_str());
    ds_grid_->addWidget (meta, row, 2);

    QLabel *foreignkey= new QLabel (it->second.getForeignKey().c_str());
    ds_grid_->addWidget (foreignkey, row, 3);

    QLabel *namecol= new QLabel (it->second.getNameColumn().c_str());
    ds_grid_->addWidget (namecol, row, 4);

    row++;
  }
}

void DBObjectWidget::addDataSource ()
{
  logdbg  << "DBObjectWidget: addDataSource";

  assert (ds_schema_box_);
  assert (ds_local_key_box_);
  assert (ds_meta_name_box_);
  assert (ds_foreign_key_box_);
  assert (ds_foreign_name_box_);

  std::string schema = ds_schema_box_->currentText().toStdString();
  std::string local_key = ds_local_key_box_->currentText().toStdString();
  std::string meta_table = ds_meta_name_box_->currentText().toStdString();
  std::string foreign_key = ds_foreign_key_box_->currentText().toStdString();
  std::string foreign_name = ds_foreign_name_box_->currentText().toStdString();

  std::string instance = "DBODataSourceDefinition"+object_->getName()+schema+"0";

  Configuration &config = object_->addNewSubConfiguration ("DBODataSourceDefinition", instance);
  config.addParameterString ("schema", schema);
  config.addParameterString ("local_key", local_key);
  config.addParameterString ("meta_table", meta_table);
  config.addParameterString ("foreign_key", foreign_key);
  config.addParameterString ("name_column", foreign_name);

  object_->generateSubConfigurable("DBODataSourceDefinition", instance);
  updateDataSourcesGrid();
}

void DBObjectWidget::changedDSSchema()
{
  logdbg  << "DBObjectWidget: changedDSSchema";
  updateDSLocalKeySelection();
  updateDSMetaTableNameSelection ();
  updateDSForeignKeySelection ();
  updateDSNameColumnSelection ();
}
void DBObjectWidget::changedDSMetaTable()
{
  logdbg  << "DBObjectWidget: changedMetaTable";
  updateDSForeignKeySelection ();
  updateDSNameColumnSelection ();
}


void DBObjectWidget::updateMetaTablesGrid()
{
  logdbg  << "DBObjectWidget: updateSchemaMetaTables";
  assert (object_);
  assert (meta_table_grid_);

  QLayoutItem *child;
  while ((child = meta_table_grid_->takeAt(0)) != 0)
  {
    if (child->widget())
        delete child->widget();
    delete child;
  }

  QFont font_bold;
  font_bold.setBold(true);

  QLabel *schema_label = new QLabel ("Schema");
  schema_label->setFont (font_bold);
  meta_table_grid_->addWidget (schema_label, 0, 0);

  QLabel *meta_label = new QLabel ("Meta table");
  meta_label->setFont (font_bold);
  meta_table_grid_->addWidget (meta_label, 0, 1);

  auto metas = object_->getMetaTables ();

  unsigned int row=1;
  for (auto it = metas.begin(); it != metas.end(); it++)
  {
    QLabel *schema = new QLabel (it->first.c_str());
    meta_table_grid_->addWidget (schema, row, 0);

    QLabel *meta= new QLabel (it->second.c_str());
    meta_table_grid_->addWidget (meta, row, 1);
    row++;
  }
}

void DBObjectWidget::addVariable()
{
  logdbg  << "DBObjectWidget: addVariable";
  assert (new_var_name_edit_);
  assert (object_);

  std::string name = new_var_name_edit_->text().toStdString();

  std::string instance = "DBOVariable"+object_->getName()+name+"0";

  Configuration &config = object_->addNewSubConfiguration ("DBOVariable", instance);
  config.addParameterString ("id", name);

  object_->generateSubConfigurable("DBOVariable", instance);
  updateDBOVarsGrid();
}

void DBObjectWidget::addAllVariables ()
{
  logdbg  << "DBObjectWidget: addAllVariables";
  assert (object_);
  assert (all_schemas_box_);

  if (all_schemas_box_->count() == 0)
    return;

  std::string schema_name = all_schemas_box_->currentText().toStdString();
  std::string meta_name = object_->getMetaTable (schema_name);

  const MetaDBTable &meta = schema_manager_.getCurrentSchema().metaTable(meta_name);
  auto columns = meta.columns();

  for (auto it = columns.begin(); it != columns.end(); it++)
  {
    std::string column_name = it->second.name();

    std::string instance = "DBOVariable"+object_->getName()+column_name+"0";

    Configuration &config = object_->addNewSubConfiguration ("DBOVariable", instance);

    assert (false);
    // TODO
//    config.addParameterString ("id", column_name);
//    config.addParameterUnsignedInt ("data_type", getDataTypeFromDB(it->second->getType()));
//    config.addParameterUnsignedInt ("dbo_type", object_->getType());

    std::string var_instance = "DBOSchemaVariableDefinition"+object_->getName()+column_name+"0";

    Configuration &var_configuration = config.addNewSubConfiguration ("DBOSchemaVariableDefinition", var_instance);
    var_configuration.addParameterString ("schema", schema_name);
    var_configuration.addParameterString ("meta_table", meta.name());
    var_configuration.addParameterString ("variable", it->second.name());
//    config.addSubConfigurable ("DBOSchemaVariableDefinition", var_instance, var_config_name);

    object_->generateSubConfigurable("DBOVariable", instance);
  }
  updateDBOVarsGrid();
}

void DBObjectWidget::addNewVariables ()
{
  logdbg  << "DBObjectWidget: addNewVariables";
  assert (object_);
  assert (all_schemas_box_);

  if (all_schemas_box_->count() == 0)
    return;

  std::string schema_name = all_schemas_box_->currentText().toStdString();
  std::string meta_name = object_->getMetaTable (schema_name);

  const MetaDBTable &meta = schema_manager_.getCurrentSchema().metaTable(meta_name);
  auto columns = meta.columns ();

  for (auto it = columns.begin(); it != columns.end(); it++)
  {
    std::string column_name = it->second.name();

    if (object_->hasVariable(column_name))
      continue;

    std::string instance = "DBOVariable"+object_->getName()+column_name+"0";

    Configuration &config = object_->addNewSubConfiguration ("DBOVariable", instance);
    config.addParameterString ("id", column_name);
    //TODO
    assert (false);
//    config.addParameterUnsignedInt ("data_type", getDataTypeFromDB(it->second->getType()));
//    config.addParameterUnsignedInt ("dbo_type", object_->getType());

    std::string var_instance = "DBOSchemaVariableDefinition"+object_->getName()+column_name+"0";

    Configuration &var_configuration = config.addNewSubConfiguration ("DBOSchemaVariableDefinition", var_instance);
    var_configuration.addParameterString ("schema", schema_name);
    var_configuration.addParameterString ("meta_table", meta.name());
    var_configuration.addParameterString ("variable", it->second.name());
    //config.addSubConfigurable ("DBOSchemaVariableDefinition", var_instance, var_config_name);

    object_->generateSubConfigurable("DBOVariable", instance);
  }
  updateDBOVarsGrid();
}


void DBObjectWidget::addMetaTable()
{
  logdbg  << "DBObjectWidget: addMetaTable";
  assert (new_meta_schema_box_);
  assert (new_meta_box_);
  assert (object_);

  std::string schema_name = new_meta_schema_box_->currentText().toStdString();
  std::string meta_name = new_meta_box_->currentText().toStdString();

  std::string table_instance = "DBOSchemaMetaTableDefinition"+schema_name+meta_name+"0";

  Configuration &table_config = object_->addNewSubConfiguration ("DBOSchemaMetaTableDefinition", table_instance);
  table_config.addParameterString ("schema", schema_name);
  table_config.addParameterString ("meta_table", meta_name);

  object_->generateSubConfigurable("DBOSchemaMetaTableDefinition", table_instance);

  updateMetaTablesGrid();
  updateDBOVarsGrid();
}

void DBObjectWidget::updateDBOVarsGrid ()
{
  logdbg  << "DBObjectWidget: updateDBOVarsGrid";
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
  dbo_vars_grid_name_edits_.clear();
  dbo_vars_grid_info_edits_.clear();
  dbo_vars_grid_data_type_boxes_.clear();
  dbo_vars_grid_representation_boxes_.clear();

  logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating first row";

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

  QLabel *unit_label = new QLabel ("Unit");
  unit_label->setFont (font_bold);
  dbovars_grid_->addWidget (unit_label, 0, 5);

  logdbg  << "DBObjectWidget: updateDBOVarsGrid: getting schemas";

  auto metas = object_->getMetaTables ();
  auto schemas  = schema_manager_.getSchemas();

  logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating schemas grid";
  unsigned col=6;
  for (auto sit = schemas.begin(); sit != schemas.end(); sit++)
  {
    if (metas.find (sit->second->name()) == metas.end())
      continue;

    QLabel *label = new QLabel ((sit->second->name()+" Variable").c_str());
    label->setFont (font_bold);
    dbovars_grid_->addWidget (label, 0, col);
    col++;
  }

  auto variables = object_->getVariables();

  QPixmap* pixmapmanage = new QPixmap("./Data/icons/close_icon.png");

  logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating variable rows";

  unsigned int row=1;
  for (auto it = variables.begin(); it != variables.end(); it++)
  {
    //logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating variable row for " << it->first;
    QPushButton *del = new QPushButton ();
    del->setIcon(QIcon(*pixmapmanage));
    del->setFixedSize ( 20, 20 );
    del->setFlat(true);
    connect(del, SIGNAL( clicked() ), this, SLOT( deleteDBOVar() ));
    assert (dbo_vars_grid_delete_buttons_.find(del) == dbo_vars_grid_delete_buttons_.end());
    dbo_vars_grid_delete_buttons_ [del] = &it->second;
    dbovars_grid_->addWidget (del, row, 0);

    //logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating variable row for " << it->first << " name";
    QLineEdit *name = new QLineEdit (it->second.getName().c_str());
    connect(name, SIGNAL( returnPressed() ), this, SLOT( editDBOVarName() ));
    assert (dbo_vars_grid_name_edits_.find(name) == dbo_vars_grid_name_edits_.end());
    dbo_vars_grid_name_edits_[name] = &it->second;
    dbovars_grid_->addWidget (name, row, 1);

    //logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating variable row for " << it->first << " info";
    QLineEdit *info = new QLineEdit (it->second.getInfo().c_str());
    connect(info, SIGNAL( returnPressed() ), this, SLOT( editDBOVarInfo() ));
    assert (dbo_vars_grid_info_edits_.find(info) == dbo_vars_grid_info_edits_.end());
    dbo_vars_grid_info_edits_[info] = &it->second;
    dbovars_grid_->addWidget (info, row, 2);

    //logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating variable row for " << it->first << " property";
    DBOVariableDataTypeComboBox *type = new DBOVariableDataTypeComboBox (&it->second);
    assert (dbo_vars_grid_data_type_boxes_.find(type) == dbo_vars_grid_data_type_boxes_.end());
    dbo_vars_grid_data_type_boxes_[type] = &it->second;
    dbovars_grid_->addWidget (type, row, 3);

    //logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating variable row for " << it->first << " stringrep";
    assert (false);
    // TODO
//    StringRepresentationComboBox *repr = new StringRepresentationComboBox (it->second);
//    assert (dbo_vars_grid_representation_boxes_.find(repr) == dbo_vars_grid_representation_boxes_.end());
//    dbo_vars_grid_representation_boxes_[repr] = it->second;
//    dbovars_grid_->addWidget (repr, row, 4);

    //logdbg  << "DBObjectWidget: updateDBOVarsGrid: creating variable row for " << it->first << " unit";
    UnitSelectionWidget *unit_sel = new UnitSelectionWidget (it->second.getUnitDimension(), it->second.getUnitUnit());
    dbovars_grid_->addWidget (unit_sel, row, 5);

    //logdbg  << "DBObjectWidget: updateDBOVarsGrid: variable row for schemas";
    unsigned col=6;
    for (auto sit = schemas.begin(); sit != schemas.end(); sit++)
    {
      if (metas.find (sit->second->name()) == metas.end())
        continue;

      DBTableColumnComboBox *box = new DBTableColumnComboBox (sit->second->name(), metas[sit->second->name()], &it->second);
      dbovars_grid_->addWidget (box, row, col);
      col++;
    }
    row++;
  }
  //logdbg  << "DBObjectWidget: updateDBOVarsGrid: done";
}

void DBObjectWidget::updateSchemaSelectionBox (QComboBox *box)
{
  logdbg  << "DBObjectWidget: updateSchemaSelectionBox";
  assert (box);

  std::string selection;

  if (box->count() > 0)
    selection = box->currentText().toStdString();

  while (box->count() > 0)
    box->removeItem (0);

  auto schemas = schema_manager_.getSchemas ();

  int index_cnt=-1;
  unsigned int cnt=0;
  for (auto it = schemas.begin(); it != schemas.end(); it++)
  {
    if (selection.size()>0 && selection.compare(it->second->name()) == 0)
      index_cnt=cnt;

    box->addItem (it->second->name().c_str());

    cnt++;
  }

  if (index_cnt != -1)
  {
    box->setCurrentIndex (index_cnt);
  }
}

void DBObjectWidget::updateVariableSelectionBox (QComboBox *box, std::string schema_name, std::string meta_table_name)
{
  logdbg  << "DBObjectWidget: updateVariableSelectionBox";
  assert (box);

  std::string selection;

  if (box->count() > 0)
    selection = box->currentText().toStdString();

  while (box->count() > 0)
    box->removeItem (0);

  DBSchema &schema = schema_manager_.getSchema(schema_name);

  assert (schema.hasMetaTable (meta_table_name));

  const MetaDBTable &meta_table = schema.metaTable (meta_table_name);

  auto table_columns =  meta_table.columns();

  int index_cnt=-1;
  unsigned int cnt=0;
  for (auto it = table_columns.begin(); it != table_columns.end(); it++)
  {
    if (selection.size()>0 && selection.compare(it->second.name()) == 0)
      index_cnt=cnt;

    box->addItem (it->second.name().c_str());

    cnt++;
  }

  if (index_cnt != -1)
  {
    box->setCurrentIndex (index_cnt);
  }
}
