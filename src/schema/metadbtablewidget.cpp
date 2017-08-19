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
#include <QMessageBox>
#include <QSettings>

#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
#include "metadbtablewidget.h"
#include "metadbtable.h"
#include "logger.h"
#include "stringconv.h"

MetaDBTableWidget::MetaDBTableWidget(MetaDBTable &meta_table, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), meta_table_ (meta_table), name_edit_ (0), info_edit_(0), key_box_(0), //table_box_ (0)
  sub_tables_grid_ (0), column_grid_(0), new_local_key_ (0), new_table_(0), new_sub_key_(0)
{
    QSettings settings("ATSDB", "MetaDBTableWidget");
    restoreGeometry(settings.value("MetaDBTableWidget/geometry").toByteArray());

    setMinimumSize(QSize(800, 600));

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

    name_edit_ = new QLineEdit (meta_table_.name().c_str());
    connect(name_edit_, SIGNAL(textChanged(const QString &)), this, SLOT( editNameSlot(const QString &) ));
    properties_layout->addWidget (name_edit_, 0, 1);

    QLabel *info_label = new QLabel ("Description");
    properties_layout->addWidget (info_label, 1, 0);

    info_edit_ = new QLineEdit (meta_table_.info().c_str());
    connect(info_edit_, SIGNAL(textChanged(const QString &)), this, SLOT( editInfoSlot(const QString &) ));
    properties_layout->addWidget (info_edit_, 1, 1);

//    QLabel *table_label = new QLabel ("Table name");
//    properties_layout->addWidget (table_label, 2, 0);

//    table_box_ = new QComboBox ();
//    updateTableSelection ();
//    connect(table_box_, SIGNAL( activated(const QString &) ), this, SLOT( selectTable() ));
//    properties_layout->addWidget (table_box_, 2, 1);

    main_layout->addLayout (properties_layout);

    QLabel *sub_tables_label = new QLabel ("Sub tables");
    sub_tables_label->setFont (font_big);
    main_layout->addWidget (sub_tables_label);

    QFrame *sub_tables_frame = new QFrame ();
    sub_tables_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    sub_tables_frame->setLineWidth(frame_width);

    sub_tables_grid_ = new QGridLayout ();
    updateSubTablesGridSlot ();

    sub_tables_frame->setLayout (sub_tables_grid_);

    QScrollArea *sub_tables_scroll = new QScrollArea ();
    sub_tables_scroll->setWidgetResizable (true);
    sub_tables_scroll->setWidget(sub_tables_frame);

    main_layout->addWidget (sub_tables_scroll);

    // add new sub structures

    QVBoxLayout *new_sub_table_layout = new QVBoxLayout ();

    QLabel *new_sub_table_label = new QLabel ("New sub table");
    new_sub_table_label->setFont (font_bold);
    new_sub_table_layout->addWidget (new_sub_table_label);

    QGridLayout *new_grid = new QGridLayout ();

    QLabel *local_key_label = new QLabel ("Local key");
    new_grid->addWidget (local_key_label,0,0);

    new_local_key_ = new QComboBox ();
    updateLocalKeySelectionSlot ();
    new_grid->addWidget (new_local_key_,1,0);

    QLabel *new_table_label = new QLabel ("Sub table");
    new_grid->addWidget (new_table_label,0,1);

    new_table_ = new QComboBox ();
    updateNewSubTableSelectionSlot();
    connect(new_table_, SIGNAL( activated(const QString &) ), this, SLOT( updateSubKeySelectionSlot() ));
    new_grid->addWidget (new_table_,1,1);

    QLabel *subkey_label = new QLabel ("Sub table key");
    new_grid->addWidget (subkey_label,0,2);

    new_sub_key_ = new QComboBox ();
    updateSubKeySelectionSlot();
    new_grid->addWidget (new_sub_key_,1,2);

    QPushButton *new_struct_add = new QPushButton ("Add");
    connect(new_struct_add, SIGNAL( clicked() ), this, SLOT( addSubTableSlot() ));
    new_grid->addWidget (new_struct_add, 1, 3);

    new_sub_table_layout->addLayout (new_grid);
    main_layout->addLayout (new_sub_table_layout);

    // columns list
    QFrame *columns_frame = new QFrame ();
    columns_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    columns_frame->setLineWidth(frame_width);

    column_grid_ = new QGridLayout ();
    updateColumnsGridSlot ();

    columns_frame->setLayout (column_grid_);

    QScrollArea *columns_scroll = new QScrollArea ();
    columns_scroll->setWidgetResizable (true);
    columns_scroll->setWidget(columns_frame);

    main_layout->addWidget (columns_scroll);

    //main_layout->addStretch ();

    setLayout (main_layout);

    show();
}

