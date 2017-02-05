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
 * DBSchemaWidget.cpp
 *
 *  Created on: Aug 19, 2012
 *      Author: sk
 */

#include "dbschemawidget.h"

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
#include <QInputDialog>
#include <QCheckBox>

#include "dbschemawidget.h"
#include "dbschemamanager.h"
#include "configuration.h"
#include "configurationmanager.h"
#include "dbtable.h"
#include "dbtableinfo.h"
#include "dbtablewidget.h"
//#include "dbtableEditWidget.h"
#include "metadbtable.h"
//#include "MetaDBTableEditWidget.h"
#include "dbschema.h"
#include "atsdb.h"
#include "buffer.h"

#include "stringconv.h"

using namespace Utils;

DBSchemaWidget::DBSchemaWidget(DBSchema &schema, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), schema_(schema), auto_populate_check_(nullptr), table_grid_(nullptr), meta_table_grid_(nullptr)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    // tables
    QVBoxLayout *tables_layout = new QVBoxLayout ();

    QLabel *tables_label = new QLabel ("Tables");
    tables_label->setFont (font_bold);
    tables_layout->addWidget (tables_label);


    QScrollArea *table_scroll_area = new QScrollArea ();
    table_scroll_area->setWidgetResizable (true);

    QWidget *table_scroll_widget = new QWidget ();

    table_grid_ = new QGridLayout ();
    table_scroll_widget->setLayout(table_grid_);

    table_scroll_area->setWidget(table_scroll_widget);

    tables_layout->addWidget (table_scroll_area);

    // table buttons
    QHBoxLayout *table_button_layout =  new QHBoxLayout ();

    QPushButton *add_table = new QPushButton ("Add Table");
    connect(add_table, SIGNAL( clicked() ), this, SLOT( addTable() ));
    table_button_layout->addWidget (add_table);

    QPushButton *add_all = new QPushButton ("Add All Tables");
    connect(add_all, SIGNAL( clicked() ), this, SLOT( addAllTables() ));
    table_button_layout->addWidget (add_all);

    auto_populate_check_ = new QCheckBox ("Auto Populate");
    auto_populate_check_->setChecked(true);
    table_button_layout->addWidget (auto_populate_check_);

    tables_layout->addLayout(table_button_layout);

    main_layout->addLayout(tables_layout);

    // meta tables

    QVBoxLayout *meta_tables_layout = new QVBoxLayout ();

    QLabel *meta_tables_label = new QLabel ("Meta Tables");
    meta_tables_label->setFont (font_bold);
    meta_tables_layout->addWidget (meta_tables_label);


    QScrollArea *meta_table_scroll_area = new QScrollArea ();
    meta_table_scroll_area->setWidgetResizable (true);

    QWidget *meta_table_scroll_widget = new QWidget ();

    meta_table_grid_ = new QGridLayout ();
    meta_table_scroll_widget->setLayout(meta_table_grid_);

    meta_table_scroll_area->setWidget(meta_table_scroll_widget);

    meta_tables_layout->addWidget (meta_table_scroll_area);

    // meta table buttons

    QHBoxLayout *add_ts_layout =  new QHBoxLayout ();

    QPushButton *add_ts_button = new QPushButton ("Add");
    connect(add_ts_button, SIGNAL( clicked() ), this, SLOT( addMetaTable() ));
    add_ts_layout->addWidget (add_ts_button);

    meta_tables_layout->addLayout (add_ts_layout);

    main_layout->addLayout(meta_tables_layout);

    setLayout(main_layout);

    updateTableGrid();
    updateMetaTablesGrid ();
}

DBSchemaWidget::~DBSchemaWidget()
{
//    std::map <DBTable *, DBTableEditWidget*>::iterator it;
//    for (it = edit_table_widgets_.begin(); it != edit_table_widgets_.end(); it++)
//    {
//        it->second->close();
//        delete it->second;
//    }
//    edit_table_widgets_.clear();


//    std::map <MetaDBTable *, MetaDBTableEditWidget*>::iterator it2;
//    for (it2 = edit_meta_table_widgets_.begin(); it2 != edit_meta_table_widgets_.end(); it2++)
//    {
//        it2->second->close();
//        delete it2->second;
//    }
//    edit_meta_table_widgets_.clear();
}

