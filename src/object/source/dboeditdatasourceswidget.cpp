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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QPushButton>

#include "dboeditdatasourceswidget.h"
#include "dbobject.h"
#include "dbodatasourcewidget.h"
#include "dboeditdatasourceactionoptionswidget.h"
#include "storeddbodatasourcewidget.h"
#include "files.h"

using namespace Utils;

DBOEditDataSourcesWidget::DBOEditDataSourcesWidget(DBObject* object, QWidget *parent, Qt::WindowFlags f)
    : QWidget (parent, f), object_(object)
{
    assert (object_);
    action_heading_ = "No actions defined";

    setMinimumSize(QSize(1000, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    int frame_width_small = 1;

    QVBoxLayout *main_layout = new QVBoxLayout ();

    std::string caption = "Edit DBObject " +object_->name()+ " DataSources";

    QLabel *main_label = new QLabel (caption.c_str());
    main_label->setFont (font_big);
    main_layout->addWidget (main_label);

    QHBoxLayout* sources_layout = new QHBoxLayout();

    // config stuff

    QVBoxLayout* config_layout = new QVBoxLayout();

    QFrame *config_frame = new QFrame ();
    config_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    config_frame->setLineWidth(frame_width_small);
    //config_frame->setMinimumWidth(250);

    QLabel *config_label = new QLabel ("Configuration Data Sources");
    config_label->setFont (font_bold);
    config_layout->addWidget (config_label);

    config_ds_layout_ = new QVBoxLayout();
    config_layout->addLayout(config_ds_layout_);

    config_layout->addStretch();

    QPushButton* add_stored_button = new QPushButton ("Add New");
    connect(add_stored_button, &QPushButton::clicked, this, &DBOEditDataSourcesWidget::addStoredDSSlot);
    config_layout->addWidget(add_stored_button);

    QIcon right_icon(Files::getIconFilepath("arrow_to_right.png").c_str());

    sync_from_cfg_button_ = new QPushButton ("Sync to DB");
    sync_from_cfg_button_->setIcon(right_icon);
    //sync_from_cfg_button_->setMaximumWidth(200);
    //sync_from_cfg_button_->setLayoutDirection(Qt::RightToLeft);
    connect(sync_from_cfg_button_, SIGNAL(clicked()), this, SLOT(syncOptionsFromCfgSlot()));
    config_layout->addWidget(sync_from_cfg_button_);

    config_frame->setLayout (config_layout);

    sources_layout->addWidget(config_frame);

    // action stuff

    QVBoxLayout* action_layout = new QVBoxLayout();

    QFrame *action_frame = new QFrame ();
    action_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    action_frame->setLineWidth(frame_width_small);
    //action_frame->setMinimumWidth(250);

    action_heading_label_ = new QLabel (action_heading_.c_str());
    action_heading_label_->setFont (font_bold);
    action_layout->addWidget (action_heading_label_);

    action_layout->addWidget(new QLabel());

    action_layout_ = new QGridLayout();
    action_layout->addLayout(action_layout_);

    // action selection buttons
    QHBoxLayout* action_select_layout = new QHBoxLayout();

    action_layout->addStretch();

    select_all_actions_ = new QPushButton ("Select All");
    connect(select_all_actions_, &QPushButton::clicked, this, &DBOEditDataSourcesWidget::selectAllActionsSlot);
    action_select_layout->addWidget(select_all_actions_);

    deselect_all_actions_ = new QPushButton ("Select None");
    connect(deselect_all_actions_, &QPushButton::clicked, this, &DBOEditDataSourcesWidget::deselectAllActionsSlot);
    action_select_layout->addWidget(deselect_all_actions_);

    action_layout->addLayout(action_select_layout);

    // perform actions
    perform_actions_button_ = new QPushButton ("Perform Actions");
    connect(perform_actions_button_, &QPushButton::clicked, this, &DBOEditDataSourcesWidget::performActionsSlot);
    action_layout->addWidget(perform_actions_button_);

    updateActionButtons();
    action_frame->setLayout (action_layout);

    sources_layout->addWidget(action_frame);

    // db stuff

    QVBoxLayout* db_layout = new QVBoxLayout();

    QFrame *db_frame = new QFrame ();
    db_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    db_frame->setLineWidth(frame_width_small);
    //db_frame->setMinimumWidth(250);

    QLabel *db_label = new QLabel ("Database Data Sources");
    db_label->setFont (font_bold);
    db_layout->addWidget (db_label);

    db_ds_layout_ = new QGridLayout();
    db_layout->addLayout(db_ds_layout_);

    db_layout->addStretch();

    QIcon left_icon(Files::getIconFilepath("arrow_to_left.png").c_str());

    sync_from_db_button_ = new QPushButton ("Sync To Config");
    sync_from_db_button_->setIcon(left_icon);
    //sync_from_db_button_->setMaximumWidth(200);
    //sync_from_db_button_->setLayoutDirection(Qt::LeftToRight);
    connect(sync_from_db_button_, SIGNAL(clicked()), this, SLOT(syncOptionsFromDBSlot()));
    db_layout->addWidget(sync_from_db_button_);

    db_frame->setLayout (db_layout);

    sources_layout->addWidget(db_frame);

    // rest

    update ();

    main_layout->addLayout(sources_layout);
    setLayout (main_layout);

    show();
}

DBOEditDataSourcesWidget::~DBOEditDataSourcesWidget()
{
    logdbg << "DBOEditDataSourcesWidget: dtor";
}

void DBOEditDataSourcesWidget::update ()
{
    loginf << "DBOEditDataSourcesWidget: update";

    QLayoutItem *child;
    while ((child = config_ds_layout_->takeAt(0)) != 0)
    {
        config_ds_layout_->removeItem(child);
        delete child;
    }

    while ((child = db_ds_layout_->takeAt(0)) != 0)
    {
        db_ds_layout_->removeItem(child);
        delete child;
    }

    assert (object_);

    for (auto ds_it = object_->storedDSBegin(); ds_it != object_->storedDSEnd(); ++ds_it)
    {
        loginf << "DBOEditDataSourcesWidget: update: config ds " << ds_it->first;
        config_ds_layout_->addWidget(ds_it->second.widget(ds_it == object_->storedDSBegin()));
    }

    for (auto ds_it = object_->dsBegin(); ds_it != object_->dsEnd(); ++ds_it)
    {
        loginf << "DBOEditDataSourcesWidget: update: db ds " << ds_it->first;
        db_ds_layout_->addWidget(ds_it->second.widget(ds_it == object_->dsBegin()));
    }
}

void DBOEditDataSourcesWidget::syncOptionsFromDBSlot()
{
    loginf << "DBOEditDataSourcesWidget: syncOptionsFromDBSlot";

    assert (object_);
    action_collection_ = object_->getSyncOptionsFromDB();

    action_heading_ = "Actions: From DB to Config";
    displaySyncOptions ();
}

void DBOEditDataSourcesWidget::addStoredDSSlot ()
{
    loginf << "DBOEditDataSourcesWidget: addStoredDSSlot";
    assert (object_);
    object_->addNewStoredDataSource ();

    update();
}

void DBOEditDataSourcesWidget::syncOptionsFromCfgSlot()
{
    loginf << "DBOEditDataSourcesWidget: syncOptionsFromCfgSlot";

    assert (object_);
    action_collection_ = object_->getSyncOptionsFromCfg();

    action_heading_ = "Actions: From Config to DB";
    displaySyncOptions ();
}


void DBOEditDataSourcesWidget::selectAllActionsSlot()
{
    loginf << "DBOEditDataSourcesWidget: selectAllActionsSlot";

    for (auto& opt_it : action_collection_)
        opt_it.second.performFlag(opt_it.second.currentActionId() != 0); // select all not None

    updateActionButtons();
}
void DBOEditDataSourcesWidget::deselectAllActionsSlot()
{
    loginf << "DBOEditDataSourcesWidget: deselectAllActionsSlot";

    for (auto& opt_it : action_collection_)
        opt_it.second.performFlag(false);

    updateActionButtons();
}

void DBOEditDataSourcesWidget::performActionsSlot()
{
    loginf << "DBOEditDataSourcesWidget: performActionsSlot";

    for (auto& opt_it : action_collection_)
        if (opt_it.second.performFlag())
            opt_it.second.perform();

    clearSyncOptions();
    displaySyncOptions();

    update();
}

void DBOEditDataSourcesWidget::clearSyncOptions()
{
    action_heading_ = "No actions defined";
    action_collection_.clear();
}

void DBOEditDataSourcesWidget::displaySyncOptions ()
{
    loginf << "DBOEditDataSourcesWidget: displaySyncOptions";

    assert (action_heading_label_);
    action_heading_label_->setText(action_heading_.c_str());


    assert (action_layout_);
    QLayoutItem *child;
    while ((child = action_layout_->takeAt(0)) != 0)
    {
        action_layout_->removeItem(child);
        delete child;
    }

    unsigned int row = 1;
    for (auto& op_it : action_collection_)
    {
        action_layout_->addWidget(op_it.second.widget(), row++, 0);
    }

    updateActionButtons();
}

void DBOEditDataSourcesWidget::updateActionButtons()
{
    assert (select_all_actions_);
    assert (deselect_all_actions_);
    assert (perform_actions_button_);

    bool disabled = !haveActionsToPerform();

    select_all_actions_->setDisabled(disabled);
    deselect_all_actions_->setDisabled(disabled);
    perform_actions_button_->setDisabled(disabled);
}


bool DBOEditDataSourcesWidget::haveActionsToPerform ()
{
//    for (auto& opt_it : action_collection_)
//        if (opt_it.second.performFlag() && opt_it.second.currentActionId() != 0)
//            return true;

    return action_collection_.size() > 0;
}
