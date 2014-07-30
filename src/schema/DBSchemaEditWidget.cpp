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
 * DBSchemaEditWidget.cpp
 *
 *  Created on: Aug 20, 2012
 *      Author: sk
 */

#include <QVBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFrame>
#include <QPushButton>
#include <QLayoutItem>
#include <QComboBox>
#include <QScrollArea>

#include "DBSchemaEditWidget.h"
#include "DBSchemaManager.h"
#include "Configuration.h"
#include "ConfigurationManager.h"
#include "DBTable.h"
#include "DBTableEditWidget.h"
#include "MetaDBTable.h"
#include "MetaDBTableEditWidget.h"
#include "DBSchema.h"
#include "ATSDB.h"
#include "Buffer.h"

#include "String.h"

using namespace Utils::String;

DBSchemaEditWidget::DBSchemaEditWidget(DBSchema *schema, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), schema_(schema), name_edit_(0), new_table_name_edit_(0), new_table_dbname_ (0), new_meta_table_name_edit_(0),
  new_meta_table_table_ (0), table_grid_(0), meta_table_grid_(0)
{
    assert (schema_);

    setMinimumSize(QSize(800, 600));

    createElements ();

    std::string name = schema_->getName();

    name_edit_->setText (name.c_str());

    updateTableGrid();

    updateDBTableComboBox ();

    updateTableComboBox ();

    show();
}

DBSchemaEditWidget::~DBSchemaEditWidget()
{
    std::map <DBTable *, DBTableEditWidget*>::iterator it;
    for (it = edit_table_widgets_.begin(); it != edit_table_widgets_.end(); it++)
    {
        it->second->close();
        delete it->second;
    }
    edit_table_widgets_.clear();


    std::map <MetaDBTable *, MetaDBTableEditWidget*>::iterator it2;
    for (it2 = edit_meta_table_widgets_.begin(); it2 != edit_meta_table_widgets_.end(); it2++)
    {
        it2->second->close();
        delete it2->second;
    }
    edit_meta_table_widgets_.clear();
}

void DBSchemaEditWidget::setName()
{
    assert (name_edit_);
    DBSchemaManager::getInstance().renameCurrentSchema (name_edit_->text().toStdString());
    emit renamed ();
}

void DBSchemaEditWidget::addTable()
{
    DBSchema *current_schema = DBSchemaManager::getInstance().getCurrentSchema ();

    assert (new_table_name_edit_);
    assert (new_table_dbname_);

    std::string table_name = new_table_name_edit_->text().toStdString();
    std::string table_db_name = new_table_dbname_->currentText().toStdString();

    std::string table_instance = "DBTable"+table_name+"0";

    Configuration &table_config = current_schema->addNewSubConfiguration ("DBTable", table_instance);
    table_config.addParameterString ("name", table_name);
    table_config.addParameterString ("db_name", table_db_name);

    current_schema->generateSubConfigurable("DBTable", table_instance);

    updateTableGrid();
    updateTableComboBox();
    updateMetaTableTables();
}

void DBSchemaEditWidget::addMetaTable()
{
    DBSchema *current_schema = DBSchemaManager::getInstance().getCurrentSchema ();

    assert (new_meta_table_name_edit_);
    assert (new_meta_table_table_);

    std::string ts_name = new_meta_table_name_edit_->text().toStdString();
    std::string ts_table_name = new_meta_table_table_->currentText().toStdString();

    std::string ts_instance = "MetaDBTable"+ts_name+"0";

    Configuration &ts_config = current_schema->addNewSubConfiguration ("MetaDBTable", ts_instance);
    ts_config.addParameterString ("name", ts_name);
    ts_config.addParameterString ("table", ts_table_name);

    current_schema->generateSubConfigurable("MetaDBTable", ts_instance);
    updateMetaTablesGrid();
}


