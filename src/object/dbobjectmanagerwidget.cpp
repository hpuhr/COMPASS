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

#include "dbobjectmanagerwidget.h"

#include "compass.h"
#include "configuration.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbobjectwidget.h"
#include "dbovariable.h"
#include "files.h"
#include "global.h"
#include "metadbovariable.h"
#include "metadbovariablewidget.h"
#include "stringconv.h"

#include <QComboBox>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

using namespace Utils;

DBObjectManagerWidget::DBObjectManagerWidget(DBObjectManager& object_manager)
    : object_manager_(object_manager)
{
    // unsigned int frame_width = FRAME_SIZE;
    setContentsMargins(0, 0, 0, 0);

    //    setFrameStyle(QFrame::Panel | QFrame::Raised);
    //    setLineWidth(frame_width);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    // database objects
    QLabel* main_label = new QLabel("Database Objects");
    main_label->setFont(font_bold);
    main_layout->addWidget(main_label);

    QFrame* dob_frame = new QFrame();
    dob_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    dob_frame->setLineWidth(FRAME_SIZE);

    dbobjects_grid_ = new QGridLayout();
    updateDBOsSlot();

    dob_frame->setLayout(dbobjects_grid_);

    QScrollArea* dbo_scroll = new QScrollArea();
    dbo_scroll->setWidgetResizable(true);
    dbo_scroll->setWidget(dob_frame);

    main_layout->addWidget(dbo_scroll);

    // new dbobject
    add_dbo_button_ = new QPushButton("Add");
    connect(add_dbo_button_, SIGNAL(clicked()), this, SLOT(addDBOSlot()));
    main_layout->addWidget(add_dbo_button_);

    main_layout->addStretch();

    // meta objects
    QLabel* metavars_label = new QLabel("Meta Variables");
    metavars_label->setFont(font_bold);
    main_layout->addWidget(metavars_label);

    QFrame* meta_frame = new QFrame();
    meta_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    meta_frame->setLineWidth(FRAME_SIZE);

    meta_variables_grid_ = new QGridLayout();
    updateMetaVariablesSlot();

    meta_frame->setLayout(meta_variables_grid_);

    QScrollArea* meta_scroll = new QScrollArea();
    meta_scroll->setWidgetResizable(true);
    meta_scroll->setWidget(meta_frame);

    main_layout->addWidget(meta_scroll);

    add_metavar_button_ = new QPushButton("Add");
    connect(add_metavar_button_, SIGNAL(clicked()), this, SLOT(addAllMetaVariablesSlot()));
    main_layout->addWidget(add_metavar_button_);

    setLayout(main_layout);

    // lock();
}

DBObjectManagerWidget::~DBObjectManagerWidget()
{
    edit_dbo_buttons_.clear();
    delete_dbo_buttons_.clear();
    edit_dbo_widgets_.clear();
}

// void DBObjectManagerWidget::lock ()
//{
////    for (auto it : edit_dbo_buttons_)
////        it.first->setDisabled (true);

//    for (auto it : delete_dbo_buttons_)
//        it.first->setDisabled (true);

//    add_dbo_button_->setDisabled (true);

//    for (auto it : delete_meta_buttons_)
//        it.first->setDisabled (true);

//    add_metavar_button_->setDisabled (true);
//}

// void DBObjectManagerWidget::unlock ()
//{
////    for (auto it : edit_dbo_buttons_)
////        it.first->setDisabled (!it.second->hasCurrentMetaTable());

//    for (auto it : delete_dbo_buttons_)
//        it.first->setDisabled (false);

//    add_dbo_button_->setDisabled (false);

//    for (auto it : delete_meta_buttons_)
//        it.first->setDisabled (false);

//    add_metavar_button_->setDisabled (false);
//}

// void DBObjectManagerWidget::databaseOpenedSlot ()
//{
//    loginf << "DBObjectManagerWidget: databaseOpenedSlot";
//}

void DBObjectManagerWidget::addDBOSlot()
{
//    if (!schema_manager_.hasCurrentSchema())
//    {
//        logerr << "DBObjectManagerWidget: addDBO: no schema was selected";
//        return;
//    }

    bool ok;
    QString text =
        QInputDialog::getText(this, tr("Database Object Name"),
                              tr("Specify a (unique) DBObject name:"), QLineEdit::Normal, "", &ok);
    if (ok && !text.isEmpty())
    {
        std::string name = text.toStdString();

        if (object_manager_.existsObject(name))
        {
            logerr << "DBObjectManagerWidget: addDBO: DBObject same name already exists";
            return;
        }

            std::string instance = "DBObject" + name + "0";

            Configuration& config = object_manager_.addNewSubConfiguration("DBObject", instance);
            config.addParameterString("name", name);
             //config.addParameterString ("meta_table", meta_table_name); // TODO add db_table_name

            object_manager_.generateSubConfigurable("DBObject", instance);

            updateDBOsSlot();
    }
}

