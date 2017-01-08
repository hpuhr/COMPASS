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
 * DBTableEditWidget.cpp
 *
 *  Created on: Aug 21, 2012
 *      Author: sk
 */

#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>

#include "Buffer.h"
#include "ConfigurationManager.h"
#include "Configuration.h"
#include "DBTableEditWidget.h"
#include "DBTable.h"
#include "DBTableColumn.h"
#include "ATSDB.h"
#include "Logger.h"
#include "UnitSelectionWidget.h"

#include "String.h"

using namespace Utils;

DBTableEditWidget::DBTableEditWidget(DBTable *table, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), table_(table), name_edit_ (0), db_name_box_ (0), info_edit_(0), key_box_ (0), column_grid_(0)
{
    assert (table_);

    setMinimumSize(QSize(800, 600));

    createElements ();

    show();
}

DBTableEditWidget::~DBTableEditWidget()
{
}

void DBTableEditWidget::createElements ()
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

    name_edit_ = new QLineEdit (table_->getName().c_str());
    connect(name_edit_, SIGNAL( returnPressed() ), this, SLOT( updateParameters() ));
    properties_layout->addWidget (name_edit_, 0, 1);

    QLabel *info_label = new QLabel ("Description");
    properties_layout->addWidget (info_label, 1, 0);

    info_edit_ = new QLineEdit (table_->getInfo().c_str());
    connect(info_edit_, SIGNAL( returnPressed() ), this, SLOT( updateParameters() ));
    properties_layout->addWidget (info_edit_, 1, 1);

    QLabel *dbname_label = new QLabel ("Database table name");
    properties_layout->addWidget (dbname_label, 2, 0);

    db_name_box_ = new QComboBox ();
    updateTableSelection ();
    connect(db_name_box_, SIGNAL( activated(const QString &) ), this, SLOT( updateParameters() ));
    properties_layout->addWidget (db_name_box_, 2, 1);


    QLabel *key_label = new QLabel ("Primary key");
    properties_layout->addWidget (key_label, 3, 0);

    key_box_ = new QComboBox ();
    updateKeySelection();
    connect(key_box_, SIGNAL( activated(const QString &) ), this, SLOT( updateParameters() ));
    properties_layout->addWidget(key_box_, 3, 1);

    main_layout->addLayout (properties_layout);

    QLabel *column_label = new QLabel ("Table columns");
    column_label->setFont (font_big);
    main_layout->addWidget (column_label);

    QFrame *colum_frame = new QFrame ();
    colum_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    colum_frame->setLineWidth(frame_width);

    column_grid_ = new QGridLayout ();
    updateColumnGrid ();

    colum_frame->setLayout (column_grid_);

    QScrollArea *column_scroll = new QScrollArea ();
    column_scroll->setWidgetResizable (true);
    column_scroll->setWidget(colum_frame);


    main_layout->addWidget (column_scroll);

    QPushButton *create_columns = new QPushButton ("Create columns from Database");
    connect(create_columns, SIGNAL( clicked() ), this, SLOT( createColumnsFromDB() ));
    create_columns->setDisabled (table_->getNumColumns() != 0);
    main_layout->addWidget (create_columns);

    QPushButton *create_new_columns = new QPushButton ("Create new columns from Database");
    connect(create_new_columns, SIGNAL( clicked() ), this, SLOT( createNewColumnsFromDB() ));
    //create_columns->setDisabled (table_->getNumColumns() != 0);
    main_layout->addWidget (create_new_columns);

    main_layout->addStretch ();

    setLayout (main_layout);
}

void DBTableEditWidget::deleteColumn ()
{
    assert (table_);
    QPushButton *button = (QPushButton*)sender();
    assert (column_grid_delete_buttons_.find(button) != column_grid_delete_buttons_.end());
    table_->deleteColumn (column_grid_delete_buttons_[button]->getName());
    updateColumnGrid();
}

void DBTableEditWidget::updateParameters ()
{
    assert (name_edit_);
    assert (db_name_box_);
    assert (info_edit_);
    assert (key_box_);

    table_->setName (name_edit_->text().toStdString());
    table_->setDBName (db_name_box_->currentText().toStdString());
    table_->setInfo (info_edit_->text().toStdString());
    table_->setKey (key_box_->currentText().toStdString());

    emit changedTable();
}

