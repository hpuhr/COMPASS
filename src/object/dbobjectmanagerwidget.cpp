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
#include <QInputDialog>

#include "configuration.h"
#include "configurationmanager.h"
#include "dbovariable.h"
#include "dbobject.h"
#include "dbobjectwidget.h"
#include "dbobjectmanagerwidget.h"
#include "dbobjectmanager.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "metadbtable.h"
#include "stringconv.h"
#include "atsdb.h"
//#include "MetaDBObjectEditWidget.h"

using Utils::String;

DBObjectManagerWidget::DBObjectManagerWidget(DBObjectManager &object_manager)
    : object_manager_(object_manager), schema_manager_(ATSDB::instance().schemaManager()), grid_ (0), unlocked_(false), new_button_(0)

{
    unsigned int frame_width = 2;
    unsigned int frame_width_small = 1;

    setFrameStyle(QFrame::Panel | QFrame::Raised);
    setLineWidth(frame_width);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout *main_layout = new QVBoxLayout ();

    QLabel *main_label = new QLabel ("Database Objects");
    main_label->setFont (font_bold);
    main_layout->addWidget (main_label);

    QFrame *dob_frame = new QFrame ();
    dob_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    dob_frame->setLineWidth(frame_width_small);

    grid_ = new QGridLayout ();
    updateDBOsSlot ();

    dob_frame->setLayout (grid_);

    QScrollArea *scroll = new QScrollArea ();
    scroll->setWidgetResizable (true);
    scroll->setWidget(dob_frame);

    main_layout->addWidget (scroll);

    // new object
    new_button_ = new QPushButton("Add");
    connect(new_button_, SIGNAL( clicked() ), this, SLOT( addDBOSlot() ));
    new_button_->setDisabled (true);
    main_layout->addWidget (new_button_);

    // meta object
    //    QHBoxLayout *new_meta_layout = new QHBoxLayout ();

    //    QLabel *new_meta_name_label = new QLabel ("New meta object");
    //    new_meta_name_label->setFont (font_bold);
    //    new_meta_layout->addWidget (new_meta_name_label);

    //    new_meta_edit_ = new QLineEdit ("Undefined");
    //    new_meta_edit_->setDisabled (true);
    //    new_meta_layout->addWidget (new_meta_edit_);

    //    new_meta_button_ = new QPushButton("Add");
    //    connect(new_meta_button_, SIGNAL( clicked() ), this, SLOT( addMetaDBO() ));
    //    new_meta_button_->setDisabled (true);
    //    new_meta_layout->addWidget (new_meta_button_);

    //    main_layout->addLayout (new_meta_layout);

    main_layout->addStretch();

    setLayout (main_layout);
}

DBObjectManagerWidget::~DBObjectManagerWidget()
{
    //  std::map <DBObject *, DBObjectEditWidget*>::iterator it;
    //  for (it = edit_dbo_widgets_.begin(); it != edit_dbo_widgets_.end(); it++)
    //  {
    //    delete it->second;
    //  }
    //  edit_dbo_widgets_.clear();

    //  std::map <DBObject *, MetaDBObjectEditWidget*>::iterator it2;
    //  for (it2 = edit_metadbo_widgets_.begin(); it2 != edit_metadbo_widgets_.end(); it2++)
    //  {
    //    delete it2->second;
    //  }
    //  edit_metadbo_widgets_.clear();
}

void DBObjectManagerWidget::databaseOpenedSlot ()
{
    unlocked_=true;

    for (auto it : edit_dbo_buttons_)
        it.first->setDisabled (false);

    for (auto it : delete_dbo_buttons_)
        it.first->setDisabled (false);

    if (new_button_)
        new_button_->setDisabled (false);
}