void DBSchemaWidget::addTable()
{
    const std::map <std::string, DBTableInfo> &table_info = ATSDB::getInstance().tableInfo ();

    QStringList items;
    for (auto it : table_info)
    {
        if (schema_.tables().count(it.first) == 0)
            items.append(it.first.c_str());
    }

    bool ok;
    QString item = QInputDialog::getItem(this, tr("Add Table"), tr("Select:"), items, 0, false, &ok);
    if (ok && !item.isEmpty())
    {
        std::string name = item.toStdString();
        schema_.addTable(name);

        assert (auto_populate_check_);
        if (auto_populate_check_->checkState() == Qt::Checked)
            schema_.populateTable(name);

        updateTableGrid();
        //updateMetaTablesGrid();
    }
}

void DBSchemaWidget::addAllTables()
{
    const std::map <std::string, DBTableInfo> &table_info = ATSDB::getInstance().tableInfo ();

    for (auto it : table_info)
    {
        if (schema_.tables().count(it.first) == 0)
            schema_.addTable(it.first);

        assert (auto_populate_check_);
        if (auto_populate_check_->checkState() == Qt::Checked)
            schema_.populateTable(it.first);

    }

   updateTableGrid();
   //updateMetaTablesGrid();

}

void DBSchemaWidget::addMetaTable()
{
//    assert (new_meta_table_name_edit_);
//    assert (new_meta_table_table_);

//    std::string ts_name = new_meta_table_name_edit_->text().toStdString();
//    std::string ts_table_name = new_meta_table_table_->currentText().toStdString();

//    std::string ts_instance = "MetaDBTable"+ts_name+"0";

//    Configuration &ts_config = schema_.addNewSubConfiguration ("MetaDBTable", ts_instance);
//    ts_config.addParameterString ("name", ts_name);
//    ts_config.addParameterString ("table", ts_table_name);

//    schema_.generateSubConfigurable("MetaDBTable", ts_instance);
    updateMetaTablesGrid();
}

