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
 * DBTableWidget.cpp
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

//#include "ConfigurationManager.h"
#include "configuration.h"
#include "dbtablewidget.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
//#include "ATSDB.h"
#include "logger.h"
//#include "UnitSelectionWidget.h"

#include "stringconv.h"

using namespace Utils;

DBTableWidget::DBTableWidget(DBTable &table, QWidget * parent, Qt::WindowFlags f)
: QWidget (parent, f), table_(table), info_edit_(0), key_box_ (0), column_grid_(0)
{
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

    QLabel *info_label = new QLabel ("Description");
    properties_layout->addWidget (info_label, 0, 0);

    info_edit_ = new QLineEdit (table_.info().c_str());
    connect(info_edit_, SIGNAL( textChanged(const QString &) ), this, SLOT( infoSlot(const QString &) ));
    properties_layout->addWidget (info_edit_, 0, 1);

    QLabel *key_label = new QLabel ("Primary key");
    properties_layout->addWidget (key_label, 1, 0);

    key_box_ = new QComboBox ();
    updateKeySelection();
    connect(key_box_, SIGNAL( currentIndexChanged(const QString & text) ), this, SLOT( keySlot(const QString &) ));
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
    create_columns->setDisabled (table_.numColumns() != 0);
    main_layout->addWidget (create_columns);

    QPushButton *create_new_columns = new QPushButton ("Create new columns from Database");
    connect(create_new_columns, SIGNAL( clicked() ), this, SLOT( createNewColumnsFromDB() ));
    //create_columns->setDisabled (table_.getNumColumns() != 0);
    main_layout->addWidget (create_new_columns);

    main_layout->addStretch ();

    setLayout (main_layout);

    show();
}

DBTableWidget::~DBTableWidget()
{
}

void DBTableWidget::infoSlot (const QString &value)
{
    table_.info (value.toStdString());
}

void DBTableWidget::keySlot (const QString &value)
{
    table_.key (value.toStdString());
}

void DBTableWidget::deleteColumn ()
{
    QPushButton *button = (QPushButton*)sender();
    assert (column_grid_delete_buttons_.find(button) != column_grid_delete_buttons_.end());
    table_.deleteColumn (column_grid_delete_buttons_[button]->name());
    updateColumnGrid();
}


void DBTableWidget::updateKeySelection()
{

}

void DBTableWidget::updateColumnGrid ()
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

    unsigned int row=1;
    QPixmap del_pixmap ("./data/icons/delete.png");
    QIcon del_icon (del_pixmap);

    for (auto it : table_.columns ())
    {
        QPushButton *del = new QPushButton ();
        del->setIcon(del_icon);
        del->setFlat(true);
        del->setIconSize( QSize(30, 30) );

        connect(del, SIGNAL( clicked() ), this, SLOT( deleteColumn() ));
        column_grid_delete_buttons_ [del] = it.second;
        column_grid_->addWidget (del, row, 0);

        QLabel *name = new QLabel (it.second->name().c_str());
        column_grid_->addWidget (name, row,1);

        QLabel *type = new QLabel (it.second->type().c_str());
        column_grid_->addWidget (type, row,2);

        QLabel *key = new QLabel (String::intToString((int)it.second->isKey()).c_str());
        column_grid_->addWidget (key, row,3);

//        UnitSelectionWidget *unit_widget = new UnitSelectionWidget (it->second->getUnitDimension (), it->second->getUnitUnit());
//        column_grid_->addWidget (unit_widget, row,4);

        QLineEdit *edit = new QLineEdit (it.second->specialNull().c_str());
        connect (edit, SIGNAL(returnPressed()), this, SLOT (setSpecialNull()));
        column_grid_->addWidget (edit, row,5);
        assert (column_grid_special_nulls_.find (edit) == column_grid_special_nulls_.end());
        column_grid_special_nulls_ [edit] = it.second;

        row++;
    }
}

void DBTableWidget::createColumnsFromDB ()
{
//    assert (table_.getNumColumns() == 0);

//    assert (ATSDB::getInstance().getDBOpened ());
//    Buffer *columns = ATSDB::getInstance().getColumnList (table_.getDBName());

//    if (columns->firstWrite())
//    {
//        delete columns;
//        return;
//    }

//    // TODO FIX READING
//    assert (false);

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

//        std::string instance_id = "DBTableColumn"+table_.getDBName()+column_name+"0";

//        Configuration &configuration = table_.addNewSubConfiguration ("DBTableColumn", instance_id);
//        configuration.addParameterString ("name", column_name);
//        configuration.addParameterString ("type", type_name);
//        configuration.addParameterBool ("is_key", key);
//        table_.generateSubConfigurable ("DBTableColumn", instance_id);
//    }

//    delete columns;

    updateColumnGrid ();

    //emit changedTable();
}

void DBTableWidget::createNewColumnsFromDB ()
{
//    //assert (table_.getNumColumns() == 0);

//    assert (ATSDB::getInstance().getDBOpened ());
//    Buffer *columns = ATSDB::getInstance().getColumnList (table_.getDBName());

//    if (columns->firstWrite())
//    {
//        delete columns;
//        return;
//    }

//    // TODO FIX READING
//    assert (false);

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

//        if (table_.hasTableColumn(column_name))
//            continue;

//        std::string instance_id = "DBTableColumn"+table_.getDBName()+column_name+"0";

//        Configuration &configuration = table_.addNewSubConfiguration ("DBTableColumn", instance_id);
//        configuration.addParameterString ("name", column_name);
//        configuration.addParameterString ("type", type_name);
//        configuration.addParameterBool ("is_key", key);
//        table_.generateSubConfigurable ("DBTableColumn", instance_id);
//    }

//    delete columns;

    updateColumnGrid ();

    //emit changedTable();
}

void DBTableWidget::setSpecialNull ()
{
    loginf << "DBTableWidget: setSpecialNull";

    QLineEdit *edit = (QLineEdit*) sender();

    assert (column_grid_special_nulls_.find (edit) != column_grid_special_nulls_.end());

    column_grid_special_nulls_[edit]->specialNull(edit->text().toStdString());
}