void DBObjectManagerWidget::changedDBOSlot() { updateDBOsSlot(); }

void DBObjectManagerWidget::editDBOSlot()
{
    assert(edit_dbo_buttons_.find((QPushButton*)sender()) != edit_dbo_buttons_.end());

    DBObject* object = edit_dbo_buttons_[(QPushButton*)sender()];

    if (edit_dbo_widgets_.find(object) == edit_dbo_widgets_.end())
    {
        DBObjectWidget* widget = object->widget();
        connect(widget, SIGNAL(changedDBOSignal()), this, SLOT(changedDBOSlot()));
        edit_dbo_widgets_[object] = widget;
    }
    else
        edit_dbo_widgets_[object]->show();
}

void DBObjectManagerWidget::deleteDBOSlot()
{
    assert(delete_dbo_buttons_.find((QPushButton*)sender()) != delete_dbo_buttons_.end());

    DBObject* object = delete_dbo_buttons_[(QPushButton*)sender()];
    object_manager_.deleteObject(object->name());

    updateDBOsSlot();
}

void DBObjectManagerWidget::updateDBOsSlot()
{
    assert(dbobjects_grid_);

    QLayoutItem* child;
    while ((child = dbobjects_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QIcon edit_icon(Files::getIconFilepath("edit.png").c_str());
    QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    edit_dbo_buttons_.clear();
    delete_dbo_buttons_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* name_label = new QLabel("Name");
    name_label->setFont(font_bold);
    dbobjects_grid_->addWidget(name_label, 0, 0);

    QLabel* numel_label = new QLabel("# columns");
    numel_label->setFont(font_bold);
    dbobjects_grid_->addWidget(numel_label, 0, 1);

    unsigned int row = 1;

    for (auto& obj_it : object_manager_)
    {
        QLabel* name = new QLabel(obj_it.second->name().c_str());
        //        if (!obj_it.second->existsInDB())
        //        {
        //            QFont font = name->font();
        //            font.setStrikeOut(true);
        //            name->setFont(font);
        //        }
        dbobjects_grid_->addWidget(name, row, 0);

        QLabel* numel = new QLabel((std::to_string(obj_it.second->numVariables())).c_str());
        dbobjects_grid_->addWidget(numel, row, 1);

        QPushButton* edit = new QPushButton();
        edit->setIcon(edit_icon);
        edit->setIconSize(UI_ICON_SIZE);
        edit->setMaximumWidth(UI_ICON_BUTTON_MAX_WIDTH);
        edit->setFlat(UI_ICON_BUTTON_FLAT);
        // edit->setDisabled(!active || locked_);
        connect(edit, SIGNAL(clicked()), this, SLOT(editDBOSlot()));
        dbobjects_grid_->addWidget(edit, row, 3);
        edit_dbo_buttons_[edit] = obj_it.second;

        QPushButton* del = new QPushButton();
        del->setIcon(del_icon);
        del->setIconSize(UI_ICON_SIZE);
        del->setMaximumWidth(UI_ICON_BUTTON_MAX_WIDTH);
        del->setFlat(UI_ICON_BUTTON_FLAT);
        // del->setDisabled(locked_);
        connect(del, SIGNAL(clicked()), this, SLOT(deleteDBOSlot()));
        dbobjects_grid_->addWidget(del, row, 4);
        delete_dbo_buttons_[del] = obj_it.second;

        row++;
    }
}

void DBObjectManagerWidget::addMetaVariableSlot() {}

void DBObjectManagerWidget::editMetaVariableSlot()
{
    assert(edit_meta_buttons_.find((QPushButton*)sender()) != edit_meta_buttons_.end());

    MetaDBOVariable* meta_var = edit_meta_buttons_[(QPushButton*)sender()];

    if (edit_meta_widgets_.find(meta_var) == edit_meta_widgets_.end())
    {
        MetaDBOVariableWidget* widget = meta_var->widget();
        connect(widget, SIGNAL(metaVariableChangedSignal()), this, SLOT(updateMetaVariablesSlot()));
        edit_meta_widgets_[meta_var] = widget;
    }
    else
        edit_meta_widgets_[meta_var]->show();
}

void DBObjectManagerWidget::deleteMetaVariableSlot()
{
    assert(delete_meta_buttons_.find((QPushButton*)sender()) != delete_meta_buttons_.end());

    MetaDBOVariable* meta_var = delete_meta_buttons_[(QPushButton*)sender()];
    object_manager_.deleteMetaVariable(meta_var->name());

    updateMetaVariablesSlot();
}

void DBObjectManagerWidget::addAllMetaVariablesSlot()
{
    std::vector<std::string> found_dbos;

    bool changed = false;

    for (auto& obj_it : object_manager_)
    {
        for (auto& var_it : *obj_it.second)
        {
            if (object_manager_.usedInMetaVariable(var_it.second))
            {
                loginf << "DBObjectManagerWidget: addAllMetaVariablesSlot: not adding dbovariable "
                       << var_it.first << " since already used";
                continue;
            }

            found_dbos.clear();
            found_dbos.push_back(obj_it.first);  // original object

            for (auto& obj_it2 : object_manager_)
            {
                if (obj_it == obj_it2)
                    continue;

                if (obj_it2.second->hasVariable(var_it.first) &&
                    var_it.second.dataType() == obj_it2.second->variable(var_it.first).dataType())
                {
                    found_dbos.push_back(obj_it2.first);
                }
            }

            if (found_dbos.size() > 1)
            {
                if (!object_manager_.existsMetaVariable(var_it.first))
                {
                    loginf
                        << "DBObjectManagerWidget: addAllMetaVariablesSlot: adding meta variable "
                        << var_it.first;

                    std::string instance = "MetaDBOVariable" + var_it.first + "0";

                    Configuration& config =
                        object_manager_.addNewSubConfiguration("MetaDBOVariable", instance);
                    config.addParameterString("name", var_it.first);

                    object_manager_.generateSubConfigurable("MetaDBOVariable", instance);
                }

                assert(object_manager_.existsMetaVariable(var_it.first));
                MetaDBOVariable& meta_var = object_manager_.metaVariable(var_it.first);

                for (auto dbo_it2 = found_dbos.begin(); dbo_it2 != found_dbos.end(); dbo_it2++)
                {
                    if (!meta_var.existsIn(*dbo_it2))
                    {
                        loginf << "DBObjectManagerWidget: addAllMetaVariablesSlot: adding meta "
                                  "variable "
                               << var_it.first << " dbo variable " << var_it.first;
                        meta_var.addVariable(*dbo_it2, var_it.first);
                    }
                }

                changed = true;
            }
        }
    }

    if (changed)
        updateMetaVariablesSlot();
}

void DBObjectManagerWidget::updateMetaVariablesSlot()
{
    assert(meta_variables_grid_);

    QLayoutItem* child;
    while ((child = meta_variables_grid_->takeAt(0)) != 0)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    QIcon edit_icon(Files::getIconFilepath("edit.png").c_str());
    QIcon del_icon(Files::getIconFilepath("delete.png").c_str());

    edit_meta_buttons_.clear();
    delete_meta_buttons_.clear();

    QFont font_bold;
    font_bold.setBold(true);

    QLabel* name_label = new QLabel("Name");
    name_label->setFont(font_bold);
    meta_variables_grid_->addWidget(name_label, 0, 0);

    QLabel* type_label = new QLabel("Data type");
    type_label->setFont(font_bold);
    meta_variables_grid_->addWidget(type_label, 0, 1);

    unsigned int row = 1;

    auto meta_variables = object_manager_.metaVariables();

    for (auto it = meta_variables.begin(); it != meta_variables.end(); it++)
    {
        QLabel* name = new QLabel(it->second->name().c_str());
        meta_variables_grid_->addWidget(name, row, 0);

        QLabel* datatype = new QLabel();
        if (it->second->hasVariables())
            datatype->setText(it->second->dataTypeString().c_str());
        meta_variables_grid_->addWidget(datatype, row, 1);

        QPushButton* edit = new QPushButton();
        edit->setIcon(edit_icon);
        edit->setIconSize(UI_ICON_SIZE);
        edit->setMaximumWidth(UI_ICON_BUTTON_MAX_WIDTH);
        edit->setFlat(UI_ICON_BUTTON_FLAT);
        // edit->setDisabled(!active || !unlocked_);
        connect(edit, SIGNAL(clicked()), this, SLOT(editMetaVariableSlot()));
        meta_variables_grid_->addWidget(edit, row, 2);
        edit_meta_buttons_[edit] = it->second;

        QPushButton* del = new QPushButton();
        del->setIcon(del_icon);
        del->setIconSize(UI_ICON_SIZE);
        del->setMaximumWidth(UI_ICON_BUTTON_MAX_WIDTH);
        del->setFlat(UI_ICON_BUTTON_FLAT);
        // del->setDisabled(!unlocked_);
        connect(del, SIGNAL(clicked()), this, SLOT(deleteMetaVariableSlot()));
        meta_variables_grid_->addWidget(del, row, 3);
        delete_meta_buttons_[del] = it->second;

        row++;
    }
}
