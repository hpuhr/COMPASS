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
#include <QScrollArea>
#include <QTableWidget>
#include <QHeaderView>

#include "dboeditdatasourceswidget.h"
#include "dbobject.h"
#include "dboeditdatasourceactionoptionswidget.h"
#include "files.h"

using namespace Utils;

DBOEditDataSourcesWidget::DBOEditDataSourcesWidget(DBObject* object, QWidget *parent, Qt::WindowFlags f)
    : QWidget (parent, f), object_(object)
{
    assert (object_);
    action_heading_ = "No actions defined";

    QFont font_bold;
    font_bold.setBold(true);

    int frame_width_small = 1;

    QHBoxLayout* main_layout = new QHBoxLayout ();

    QVBoxLayout* sources_layout = new QVBoxLayout();

    // config ds stuff
    {
        QVBoxLayout* config_layout = new QVBoxLayout();

        QFrame *config_frame = new QFrame ();
        config_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        config_frame->setLineWidth(frame_width_small);

        QHBoxLayout* top_layout = new QHBoxLayout();

        QLabel *config_label = new QLabel ("Configuration Data Sources");
        config_label->setFont (font_bold);
        top_layout->addWidget (config_label);

        QPushButton* add_stored_button = new QPushButton ("Add New");
        connect(add_stored_button, &QPushButton::clicked, this, &DBOEditDataSourcesWidget::addStoredDSSlot);
        top_layout->addWidget(add_stored_button);

        QIcon down_icon(Files::getIconFilepath("down.png").c_str());

        sync_from_cfg_button_ = new QPushButton ("Sync to DB");
        sync_from_cfg_button_->setIcon(down_icon);
        connect(sync_from_cfg_button_, SIGNAL(clicked()), this, SLOT(syncOptionsFromCfgSlot()));
        top_layout->addWidget(sync_from_cfg_button_);

        config_layout->addLayout(top_layout);

        config_ds_table_ = new QTableWidget ();
        config_ds_table_->setEditTriggers(QAbstractItemView::AllEditTriggers);
        config_ds_table_->setColumnCount(table_columns_.size());
        config_ds_table_->setHorizontalHeaderLabels(table_columns_);
        config_ds_table_->verticalHeader()->setVisible(false);
        connect (config_ds_table_, &QTableWidget::itemChanged, this, &DBOEditDataSourcesWidget::configItemChanged);
        // update done later

        config_layout->addWidget(config_ds_table_);

        config_frame->setLayout (config_layout);

        QScrollArea *config_scroll = new QScrollArea ();
        config_scroll->setWidgetResizable (true);
        config_scroll->setWidget(config_frame);

        sources_layout->addWidget(config_scroll);
    }

    // db stuff
    {
        QVBoxLayout* db_layout = new QVBoxLayout();

        QFrame *db_frame = new QFrame ();
        db_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        db_frame->setLineWidth(frame_width_small);

        QHBoxLayout* top_layout = new QHBoxLayout();

        QLabel *db_label = new QLabel ("Database Data Sources");
        db_label->setFont (font_bold);
        top_layout->addWidget (db_label);

        QIcon up_icon(Files::getIconFilepath("up.png").c_str());

        sync_from_db_button_ = new QPushButton ("Sync To Config");
        sync_from_db_button_->setIcon(up_icon);
        connect(sync_from_db_button_, SIGNAL(clicked()), this, SLOT(syncOptionsFromDBSlot()));
        top_layout->addWidget(sync_from_db_button_);

        db_layout->addLayout(top_layout);

        db_ds_table_ = new QTableWidget ();
        db_ds_table_->setEditTriggers(QAbstractItemView::AllEditTriggers);
        db_ds_table_->setColumnCount(table_columns_.size());
        db_ds_table_->setHorizontalHeaderLabels(table_columns_);
        db_ds_table_->verticalHeader()->setVisible(false);
        connect (db_ds_table_, &QTableWidget::itemChanged, this, &DBOEditDataSourcesWidget::dbItemChanged);
        // update done later

        db_layout->addWidget(db_ds_table_);

        db_frame->setLayout (db_layout);

        QScrollArea *db_scroll = new QScrollArea ();
        db_scroll->setWidgetResizable (true);
        db_scroll->setWidget(db_frame);

        sources_layout->addWidget(db_scroll);
    }

    main_layout->addLayout(sources_layout);

    // action stuff
    {
        QVBoxLayout* action_layout = new QVBoxLayout();

        QFrame *action_frame = new QFrame ();
        action_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        action_frame->setLineWidth(frame_width_small);

        action_heading_label_ = new QLabel (action_heading_.c_str());
        action_heading_label_->setFont (font_bold);
        action_layout->addWidget (action_heading_label_);

        action_layout->addWidget(new QLabel());

        action_layout_ = new QGridLayout();
        action_layout->addLayout(action_layout_);

        // action selection buttons
        QHBoxLayout* action_select_layout = new QHBoxLayout();

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

        main_layout->addWidget(action_frame);
    }

    update ();

    //main_layout->addLayout(sources_layout);
    setLayout (main_layout);
}