void DBSchemaEditWidget::createElements ()
{
    unsigned int frame_width = 1;

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QScrollArea *scroll_area = new QScrollArea ();
    scroll_area->setWidgetResizable (true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    // name edit

    QLabel *main_label = new QLabel ("Edit schema");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    QFrame *name_frame = new QFrame ();
    name_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    name_frame->setLineWidth(frame_width);

    QHBoxLayout *name_layout = new QHBoxLayout ();
    name_frame->setLayout (name_layout);

    QLabel *name_label = new QLabel ("Change Name");
    name_label->setFont (font_bold);
    name_layout->addWidget (name_label);

    name_edit_ = new QLineEdit ();
    name_layout->addWidget (name_edit_);

    QPushButton *name_button = new QPushButton ("Set");
    connect(name_button, SIGNAL( clicked() ), this, SLOT( setName() ));
    name_layout->addWidget (name_button);

    main_layout->addWidget (name_frame);

    // tables

    QFrame *tables_frame = new QFrame ();
    tables_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    tables_frame->setLineWidth(frame_width);
    QVBoxLayout *tables_layout = new QVBoxLayout ();

    tables_frame->setLayout (tables_layout);

    QLabel *table_label = new QLabel ("Tables");
    table_label->setFont (font_big);
    tables_layout->addWidget (table_label);

    table_grid_ = new QGridLayout ();
    tables_layout->addLayout (table_grid_);

    main_layout->addWidget (tables_frame);

    // new table

    QHBoxLayout *new_table_layout = new QHBoxLayout ();

    QLabel *new_table_label = new QLabel ("New Table");
    new_table_label->setFont (font_bold);
    new_table_layout->addWidget (new_table_label);

    QLabel *new_dbname_label = new QLabel ("Name in database");
    new_table_layout->addWidget (new_dbname_label);

    new_table_dbname_ = new QComboBox ();
    new_table_layout->addWidget (new_table_dbname_);

    QLabel *new_tablename_label = new QLabel ("Name");
    new_table_layout->addWidget (new_tablename_label);

    new_table_name_edit_ = new QLineEdit ("Undefined");
    new_table_layout->addWidget (new_table_name_edit_);

    QPushButton *new_table_add = new QPushButton ("Add");
    connect(new_table_add, SIGNAL( clicked() ), this, SLOT( addTable() ));
    new_table_layout->addWidget (new_table_add);

    main_layout->addLayout (new_table_layout);

    // table structures

    QFrame *table_structures_frame = new QFrame ();
    table_structures_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    table_structures_frame->setLineWidth(frame_width);
    QVBoxLayout *table_structures_layout = new QVBoxLayout ();

    table_structures_frame->setLayout (table_structures_layout);

    QLabel *table_structures_label = new QLabel ("Meta tables");
    table_structures_label->setFont (font_big);
    table_structures_layout->addWidget (table_structures_label);

    meta_table_grid_ = new QGridLayout ();
    table_structures_layout->addLayout (meta_table_grid_);

    main_layout->addWidget (table_structures_frame);

    QHBoxLayout *add_ts_layout =  new QHBoxLayout ();

    QLabel *add_ts_label = new QLabel ("New meta table");
    add_ts_label->setFont (font_bold);
    add_ts_layout->addWidget (add_ts_label);

    QLabel *add_ts_table_label = new QLabel ("Table");
    add_ts_layout->addWidget (add_ts_table_label);

    new_meta_table_table_ = new QComboBox ();
    add_ts_layout->addWidget (new_meta_table_table_);

    QLabel *add_ts_name_label = new QLabel ("Name");
    add_ts_layout->addWidget (add_ts_name_label);

    new_meta_table_name_edit_ = new QLineEdit ("Undefined");
    add_ts_layout->addWidget (new_meta_table_name_edit_);

    QPushButton *add_ts_button = new QPushButton ("Add");
    connect(add_ts_button, SIGNAL( clicked() ), this, SLOT( addMetaTable() ));
    add_ts_layout->addWidget (add_ts_button);

    updateMetaTablesGrid();

    main_layout->addLayout (add_ts_layout);

    main_layout->addStretch();

    QWidget *tmp = new QWidget ();
    tmp->setLayout (main_layout);
    scroll_area->setWidget(tmp);

    QVBoxLayout *tmp_lay = new QVBoxLayout ();
    tmp_lay->addWidget (scroll_area);
    setLayout(tmp_lay);
}

void DBSchemaEditWidget::updateDBTableComboBox ()
{
    assert (ATSDB::getInstance().getDBOpened ());
    Buffer *tables = ATSDB::getInstance().getTableList ();

    if (tables->getFirstWrite())
    {
        delete tables;
        return;
    }

    tables->setIndex(0);
    std::string table_name;

    assert (new_table_dbname_);
    while (new_table_dbname_->count() > 0)
        new_table_dbname_->removeItem (0);

    for (unsigned int cnt=0; cnt < tables->getSize(); cnt++)
    {
        if (cnt != 0)
            tables->incrementIndex();

        table_name = *(std::string *) tables->get(0);
        new_table_dbname_->addItem (table_name.c_str());
    }
    delete tables;
}

void DBSchemaEditWidget::updateTableComboBox ()
{
    assert (new_meta_table_table_);
    while (new_meta_table_table_->count() > 0)
        new_meta_table_table_->removeItem (0);

    DBSchema *schema = DBSchemaManager::getInstance().getCurrentSchema ();

    std::map <std::string, DBTable*> &tables = schema->getTables ();
    std::map <std::string, DBTable*>::iterator it;

    for (it = tables.begin(); it != tables.end(); it++)
    {
        new_meta_table_table_->addItem (it->second->getName().c_str());
    }

}

void DBSchemaEditWidget::updateTableGrid()
{
    logdbg  << "DBSchemaEditWidget: updateTableGrid";
    QLayoutItem *child;
    while ((child = table_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }
    edit_table_buttons_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *name_label = new QLabel ("Name");
    name_label->setFont (font_bold);
    table_grid_->addWidget (name_label, 0, 0);

    QLabel *info_label = new QLabel ("Description");
    info_label->setFont (font_bold);
    table_grid_->addWidget (info_label, 0, 1);

    QLabel *db_name_label = new QLabel ("DB table");
    db_name_label->setFont (font_bold);
    table_grid_->addWidget (db_name_label, 0, 2);

    QLabel *numel_label = new QLabel ("# columns");
    numel_label->setFont (font_bold);
    table_grid_->addWidget (numel_label, 0, 3);

    QLabel *key_label = new QLabel ("Key");
    key_label->setFont (font_bold);
    table_grid_->addWidget (key_label, 0, 4);

    unsigned int row=1;
    DBSchema *schema = DBSchemaManager::getInstance().getCurrentSchema ();

    std::map <std::string, DBTable*> &tables = schema->getTables ();
    std::map <std::string, DBTable*>::iterator it;

    for (it = tables.begin(); it != tables.end(); it++)
    {
        QLabel *name = new QLabel (it->first.c_str());
        table_grid_->addWidget (name, row, 0);

        QLabel *info = new QLabel (it->second->getInfo().c_str());
        table_grid_->addWidget (info, row, 1);

        QLabel *dbname = new QLabel (it->second->getDBName().c_str());
        table_grid_->addWidget (dbname, row, 2);

        QLabel *numel = new QLabel (intToString(it->second->getNumColumns()).c_str());
        table_grid_->addWidget (numel, row, 3);

        QLabel *key = new QLabel (it->second->getKeyName().c_str());
        table_grid_->addWidget (key, row, 4);

        QPushButton *edit = new QPushButton ("Edit");
        connect(edit, SIGNAL( clicked() ), this, SLOT( editTable() ));
        table_grid_->addWidget (edit, row, 5);

        edit_table_buttons_ [edit] = it->second;

        row++;
    }

}

void DBSchemaEditWidget::updateMetaTablesGrid()
{
    logdbg  << "DBSchemaEditWidget: updateMetaTablesGrid";
    QLayoutItem *child;
    while ((child = meta_table_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }
    edit_meta_table_buttons_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *name_label = new QLabel ("Name");
    name_label->setFont (font_bold);
    meta_table_grid_->addWidget (name_label, 0, 0);

    QLabel *info_label = new QLabel ("Description");
    info_label->setFont (font_bold);
    meta_table_grid_->addWidget (info_label, 0, 1);

    QLabel *db_name_label = new QLabel ("Table");
    db_name_label->setFont (font_bold);
    meta_table_grid_->addWidget (db_name_label, 0, 2);

    QLabel *subtables_label = new QLabel ("Sub table structures");
    subtables_label->setFont (font_bold);
    meta_table_grid_->addWidget (subtables_label, 0, 3);

    QLabel *numcols_label = new QLabel ("#columns");
    numcols_label->setFont (font_bold);
    meta_table_grid_->addWidget (numcols_label, 0, 4);

    unsigned int row=1;
    DBSchema *schema = DBSchemaManager::getInstance().getCurrentSchema ();

    std::map <std::string, MetaDBTable*> &table_structures= schema->getMetaTables ();
    std::map <std::string, MetaDBTable*>::iterator it;

    for (it = table_structures.begin(); it != table_structures.end(); it++)
    {
        QLabel *name = new QLabel (it->second->getName().c_str());
        meta_table_grid_->addWidget (name, row, 0);

        QLabel *info = new QLabel (it->second->getInfo().c_str());
        meta_table_grid_->addWidget (info, row, 1);

        QLabel *db_name = new QLabel (it->second->getTableName().c_str());
        meta_table_grid_->addWidget (db_name, row, 2);

        QLabel *sub = new QLabel ("None");
        sub->setText (it->second->getSubTableNames().c_str());

        meta_table_grid_->addWidget (sub, row, 3);

        QLabel *numcols = new QLabel (intToString(it->second->getNumColumns()).c_str());
        meta_table_grid_->addWidget (numcols, row, 4);

        QPushButton *edit = new QPushButton ("Edit");
        connect(edit, SIGNAL( clicked() ), this, SLOT( editMetaTable() ));
        meta_table_grid_->addWidget (edit, row, 5);

        edit_meta_table_buttons_ [edit] = it->second;

        row++;
    }

}

void DBSchemaEditWidget::updateMetaTableTables ()
{
    std::map <MetaDBTable *, MetaDBTableEditWidget*>::iterator it;

    for (it = edit_meta_table_widgets_.begin(); it != edit_meta_table_widgets_.end(); it++)
    {
        it->second->updateTableSelection();
    }
}


void DBSchemaEditWidget::editTable()
{
    assert (edit_table_buttons_.find((QPushButton*)sender()) != edit_table_buttons_.end());

    DBTable *table = edit_table_buttons_ [(QPushButton*)sender()];

    if (edit_table_widgets_.find (table) == edit_table_widgets_.end())
    {
        DBTableEditWidget *widget = new DBTableEditWidget (table);
        connect(widget, SIGNAL( changedTable() ), this, SLOT( changedTable() ));
        edit_table_widgets_[table] = widget;
        widget->updateParameters();
    }
    else
        edit_table_widgets_[table]->show();
}

void DBSchemaEditWidget::editMetaTable ()
{
    assert (edit_meta_table_buttons_.find((QPushButton*)sender()) != edit_meta_table_buttons_.end());

    MetaDBTable *table_structure = edit_meta_table_buttons_ [(QPushButton*)sender()];

    if (edit_meta_table_widgets_.find (table_structure) == edit_meta_table_widgets_.end())
    {
        MetaDBTableEditWidget *widget = new MetaDBTableEditWidget (table_structure);
        connect(widget, SIGNAL( changedMetaTable() ), this, SLOT( changedMetaTable() ));
        edit_meta_table_widgets_[table_structure] = widget;
        //    widget->updateParameters();
        // for id autoset - not required?
    }
    else
        edit_meta_table_widgets_[table_structure]->show();
}

void DBSchemaEditWidget::changedTable()
{
    DBSchemaManager::getInstance().getCurrentSchema()->updateTables();
    updateTableGrid();
    updateMetaTableTables();
}

void DBSchemaEditWidget::changedMetaTable ()
{
    DBSchemaManager::getInstance().getCurrentSchema()->updateMetaTables();
    updateMetaTablesGrid ();
}
