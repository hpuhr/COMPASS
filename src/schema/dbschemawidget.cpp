/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dbschemawidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLayoutItem>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "atsdb.h"
#include "buffer.h"
#include "configuration.h"
#include "configurationmanager.h"
#include "dbinterface.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbschemawidget.h"
#include "dbtable.h"
#include "dbtableinfo.h"
#include "dbtablewidget.h"
#include "files.h"
#include "global.h"
#include "metadbtable.h"
#include "metadbtablewidget.h"
#include "stringconv.h"

using namespace Utils;

DBSchemaWidget::DBSchemaWidget(DBSchema& schema, QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), schema_(schema)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    // tables
    QVBoxLayout* tables_layout = new QVBoxLayout();

    QLabel* tables_label = new QLabel("Tables");
    tables_label->setFont(font_bold);
    tables_layout->addWidget(tables_label);

    QScrollArea* table_scroll_area = new QScrollArea();
    table_scroll_area->setWidgetResizable(true);

    QWidget* table_scroll_widget = new QWidget();

    table_grid_ = new QGridLayout();
    table_scroll_widget->setLayout(table_grid_);

    table_scroll_area->setWidget(table_scroll_widget);

    tables_layout->addWidget(table_scroll_area);

    // table buttons
    QHBoxLayout* table_button_layout = new QHBoxLayout();

    add_table_button_ = new QPushButton("Add");
    connect(add_table_button_, &QPushButton::clicked, this, &DBSchemaWidget::addTableSlot);
    table_button_layout->addWidget(add_table_button_);

    add_all_tables_button_ = new QPushButton("Add All");
    connect(add_all_tables_button_, &QPushButton::clicked, this, &DBSchemaWidget::addAllTablesSlot);
    table_button_layout->addWidget(add_all_tables_button_);

    auto_populate_check_ = new QCheckBox("Auto Populate");
    auto_populate_check_->setChecked(true);
    table_button_layout->addWidget(auto_populate_check_);

    tables_layout->addLayout(table_button_layout);

    main_layout->addLayout(tables_layout);
    main_layout->addStretch();

    // meta tables
    QVBoxLayout* meta_tables_layout = new QVBoxLayout();

    QLabel* meta_tables_label = new QLabel("Meta Tables");
    meta_tables_label->setFont(font_bold);
    meta_tables_layout->addWidget(meta_tables_label);

    QScrollArea* meta_table_scroll_area = new QScrollArea();
    meta_table_scroll_area->setWidgetResizable(true);

    QWidget* meta_table_scroll_widget = new QWidget();

    meta_table_grid_ = new QGridLayout();
    meta_table_scroll_widget->setLayout(meta_table_grid_);

    meta_table_scroll_area->setWidget(meta_table_scroll_widget);

    meta_tables_layout->addWidget(meta_table_scroll_area);

    // meta table buttons
    QHBoxLayout* add_ts_layout = new QHBoxLayout();

    add_ts_button_ = new QPushButton("Add");
    connect(add_ts_button_, &QPushButton::clicked, this, &DBSchemaWidget::addMetaTableSlot);
    add_ts_layout->addWidget(add_ts_button_);

    meta_tables_layout->addLayout(add_ts_layout);

    main_layout->addLayout(meta_tables_layout);
    main_layout->addStretch();

    setLayout(main_layout);

    updateTableGrid();
    updateMetaTableGrid();
}

DBSchemaWidget::~DBSchemaWidget()
{
    edit_table_buttons_.clear();
    delete_table_buttons_.clear();
    edit_meta_table_buttons_.clear();
    delete_meta_table_buttons_.clear();
    // layouts will delete the rest
}

void DBSchemaWidget::lock()
{
    for (auto& it : delete_table_buttons_)
        it.first->setDisabled(true);

    add_table_button_->setDisabled(true);
    add_all_tables_button_->setDisabled(true);
    auto_populate_check_->setDisabled(true);

    for (auto& it : delete_meta_table_buttons_)
        it.first->setDisabled(true);

    add_ts_button_->setDisabled(true);
}

void DBSchemaWidget::addTableSlot()
{
    const std::map<std::string, DBTableInfo>& table_info =
        ATSDB::instance().interface().tableInfo();

    QStringList items;
    for (auto it : table_info)
    {
        if (schema_.tables().count(it.first) == 0)
            items.append(it.first.c_str());
    }

    bool ok;
    QString item =
        QInputDialog::getItem(this, tr("Add Table"), tr("Select:"), items, 0, false, &ok);
    if (ok && !item.isEmpty())
    {
        std::string name = item.toStdString();

        if (schema_.hasTable(name))
        {
            logerr << "DBSchemaWidget: addTable: table with same name already exists";
            return;
        }
        schema_.addTable(name);

        assert(auto_populate_check_);
        if (auto_populate_check_->checkState() == Qt::Checked)
            schema_.populateTable(name);

        updateTableGrid();
    }
}