void DBObjectManagerWidget::addDBOSlot ()
{
    if (!schema_manager_.hasCurrentSchema())
    {
        logerr << "DBObjectManagerWidget: addDBO: no schema was selected";
        return;
    }

    bool ok;
    QString text = QInputDialog::getText(this, tr("Database Object Name"),
                                         tr("Specify a (unique) DBObject name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty())
    {
        std::string name = text.toStdString();

        if (object_manager_.exists(name))
        {
            logerr << "DBObjectManagerWidget: addDBO: DBObject same name already exists";
            return;
        }


        auto metas = schema_manager_.getCurrentSchema().metaTables();
        QStringList items;

        for (auto it = metas.begin(); it != metas.end(); it++)
        {
            items.append(it->second->name().c_str());
        }

        bool ok;
        QString item = QInputDialog::getItem(this, tr("Main Meta Table For DBObject"), tr("Select:"), items, 0, false, &ok);
        if (ok && !item.isEmpty())
        {
            std::string meta_table_name = item.toStdString();

            std::string instance = "DBObject"+name+"0";

            Configuration &config = object_manager_.addNewSubConfiguration ("DBObject", instance);
            config.addParameterString ("name", name);
            //config.addParameterString ("meta_table", meta_table_name);
            Configuration &metatable_config = config.addNewSubConfiguration("DBOSchemaMetaTableDefinition");
            metatable_config.addParameterString ("schema", schema_manager_.getCurrentSchemaName());
            metatable_config.addParameterString ("meta_table", meta_table_name);

            object_manager_.generateSubConfigurable("DBObject", instance);

            updateDBOsSlot();
        }
    }
}

//void DBObjectManagerWidget::addMetaDBO ()
//{
//  assert (new_meta_edit_);

//  std::string name = new_meta_edit_->text().toStdString();

//  std::string instance = "DBObject"+name+"0";


//  Configuration &config = DBObjectManager::getInstance().addNewSubConfiguration ("DBObject", instance);
//  config.addParameterString ("name", name);
//  config.addParameterBool ("is_meta", true);

//  DBObjectManager::getInstance().generateSubConfigurable("DBObject", instance);
//  updateDBOs();
//}

void DBObjectManagerWidget::changedDBOSlot ()
{
    updateDBOsSlot ();
}

void DBObjectManagerWidget::editDBOSlot ()
{
    assert (edit_dbo_buttons_.find((QPushButton*)sender()) != edit_dbo_buttons_.end());

    DBObject *object = edit_dbo_buttons_ [(QPushButton*)sender()];

    //  if (object->isMeta())
    //  {
    //    if (edit_metadbo_widgets_.find (object) == edit_metadbo_widgets_.end())
    //    {
    //      MetaDBObjectEditWidget *widget = new MetaDBObjectEditWidget (object);
    //      connect(widget, SIGNAL( changedDBO() ), this, SLOT( changedDBO() ));
    //      edit_metadbo_widgets_[object] = widget;
    //    }
    //    else
    //      edit_metadbo_widgets_[object]->show();

    //  }
    //  else
    //  {

    if (edit_dbo_widgets_.find (object) == edit_dbo_widgets_.end())
    {
        DBObjectWidget *widget = object->widget();
        connect(widget, SIGNAL( changedDBOSlot() ), this, SLOT( changedDBOSlot() ));
        edit_dbo_widgets_[object] = widget;
    }
    else
        edit_dbo_widgets_[object]->show();

    //  }
}

void DBObjectManagerWidget::deleteDBOSlot ()
{
    assert (delete_dbo_buttons_.find((QPushButton*)sender()) != delete_dbo_buttons_.end());

    DBObject *object = delete_dbo_buttons_ [(QPushButton*)sender()];
    object_manager_.remove (object->name());

    updateDBOsSlot();
}


void DBObjectManagerWidget::updateDBOsSlot ()
{
    QLayoutItem *child;
    while ((child = grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QPixmap edit_pixmap("./data/icons/edit.png");
    QIcon edit_icon(edit_pixmap);

    QPixmap del_pixmap("./data/icons/delete.png");
    QIcon del_icon(del_pixmap);

    edit_dbo_buttons_.clear();
    delete_dbo_buttons_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel *name_label = new QLabel ("Name");
    name_label->setFont (font_bold);
    grid_->addWidget (name_label, 0, 0);

    QLabel *numel_label = new QLabel ("# columns");
    numel_label->setFont (font_bold);
    grid_->addWidget (numel_label, 0, 1);

    QLabel *meta_label = new QLabel ("Meta Table");
    meta_label->setFont (font_bold);
    grid_->addWidget (meta_label, 0, 2);

    QLabel *edit_label = new QLabel ("Edit");
    edit_label->setFont (font_bold);
    edit_label->setAlignment(Qt::AlignCenter);
    grid_->addWidget (edit_label, 0, 3);

    QLabel *del_label = new QLabel ("Delete");
    del_label->setFont (font_bold);
    del_label->setAlignment(Qt::AlignCenter);
    grid_->addWidget (del_label, 0, 4);

    unsigned int row=1;

    auto objects = object_manager_.objects();

    for (auto it = objects.begin(); it != objects.end(); it++)
    {
        QLabel *name = new QLabel (it->second->name().c_str());
        grid_->addWidget (name, row, 0);

        QLabel *numel = new QLabel ((String::intToString(it->second->numVariables())).c_str());
        grid_->addWidget (numel, row, 1);

        bool active = it->second->hasCurrentMetaTable();
        QLabel *meta = new QLabel ("None");
        if (active)
            meta->setText(it->second->currentMetaTable().name().c_str());
        grid_->addWidget (meta, row, 2);

        QPushButton *edit = new QPushButton ();
        edit->setIcon(edit_icon);
        edit->setIconSize(UI_ICON_SIZE);
        edit->setDisabled(!active || !unlocked_);
        connect(edit, SIGNAL( clicked() ), this, SLOT( editDBOSlot() ));
        grid_->addWidget (edit, row, 3);
        edit_dbo_buttons_[edit] = it->second;

        QPushButton *del = new QPushButton ();
        del->setIcon(del_icon);
        del->setIconSize(UI_ICON_SIZE);
        del->setDisabled(!active || !unlocked_);
        connect(del, SIGNAL( clicked() ), this, SLOT( deleteDBOSlot() ));
        grid_->addWidget (del, row, 4);
        delete_dbo_buttons_[del] = it->second;

        row++;
    }
}
