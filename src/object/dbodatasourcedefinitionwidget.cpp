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

    QLabel *main_label = new QLabel ("Edit data source definition");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    // object parameters
    QGridLayout *grid = new QGridLayout ();

    unsigned int row=0;

    grid->addWidget (new QLabel ("Object"), row, 0);
    grid->addWidget (new QLabel (object_.name().c_str()), row, 1);

    row++;
    grid->addWidget (new QLabel ("Schema"), row, 0);

    QLabel *schema = new QLabel (definition_.schema().c_str());
    grid->addWidget (schema, row, 1);

    row++;
    grid->addWidget (new QLabel ("Local key"), row, 0);

    local_key_box_ = new QComboBox ();
    updateLocalKeySlot();
    connect(local_key_box_, SIGNAL( activated(int) ), this, SLOT(changedLocalKeySlot()));
    grid->addWidget (local_key_box_, row, 1);

    row++;
    grid->addWidget (new QLabel ("Meta table"), row, 0);

    meta_name_box_ = new QComboBox ();
    updateMetaTableSlot();
    connect(meta_name_box_, SIGNAL( activated(int) ), this, SLOT(changedMetaTableSlot()));
    grid->addWidget (meta_name_box_, row, 1);

    row++;
    grid->addWidget (new QLabel ("Foreign key"), row, 0);

    foreign_key_box_ = new QComboBox ();
    updateForeignKeySlot();
    connect(foreign_key_box_, SIGNAL( activated(int) ), this, SLOT(changedForeignKeySlot()));
    grid->addWidget (foreign_key_box_, row, 1);

    row++;
    grid->addWidget (new QLabel ("Name column"), row, 0);

    foreign_name_box_ = new QComboBox ();
    updateNameColumnSlot ();
    connect(foreign_name_box_, SIGNAL( activated(int) ), this, SLOT(changedNameColumnSlot()));
    grid->addWidget (foreign_name_box_, row, 1);

    main_layout->addLayout(grid);

    setLayout(main_layout);
}

DBODataSourceDefinitionWidget::~DBODataSourceDefinitionWidget()
{

}

void DBODataSourceDefinitionWidget::changedLocalKeySlot()
{
    logdbg  << "DBODataSourceDefinitionWidget: changedLocalKeySlot";
    assert (local_key_box_);
    std::string value = local_key_box_->currentText().toStdString();
    definition_.localKey(value);
}


void DBODataSourceDefinitionWidget::changedMetaTableSlot()
{
    logdbg  << "DBODataSourceDefinitionWidget: changedMetaTableSlot";
    assert (meta_name_box_);
    std::string value = meta_name_box_->currentText().toStdString();
    definition_.metaTable(value);

    updateForeignKeySlot ();
    updateNameColumnSlot ();
}

void DBODataSourceDefinitionWidget::changedForeignKeySlot ()
{
    logdbg  << "DBODataSourceDefinitionWidget: changedForeignKeySlot";
    assert (foreign_key_box_);
    std::string value = foreign_key_box_->currentText().toStdString();
    definition_.foreignKey(value);
}

void DBODataSourceDefinitionWidget::changedNameColumnSlot ()
{
    logdbg  << "DBODataSourceDefinitionWidget: changedNameColumnSlot";
    assert (foreign_name_box_);
    std::string value = foreign_name_box_->currentText().toStdString();
    definition_.nameColumn(value);
}


void DBODataSourceDefinitionWidget::updateLocalKeySlot()
{
    logdbg  << "DBODataSourceDefinitionWidget: updateLocalKeySlot";
    auto variables = object_.variables ();

    std::string selection;

    if (local_key_box_->count() > 0)
        selection = local_key_box_->currentText().toStdString();
    else
        selection = definition_.localKey();

    while (local_key_box_->count() > 0)
        local_key_box_->removeItem (0);

    int index_cnt=-1;
    unsigned int cnt=0;
    for (auto it = variables.begin(); it != variables.end(); it++)
    {
        if (selection.size()>0 && selection.compare(it->second->name()) == 0)
            index_cnt=cnt;

        local_key_box_->addItem (it->second->name().c_str());

        cnt++;
    }

    if (index_cnt != -1)
    {
        local_key_box_->setCurrentIndex (index_cnt);
    }
}
void DBODataSourceDefinitionWidget::updateMetaTableSlot ()
{
    logdbg  << "DBODataSourceDefinitionWidget: updateMetaTableSlot";
    assert (meta_name_box_);

    std::string selection;

    if (meta_name_box_->count() > 0)
        selection = meta_name_box_->currentText().toStdString();
    else
        selection = definition_.metaTableName();

    while (meta_name_box_->count() > 0)
        meta_name_box_->removeItem (0);

    std::string schema_name = definition_.schema();

    DBSchema& schema = schema_manager_.getSchema(schema_name);

    auto metatables = schema.metaTables();

    int index_cnt=-1;
    unsigned int cnt=0;
    for (auto it = metatables.begin(); it != metatables.end(); it++)
    {
        if (selection.size()>0 && selection.compare(it->second->name()) == 0)
            index_cnt=cnt;

        meta_name_box_->addItem (it->second->name().c_str());

        cnt++;
    }

    if (index_cnt != -1)
    {
        meta_name_box_->setCurrentIndex (index_cnt);
    }

}
void DBODataSourceDefinitionWidget::updateForeignKeySlot ()
{
    logdbg  << "DBODataSourceDefinitionWidget: updateForeignKeySlot";
    assert (foreign_key_box_);
    assert (meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();
    std::string schema_name = definition_.schema();

    updateVariableSelectionBox (foreign_key_box_, schema_name, meta_table_name, definition_.foreignKey());
}

void DBODataSourceDefinitionWidget::updateNameColumnSlot ()
{
    logdbg  << "DBODataSourceDefinitionWidget: updateNameColumnSlot";
    assert (foreign_name_box_);
    assert (meta_name_box_);

    std::string meta_table_name = meta_name_box_->currentText().toStdString();
    std::string schema_name = definition_.schema();

    updateVariableSelectionBox (foreign_name_box_, schema_name, meta_table_name, definition_.nameColumn());

}

void DBODataSourceDefinitionWidget::updateVariableSelectionBox (QComboBox* box, const std::string& schema_name, const std::string& meta_table_name, const std::string& value)
{
    logdbg  << "DBODataSourceDefinitionWidget: updateVariableSelectionBox: value " << value;
    assert (box);

    std::string selection;

    if (box->count() > 0)
        selection = box->currentText().toStdString();
    else
        selection = value;

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
        if (selection.size()>0 && selection == it->first)
            index_cnt=cnt;

        box->addItem (it->first.c_str());

        cnt++;
    }

    assert (index_cnt != -1);
    box->setCurrentIndex (index_cnt);
}
