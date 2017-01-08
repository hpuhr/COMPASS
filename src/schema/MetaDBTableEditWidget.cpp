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
 * MetaDBTableEditWidget.cpp
 *
 *  Created on: Aug 26, 2012
 *      Author: sk
 */

#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>

#include "ConfigurationManager.h"
#include "DBSchema.h"
#include "DBSchemaManager.h"
#include "DBTable.h"
#include "DBTableColumn.h"
#include "MetaDBTableEditWidget.h"
#include "MetaDBTable.h"
#include "Logger.h"

MetaDBTableEditWidget::MetaDBTableEditWidget(MetaDBTable *table_structure, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), meta_table_ (table_structure), name_edit_ (0), info_edit_(0), table_box_ (0), key_box_(0),
  sub_meta_tables_grid_ (0), columns_grid_(0), new_local_key_ (0), new_table_(0), new_sub_key_(0)
{
    assert (meta_table_);

    setMinimumSize(QSize(800, 600));

    createElements ();

    show();
}

MetaDBTableEditWidget::~MetaDBTableEditWidget()
{
    std::map <MetaDBTable *, MetaDBTableEditWidget*>::iterator it2;
    for (it2 = edit_sub_meta_table_widgets_.begin(); it2 != edit_sub_meta_table_widgets_.end(); it2++)
    {
        it2->second->close();
        delete it2->second;
    }
    edit_sub_meta_table_widgets_.clear();
}

void MetaDBTableEditWidget::createElements ()
{
    unsigned int frame_width = 1;

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Edit table");
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    QGridLayout *properties_layout = new QGridLayout ();

    QLabel *name_label = new QLabel ("Table name");
    properties_layout->addWidget (name_label, 0, 0);

    name_edit_ = new QLineEdit (meta_table_->getName().c_str());
    connect(name_edit_, SIGNAL( returnPressed() ), this, SLOT( editName() ));
    properties_layout->addWidget (name_edit_, 0, 1);

    QLabel *info_label = new QLabel ("Description");
    properties_layout->addWidget (info_label, 1, 0);

    info_edit_ = new QLineEdit (meta_table_->getInfo().c_str());
    connect(info_edit_, SIGNAL( returnPressed() ), this, SLOT( editInfo() ));
    properties_layout->addWidget (info_edit_, 1, 1);

    QLabel *table_label = new QLabel ("Table name");
    properties_layout->addWidget (table_label, 2, 0);

    table_box_ = new QComboBox ();
    updateTableSelection ();
    connect(table_box_, SIGNAL( activated(const QString &) ), this, SLOT( selectTable() ));
    properties_layout->addWidget (table_box_, 2, 1);

    main_layout->addLayout (properties_layout);

    QLabel *structures_label = new QLabel ("Sub table structures");
    structures_label->setFont (font_big);
    main_layout->addWidget (structures_label);

    QFrame *structures_frame = new QFrame ();
    structures_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    structures_frame->setLineWidth(frame_width);

    sub_meta_tables_grid_ = new QGridLayout ();
    updateSubMetaTablesGrid ();

    structures_frame->setLayout (sub_meta_tables_grid_);

    QScrollArea *structures_scroll = new QScrollArea ();
    structures_scroll->setWidgetResizable (true);
    structures_scroll->setWidget(structures_frame);

    main_layout->addWidget (structures_scroll);

    // add new sub structures

    QVBoxLayout *new_struct_layout = new QVBoxLayout ();

    QLabel *new_struct_label = new QLabel ("New sub structure");
    new_struct_label->setFont (font_bold);
    new_struct_layout->addWidget (new_struct_label);

    QGridLayout *new_grid = new QGridLayout ();

    QLabel *local_key_label = new QLabel ("Local key");
    new_grid->addWidget (local_key_label,0,0);

    new_local_key_ = new QComboBox ();
    updateLocalKeySelection ();
    new_grid->addWidget (new_local_key_,1,0);

    QLabel *new_table_label = new QLabel ("Sub table");
    new_grid->addWidget (new_table_label,0,1);

    new_table_ = new QComboBox ();
    updateNewMetaTableSelection ();
    connect(new_table_, SIGNAL( activated(const QString &) ), this, SLOT( updateSubKeySelection() ));
    new_grid->addWidget (new_table_,1,1);

    QLabel *subkey_label = new QLabel ("Sub table key");
    new_grid->addWidget (subkey_label,0,2);

    new_sub_key_ = new QComboBox ();
    updateSubKeySelection();
    new_grid->addWidget (new_sub_key_,1,2);


    QPushButton *new_struct_add = new QPushButton ("Add");
    connect(new_struct_add, SIGNAL( clicked() ), this, SLOT( addSubMetaTable() ));
    new_grid->addWidget (new_struct_add, 1, 3);

    new_struct_layout->addLayout (new_grid);
    main_layout->addLayout (new_struct_layout);

    // columns list
    QFrame *columns_frame = new QFrame ();
    columns_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    columns_frame->setLineWidth(frame_width);

    columns_grid_ = new QGridLayout ();
    updateColumnsGrid ();

    columns_frame->setLayout (columns_grid_);

    QScrollArea *columns_scroll = new QScrollArea ();
    columns_scroll->setWidgetResizable (true);
    columns_scroll->setWidget(columns_frame);

    main_layout->addWidget (columns_scroll);

    main_layout->addStretch ();

    setLayout (main_layout);
}