void DBSchemaWidget::addAllTablesSlot()
{
    const std::map<std::string, DBTableInfo>& table_info =
        ATSDB::instance().interface().tableInfo();

    for (auto it : table_info)
    {
        if (schema_.tables().count(it.first) == 0)
            schema_.addTable(it.first);

        assert(auto_populate_check_);
        if (auto_populate_check_->checkState() == Qt::Checked)
            schema_.populateTable(it.first);
    }

    updateTableGrid();
}

void DBSchemaWidget::addMetaTableSlot()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Meta Table Name"),
                                         tr("Specify a (unique) meta table name:"),
                                         QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
        std::string name = text.toStdString();

        if (schema_.hasMetaTable(name))
        {
            logerr << "DBSchemaWidget: addMetaTable: table with same name already exists";
            return;
        }

        QStringList items;
        for (auto it : schema_.tables())  // make list of all tables
        {
            items.append(it.first.c_str());
        }

        bool ok;
        QString item = QInputDialog::getItem(this, tr("Meta Table Main Table"), tr("Select:"),
                                             items, 0, false, &ok);
        if (ok && !item.isEmpty())
        {
            std::string main_table_name = item.toStdString();

            schema_.addMetaTable(name, main_table_name);

            updateMetaTableGrid();
        }
    }
}

