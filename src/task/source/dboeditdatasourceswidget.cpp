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

#include "dboeditdatasourceswidget.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>

#include "dbobject.h"
#include "dboeditdatasourceactionoptionswidget.h"
#include "files.h"
#include "managedatasourcestask.h"

using namespace Utils;

DBOEditDataSourcesWidget::DBOEditDataSourcesWidget(ManageDataSourcesTask& task, DBObject& object,
                                                   QWidget* parent, Qt::WindowFlags f)
    : QWidget(parent, f), task_(task), object_(object)
{
    action_heading_ = "No actions defined";

    QFont font_bold;
    font_bold.setBold(true);

    // int frame_width_small = 1;

    QHBoxLayout* main_layout = new QHBoxLayout();

    QVBoxLayout* sources_layout = new QVBoxLayout();

    // check if additional columns are required
    if (object_.hasCurrentDataSourceDefinition())
    {
        const DBODataSourceDefinition& ds_def = object_.currentDataSourceDefinition();

        // psr ranges
        has_primary_range_min_ = ds_def.hasPrimaryIRMinColumn();
        if (has_primary_range_min_)
            table_columns_.push_back("Primary IR Range Min");

        has_primary_range_max_ = ds_def.hasPrimaryIRMaxColumn();
        if (has_primary_range_max_)
            table_columns_.push_back("Primary IR Range Max");

        // ssr ranges

        has_secondary_range_min_ = ds_def.hasSecondaryIRMinColumn();
        if (has_secondary_range_min_)
            table_columns_.push_back("Secondary IR Range Min");

        has_secondary_range_max_ = ds_def.hasSecondaryIRMaxColumn();
        if (has_secondary_range_max_)
            table_columns_.push_back("Secondary IR Range Max");

        // mode s ranges
        has_mode_s_range_min_ = ds_def.hasModeSIRMinColumn();
        if (has_mode_s_range_min_)
            table_columns_.push_back("Mode S IR Range Min");

        has_mode_s_range_max_ = ds_def.hasModeSIRMaxColumn();
        if (has_mode_s_range_max_)
            table_columns_.push_back("Mode S IR Range Max");

        // psr stddevs
        has_primary_azimuth_stddev_ = ds_def.hasPrimaryAzimuthStdDevColumn();
        if (has_primary_azimuth_stddev_)
            table_columns_.push_back("Primary Azimuth StdDev");

        has_primary_range_stddev_ = ds_def.hasPrimaryRangeStdDevColumn();
        if (has_primary_range_stddev_)
            table_columns_.push_back("Primary Range StdDev");

        // ssr stddevs
        has_secondary_azimuth_stddev_ = ds_def.hasSecondaryAzimuthStdDevColumn();
        if (has_secondary_azimuth_stddev_)
            table_columns_.push_back("Secondary Azimuth StdDev");

        has_secondary_range_stddev_ = ds_def.hasSecondaryRangeStdDevColumn();
        if (has_secondary_range_stddev_)
            table_columns_.push_back("Secondary Range StdDev");


        // mode s stddevs
        has_mode_s_azimuth_stddev_ = ds_def.hasModeSAzimuthStdDevColumn();
        if (has_mode_s_azimuth_stddev_)
            table_columns_.push_back("Mode S Azimuth StdDev");

        has_mode_s_range_stddev_ = ds_def.hasModeSRangeStdDevColumn();
        if (has_mode_s_range_stddev_)
            table_columns_.push_back("Mode S Range StdDev");
    }

    // config ds stuff
    {
        QVBoxLayout* config_layout = new QVBoxLayout();

        QFrame* config_frame = new QFrame();
        config_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        // config_frame->setLineWidth(frame_width_small);

        QHBoxLayout* top_layout = new QHBoxLayout();

        QLabel* config_label = new QLabel("Configuration Data Sources");
        config_label->setFont(font_bold);
        top_layout->addWidget(config_label);

        QPushButton* add_stored_button = new QPushButton("Add New");
        connect(add_stored_button, &QPushButton::clicked, this,
                &DBOEditDataSourcesWidget::addStoredDSSlot);
        top_layout->addWidget(add_stored_button);

        QIcon down_icon(Files::getIconFilepath("down.png").c_str());

        sync_from_cfg_button_ = new QPushButton("Sync to DB");
        sync_from_cfg_button_->setIcon(down_icon);
        connect(sync_from_cfg_button_, SIGNAL(clicked()), this, SLOT(syncOptionsFromCfgSlot()));
        top_layout->addWidget(sync_from_cfg_button_);

        config_layout->addLayout(top_layout);

        config_ds_table_ = new QTableWidget();
        config_ds_table_->setEditTriggers(QAbstractItemView::AllEditTriggers);
        config_ds_table_->setColumnCount(table_columns_.size());
        config_ds_table_->setHorizontalHeaderLabels(table_columns_);
        config_ds_table_->verticalHeader()->setVisible(false);
        connect(config_ds_table_, &QTableWidget::itemChanged, this,
                &DBOEditDataSourcesWidget::configItemChangedSlot);
        // update done later

        config_layout->addWidget(config_ds_table_);

        config_frame->setLayout(config_layout);

        QScrollArea* config_scroll = new QScrollArea();
        config_scroll->setWidgetResizable(true);
        config_scroll->setWidget(config_frame);

        sources_layout->addWidget(config_scroll);
    }

    // db stuff
    {
        QVBoxLayout* db_layout = new QVBoxLayout();

        QFrame* db_frame = new QFrame();
        db_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        // db_frame->setLineWidth(frame_width_small);

        QHBoxLayout* top_layout = new QHBoxLayout();

        QLabel* db_label = new QLabel("Database Data Sources");
        db_label->setFont(font_bold);
        top_layout->addWidget(db_label);

        QIcon up_icon(Files::getIconFilepath("up.png").c_str());

        sync_from_db_button_ = new QPushButton("Sync To Config");
        sync_from_db_button_->setIcon(up_icon);
        connect(sync_from_db_button_, SIGNAL(clicked()), this, SLOT(syncOptionsFromDBSlot()));
        top_layout->addWidget(sync_from_db_button_);

        db_layout->addLayout(top_layout);

        db_ds_table_ = new QTableWidget();
        db_ds_table_->setEditTriggers(QAbstractItemView::AllEditTriggers);
        db_ds_table_->setColumnCount(table_columns_.size());
        db_ds_table_->setHorizontalHeaderLabels(table_columns_);
        db_ds_table_->verticalHeader()->setVisible(false);
        connect(db_ds_table_, &QTableWidget::itemChanged, this,
                &DBOEditDataSourcesWidget::dbItemChangedSlot);
        // update done later

        db_layout->addWidget(db_ds_table_);

        db_frame->setLayout(db_layout);

        QScrollArea* db_scroll = new QScrollArea();
        db_scroll->setWidgetResizable(true);
        db_scroll->setWidget(db_frame);

        sources_layout->addWidget(db_scroll);
    }

    main_layout->addLayout(sources_layout);

    // action stuff
    {
        QVBoxLayout* action_frame_layout = new QVBoxLayout();

        action_heading_label_ = new QLabel(action_heading_.c_str());
        action_heading_label_->setFont(font_bold);
        action_frame_layout->addWidget(action_heading_label_);

        // actions
        action_layout_ = new QGridLayout();

        QWidget* action_layout_widget = new QWidget();
        action_layout_widget->setContentsMargins(0, 0, 0, 0);
        action_layout_widget->setLayout(action_layout_);

        QScrollArea* action_scroll = new QScrollArea();
        action_scroll->setMaximumWidth(270);
        action_scroll->setWidgetResizable(true);
        action_scroll->setWidget(action_layout_widget);

        action_frame_layout->addWidget(action_scroll);

        // action selection buttons

        QVBoxLayout* action_button_layout = new QVBoxLayout();

        QHBoxLayout* action_select_layout = new QHBoxLayout();

        select_all_actions_ = new QPushButton("Select All");
        connect(select_all_actions_, &QPushButton::clicked, this,
                &DBOEditDataSourcesWidget::selectAllActionsSlot);
        action_select_layout->addWidget(select_all_actions_);

        deselect_all_actions_ = new QPushButton("Select None");
        connect(deselect_all_actions_, &QPushButton::clicked, this,
                &DBOEditDataSourcesWidget::deselectAllActionsSlot);
        action_select_layout->addWidget(deselect_all_actions_);

        action_button_layout->addLayout(action_select_layout);

        // perform actions
        perform_actions_button_ = new QPushButton("Perform Actions");
        connect(perform_actions_button_, &QPushButton::clicked, this,
                &DBOEditDataSourcesWidget::performActionsSlot);
        action_button_layout->addWidget(perform_actions_button_);

        updateActionButtons();
        action_frame_layout->addLayout(action_button_layout);

        main_layout->addLayout(action_frame_layout);
    }

    update();

    setLayout(main_layout);
}