void MetaDBTableEditWidget::addSubMetaTable ()
{
    assert (meta_table_);
    assert (new_local_key_);
    assert (new_table_);
    assert (new_sub_key_);

    //std::string name = new_name_edit_->text().toStdString();
    std::string local_key = new_local_key_->currentText().toStdString();
    std::string sub_table_name = new_table_->currentText().toStdString();
    std::string sub_table_key = new_sub_key_->currentText().toStdString();

    std::string instance_id = "SubTableDefinition"+meta_table_->getName()+sub_table_name+"0";

    Configuration &configuration = meta_table_->addNewSubConfiguration ("SubTableDefinition", instance_id);
    configuration.addParameterString ("local_key", local_key);
    configuration.addParameterString ("sub_table_name", sub_table_name);
    configuration.addParameterString ("sub_table_key", sub_table_key);
    meta_table_->generateSubConfigurable ("SubTableDefinition", instance_id);

    updateSubMetaTablesGrid();
    emit changedMetaTable();
}

void MetaDBTableEditWidget::editName ()
{
    assert (name_edit_);
    meta_table_->setName (name_edit_->text().toStdString());
    emit changedMetaTable();
}
void MetaDBTableEditWidget::editInfo ()
{
    assert (info_edit_);
    meta_table_->setInfo (info_edit_->text().toStdString());
    emit changedMetaTable();
}

void MetaDBTableEditWidget::selectTable ()
{
    assert (table_box_);
    meta_table_->setTableName (table_box_->currentText().toStdString());
    updateLocalKeySelection ();
    emit changedMetaTable();
}

void MetaDBTableEditWidget::updateTableSelection()
{
    loginf  << "MetaDBTableEditWidget: updateTableSelection";

    assert (table_box_);

    std::string selection;
    if (table_box_->count() > 0)
        selection = table_box_->currentText().toStdString();
    else
        selection = meta_table_->getTableName();

    while (table_box_->count() > 0)
        table_box_->removeItem (0);

    std::map <std::string, DBTable*> &tables = DBSchemaManager::getInstance().getCurrentSchema()->getTables ();
    std::map <std::string, DBTable*>::iterator it;


    int index_cnt=-1;
    int cnt=0;
    for (it = tables.begin(); it != tables.end(); it++)
    {
        table_box_->addItem (it->second->getName().c_str());

        if (selection.size() > 0 && it->second->getName().compare(selection) == 0)
            index_cnt=cnt;

        cnt++;
    }

    if (index_cnt != -1)
    {
        table_box_->setCurrentIndex (index_cnt);
    }

}

void MetaDBTableEditWidget::updateNewMetaTableSelection()
{
    logdbg  << "MetaDBTableEditWidget: updateNewMetaTableSelection";

    assert (new_table_);

    std::string selection;
    if (new_table_->count() > 0)
        selection = new_table_->currentText().toStdString();

    while (new_table_->count() > 0)
        new_table_->removeItem (0);

    std::map <std::string, MetaDBTable*> &tables = DBSchemaManager::getInstance().getCurrentSchema()->getMetaTables ();
    std::map <std::string, MetaDBTable*>::iterator it;


    int index_cnt=-1;
    int cnt=0;
    for (it = tables.begin(); it != tables.end(); it++)
    {
        new_table_->addItem (it->second->getName().c_str());

        if (selection.size() > 0 && it->second->getName().compare(selection) == 0)
            index_cnt=cnt;

        cnt++;
    }

    if (index_cnt != -1)
    {
        new_table_->setCurrentIndex (index_cnt);
    }

}

void MetaDBTableEditWidget::updateLocalKeySelection ()
{
    assert (table_box_);
    assert (new_local_key_);

    if (table_box_->count() == 0)
        return;

    std::string tablename = table_box_->currentText().toStdString();
    assert (tablename.size() > 0);

    DBSchema *schema = DBSchemaManager::getInstance().getCurrentSchema();
    assert (schema->hasTable (tablename));
    DBTable *table = schema->getTable (tablename);

    std::string selection;
    if (new_local_key_->count() > 0)
        selection = new_local_key_->currentText().toStdString();

    while (new_local_key_->count() > 0)
        new_local_key_->removeItem (0);


    std::map <std::string, DBTableColumn *> &columns = table->getColumns ();
    std::map <std::string, DBTableColumn *>::iterator it;

    //loginf  << "MetaDBTable: updateSubKeySelection: " << columns.size();

    int index_cnt=-1;
    int cnt=0;
    for (it = columns.begin(); it != columns.end(); it++)
    {
        std::string name = it->second->getName();

        if (selection.size() > 0 && selection.compare(name) == 0)
            index_cnt=cnt;

        new_local_key_->addItem (name.c_str());
        cnt++;
    }

    if (index_cnt != -1)
    {
        new_local_key_->setCurrentIndex (index_cnt);
    }
}