void DBSchemaWidget::updateTableGrid()
{
    logdbg << "DBSchemaWidget: updateTableGrid";
    QLayoutItem* child;
    while ((child = table_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* name_label = new QLabel("Name");
    name_label->setFont(font_bold);
    table_grid_->addWidget(name_label, 0, 0);

    QLabel* key_label = new QLabel("Key");
    key_label->setFont(font_bold);
    table_grid_->addWidget(key_label, 0, 1);

    //    QLabel *edit_label = new QLabel ("Edit");
    //    edit_label->setFont (font_bold);
    //    edit_label->setAlignment(Qt::AlignCenter);
    //    table_grid_->addWidget (edit_label, 0, 3);

    //    QLabel *del_label = new QLabel ("Delete");
    //    del_label->setFont (font_bold);
    //    del_label->setAlignment(Qt::AlignCenter);
    //    table_grid_->addWidget (del_label, 0, 4);

    unsigned int row = 1;

    QIcon edit_icon(Files::getIconFilepath("edit.png").c_str());
    QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    edit_table_buttons_.clear();
    delete_table_buttons_.clear();

    for (auto it : schema_.tables())
    {
        QLabel* name = new QLabel(it.first.c_str());
        table_grid_->addWidget(name, row, 0);

        //        QLabel *numel = new QLabel ((std::to_string(it.second->numColumns())).c_str());
        //        table_grid_->addWidget (numel, row, 1);

        QLabel* key = new QLabel(it.second->key().c_str());
        table_grid_->addWidget(key, row, 1);

        QPushButton* edit = new QPushButton();
        edit->setIcon(edit_icon);
        edit->setIconSize(UI_ICON_SIZE);
        edit->setMaximumWidth(UI_ICON_BUTTON_MAX_WIDTH);
        edit->setFlat(UI_ICON_BUTTON_FLAT);
        connect(edit, SIGNAL(clicked()), this, SLOT(editTableSlot()));
        table_grid_->addWidget(edit, row, 2);
        edit_table_buttons_[edit] = it.second;

        QPushButton* del = new QPushButton();
        del->setIcon(del_icon);
        del->setIconSize(UI_ICON_SIZE);
        del->setMaximumWidth(UI_ICON_BUTTON_MAX_WIDTH);
        del->setFlat(UI_ICON_BUTTON_FLAT);
        connect(del, SIGNAL(clicked()), this, SLOT(deleteTableSlot()));
        table_grid_->addWidget(del, row, 3);
        delete_table_buttons_[del] = it.second;

        row++;
    }
}

void DBSchemaWidget::updateMetaTableGrid()
{
    logdbg << "DBSchemaWidget: updateMetaTablesGrid";
    QLayoutItem* child;
    while ((child = meta_table_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }
    edit_meta_table_buttons_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* name_label = new QLabel("Name");
    name_label->setFont(font_bold);
    meta_table_grid_->addWidget(name_label, 0, 0);

    QLabel* db_name_label = new QLabel("Main Table");
    db_name_label->setFont(font_bold);
    meta_table_grid_->addWidget(db_name_label, 0, 1);

    //    QLabel *subtables_label = new QLabel ("Sub tables");
    //    subtables_label->setFont (font_bold);
    //    meta_table_grid_->addWidget (subtables_label, 0, 2);

    //    QLabel *numcols_label = new QLabel ("#columns");
    //    numcols_label->setFont (font_bold);
    //    meta_table_grid_->addWidget (numcols_label, 0, 2);

    //    QLabel *edit_label = new QLabel ("Edit");
    //    edit_label->setFont (font_bold);
    //    edit_label->setAlignment(Qt::AlignCenter);
    //    meta_table_grid_->addWidget (edit_label, 0, 3);

    //    QLabel *del_label = new QLabel ("Delete");
    //    del_label->setFont (font_bold);
    //    del_label->setAlignment(Qt::AlignCenter);
    //    meta_table_grid_->addWidget (del_label, 0, 4);

    unsigned int row = 1;

    QIcon edit_icon(Files::getIconFilepath("edit.png").c_str());
    QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    edit_meta_table_buttons_.clear();
    delete_meta_table_buttons_.clear();

    for (auto it : schema_.metaTables())
    {
        QLabel* name = new QLabel(it.second->name().c_str());
        meta_table_grid_->addWidget(name, row, 0);

        QLabel* db_name = new QLabel(it.second->mainTableName().c_str());
        meta_table_grid_->addWidget(db_name, row, 1);

        //        QLabel *sub = new QLabel ("None");
        //        sub->setText (it.second->subTableNames().c_str());
        //        meta_table_grid_->addWidget (sub, row, 2);

        //        QLabel *numcols = new QLabel (std::to_string(it.second->numColumns()).c_str());
        //        meta_table_grid_->addWidget (numcols, row, 2);

        QPushButton* edit = new QPushButton();
        edit->setIcon(edit_icon);
        edit->setIconSize(UI_ICON_SIZE);
        edit->setMaximumWidth(UI_ICON_BUTTON_MAX_WIDTH);
        edit->setFlat(UI_ICON_BUTTON_FLAT);
        connect(edit, SIGNAL(clicked()), this, SLOT(editMetaTableSlot()));
        meta_table_grid_->addWidget(edit, row, 2);
        edit_meta_table_buttons_[edit] = it.second;

        QPushButton* del = new QPushButton();
        del->setIcon(del_icon);
        del->setIconSize(UI_ICON_SIZE);
        del->setMaximumWidth(UI_ICON_BUTTON_MAX_WIDTH);
        del->setFlat(UI_ICON_BUTTON_FLAT);
        connect(del, SIGNAL(clicked()), this, SLOT(deleteMetaTableSlot()));
        meta_table_grid_->addWidget(del, row, 3);
        delete_meta_table_buttons_[del] = it.second;
        row++;
    }
}

void DBSchemaWidget::editTableSlot()
{
    logdbg << "DBSchemaWidget: editTable";

    QPushButton* sender = dynamic_cast<QPushButton*>(QObject::sender());
    assert(edit_table_buttons_.count(sender) == 1);
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

void DBSchemaWidget::deleteTableSlot()
{
    logdbg << "DBSchemaWidget: deleteTable";

    QPushButton* sender = dynamic_cast<QPushButton*>(QObject::sender());
    assert(delete_table_buttons_.count(sender) == 1);
    schema_.deleteTable(delete_table_buttons_.at(sender)->name());

    updateTableGrid();
    updateMetaTableGrid();
}

void DBSchemaWidget::editMetaTableSlot()
{
    logdbg << "DBSchemaWidget: editMetaTableSlot";

    QPushButton* sender = dynamic_cast<QPushButton*>(QObject::sender());
    assert(edit_meta_table_buttons_.count(sender) == 1);
    edit_meta_table_buttons_.at(sender)->widget()->show();

    //    assert (edit_meta_table_buttons_.find((QPushButton*)sender()) !=
    //    edit_meta_table_buttons_.end());

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

void DBSchemaWidget::deleteMetaTableSlot()
{
    logdbg << "DBSchemaWidget: deleteTable";

    QPushButton* sender = dynamic_cast<QPushButton*>(QObject::sender());
    assert(delete_meta_table_buttons_.count(sender) == 1);
    schema_.deleteMetaTable(delete_meta_table_buttons_.at(sender)->name());

    updateMetaTableGrid();
}

void DBSchemaWidget::changedTableSlot()
{
    schema_.updateTables();
    updateTableGrid();
}

void DBSchemaWidget::changedMetaTableSlot()
{
    schema_.updateMetaTables();
    updateMetaTableGrid();
}