DBOEditDataSourcesWidget::~DBOEditDataSourcesWidget()
{
    logdbg << "DBOEditDataSourcesWidget: dtor";
}

void DBOEditDataSourcesWidget::update()
{
    logdbg << "DBOEditDataSourcesWidget: update";

    assert(config_ds_table_);
    assert(db_ds_table_);

    config_ds_table_->blockSignals(true);
    db_ds_table_->blockSignals(true);

    updateConfigDSTable();
    updateDBDSTable();
    updateColumnSizes();

    config_ds_table_->blockSignals(false);
    db_ds_table_->blockSignals(false);
}

void DBOEditDataSourcesWidget::syncOptionsFromDBSlot()
{
    loginf << "DBOEditDataSourcesWidget: syncOptionsFromDBSlot";

    action_collection_ = task_.getSyncOptionsFromDB(object_.name());

    action_heading_ = "Actions: From DB to Config";
    displaySyncOptions();
}

void DBOEditDataSourcesWidget::addStoredDSSlot()
{
    loginf << "DBOEditDataSourcesWidget: addStoredDSSlot";
    task_.addNewStoredDataSource(object_.name());

    update();
}

void DBOEditDataSourcesWidget::syncOptionsFromCfgSlot()
{
    loginf << "DBOEditDataSourcesWidget: syncOptionsFromCfgSlot";

    action_collection_ = task_.getSyncOptionsFromCfg(object_.name());

    action_heading_ = "Actions: From Config to DB";
    displaySyncOptions();
}