void DBTableEditWidget::updateTableSelection()
{
    assert (ATSDB::getInstance().getDBOpened ());
    Buffer *tables = ATSDB::getInstance().getTableList ();

    std::string dbname = table_->getDBName();

    if (tables->firstWrite())
    {
        delete tables;
        return;
    }

    assert (db_name_box_);
    while (db_name_box_->count() > 0)
        db_name_box_->removeItem (0);

    // TODO FIX READING
    assert (false);

//    tables->setIndex(0);
//    std::string table_name;

//    int index=-1;
//    for (unsigned int cnt=0; cnt < tables->getSize(); cnt++)
//    {
//        if (cnt != 0)
//            tables->incrementIndex();

//        table_name = *(std::string *) tables->get(0);
//        db_name_box_->addItem (table_name.c_str());

//        if (table_name.compare(dbname) == 0)
//            index=cnt;
//    }

//    if (index != -1)
//    {
//        db_name_box_->setCurrentIndex (index);
//    }

//    delete tables;
}

void DBTableEditWidget::updateKeySelection()
{
    assert (ATSDB::getInstance().getDBOpened ());
    Buffer *columns = ATSDB::getInstance().getColumnList (table_->getDBName());

    if (columns->firstWrite())
    {
        delete columns;
        return;
    }

    assert (key_box_);
    while (key_box_->count() > 0)
        key_box_->removeItem (0);

    // TODO FIX READING
    assert (false);

//    columns->setIndex(0);
//    std::string column_name;
//    bool key;

//    PropertyList *list = columns->getPropertyList();
//    unsigned int name_index = list->getPropertyIndex ("name");
//    unsigned int key_index = list->getPropertyIndex ("key");

//    key_box_->addItem ("");

//    bool found_key = false;
//    for (unsigned int cnt=0; cnt < columns->getSize(); cnt++)
//    {
//        if (cnt != 0)
//            columns->incrementIndex();

//        column_name = *(std::string *) columns->get(name_index);
//        key = *(bool *) columns->get(key_index);

//        if (key)
//        {
//            key_box_->addItem (column_name.c_str());
//            found_key=true;
//        }

//    }

//    if (found_key)
//    {
//        key_box_->setCurrentIndex (1);
//    }
//    else // no key defined
//    {
//        columns->setIndex(0);
//        for (unsigned int cnt=0; cnt < columns->getSize(); cnt++)
//        {
//            if (cnt != 0)
//                columns->incrementIndex();

//            column_name = *(std::string *) columns->get(name_index);

//                key_box_->addItem (column_name.c_str());
//        }
//    }

//    delete columns;
}