void MetaDBTableEditWidget::updateSubKeySelection ()
{
    assert (new_table_);
    assert (new_sub_key_);

    if (new_table_->count() == 0)
        return;

    std::string metatablename = new_table_->currentText().toStdString();
    assert (metatablename.size() > 0);

    DBSchema *schema = DBSchemaManager::getInstance().getCurrentSchema();
    assert (schema->hasMetaTable (metatablename));
    MetaDBTable *metatable = schema->getMetaTable (metatablename);

    std::string selection;
    if (new_sub_key_->count() > 0)
        selection = new_sub_key_->currentText().toStdString();

    while (new_sub_key_->count() > 0)
        new_sub_key_->removeItem (0);


    std::map <std::string, DBTableColumn *> &columns = metatable->getColumns ();
    std::map <std::string, DBTableColumn *>::iterator it;

    //loginf  << "MetaDBTable: updateSubKeySelection: " << columns.size();

    int index_cnt=-1;
    int cnt=0;
    for (it = columns.begin(); it != columns.end(); it++)
    {
        std::string name = it->second->getName();

        if (selection.size() > 0 && selection.compare(name) == 0)
            index_cnt=cnt;

        new_sub_key_->addItem (name.c_str());
        cnt++;
    }

    if (index_cnt != -1)
    {
        new_sub_key_->setCurrentIndex (index_cnt);
    }
}

void MetaDBTableEditWidget::updateSubMetaTablesGrid ()
{
    assert (sub_meta_tables_grid_);

    QLayoutItem *child;
    while ((child = sub_meta_tables_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *local_key_label = new QLabel ("Local key");
    local_key_label->setFont(font_bold);
    sub_meta_tables_grid_->addWidget (local_key_label, 0,0);

    QLabel *sub_table_name_label = new QLabel ("Sub table name");
    sub_table_name_label->setFont(font_bold);
    sub_meta_tables_grid_->addWidget (sub_table_name_label, 0,1);

    QLabel *sub_table_key_label = new QLabel ("Sub table key");
    sub_table_key_label->setFont(font_bold);
    sub_meta_tables_grid_->addWidget (sub_table_key_label, 0,2);

    QLabel *subtablestructs_label = new QLabel ("Sub table structures");
    subtablestructs_label->setFont(font_bold);
    sub_meta_tables_grid_->addWidget (subtablestructs_label, 0, 3);

    assert (meta_table_);

    std::map <SubTableDefinition*, MetaDBTable*> &sub_tables = meta_table_->getSubTables ();
    std::map <SubTableDefinition*, MetaDBTable*>::iterator it;

    unsigned int row=1;

    for (it = sub_tables.begin(); it != sub_tables.end(); it++)
    {
        QLabel *local_key = new QLabel (it->first->getLocalKey().c_str());
        sub_meta_tables_grid_->addWidget (local_key, row,0);

        QLabel *table = new QLabel (it->second->getTableName().c_str());
        sub_meta_tables_grid_->addWidget (table, row,1);

        QLabel *sub_key = new QLabel (it->first->getSubTableKey().c_str());
        sub_meta_tables_grid_->addWidget (sub_key, row,2);


        QLabel *sub = new QLabel ("None");
        sub->setText (it->second->getSubTableNames().c_str());
        sub_meta_tables_grid_->addWidget (sub, row, 3);


        QPushButton *edit = new QPushButton ("Edit");
        connect(edit, SIGNAL( clicked() ), this, SLOT( editSubMetaTable() ));
        sub_meta_tables_grid_->addWidget (edit, row, 4);
        edit_sub_meta_table_buttons_ [edit] = it->second;

        row++;
    }
}

void MetaDBTableEditWidget::updateColumnsGrid ()
{
    assert (columns_grid_);

    QLayoutItem *child;
    while ((child = columns_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *name_label = new QLabel ("Name");
    name_label->setFont(font_bold);
    columns_grid_->addWidget (name_label, 0,0);

    std::map <std::string, DBTableColumn*>&columns = meta_table_->getColumns();
    std::map <std::string, DBTableColumn*>::iterator it;

    unsigned int row=1;

    for (it = columns.begin(); it != columns.end(); it++)
    {
        QLabel *name = new QLabel (it->second->getName().c_str());
        columns_grid_->addWidget (name, row,0);

        row++;
    }
}

void MetaDBTableEditWidget::editSubMetaTable ()
{
    assert (edit_sub_meta_table_buttons_.find((QPushButton*)sender()) != edit_sub_meta_table_buttons_.end());

    MetaDBTable *table_structure = edit_sub_meta_table_buttons_ [(QPushButton*)sender()];

    if (edit_sub_meta_table_widgets_.find (table_structure) == edit_sub_meta_table_widgets_.end())
    {
        MetaDBTableEditWidget *widget = new MetaDBTableEditWidget (table_structure);
        edit_sub_meta_table_widgets_[table_structure] = widget;
    }
    else
        edit_sub_meta_table_widgets_[table_structure]->show();
}