void DBOEditDataSourcesWidget::selectAllActionsSlot()
{
    loginf << "DBOEditDataSourcesWidget: selectAllActionsSlot";

    for (auto& opt_it : action_collection_)
        opt_it.second.performFlag(opt_it.second.currentActionId() != 0);  // select all not None

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

    bool db_action_performed = false;

    for (auto& opt_it : action_collection_)
    {
        if (opt_it.second.performFlag())
        {
            opt_it.second.perform();

            if (opt_it.second.currentAction().targetType() == "DB")
                db_action_performed = true;
        }
    }

    clearSyncOptions();
    displaySyncOptions();

    update();

    if (db_action_performed)
        emit dbItemChangedSignal();
}

void DBOEditDataSourcesWidget::updateConfigDSTable()
{
    logdbg << "DBOEditDataSourcesWidget: updateConfigDSTable";

    assert(config_ds_table_);

    config_ds_table_->clearContents();
    config_ds_table_->setRowCount(task_.storedDataSources(object_.name()).size());

    int row = 0;
    int col = 0;
    for (auto& ds_it : task_.storedDataSources(object_.name()))
    {
        unsigned int id = ds_it.second.id();

        col = 0;
        {  // ID
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(id));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setData(Qt::UserRole, QVariant(id));
            config_ds_table_->setItem(row, col, item);
        }

        {  // name
            ++col;
            QTableWidgetItem* item = new QTableWidgetItem(ds_it.second.name().c_str());
            item->setData(Qt::UserRole, QVariant(id));
            config_ds_table_->setItem(row, col, item);
        }

        {  // short name
            ++col;
            if (ds_it.second.hasShortName())
            {
                QTableWidgetItem* item = new QTableWidgetItem(ds_it.second.shortName().c_str());
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        {  // sac
            ++col;
            if (ds_it.second.hasSac())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number((uint)ds_it.second.sac()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        {  // sic
            ++col;
            if (ds_it.second.hasSic())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number((uint)ds_it.second.sic()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        {  // latitude
            ++col;
            if (ds_it.second.hasLatitude())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.latitude(), 'g', 10));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        {  // longitude
            ++col;
            if (ds_it.second.hasLongitude())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.longitude(), 'g', 10));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        {  // altitude
            ++col;
            if (ds_it.second.hasAltitude())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.altitude()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }
        // psr ranges
        if (has_primary_range_min_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasPrimaryRangeMin())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.primaryRangeMin()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        if (has_primary_range_max_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasPrimaryRangeMax())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.primaryRangeMax()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        // ssr ranges
        if (has_secondary_range_min_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasSecondaryRangeMin())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.secondaryRangeMin()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        if (has_secondary_range_max_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasSecondaryRangeMax())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.secondaryRangeMax()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        // mode s ranges
        if (has_mode_s_range_min_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasModeSRangeMin())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.modeSRangeMin()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        if (has_mode_s_range_max_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasModeSRangeMax())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.modeSRangeMax()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        // psr std devs
        if (has_primary_azimuth_stddev_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasPrimaryAzimuthStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.primaryAzimuthStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        if (has_primary_range_stddev_)   // psr range
        {
            ++col;
            if (ds_it.second.hasPrimaryRangeStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.primaryRangeStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        // ssr std devs
        if (has_secondary_azimuth_stddev_)   // ssr azm
        {
            ++col;
            if (ds_it.second.hasSecondaryAzimuthStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.secondaryAzimuthStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        if (has_secondary_range_stddev_)   // ssr range
        {
            ++col;
            if (ds_it.second.hasSecondaryRangeStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.secondaryRangeStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        // mode s std devs
        if (has_mode_s_azimuth_stddev_)   // mode s azm
        {
            ++col;
            if (ds_it.second.hasModeSAzimuthStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.modeSAzimuthStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        if (has_mode_s_range_stddev_)   // mode s range
        {
            ++col;
            if (ds_it.second.hasModeSRangeStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.modeSRangeStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        ++row;
    }
}
void DBOEditDataSourcesWidget::updateDBDSTable()
{
    logdbg << "DBOEditDataSourcesWidget: updateDBDSTable";

    assert(db_ds_table_);

    db_ds_table_->clearContents();

    db_ds_table_->setRowCount(object_.dataSources().size());

    int row = 0;
    int col = 0;
    for (auto& ds_it : object_.dataSources())
    {
        unsigned int id = ds_it.second.id();

        col = 0;
        {  // ID
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(id));
            item->setData(Qt::UserRole, QVariant(id));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            db_ds_table_->setItem(row, col, item);
        }

        {  // name
            ++col;
            QTableWidgetItem* item = new QTableWidgetItem(ds_it.second.name().c_str());
            item->setData(Qt::UserRole, QVariant(id));
            db_ds_table_->setItem(row, col, item);
        }

        {  // short namme
            ++col;
            if (ds_it.second.hasShortName())
            {
                QTableWidgetItem* item = new QTableWidgetItem(ds_it.second.shortName().c_str());
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        {  // sac
            ++col;
            if (ds_it.second.hasSac())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number((uint)ds_it.second.sac()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        {  // sic
            ++col;
            if (ds_it.second.hasSic())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number((uint)ds_it.second.sic()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        {  // latitude
            ++col;
            if (ds_it.second.hasLatitude())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.latitude(), 'g', 10));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        {  // longitude
            ++col;
            if (ds_it.second.hasLongitude())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.longitude(), 'g', 10));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        {  // altitude
            ++col;
            if (ds_it.second.hasAltitude())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.altitude()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        // psr ranges
        if (has_primary_range_min_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasPrimaryRangeMin())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.primaryRangeMin()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        if (has_primary_range_max_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasPrimaryRangeMax())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.primaryRangeMax()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        // ssr ranges
        if (has_secondary_range_min_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasSecondaryRangeMin())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.secondaryRangeMin()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        if (has_secondary_range_max_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasSecondaryRangeMax())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.secondaryRangeMax()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        // mode s ranges
        if (has_mode_s_range_min_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasModeSRangeMin())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.modeSRangeMin()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        if (has_mode_s_range_max_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasModeSRangeMax())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.modeSRangeMax()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        // psr std devs
        if (has_primary_azimuth_stddev_)  // psr azm
        {
            ++col;
            if (ds_it.second.hasPrimaryAzimuthStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.primaryAzimuthStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        if (has_primary_range_stddev_)   // psr range
        {
            ++col;
            if (ds_it.second.hasPrimaryRangeStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.primaryRangeStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        // ssr std devs
        if (has_secondary_azimuth_stddev_)   // ssr azm
        {
            ++col;
            if (ds_it.second.hasSecondaryAzimuthStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.secondaryAzimuthStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        if (has_secondary_range_stddev_)   // ssr range
        {
            ++col;
            if (ds_it.second.hasSecondaryRangeStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.secondaryRangeStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        // mode s std devs
        if (has_mode_s_azimuth_stddev_)   // mode s azm
        {
            ++col;
            if (ds_it.second.hasModeSAzimuthStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.modeSAzimuthStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        if (has_mode_s_range_stddev_)   // mode s range
        {
            ++col;
            if (ds_it.second.hasModeSRangeStdDev())
            {
                QTableWidgetItem* item =
                    new QTableWidgetItem(QString::number(ds_it.second.modeSRangeStdDev()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        ++row;
    }
}

void DBOEditDataSourcesWidget::updateColumnSizes()
{
    assert(config_ds_table_);
    assert(db_ds_table_);

    config_ds_table_->horizontalHeader()->blockSignals(true);
    db_ds_table_->horizontalHeader()->blockSignals(true);

    for (int cnt = 0; cnt < table_columns_.size(); ++cnt)
    {
        config_ds_table_->resizeColumnToContents(cnt);
        db_ds_table_->resizeColumnToContents(cnt);
        int cfgsize = config_ds_table_->horizontalHeader()->sectionSize(cnt);
        int dbsize = db_ds_table_->horizontalHeader()->sectionSize(cnt);
        config_ds_table_->horizontalHeader()->resizeSection(cnt, qMax(cfgsize, dbsize));
        db_ds_table_->horizontalHeader()->resizeSection(cnt, qMax(cfgsize, dbsize));
    }
    config_ds_table_->horizontalHeader()->blockSignals(false);
    db_ds_table_->horizontalHeader()->blockSignals(false);
}

void DBOEditDataSourcesWidget::configItemChangedSlot(QTableWidgetItem* item)
{
    assert(item);
    assert(config_ds_table_);

    bool ok;
    unsigned int id = item->data(Qt::UserRole).toUInt(&ok);
    assert(ok);
    int col = config_ds_table_->currentColumn();

    assert (col < table_columns_.size());
    std::string col_name = table_columns_.at(col).toStdString();

    std::string text = item->text().toStdString();

    loginf << "DBOEditDataSourcesWidget: configItemChanged: id " << id << " col " << col
           << " text '" << text << "'";

    assert(task_.hasStoredDataSource(object_.name(), id));

    StoredDBODataSource& ds = task_.storedDataSource(object_.name(), id);

    if (col_name == "Name")
    {
        if (text.size())
            ds.name(text);
    }
    else if (col_name == "Short Name")
    {
        if (!text.size())
            ds.removeShortName();
        else
            ds.shortName(text);
    }
    else if (col_name == "SAC")
    {
        if (!text.size())
            ds.removeSac();
        else
        {
            int sac = item->text().toInt(&ok);

            if (ok && sac >= 0 && sac <= 255)
                ds.sac(sac);
        }
    }
    else if (col_name == "SIC")
    {
        if (!text.size())
            ds.removeSic();
        else
        {
            int sic = item->text().toInt(&ok);

            if (ok && sic >= 0 && sic <= 255)
                ds.sic(sic);
        }
    }
    else if (col_name == "Latitude")
    {
        if (!text.size())
            ds.removeLatitude();
        else
        {
            double latitude = item->text().toDouble(&ok);

            if (ok)
                ds.latitude(latitude);
        }
    }
    else if (col_name == "Longitude")
    {
        if (!text.size())
            ds.removeLongitude();
        else
        {
            double longitude = item->text().toDouble(&ok);

            if (ok)
                ds.longitude(longitude);
        }
    }
    else if (col_name == "Altitude")
    {
        if (!text.size())
            ds.removeAltitude();
        else
        {
            double altitude = item->text().toDouble(&ok);

            if (ok)
                ds.altitude(altitude);
        }
    }
    else if (col_name == "Primary Azimuth StdDev")
    {
        if (!text.size())
            ds.removePrimaryAzimuthStdDev();
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
                ds.primaryAzimuthStdDev(value);
        }
    }
    else if (col_name == "Primary Range StdDev")
    {
        if (!text.size())
            ds.removePrimaryRangeStdDev();
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
                ds.primaryRangeStdDev(value);
        }
    }
    else if (col_name == "Primary IR Range Min")
    {
        if (!text.size())
            ds.removePrimaryRangeMin();
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
                ds.primaryRangeMin(value);
        }
    }
    else if (col_name == "Primary IR Range Max")
    {
        if (!text.size())
            ds.removePrimaryRangeMax();
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
                ds.primaryRangeMax(value);
        }
    }
    else if (col_name == "Secondary Azimuth StdDev")
    {
        if (!text.size())
            ds.removeSecondaryAzimuthStdDev();
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
                ds.secondaryAzimuthStdDev(value);
        }
    }
    else if (col_name == "Secondary Range StdDev")
    {
        if (!text.size())
            ds.removeSecondaryRangeStdDev();
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
                ds.secondaryRangeStdDev(value);
        }
    }
    else if (col_name == "Secondary IR Range Min")
    {
        if (!text.size())
            ds.removeSecondaryRangeMin();
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
                ds.secondaryRangeMin(value);
        }
    }
    else if (col_name == "Secondary IR Range Max")
    {
        if (!text.size())
            ds.removeSecondaryRangeMax();
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
                ds.secondaryRangeMax(value);
        }
    }
    else if (col_name == "Mode S Azimuth StdDev")
    {
        if (!text.size())
            ds.removeModeSAzimuthStdDev();
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
                ds.modeSAzimuthStdDev(value);
        }
    }
    else if (col_name == "Mode S Range StdDev")
    {
        if (!text.size())
            ds.removeModeSRangeStdDev();
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
                ds.modeSRangeStdDev(value);
        }
    }
    else if (col_name == "Mode S IR Range Min")
    {
        if (!text.size())
            ds.removeModeSRangeMin();
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
                ds.modeSRangeMin(value);
        }
    }
    else if (col_name == "Mode S IR Range Max")
    {
        if (!text.size())
            ds.removeModeSRangeMax();
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
                ds.modeSRangeMax(value);
        }
    }
    else
        assert(false);  // impossible

    update();
}

void DBOEditDataSourcesWidget::dbItemChangedSlot(QTableWidgetItem* item)
{
    assert(item);
    assert(db_ds_table_);

    bool ok;
    unsigned int id = item->data(Qt::UserRole).toUInt(&ok);
    assert(ok);
    int col = db_ds_table_->currentColumn();
    assert (col < table_columns_.size());
    std::string col_name = table_columns_.at(col).toStdString();

    std::string text = item->text().toStdString();

    loginf << "DBOEditDataSourcesWidget: dbItemChanged: id " << id << " col " << col << " text '"
           << text << "'";

    assert(object_.hasDataSource(id));

    DBODataSource& ds = object_.getDataSource(id);

    if (col_name == "Name")
    {
        if (text.size())
        {
            ds.name(text);
            ds.updateInDatabase();
        }
    }
    else if (col_name == "Short Name")
    {
        if (!text.size())
            ds.removeShortName();
        else
            ds.shortName(text);

        ds.updateInDatabase();
    }
    else if (col_name == "SAC")
    {
        if (!text.size())
        {
            ds.removeSac();
            ds.updateInDatabase();
        }
        else
        {
            int sac = item->text().toInt(&ok);

            if (ok && sac >= 0 && sac <= 255)
            {
                ds.sac(sac);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "SIC")
    {
        if (!text.size())
        {
            ds.removeSic();
            ds.updateInDatabase();
        }
        else
        {
            int sic = item->text().toInt(&ok);

            if (ok && sic >= 0 && sic <= 255)
            {
                ds.sic(sic);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Latitude")
    {
        if (!text.size())
        {
            ds.removeLatitude();
            ds.updateInDatabase();
        }
        else
        {
            double latitude = item->text().toDouble(&ok);

            if (ok)
            {
                ds.latitude(latitude);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Longitude")
    {
        if (!text.size())
        {
            ds.removeLongitude();
            ds.updateInDatabase();
        }
        else
        {
            double longitude = item->text().toDouble(&ok);

            if (ok)
            {
                ds.longitude(longitude);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Altitude")
    {
        if (!text.size())
        {
            ds.removeAltitude();
            ds.updateInDatabase();
        }
        else
        {
            double altitude = item->text().toDouble(&ok);

            if (ok)
            {
                ds.altitude(altitude);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Primary Azimuth StdDev")
    {
        if (!text.size())
        {
            ds.removePrimaryAzimuthStdDev();
            ds.updateInDatabase();
        }
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
            {
                ds.primaryAzimuthStdDev(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Primary Range StdDev")
    {
        if (!text.size())
        {
            ds.removePrimaryRangeStdDev();
            ds.updateInDatabase();
        }
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
            {
                ds.primaryRangeStdDev(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Primary IR Range Min")
    {
        if (!text.size())
        {
            ds.removePrimaryRangeMin();
            ds.updateInDatabase();
        }
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
            {
                ds.primaryRangeMin(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Primary IR Range Max")
    {
        if (!text.size())
        {
            ds.removePrimaryRangeMax();
            ds.updateInDatabase();
        }
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
            {
                ds.primaryRangeMax(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Secondary Azimuth StdDev")
    {
        if (!text.size())
        {
            ds.removeSecondaryAzimuthStdDev();
            ds.updateInDatabase();
        }
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
            {
                ds.secondaryAzimuthStdDev(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Secondary Range StdDev")
    {
        if (!text.size())
        {
            ds.removeSecondaryRangeStdDev();
            ds.updateInDatabase();
        }
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
            {
                ds.secondaryRangeStdDev(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Secondary IR Range Min")
    {
        if (!text.size())
        {
            ds.removeSecondaryRangeMin();
            ds.updateInDatabase();
        }
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
            {
                ds.secondaryRangeMin(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Secondary IR Range Max")
    {
        if (!text.size())
        {
            ds.removeSecondaryRangeMax();
            ds.updateInDatabase();
        }
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
            {
                ds.secondaryRangeMax(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Mode S Azimuth StdDev")
    {
        if (!text.size())
        {
            ds.removeModeSAzimuthStdDev();
            ds.updateInDatabase();
        }
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
            {
                ds.modeSAzimuthStdDev(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Mode S Range StdDev")
    {
        if (!text.size())
        {
            ds.removeModeSRangeStdDev();
            ds.updateInDatabase();
        }
        else
        {
            double value = item->text().toDouble(&ok);

            if (ok)
            {
                ds.modeSRangeStdDev(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Mode S IR Range Min")
    {
        if (!text.size())
        {
            ds.removeModeSRangeMin();
            ds.updateInDatabase();
        }
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
            {
                ds.modeSRangeMin(value);
                ds.updateInDatabase();
            }
        }
    }
    else if (col_name == "Mode S IR Range Max")
    {
        if (!text.size())
        {
            ds.removeModeSRangeMax();
            ds.updateInDatabase();
        }
        else
        {
            int value = item->text().toInt(&ok);

            if (ok)
            {
                ds.modeSRangeMax(value);
                ds.updateInDatabase();
            }
        }
    }
    else
        assert(false);  // impossible

    update();
    emit dbItemChangedSignal();
}

void DBOEditDataSourcesWidget::clearSyncOptions()
{
    action_heading_ = "No actions defined";
    action_collection_.clear();
}

void DBOEditDataSourcesWidget::displaySyncOptions()
{
    loginf << "DBOEditDataSourcesWidget: displaySyncOptions";

    assert(action_heading_label_);
    action_heading_label_->setText(action_heading_.c_str());

    assert(action_layout_);
    QLayoutItem* child;
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
    assert(select_all_actions_);
    assert(deselect_all_actions_);
    assert(perform_actions_button_);

    bool disabled = !haveActionsToPerform();

    select_all_actions_->setDisabled(disabled);
    deselect_all_actions_->setDisabled(disabled);
    perform_actions_button_->setDisabled(disabled);
}

bool DBOEditDataSourcesWidget::haveActionsToPerform()
{
    //    for (auto& opt_it : action_collection_)
    //        if (opt_it.second.performFlag() && opt_it.second.currentActionId() != 0)
    //            return true;

    return action_collection_.size() > 0;
}