DBOEditDataSourcesWidget::~DBOEditDataSourcesWidget()
{
    logdbg << "DBOEditDataSourcesWidget: dtor";
}

void DBOEditDataSourcesWidget::update ()
{
    logdbg << "DBOEditDataSourcesWidget: update";

    assert (config_ds_table_);
    assert (db_ds_table_);

    config_ds_table_->blockSignals(true);
    db_ds_table_->blockSignals(true);

    updateConfigDSTable ();
    updateDBDSTable ();
    updateColumnSizes();

    config_ds_table_->blockSignals(false);
    db_ds_table_->blockSignals(false);
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

void DBOEditDataSourcesWidget::updateConfigDSTable ()
{
    logdbg << "DBOEditDataSourcesWidget: updateConfigDSTable";

    assert (object_);
    assert (config_ds_table_);

    config_ds_table_->clearContents();
    config_ds_table_->setRowCount(object_->storedDataSources().size());

    int row = 0;
    int col = 0;
    for (auto& ds_it : object_->storedDataSources())
    {
        unsigned int id = ds_it.second.id();

        col = 0;
        { // ID
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(id));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            item->setData(Qt::UserRole, QVariant(id));
            config_ds_table_->setItem(row, col, item);
        }

        { // name
            ++col;
            QTableWidgetItem* item = new QTableWidgetItem(ds_it.second.name().c_str());
            item->setData(Qt::UserRole, QVariant(id));
            config_ds_table_->setItem(row, col, item);
        }

        { // short name
            ++col;
            if (ds_it.second.hasShortName())
            {
                QTableWidgetItem* item = new QTableWidgetItem(ds_it.second.shortName().c_str());
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }


        { // sac
            ++col;
            if (ds_it.second.hasSac())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number((uint)ds_it.second.sac()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        { // sic
            ++col;
            if (ds_it.second.hasSic())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number((uint)ds_it.second.sic()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        { // latitude
            ++col;
            if (ds_it.second.hasLatitude())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number(ds_it.second.latitude(), 'g', 10));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        { // longitude
            ++col;
            if (ds_it.second.hasLongitude())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number(ds_it.second.longitude(), 'g', 10));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);
            }
        }

        { // altitude
            ++col;
            if (ds_it.second.hasAltitude())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number(ds_it.second.altitude()));
                item->setData(Qt::UserRole, QVariant(id));
                config_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                config_ds_table_->setItem(row, col, item);;
            }
        }

        ++row;
    }
}
void DBOEditDataSourcesWidget::updateDBDSTable ()
{
    logdbg << "DBOEditDataSourcesWidget: updateDBDSTable";

    assert (db_ds_table_);

    db_ds_table_->clearContents();

    db_ds_table_->setRowCount(object_->dataSources().size());

    int row = 0;
    int col = 0;
    for (auto& ds_it : object_->dataSources())
    {
        unsigned int id = ds_it.second.id();

        col = 0;
        { // ID
            QTableWidgetItem* item = new QTableWidgetItem(QString::number(id));
            item->setData(Qt::UserRole, QVariant(id));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            db_ds_table_->setItem(row, col, item);
        }

        { // name
            ++col;
            QTableWidgetItem* item = new QTableWidgetItem(ds_it.second.name().c_str());
            item->setData(Qt::UserRole, QVariant(id));
            db_ds_table_->setItem(row, col, item);
        }

        { // short namme
            ++col;
            if (ds_it.second.hasShortName())
            {
                QTableWidgetItem* item = new QTableWidgetItem(ds_it.second.shortName().c_str());
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }


        { // sac
            ++col;
            if (ds_it.second.hasSac())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number((uint)ds_it.second.sac()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        { // sic
            ++col;
            if (ds_it.second.hasSic())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number((uint)ds_it.second.sic()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        { // latitude
            ++col;
            if (ds_it.second.hasLatitude())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number(ds_it.second.latitude(), 'g', 10));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        { // longitude
            ++col;
            if (ds_it.second.hasLongitude())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number(ds_it.second.longitude(), 'g', 10));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }

        { // altitude
            ++col;
            if (ds_it.second.hasAltitude())
            {
                QTableWidgetItem* item = new QTableWidgetItem(QString::number(ds_it.second.altitude()));
                item->setData(Qt::UserRole, QVariant(id));
                db_ds_table_->setItem(row, col, item);
            }
            else
            {
                QTableWidgetItem* item = new QTableWidgetItem ();
                item->setData(Qt::UserRole, QVariant(id));
                item->setBackground(Qt::darkGray);
                db_ds_table_->setItem(row, col, item);
            }
        }
        ++row;
    }
}

void DBOEditDataSourcesWidget::updateColumnSizes ()
{
    assert (config_ds_table_);
    assert (db_ds_table_);

    config_ds_table_->horizontalHeader()->blockSignals(true);
    db_ds_table_->horizontalHeader()->blockSignals(true);

    for (int cnt=0; cnt < table_columns_.size(); ++cnt)
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


void DBOEditDataSourcesWidget::configItemChanged(QTableWidgetItem *item)
{
    assert (item);
    assert (config_ds_table_);

    bool ok;
    unsigned int id = item->data(Qt::UserRole).toUInt(&ok);
    assert (ok);
    int col = config_ds_table_->currentColumn();

    std::string text = item->text().toStdString();

    loginf << "DBOEditDataSourcesWidget: configItemChanged: id " << id << " col " << col << " text '" << text << "'";

    assert (object_->hasStoredDataSource(id));

    StoredDBODataSource& ds = object_->storedDataSource (id);

    if (col == 1)
    {
        if (text.size())
            ds.name(text);
    }
    else if (col == 2)
    {
        if (!text.size())
            ds.removeShortName();
        else
            ds.shortName(text);
    }
    else if (col == 3)
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
    else if (col == 4)
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
    else if (col == 5)
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
    else if (col == 6)
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
    else if (col == 7)
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
    else
        assert (false); // impossible

    update();
}

void DBOEditDataSourcesWidget::dbItemChanged(QTableWidgetItem* item)
{
    assert (item);
    assert (db_ds_table_);

    bool ok;
    unsigned int id = item->data(Qt::UserRole).toUInt(&ok);
    assert (ok);
    int col = db_ds_table_->currentColumn();

    std::string text = item->text().toStdString();

    loginf << "DBOEditDataSourcesWidget: dbItemChanged: id " << id << " col " << col << " text '" << text << "'";

    assert (object_->hasDataSource(id));

    DBODataSource& ds = object_->getDataSource(id);

    if (col == 1)
    {
        if (text.size())
        {
            ds.name(text);
            ds.updateInDatabase();
        }
    }
    else if (col == 2)
    {
        if (!text.size())
            ds.removeShortName();
        else
            ds.shortName(text);

        ds.updateInDatabase();
    }
    else if (col == 3)
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
    else if (col == 4)
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
    else if (col == 5)
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
    else if (col == 6)
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
    else if (col == 7)
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
    else
        assert (false); // impossible

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