void DBSchemaWidget::updateTableGrid()
{
    logdbg  << "DBSchemaWidget: updateTableGrid";
    QLayoutItem *child;
    while ((child = table_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *name_label = new QLabel ("Name");
    name_label->setFont (font_bold);
    table_grid_->addWidget (name_label, 0, 0);

    QLabel *numel_label = new QLabel ("# columns");
    numel_label->setFont (font_bold);
    table_grid_->addWidget (numel_label, 0, 1);

    QLabel *key_label = new QLabel ("Key");
    key_label->setFont (font_bold);
    table_grid_->addWidget (key_label, 0, 2);

    QLabel *edit_label = new QLabel ("Edit");
    edit_label->setFont (font_bold);
    table_grid_->addWidget (edit_label, 0, 3);

    QLabel *del_label = new QLabel ("Delete");
    del_label->setFont (font_bold);
    table_grid_->addWidget (del_label, 0, 4);

    unsigned int row=1;

    QPixmap edit_pixmap("./data/icons/edit.png");
    QIcon edit_icon(edit_pixmap);

    QPixmap del_pixmap("./data/icons/delete.png");
    QIcon del_icon(del_pixmap);

    edit_table_buttons_.clear();
    delete_table_buttons_.clear();

    for (auto it : schema_.tables())
    {
        QLabel *name = new QLabel (it.first.c_str());
        table_grid_->addWidget (name, row, 0);

        QLabel *numel = new QLabel ((String::intToString(it.second->numColumns())).c_str());
        table_grid_->addWidget (numel, row, 1);

        QLabel *key = new QLabel (it.second->key().c_str());
        table_grid_->addWidget (key, row, 2);

        QPushButton *edit = new QPushButton ();
        edit->setIcon(edit_icon);
        edit->setIconSize(QSize(30,30));
        edit->setFlat(true);
        connect(edit, SIGNAL( clicked() ), this, SLOT( editTable() ));
        table_grid_->addWidget (edit, row, 3);
        edit_table_buttons_[edit] = it.second;

        QPushButton *del = new QPushButton ();
        del->setIcon(del_icon);
        del->setIconSize(QSize(30,30));
        del->setFlat(true);
        connect(del, SIGNAL( clicked() ), this, SLOT( deleteTable() ));
        table_grid_->addWidget (del, row, 4);
        delete_table_buttons_[del] = it.second;

        row++;
    }

}

void DBSchemaWidget::updateMetaTablesGrid()
{
    logdbg  << "DBSchemaWidget: updateMetaTablesGrid";
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

    for (auto it : schema_.metaTables())
    {
        QLabel *name = new QLabel (it.second->name().c_str());
        meta_table_grid_->addWidget (name, row, 0);

        QLabel *info = new QLabel (it.second->info().c_str());
        meta_table_grid_->addWidget (info, row, 1);

        QLabel *db_name = new QLabel (it.second->tableName().c_str());
        meta_table_grid_->addWidget (db_name, row, 2);

        QLabel *sub = new QLabel ("None");
        sub->setText (it.second->subTableNames().c_str());

        meta_table_grid_->addWidget (sub, row, 3);

        QLabel *numcols = new QLabel (String::intToString(it.second->numColumns()).c_str());
        meta_table_grid_->addWidget (numcols, row, 4);

        QPushButton *edit = new QPushButton ("Edit");
        connect(edit, SIGNAL( clicked() ), this, SLOT( editMetaTable() ));
        meta_table_grid_->addWidget (edit, row, 5);

        //edit_meta_table_buttons_ [edit] = it->second;

        row++;
    }

}

void DBSchemaWidget::editTable()
{
    logdbg << "DBSchemaWidget: editTable";

    QPushButton *sender = dynamic_cast <QPushButton*> (QObject::sender());
    assert (edit_table_buttons_.count(sender) == 1);
    edit_table_buttons_.at(sender)->widget()->show();
//    assert (edit_table_buttons_.find((QPushButton*)sender()) != edit_table_buttons_.end());

//    DBTable *table = edit_table_buttons_ [(QPushButton*)sender()];

//    if (edit_table_widgets_.find (table) == edit_table_widgets_.end())
//    {
//        DBTableEditWidget *widget = new DBTableEditWidget (table);
//        connect(widget, SIGNAL( changedTable() ), this, SLOT( changedTable() ));
//        edit_table_widgets_[table] = widget;
//        widget->updateParameters();
//    }
//    else
//        edit_table_widgets_[table]->show();
}

void DBSchemaWidget::deleteTable()
{
    logdbg << "DBSchemaWidget: deleteTable";

    QPushButton *sender = dynamic_cast <QPushButton*> (QObject::sender());
    assert (delete_table_buttons_.count(sender) == 1);
    schema_.deleteTable(delete_table_buttons_.at(sender)->name());

    updateTableGrid();

    // update meta tables
}

void DBSchemaWidget::editMetaTable ()
{
//    assert (edit_meta_table_buttons_.find((QPushButton*)sender()) != edit_meta_table_buttons_.end());

//    MetaDBTable *table_structure = edit_meta_table_buttons_ [(QPushButton*)sender()];

//    if (edit_meta_table_widgets_.find (table_structure) == edit_meta_table_widgets_.end())
//    {
//        MetaDBTableEditWidget *widget = new MetaDBTableEditWidget (table_structure);
//        connect(widget, SIGNAL( changedMetaTable() ), this, SLOT( changedMetaTable() ));
//        edit_meta_table_widgets_[table_structure] = widget;
//        //    widget->updateParameters();
//        // for id autoset - not required?
//    }
//    else
//        edit_meta_table_widgets_[table_structure]->show();
}

void DBSchemaWidget::changedTable()
{
    schema_.updateTables();
    updateTableGrid();
}

void DBSchemaWidget::changedMetaTable ()
{
    schema_.updateMetaTables();
    updateMetaTablesGrid ();
}
