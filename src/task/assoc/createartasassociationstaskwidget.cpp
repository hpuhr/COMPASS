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

#include "createartasassociationstaskwidget.h"

#include "createartasassociationstask.h"
//#include "dbodatasourceselectioncombobox.h"
#include "compass.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "dbovariableselectionwidget.h"
#include "logger.h"
#include "metadbovariable.h"
#include "taskmanager.h"
#include "QDoubleValidator"

#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>


CreateARTASAssociationsTaskWidget::CreateARTASAssociationsTaskWidget(
        CreateARTASAssociationsTask& task, QWidget* parent, Qt::WindowFlags f)
    : TaskWidget(parent, f), task_(task)
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_layout = new QVBoxLayout();

    {
        QGridLayout* grid = new QGridLayout();
        int row_cnt = 0;

        TODO_ASSERT

        // tracker data source
//        grid->addWidget(new QLabel("Tracker Data Source"), row_cnt, 0);

//        assert(COMPASS::instance().objectManager().existsObject("Tracker"));
//        DBObject& dbo_tracker = COMPASS::instance().objectManager().object("Tracker");

//        ds_combo_ = new DBODataSourceSelectionComboBox(dbo_tracker);
//        connect(ds_combo_, &DBODataSourceSelectionComboBox::changedDataSourceSignal, this,
//                &CreateARTASAssociationsTaskWidget::currentDataSourceChangedSlot);

//        grid->addWidget(ds_combo_, row_cnt, 1);

        // tracker vars
        row_cnt++;
        grid->addWidget(new QLabel("Tracker Data Source ID Variable"), row_cnt, 0);
        ds_id_box_ = new DBContentVariableSelectionWidget();
        ds_id_box_->showDBOOnly("Tracker");
        connect(ds_id_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
        grid->addWidget(ds_id_box_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Tracker Track Number Variable"), row_cnt, 0);
        track_num_box_ = new DBContentVariableSelectionWidget();
        track_num_box_->showDBOOnly("Tracker");
        connect(track_num_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
        grid->addWidget(track_num_box_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Tracker Track Begin Variable"), row_cnt, 0);
        track_begin_box_ = new DBContentVariableSelectionWidget();
        track_begin_box_->showDBOOnly("Tracker");
        connect(track_begin_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
        grid->addWidget(track_begin_box_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Tracker Track Number Variable"), row_cnt, 0);
        track_end_box_ = new DBContentVariableSelectionWidget();
        track_end_box_->showDBOOnly("Tracker");
        connect(track_end_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
        grid->addWidget(track_end_box_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Tracker Track Coasting Variable"), row_cnt, 0);
        track_coasting_box_ = new DBContentVariableSelectionWidget();
        track_coasting_box_->showDBOOnly("Tracker");
        connect(track_coasting_box_, SIGNAL(selectionChanged()), this,
                SLOT(anyVariableChangedSlot()));
        grid->addWidget(track_coasting_box_, row_cnt, 1);

        // meta key var
        row_cnt++;
        grid->addWidget(new QLabel("Key Meta Variable"), row_cnt, 0);
        key_box_ = new DBContentVariableSelectionWidget();
        key_box_->showMetaVariablesOnly(true);
        connect(key_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
        grid->addWidget(key_box_, row_cnt, 1);

        // meta hash var
        row_cnt++;
        grid->addWidget(new QLabel("Hash Meta Variable"), row_cnt, 0);
        hash_box_ = new DBContentVariableSelectionWidget();
        hash_box_->showMetaVariablesOnly(true);
        connect(hash_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
        grid->addWidget(hash_box_, row_cnt, 1);

        // meta tod var
        row_cnt++;
        grid->addWidget(new QLabel("Time-of-day Meta Variable"), row_cnt, 0);
        tod_box_ = new DBContentVariableSelectionWidget();
        tod_box_->showMetaVariablesOnly(true);
        connect(tod_box_, SIGNAL(selectionChanged()), this, SLOT(anyVariableChangedSlot()));
        grid->addWidget(tod_box_, row_cnt, 1);

        // time stuff
        row_cnt++;
        grid->addWidget(new QLabel("End Track Time (s)"), row_cnt, 0);
        end_track_time_edit_ = new QLineEdit(QString::number(task.endTrackTime()));
        end_track_time_edit_->setValidator(new QDoubleValidator(10, 60 * 3600, 2, this));
        connect(end_track_time_edit_, &QLineEdit::textEdited, this,
                &CreateARTASAssociationsTaskWidget::endTrackTimeEditSlot);
        grid->addWidget(end_track_time_edit_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Association Time Past (s)"), row_cnt, 0);
        association_time_past_edit_ = new QLineEdit(QString::number(task.associationTimePast()));
        association_time_past_edit_->setValidator(new QDoubleValidator(0, 600, 2, this));
        connect(association_time_past_edit_, &QLineEdit::textEdited, this,
                &CreateARTASAssociationsTaskWidget::associationTimePastEditSlot);
        grid->addWidget(association_time_past_edit_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Association Time Future (s)"), row_cnt, 0);
        association_time_future_edit_ =
                new QLineEdit(QString::number(task.associationTimeFuture()));
        association_time_future_edit_->setValidator(new QDoubleValidator(0, 600, 2, this));
        connect(association_time_future_edit_, &QLineEdit::textEdited, this,
                &CreateARTASAssociationsTaskWidget::associationTimeFutureEditSlot);
        grid->addWidget(association_time_future_edit_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Acceptable Time for Misses (s)"), row_cnt, 0);
        misses_acceptable_time_edit_ = new QLineEdit(QString::number(task.missesAcceptableTime()));
        misses_acceptable_time_edit_->setValidator(new QDoubleValidator(0, 600, 2, this));
        connect(misses_acceptable_time_edit_, &QLineEdit::textEdited, this,
                &CreateARTASAssociationsTaskWidget::missesAcceptableTimeEditSlot);
        grid->addWidget(misses_acceptable_time_edit_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Dubious Association Distant Time (s)"), row_cnt, 0);
        associations_dubious_distant_time_edit_ =
                new QLineEdit(QString::number(task.associationsDubiousDistantTime()));
        associations_dubious_distant_time_edit_->setValidator(
                    new QDoubleValidator(0, 600, 2, this));
        connect(associations_dubious_distant_time_edit_, &QLineEdit::textEdited, this,
                &CreateARTASAssociationsTaskWidget::associationsDubiousDistantTimeEditSlot);
        grid->addWidget(associations_dubious_distant_time_edit_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Dubious Association Close Time Past (s)"), row_cnt, 0);
        association_dubious_close_time_past_edit_ =
                new QLineEdit(QString::number(task.associationDubiousCloseTimePast()));
        association_dubious_close_time_past_edit_->setValidator(
                    new QDoubleValidator(0, 600, 2, this));
        connect(association_dubious_close_time_past_edit_, &QLineEdit::textEdited, this,
                &CreateARTASAssociationsTaskWidget::associationDubiousCloseTimePastEditSlot);
        grid->addWidget(association_dubious_close_time_past_edit_, row_cnt, 1);

        row_cnt++;
        grid->addWidget(new QLabel("Dubious Association Close Time Future (s)"), row_cnt, 0);
        association_dubious_close_time_future_edit_ =
                new QLineEdit(QString::number(task.associationDubiousCloseTimeFuture()));
        association_dubious_close_time_future_edit_->setValidator(
                    new QDoubleValidator(0, 600, 2, this));
        connect(association_dubious_close_time_future_edit_, &QLineEdit::textEdited, this,
                &CreateARTASAssociationsTaskWidget::associationDubiousCloseTimeFutureEditSlot);
        grid->addWidget(association_dubious_close_time_future_edit_, row_cnt, 1);

        main_layout->addLayout(grid);
    }

    // track flag stuff
    ignore_track_end_associations_check_ = new QCheckBox("Ignore Track End Associations");
    ignore_track_end_associations_check_->setChecked(task_.ignoreTrackEndAssociations());
    connect(ignore_track_end_associations_check_, &QCheckBox::clicked, this,
            &CreateARTASAssociationsTaskWidget::anyTrackFlagChangedSlot);
    main_layout->addWidget(ignore_track_end_associations_check_);

    mark_track_end_associations_dubious_check_ =
            new QCheckBox("Mark Track End Associations Dubious");
    mark_track_end_associations_dubious_check_->setChecked(task_.markTrackEndAssociationsDubious());
    connect(mark_track_end_associations_dubious_check_, &QCheckBox::clicked, this,
            &CreateARTASAssociationsTaskWidget::anyTrackFlagChangedSlot);
    main_layout->addWidget(mark_track_end_associations_dubious_check_);

    ignore_track_coasting_associations_check_ = new QCheckBox("Ignore Track Coasting Associations");
    ignore_track_coasting_associations_check_->setChecked(task_.ignoreTrackCoastingAssociations());
    connect(ignore_track_coasting_associations_check_, &QCheckBox::clicked, this,
            &CreateARTASAssociationsTaskWidget::anyTrackFlagChangedSlot);
    main_layout->addWidget(ignore_track_coasting_associations_check_);

    mark_track_coasting_associations_dubious_check_ =
            new QCheckBox("Mark Track Costing Associations Dubious");
    mark_track_coasting_associations_dubious_check_->setChecked(
                task_.markTrackCoastingAssociationsDubious());
    connect(mark_track_coasting_associations_dubious_check_, &QCheckBox::clicked, this,
            &CreateARTASAssociationsTaskWidget::anyTrackFlagChangedSlot);
    main_layout->addWidget(mark_track_coasting_associations_dubious_check_);

    update();
    expertModeChangedSlot();

    setLayout(main_layout);
}

CreateARTASAssociationsTaskWidget::~CreateARTASAssociationsTaskWidget() {}

//void CreateARTASAssociationsTaskWidget::currentDataSourceChangedSlot()
//{
//    assert(ds_combo_);
//    task_.currentDataSourceName(ds_combo_->getDSName());
//}

void CreateARTASAssociationsTaskWidget::update()
{
    TODO_ASSERT

//    if (task_.currentDataSourceName().size() &&
//            ds_combo_->hasDataSource(task_.currentDataSourceName()))
//        ds_combo_->setDataSource(task_.currentDataSourceName());

//    if (ds_combo_->getDSName() != task_.currentDataSourceName())
//        task_.currentDataSourceName(ds_combo_->getDSName());

    DBContentManager& object_man = COMPASS::instance().objectManager();

    DBContent& track_object = object_man.object("Tracker");

    // tracker vats
    assert(ds_id_box_);
    if (task_.trackerDsIdVarStr().size() && track_object.hasVariable(task_.trackerDsIdVarStr()))
        ds_id_box_->selectedVariable(track_object.variable(task_.trackerDsIdVarStr()));

    assert(track_num_box_);
    if (task_.trackerTrackNumVarStr().size() &&
            track_object.hasVariable(task_.trackerTrackNumVarStr()))
        track_num_box_->selectedVariable(track_object.variable(task_.trackerTrackNumVarStr()));

    assert(track_begin_box_);
    if (task_.trackerTrackBeginVarStr().size() &&
            track_object.hasVariable(task_.trackerTrackBeginVarStr()))
        track_begin_box_->selectedVariable(track_object.variable(task_.trackerTrackBeginVarStr()));

    assert(track_end_box_);
    if (task_.trackerTrackEndVarStr().size() &&
            track_object.hasVariable(task_.trackerTrackEndVarStr()))
        track_end_box_->selectedVariable(track_object.variable(task_.trackerTrackEndVarStr()));

    assert(track_coasting_box_);
    if (task_.trackerTrackCoastingVarStr().size() &&
            track_object.hasVariable(task_.trackerTrackCoastingVarStr()))
        track_coasting_box_->selectedVariable(
                    track_object.variable(task_.trackerTrackCoastingVarStr()));

    // meta vars
    assert(key_box_);
    if (task_.keyVarStr().size() && object_man.existsMetaVariable(task_.keyVarStr()))
        key_box_->selectedMetaVariable(object_man.metaVariable(task_.keyVarStr()));

    assert(hash_box_);
    if (task_.hashVarStr().size() && object_man.existsMetaVariable(task_.hashVarStr()))
        hash_box_->selectedMetaVariable(object_man.metaVariable(task_.hashVarStr()));

    assert(tod_box_);
    if (task_.todVarStr().size() && object_man.existsMetaVariable(task_.todVarStr()))
        tod_box_->selectedMetaVariable(object_man.metaVariable(task_.todVarStr()));

    // track flag stuff
}

void CreateARTASAssociationsTaskWidget::anyVariableChangedSlot()
{
    // tracker vars
    assert(ds_id_box_);
    if (ds_id_box_->hasVariable())
        task_.trackerDsIdVarStr(ds_id_box_->selectedVariable().name());
    else
        task_.trackerDsIdVarStr("");

    assert(track_num_box_);
    if (track_num_box_->hasVariable())
        task_.trackerTrackNumVarStr(track_num_box_->selectedVariable().name());
    else
        task_.trackerTrackNumVarStr("");

    assert(track_begin_box_);
    if (track_begin_box_->hasVariable())
        task_.trackerTrackBeginVarStr(track_begin_box_->selectedVariable().name());
    else
        task_.trackerTrackBeginVarStr("");

    assert(track_end_box_);
    if (track_end_box_->hasVariable())
        task_.trackerTrackEndVarStr(track_end_box_->selectedVariable().name());
    else
        task_.trackerTrackEndVarStr("");

    assert(track_coasting_box_);
    if (track_coasting_box_->hasVariable())
        task_.trackerTrackCoastingVarStr(track_coasting_box_->selectedVariable().name());
    else
        task_.trackerTrackCoastingVarStr("");

    // meta vars
    assert(key_box_);
    if (key_box_->hasMetaVariable())
        task_.keyVarStr(key_box_->selectedMetaVariable().name());
    else
        task_.keyVarStr("");

    assert(hash_box_);
    if (hash_box_->hasMetaVariable())
        task_.hashVarStr(hash_box_->selectedMetaVariable().name());
    else
        task_.hashVarStr("");

    assert(tod_box_);
    if (tod_box_->hasMetaVariable())
        task_.todVarStr(tod_box_->selectedMetaVariable().name());
    else
        task_.todVarStr("");
}

void CreateARTASAssociationsTaskWidget::endTrackTimeEditSlot(QString value)
{
    bool ok;
    float val = value.toFloat(&ok);
    assert(ok);
    task_.endTrackTime(val);
}

void CreateARTASAssociationsTaskWidget::associationTimePastEditSlot(QString value)
{
    bool ok;
    float val = value.toFloat(&ok);
    assert(ok);
    task_.associationTimePast(val);
}

void CreateARTASAssociationsTaskWidget::associationTimeFutureEditSlot(QString value)
{
    bool ok;
    float val = value.toFloat(&ok);
    assert(ok);
    task_.associationTimeFuture(val);
}

void CreateARTASAssociationsTaskWidget::missesAcceptableTimeEditSlot(QString value)
{
    bool ok;
    float val = value.toFloat(&ok);
    assert(ok);
    task_.missesAcceptableTime(val);
}

void CreateARTASAssociationsTaskWidget::associationsDubiousDistantTimeEditSlot(QString value)
{
    bool ok;
    float val = value.toFloat(&ok);
    assert(ok);
    task_.associationsDubiousDistantTime(val);
}

void CreateARTASAssociationsTaskWidget::associationDubiousCloseTimePastEditSlot(QString value)
{
    bool ok;
    float val = value.toFloat(&ok);
    assert(ok);
    task_.associationDubiousCloseTimePast(val);
}

void CreateARTASAssociationsTaskWidget::associationDubiousCloseTimeFutureEditSlot(QString value)
{
    bool ok;
    float val = value.toFloat(&ok);
    assert(ok);
    task_.associationDubiousCloseTimeFuture(val);
}

void CreateARTASAssociationsTaskWidget::anyTrackFlagChangedSlot()
{
    assert(ignore_track_end_associations_check_);
    if ((ignore_track_end_associations_check_->checkState() == Qt::Checked) !=
            task_.ignoreTrackEndAssociations())
        task_.ignoreTrackEndAssociations(ignore_track_end_associations_check_->checkState() ==
                                         Qt::Checked);

    assert(mark_track_end_associations_dubious_check_);
    if ((mark_track_end_associations_dubious_check_->checkState() == Qt::Checked) !=
            task_.markTrackEndAssociationsDubious())
        task_.markTrackEndAssociationsDubious(
                    mark_track_end_associations_dubious_check_->checkState() == Qt::Checked);

    assert(ignore_track_coasting_associations_check_);
    if ((ignore_track_coasting_associations_check_->checkState() == Qt::Checked) !=
            task_.ignoreTrackCoastingAssociations())
        task_.ignoreTrackCoastingAssociations(
                    ignore_track_coasting_associations_check_->checkState() == Qt::Checked);

    assert(mark_track_coasting_associations_dubious_check_);
    if ((mark_track_coasting_associations_dubious_check_->checkState() == Qt::Checked) !=
            task_.markTrackCoastingAssociationsDubious())
        task_.markTrackCoastingAssociationsDubious(
                    mark_track_coasting_associations_dubious_check_->checkState() == Qt::Checked);
}

void CreateARTASAssociationsTaskWidget::expertModeChangedSlot()
{
    bool expert_mode = task_.manager().expertMode();

    assert(ds_id_box_);
    ds_id_box_->setEnabled(expert_mode);

    assert(track_num_box_);
    track_num_box_->setEnabled(expert_mode);

    assert(track_begin_box_);
    track_begin_box_->setEnabled(expert_mode);

    assert(track_end_box_);
    track_end_box_->setEnabled(expert_mode);

    assert(track_coasting_box_);
    track_coasting_box_->setEnabled(expert_mode);

    assert(key_box_);
    key_box_->setEnabled(expert_mode);

    assert(hash_box_);
    hash_box_->setEnabled(expert_mode);

    assert(tod_box_);
    tod_box_->setEnabled(expert_mode);
}