void DBTableEditWidget::updateColumnGrid ()
{
    assert (column_grid_);

    QLayoutItem *child;
    while ((child = column_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    column_grid_delete_buttons_.clear();
    column_grid_special_nulls_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *name_label = new QLabel ("Name");
    name_label->setFont(font_bold);
    column_grid_->addWidget (name_label, 0,1);

    QLabel *type_label = new QLabel ("Data type");
    type_label->setFont(font_bold);
    column_grid_->addWidget (type_label, 0,2);

    QLabel *key_label = new QLabel ("Is key");
    key_label->setFont(font_bold);
    column_grid_->addWidget (key_label, 0,3);

    QLabel *unit_label = new QLabel ("Unit");
    unit_label->setFont(font_bold);
    column_grid_->addWidget (unit_label, 0,4);

    QLabel *null_label = new QLabel ("Special null");
    null_label->setFont(font_bold);
    column_grid_->addWidget (null_label, 0,5);

    assert (table_);

    std::map <std::string, DBTableColumn *> &columns = table_->getColumns ();
    std::map <std::string, DBTableColumn *>::iterator it;

    unsigned int row=1;
    QPixmap* pixmapmanage = new QPixmap("./Data/icons/close_icon.png");

    for (it = columns.begin(); it != columns.end(); it++)
    {
        QPushButton *del = new QPushButton ();
        del->setIcon(QIcon(*pixmapmanage));
        del->setFixedSize ( 20, 20 );
        del->setFlat(true);
        connect(del, SIGNAL( clicked() ), this, SLOT( deleteColumn() ));
        column_grid_delete_buttons_ [del] = it->second;
        column_grid_->addWidget (del, row, 0);

        QLabel *name = new QLabel (it->second->getName().c_str());
        column_grid_->addWidget (name, row,1);

        QLabel *type = new QLabel (it->second->getType().c_str());
        column_grid_->addWidget (type, row,2);

        QLabel *key = new QLabel (String::intToString((int)it->second->isKey()).c_str());
        column_grid_->addWidget (key, row,3);

        UnitSelectionWidget *unit_widget = new UnitSelectionWidget (it->second->getUnitDimension (), it->second->getUnitUnit());
        column_grid_->addWidget (unit_widget, row,4);

        QLineEdit *edit = new QLineEdit (it->second->getSpecialNull().c_str());
        connect (edit, SIGNAL(returnPressed()), this, SLOT (setSpecialNull()));
        column_grid_->addWidget (edit, row,5);
        assert (column_grid_special_nulls_.find (edit) == column_grid_special_nulls_.end());
        column_grid_special_nulls_ [edit] = it->second;

        row++;
    }
}

void DBTableEditWidget::createColumnsFromDB ()
{
    assert (table_->getNumColumns() == 0);

    assert (ATSDB::getInstance().getDBOpened ());
    Buffer *columns = ATSDB::getInstance().getColumnList (table_->getDBName());

    if (columns->firstWrite())
    {
        delete columns;
        return;
    }

    // TODO FIX READING
    assert (false);

//    columns->setIndex(0);
//    std::string column_name;
//    std::string type_name;
//    bool key;

//    PropertyList *list = columns->getPropertyList();
//    unsigned int name_index = list->getPropertyIndex ("name");
//    unsigned int type_index = list->getPropertyIndex ("type");
//    unsigned int key_index = list->getPropertyIndex ("key");

//    for (unsigned int cnt=0; cnt < columns->getSize(); cnt++)
//    {
//        if (cnt != 0)
//            columns->incrementIndex();

//        column_name = *(std::string *) columns->get(name_index);
//        type_name = *(std::string *) columns->get(type_index);
//        key = *(bool *) columns->get(key_index);

//        std::string instance_id = "DBTableColumn"+table_->getDBName()+column_name+"0";

//        Configuration &configuration = table_->addNewSubConfiguration ("DBTableColumn", instance_id);
//        configuration.addParameterString ("name", column_name);
//        configuration.addParameterString ("type", type_name);
//        configuration.addParameterBool ("is_key", key);
//        table_->generateSubConfigurable ("DBTableColumn", instance_id);
//    }

//    delete columns;

    updateColumnGrid ();

    emit changedTable();
}

void DBTableEditWidget::createNewColumnsFromDB ()
{
    //assert (table_->getNumColumns() == 0);

    assert (ATSDB::getInstance().getDBOpened ());
    Buffer *columns = ATSDB::getInstance().getColumnList (table_->getDBName());

    if (columns->firstWrite())
    {
        delete columns;
        return;
    }

    // TODO FIX READING
    assert (false);

//    columns->setIndex(0);
//    std::string column_name;
//    std::string type_name;
//    bool key;

//    PropertyList *list = columns->getPropertyList();
//    unsigned int name_index = list->getPropertyIndex ("name");
//    unsigned int type_index = list->getPropertyIndex ("type");
//    unsigned int key_index = list->getPropertyIndex ("key");

//    for (unsigned int cnt=0; cnt < columns->getSize(); cnt++)
//    {
//        if (cnt != 0)
//            columns->incrementIndex();

//        column_name = *(std::string *) columns->get(name_index);
//        type_name = *(std::string *) columns->get(type_index);
//        key = *(bool *) columns->get(key_index);

//        if (table_->hasTableColumn(column_name))
//            continue;

//        std::string instance_id = "DBTableColumn"+table_->getDBName()+column_name+"0";

//        Configuration &configuration = table_->addNewSubConfiguration ("DBTableColumn", instance_id);
//        configuration.addParameterString ("name", column_name);
//        configuration.addParameterString ("type", type_name);
//        configuration.addParameterBool ("is_key", key);
//        table_->generateSubConfigurable ("DBTableColumn", instance_id);
//    }

//    delete columns;

    updateColumnGrid ();

    emit changedTable();
}

void DBTableEditWidget::setSpecialNull ()
{
    loginf << "DBTableEditWidget: setSpecialNull";

    QLineEdit *edit = (QLineEdit*) sender();

    assert (column_grid_special_nulls_.find (edit) != column_grid_special_nulls_.end());

    column_grid_special_nulls_[edit]->setSpecialNull(edit->text().toStdString());
}
