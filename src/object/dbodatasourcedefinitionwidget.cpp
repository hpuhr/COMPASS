#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

#include "dbodatasourcedefinitionwidget.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "dbodatasource.h"
#include "atsdb.h"
#include "dbschemamanager.h"
#include "dbschema.h"
#include "metadbtable.h"

DBODataSourceDefinitionWidget::DBODataSourceDefinitionWidget(DBObject& object, DBODataSourceDefinition &definition, QWidget *parent, Qt::WindowFlags f)
    : QWidget (parent, f), object_(object), definition_(definition), schema_manager_(ATSDB::instance().schemaManager())
{
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Edit DB object data source definition");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    // object parameters
    QGridLayout *grid = new QGridLayout ();

    unsigned int row=0;

    grid->addWidget (new QLabel ("Schema"), row, 0);

    QLabel *schema = new QLabel (definition_.schema().c_str());
    grid->addWidget (schema, row, 0);

    row++;
    grid->addWidget (new QLabel ("Local key"), row, 0);

    ds_local_key_box_ = new QComboBox ();
    updateDSLocalKeySelection();
    grid->addWidget (ds_local_key_box_, row, 1);

    row++;
    grid->addWidget (new QLabel ("Meta table"), row, 0);

    ds_meta_name_box_ = new QComboBox ();
    updateDSMetaTableNameSelection();
    connect(ds_meta_name_box_, SIGNAL( activated(int) ), this, SLOT( changedDSMetaTable() ));
    grid->addWidget (ds_meta_name_box_, row, 2);

    row++;
    grid->addWidget (new QLabel ("Foreign key"), row, 0);

    ds_foreign_key_box_ = new QComboBox ();
    updateDSForeignKeySelection();
    grid->addWidget (ds_foreign_key_box_, row, 3);

    row++;
    grid->addWidget (new QLabel ("Name column"), row, 0);

    ds_foreign_name_box_ = new QComboBox ();
    updateDSNameColumnSelection ();
    grid->addWidget (ds_foreign_name_box_, row, 4);
}

DBODataSourceDefinitionWidget::~DBODataSourceDefinitionWidget()
{

}

void DBODataSourceDefinitionWidget::updateDSLocalKeySelection()
{
  logdbg  << "DBODataSourceDefinitionWidget: updateDSLocalKeySelection";
  auto variables = object_.variables ();

  std::string selection;

  if (ds_local_key_box_->count() > 0)
    selection = ds_local_key_box_->currentText().toStdString();

  while (ds_local_key_box_->count() > 0)
    ds_local_key_box_->removeItem (0);

  int index_cnt=-1;
  unsigned int cnt=0;
  for (auto it = variables.begin(); it != variables.end(); it++)
  {
    if (selection.size()>0 && selection.compare(it->second->name()) == 0)
      index_cnt=cnt;

    ds_local_key_box_->addItem (it->second->name().c_str());

    cnt++;
  }

  if (index_cnt != -1)
  {
    ds_local_key_box_->setCurrentIndex (index_cnt);
  }
}
void DBODataSourceDefinitionWidget::updateDSMetaTableNameSelection ()
{
  logdbg  << "DBODataSourceDefinitionWidget: updateDSMetaTableNameSelection";
  assert (ds_meta_name_box_);

  std::string selection;

  if (ds_meta_name_box_->count() > 0)
    selection = ds_meta_name_box_->currentText().toStdString();

  while (ds_meta_name_box_->count() > 0)
    ds_meta_name_box_->removeItem (0);

  std::string schema_name = definition_.schema();

  DBSchema& schema = schema_manager_.getSchema(schema_name);

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
void DBODataSourceDefinitionWidget::updateDSForeignKeySelection ()
{
  logdbg  << "DBODataSourceDefinitionWidget: updateDSForeignKeySelection";
  assert (ds_foreign_key_box_);
  assert (ds_meta_name_box_);

  std::string meta_table_name = ds_meta_name_box_->currentText().toStdString();
  std::string schema_name = definition_.schema();

  updateVariableSelectionBox (ds_foreign_key_box_, schema_name, meta_table_name);
}

void DBODataSourceDefinitionWidget::updateDSNameColumnSelection ()
{
  logdbg  << "DBODataSourceDefinitionWidget: updateDSNameColumnSelection";
  assert (ds_foreign_name_box_);
  assert (ds_meta_name_box_);

  std::string meta_table_name = ds_meta_name_box_->currentText().toStdString();
  std::string schema_name = definition_.schema();

  updateVariableSelectionBox (ds_foreign_name_box_, schema_name, meta_table_name);

}

void DBODataSourceDefinitionWidget::updateVariableSelectionBox (QComboBox *box, std::string schema_name, std::string meta_table_name)
{
  logdbg  << "DBODataSourceDefinitionWidget: updateVariableSelectionBox";
  assert (box);

  std::string selection;

  if (box->count() > 0)
    selection = box->currentText().toStdString();

  while (box->count() > 0)
    box->removeItem (0);

  DBSchema& schema = schema_manager_.getSchema(schema_name);

  assert (schema.hasMetaTable (meta_table_name));

  const MetaDBTable &meta_table = schema.metaTable (meta_table_name);

  auto table_columns =  meta_table.columns();

  int index_cnt=-1;
  unsigned int cnt=0;
  for (auto it = table_columns.begin(); it != table_columns.end(); it++)
  {
    if (selection.size()>0 && selection.compare(it->second.name()) == 0)
      index_cnt=cnt;

    box->addItem (it->first.c_str());

    cnt++;
  }

  if (index_cnt != -1)
  {
    box->setCurrentIndex (index_cnt);
  }
}
