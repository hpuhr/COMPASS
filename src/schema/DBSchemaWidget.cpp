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

#include <QLabel>
#include <QRadioButton>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "DBSchema.h"
#include "DBSchemaWidget.h"
#include "DBSchemaEditWidget.h"
#include "DBSchemaManager.h"
#include "Logger.h"

DBSchemaWidget::DBSchemaWidget(QWidget * parent, Qt::WindowFlags f)
: QFrame (parent, f), current_schema_select_(0), new_schema_name_edit_(0), edit_widget_(0),
  edit_button_(0), add_button_ (0)
{
    createElements();
}

DBSchemaWidget::~DBSchemaWidget()
{
    if (edit_widget_)
    {
        edit_widget_->close();
        delete edit_widget_;
        edit_widget_=0;
    }
}

void DBSchemaWidget::updateSchemaCombo()
{
    assert (current_schema_select_ != 0);

    while (current_schema_select_->count() != 0)
        current_schema_select_->removeItem (0);

    std::string current_schema;
    bool exists_current_schema = DBSchemaManager::getInstance().hasCurrentSchema ();
    int current_index = -1;

    if (exists_current_schema)
        current_schema = DBSchemaManager::getInstance().getCurrentSchemaName();

    std::map <std::string, DBSchema *> &schemas =  DBSchemaManager::getInstance().getSchemas();
    std::map <std::string, DBSchema *>::iterator it;

    int cnt=0;
    for (it = schemas.begin(); it != schemas.end(); it++)
    {
        current_schema_select_->addItem (it->first.c_str());

        if (exists_current_schema)
        {
            if (it->first.compare (current_schema) == 0)
                current_index=cnt;
        }
        cnt++;
    }

    if (exists_current_schema)
    {
        assert (current_index != -1);
        current_schema_select_->setCurrentIndex (current_index);
    }
}

void DBSchemaWidget::addEmptySchema ()
{
    assert (new_schema_name_edit_);
    DBSchemaManager::getInstance().addEmptySchema(new_schema_name_edit_->text().toStdString());
    updateSchemaCombo();
}
//void DBSchemaWidget::addRDLSchema ()
//{
//    assert (new_schema_name_edit_);
//    DBSchemaManager::getInstance().addRDLSchema(new_schema_name_edit_->text().toStdString());
//    updateSchemaCombo();
//}

void DBSchemaWidget::editSchema ()
{
    assert (current_schema_select_);

    if (edit_widget_ == 0)
    {
        if (DBSchemaManager::getInstance().hasCurrentSchema())
        {
            edit_widget_ = new DBSchemaEditWidget (DBSchemaManager::getInstance().getCurrentSchema());
            connect(edit_widget_, SIGNAL( renamed() ), this, SLOT( renamed() ));
        }
    }
    else
        edit_widget_->show();

}

void DBSchemaWidget::selectSchema (int index)
{
    assert (current_schema_select_);
    assert  (index >= 0  && index <= current_schema_select_->count());

    DBSchemaManager::getInstance().setCurrentSchema(current_schema_select_->currentText().toStdString());
}

void DBSchemaWidget::setSchema (std::string schema)
{
    assert (current_schema_select_);
    if (!DBSchemaManager::getInstance().hasSchema(schema))
    {
        logerr  << "DBSchemaWidget: setSchema: schema '" << schema << "' does not exist";
    }
    else
    {
        DBSchemaManager::getInstance().setCurrentSchema(schema);

        int index = current_schema_select_->findData(schema.c_str());
        if ( index != -1 )
        { // -1 for not found
            current_schema_select_->setCurrentIndex(index);
        }
        else
        {
            logerr  << "DBSchemaWidget: setSchema: combobox doesn't contain " << schema;
        }
    }
}

bool DBSchemaWidget::hasSelectedSchema ()
{
    assert (current_schema_select_);
    if (current_schema_select_->count() == 0)
        return false;

    return current_schema_select_->currentText().size() != 0;
}
std::string DBSchemaWidget::getSelectedSchema ()
{
    assert (hasSelectedSchema());

    return current_schema_select_->currentText().toStdString();
}

void DBSchemaWidget::unlock ()
{
    if (current_schema_select_)
        current_schema_select_->setDisabled (false);

    if (new_schema_name_edit_)
        new_schema_name_edit_->setDisabled (false);

    if (edit_button_)
        edit_button_->setDisabled (false);

    if (add_button_)
        add_button_->setDisabled (false);
}

void DBSchemaWidget::renamed ()
{
    assert (edit_widget_);
    updateSchemaCombo ();
}

void DBSchemaWidget::createElements ()
{
    unsigned int frame_width = 2;
    unsigned int frame_width_small = 1;
    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(frame_width);

    QVBoxLayout *db_schema_layout = new QVBoxLayout ();

    QLabel *db_schema_label = new QLabel (tr("Please select a Database schema"));
    db_schema_label->setFont (font_big);
    db_schema_layout->addWidget (db_schema_label);

    QFrame *new_schema_widget = new QFrame ();
    new_schema_widget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    new_schema_widget->setLineWidth(frame_width_small);

    QVBoxLayout *select_schema_layout = new QVBoxLayout ();

    QLabel *existing_schema_label = new QLabel (tr("Schema selection"));
    existing_schema_label->setFont (font_bold);
    select_schema_layout->addWidget (existing_schema_label);

    current_schema_select_ = new QComboBox ();
    updateSchemaCombo();
    connect(current_schema_select_, SIGNAL( activated(int) ), this, SLOT( selectSchema(int) ));
    current_schema_select_->setDisabled(true);
    select_schema_layout->addWidget(current_schema_select_);

    db_schema_layout->addLayout (select_schema_layout);

    edit_button_ = new QPushButton(tr("Edit schema"));
    edit_button_->setDisabled (true);
    connect(edit_button_, SIGNAL( clicked() ), this, SLOT( editSchema() ));
    db_schema_layout->addWidget (edit_button_);

    QVBoxLayout *new_schema_layout = new QVBoxLayout ();

    QHBoxLayout *new_schema_name_layout = new QHBoxLayout ();

    QLabel *new_schema_label = new QLabel (tr("Add new schema"));
    new_schema_label->setFont (font_bold);
    new_schema_layout->addWidget (new_schema_label);

    QLabel *new_schema_name_label = new QLabel (tr("Name"));
    new_schema_name_layout->addWidget (new_schema_name_label);

    new_schema_name_edit_ = new QLineEdit ();
    new_schema_name_edit_->setText ("Undefined");
    new_schema_name_edit_->setDisabled(true);
    new_schema_name_layout->addWidget (new_schema_name_edit_);

    add_button_ = new QPushButton(tr("Add"));
    connect(add_button_, SIGNAL( clicked() ), this, SLOT( addEmptySchema() ));
    add_button_->setDisabled (true);

    new_schema_layout->addLayout (new_schema_name_layout);

    new_schema_name_layout->addWidget (add_button_);

    new_schema_widget->setLayout (new_schema_layout);

    db_schema_layout->addWidget(new_schema_widget);

    setLayout (db_schema_layout);
}