MetaDBTableWidget::~MetaDBTableWidget()
{
    QSettings settings("ATSDB", "MetaDBTableWidget");
    settings.setValue("MetaDBTableWidget/geometry", saveGeometry());
}

void MetaDBTableWidget::addSubTableSlot ()
{
    assert (new_local_key_);
    assert (new_table_);
    assert (new_sub_key_);

    std::string local_key = new_local_key_->currentText().toStdString();
    std::string sub_table_name = new_table_->currentText().toStdString();
    std::string sub_table_key = new_sub_key_->currentText().toStdString();

    if (sub_table_name.size() > 0 && !meta_table_.hasSubTable(sub_table_name))
    {
        meta_table_.addSubTable (local_key, sub_table_name, sub_table_key);
        updateSubTablesGridSlot();
        emit changedMetaTable();
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.exec();
    }
}

void MetaDBTableWidget::editNameSlot (const QString &text)
{
    meta_table_.name (text.toStdString());
    emit changedMetaTable();
}
void MetaDBTableWidget::editInfoSlot (const QString &text)
{
    assert (info_edit_);
    meta_table_.info (text.toStdString());
    emit changedMetaTable();
}

//void MetaDBTableWidget::selectTable ()
//{
//    assert (table_box_);
//    meta_table_->setTableName (table_box_->currentText().toStdString());
//    updateLocalKeySelection ();
//    emit changedMetaTable();
//}

//void MetaDBTableWidget::updateTableSelection()
//{
//    loginf  << "MetaDBTableEditWidget: updateTableSelection";

//    assert (table_box_);

//    std::string selection;
//    if (table_box_->count() > 0)
//        selection = table_box_->currentText().toStdString();
//    else
//        selection = meta_table_->getTableName();

//    while (table_box_->count() > 0)
//        table_box_->removeItem (0);

//    std::map <std::string, DBTable*> &tables = DBSchemaManager::getInstance().getCurrentSchema()->getTables ();
//    std::map <std::string, DBTable*>::iterator it;


//    int index_cnt=-1;
//    int cnt=0;
//    for (it = tables.begin(); it != tables.end(); it++)
//    {
//        table_box_->addItem (it->second->getName().c_str());

//        if (selection.size() > 0 && it->second->getName().compare(selection) == 0)
//            index_cnt=cnt;

//        cnt++;
//    }

//    if (index_cnt != -1)
//    {
//        table_box_->setCurrentIndex (index_cnt);
//    }

//}

void MetaDBTableWidget::updateNewSubTableSelectionSlot()
{
    logdbg  << "MetaDBTableEditWidget: updateNewSubTableSelection";

    assert (new_table_);

    std::string selection;
    if (new_table_->count() > 0)
        selection = new_table_->currentText().toStdString();

    while (new_table_->count() > 0)
        new_table_->removeItem (0);

    int index_cnt=-1;
    int cnt=0;
    for (auto it : meta_table_.schema().tables())
    {
        if (meta_table_.hasSubTable(it.first))
            continue;

        new_table_->addItem (it.second->name().c_str());

        if (selection.size() > 0 && it.second->name().compare(selection) == 0)
            index_cnt=cnt;

        cnt++;
    }

    if (index_cnt != -1)
    {
        new_table_->setCurrentIndex (index_cnt);
    }

}

void MetaDBTableWidget::updateLocalKeySelectionSlot ()
{
    //assert (table_box_);
    assert (new_local_key_);

//    if (table_box_->count() == 0)
//        return;

//    std::string tablename = meta_table_.mainTableName();
//    assert (tablename.size() > 0);

//    DBSchema *schema = DBSchemaManager::getInstance().getCurrentSchema();
//    assert (schema->hasTable (tablename));
//    DBTable *table = schema->getTable (tablename);

    std::string selection;
    if (new_local_key_->count() > 0)
        selection = new_local_key_->currentText().toStdString();

    while (new_local_key_->count() > 0)
        new_local_key_->removeItem (0);


//    std::map <std::string, DBTableColumn *> &columns = table->getColumns ();
//    std::map <std::string, DBTableColumn *>::iterator it;

    //loginf  << "MetaDBTable: updateSubKeySelection: " << columns.size();

    int index_cnt=-1;
    int cnt=0;
    for (auto it : meta_table_.mainTable().columns())
    {
        std::string name = it.first;

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

void MetaDBTableWidget::updateSubKeySelectionSlot ()
{
    assert (new_table_);
    assert (new_sub_key_);

    if (new_table_->count() == 0)
        return;

    std::string metatablename = new_table_->currentText().toStdString();
    assert (metatablename.size() > 0);

    logdbg << "MetaDBTableWidget: updateSubKeySelectionSlot: new table '" << metatablename << "'";

    const DBSchema &schema = meta_table_.schema();
    assert (schema.hasTable (metatablename));
    const DBTable &metatable = schema.table(metatablename);

    std::string selection;
    if (new_sub_key_->count() > 0)
        selection = new_sub_key_->currentText().toStdString();

    while (new_sub_key_->count() > 0)
        new_sub_key_->removeItem (0);


    //loginf  << "MetaDBTable: updateSubKeySelection: " << columns.size();

    int index_cnt=-1;
    int cnt=0;
    for (auto it : metatable.columns())
    {
        std::string name = it.second->name();

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

void MetaDBTableWidget::updateSubTablesGridSlot ()
{
    assert (sub_tables_grid_);

    QLayoutItem *child;
    while ((child = sub_tables_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *local_key_label = new QLabel ("Main table key");
    local_key_label->setFont(font_bold);
    sub_tables_grid_->addWidget (local_key_label, 0,0);

    QLabel *sub_table_name_label = new QLabel ("Sub table name");
    sub_table_name_label->setFont(font_bold);
    sub_tables_grid_->addWidget (sub_table_name_label, 0,1);

    QLabel *sub_table_key_label = new QLabel ("Sub table key");
    sub_table_key_label->setFont(font_bold);
    sub_tables_grid_->addWidget (sub_table_key_label, 0,2);

    unsigned int row=1;

    for (auto it : meta_table_.subTableDefinitions())
    {
        QLabel *local_key = new QLabel (it.second->mainTableKey().c_str());
        sub_tables_grid_->addWidget (local_key, row,0);

        QLabel *table = new QLabel (it.second->subTableName().c_str());
        sub_tables_grid_->addWidget (table, row,1);

        QLabel *sub_key = new QLabel (it.second->subTableKey().c_str());
        sub_tables_grid_->addWidget (sub_key, row,2);

//        QPushButton *edit = new QPushButton ("Edit");
//        connect(edit, SIGNAL( clicked() ), this, SLOT( editSubMetaTable() ));
//        sub_tables_grid_->addWidget (edit, row, 4);
//        edit_sub_meta_table_buttons_ [edit] = it->second;
        row++;
    }
}

void MetaDBTableWidget::updateColumnsGridSlot ()
{
    assert (column_grid_);

    QLayoutItem *child;
    while ((child = column_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *name_label = new QLabel ("Name");
    name_label->setFont(font_bold);
    column_grid_->addWidget (name_label, 0,0);

    QLabel *type_label = new QLabel ("Data type");
    type_label->setFont(font_bold);
    column_grid_->addWidget (type_label, 0,1);

    QLabel *key_label = new QLabel ("Is key");
    key_label->setFont(font_bold);
    column_grid_->addWidget (key_label, 0,2);

    QLabel *unit_label = new QLabel ("Unit");
    unit_label->setFont(font_bold);
    column_grid_->addWidget (unit_label, 0,3);

//    QLabel *null_label = new QLabel ("Special null"); //TODO
//    null_label->setFont(font_bold);
//    column_grid_->addWidget (null_label, 0,4);

    QLabel *comment_label = new QLabel ("Comment");
    comment_label->setFont(font_bold);
    column_grid_->addWidget (comment_label, 0,4);


    unsigned int row=1;
    for (auto it : meta_table_.columns ())
    {
        QLabel *name = new QLabel (it.second.name().c_str());
        column_grid_->addWidget (name, row, 0);

        QLabel *type = new QLabel (it.second.type().c_str());
        column_grid_->addWidget (type, row, 1);

        QLabel *key = new QLabel (QString::number(it.second.isKey()));
        column_grid_->addWidget (key, row, 2);

        QLabel *unit = new QLabel ((it.second.unit()+":"+it.second.dimension()).c_str());
        column_grid_->addWidget (unit, row, 3);

//        QLabel *edit = new QLabel (it.second.specialNull().c_str());
//        column_grid_->addWidget (edit, row, 4);

        QLabel *comment = new QLabel (it.second.comment().c_str());
        column_grid_->addWidget (comment, row, 4);

        row++;
    }
}

//void MetaDBTableWidget::editSubMetaTable ()
//{
//    assert (edit_sub_meta_table_buttons_.find((QPushButton*)sender()) != edit_sub_meta_table_buttons_.end());

//    MetaDBTable *table_structure = edit_sub_meta_table_buttons_ [(QPushButton*)sender()];

//    if (edit_sub_meta_table_widgets_.find (table_structure) == edit_sub_meta_table_widgets_.end())
//    {
//        MetaDBTableWidget *widget = new MetaDBTableWidget (table_structure);
//        edit_sub_meta_table_widgets_[table_structure] = widget;
//    }
//    else
//        edit_sub_meta_table_widgets_[table_structure]->show();
//}
