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

#include "createassociationstask.h"

#include "compass.h"
#include "createassociationstaskdialog.h"
#include "createassociationsjob.h"
#include "datasourcemanager.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "jobmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "viewmanager.h"
#include "buffer.h"
#include "sqliteconnection.h"

#include <QApplication>
#include <QMessageBox>

#include <sstream>

using namespace std;
using namespace Utils;
using namespace dbContent;

const std::string CreateAssociationsTask::DONE_PROPERTY_NAME = "associations_created";

CreateAssociationsTask::CreateAssociationsTask(const std::string& class_id,
                                               const std::string& instance_id,
                                               TaskManager& task_manager)
    : Task("CreateAssociationsTask", "Calculate Unique Targets", task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_assoc.json")
{
    tooltip_ = "Create Unique Targets based on all DB Content.";

    // common
    registerParameter("associate_non_mode_s", &associate_non_mode_s_, true);
    registerParameter("clean_dubious_utns", &clean_dubious_utns_, true);
    registerParameter("mark_dubious_utns_unused", &mark_dubious_utns_unused_, false);
    registerParameter("comment_dubious_utns", &comment_dubious_utns_, true);

    // tracker stuff
    registerParameter("max_time_diff_tracker", &max_time_diff_tracker_, 15.0);

    registerParameter("max_distance_quit_tracker", &max_distance_quit_tracker_, 10*NM2M); // kb 5nm
    registerParameter("max_distance_dubious_tracker", &max_distance_dubious_tracker_, 3*NM2M); //kb 2.5? 2.5 lowest
    registerParameter("max_positions_dubious_tracker", &max_positions_dubious_tracker_, 5);

    registerParameter("max_distance_acceptable_tracker", &max_distance_acceptable_tracker_, NM2M/2.0);
    registerParameter("max_altitude_diff_tracker", &max_altitude_diff_tracker_, 300.0);

    registerParameter("min_updates_tracker", &min_updates_tracker_, 2); // kb 3!!!
    registerParameter("prob_min_time_overlap_tracker", &prob_min_time_overlap_tracker_, 0.5); //kb 0.7
    registerParameter("max_speed_tracker_kts", &max_speed_tracker_kts_, 100000);

    registerParameter("cont_max_time_diff_tracker", &cont_max_time_diff_tracker_, 30.0);
    registerParameter("cont_max_distance_acceptable_tracker", &cont_max_distance_acceptable_tracker_, 1852.0);

    // sensor
    registerParameter("max_time_diff_sensor", &max_time_diff_sensor_, 15.0);
    registerParameter("max_distance_acceptable_sensor", &max_distance_acceptable_sensor_, 2*NM2M);
    registerParameter("max_altitude_diff_sensor", &max_altitude_diff_sensor_, 300.0);

    // target id? kb: nope
    // kb: TODO ma 1bit hamming distance, especially g (1bit wrong)/v (!->at least 1bit wrong)
}

CreateAssociationsTask::~CreateAssociationsTask() {}


CreateAssociationsTaskDialog* CreateAssociationsTask::dialog()
{
    if (!dialog_)
    {
        dialog_.reset(new CreateAssociationsTaskDialog(*this));

        connect(dialog_.get(), &CreateAssociationsTaskDialog::runSignal,
                this, &CreateAssociationsTask::dialogRunSlot);

        connect(dialog_.get(), &CreateAssociationsTaskDialog::cancelSignal,
                this, &CreateAssociationsTask::dialogCancelSlot);
    }

    assert(dialog_);
    return dialog_.get();

}

bool CreateAssociationsTask::canRun()
{
    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    loginf << "CreateAssociationsTask: canRun: metas ";
    if (!dbcontent_man.existsMetaVariable(DBContent::meta_var_rec_num_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_datasource_id_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_timestamp_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_ta_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_ti_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_track_num_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_track_end_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_m3a_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_mc_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_latitude_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_longitude_.name()))
        return false;

    loginf << "CreateAssociationsTask: canRun: metas in dbcontent";
    for (auto& dbo_it : dbcontent_man)
    {
        loginf << "CreateAssociationsTask: canRun: metas in dbcontent " << dbo_it.first;

        if (!dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_datasource_id_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_m3a_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_mc_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_latitude_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_longitude_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).existsIn(dbo_it.first)
                )
            return false;

        if (dbo_it.first != "CAT001")  // check metas specific track vars
        {
            if (!dbcontent_man.metaVariable(DBContent::meta_var_ta_.name()).existsIn(dbo_it.first)
                    || !dbcontent_man.metaVariable(DBContent::meta_var_ti_.name()).existsIn(dbo_it.first)
                    )
                return false;
        }

        if (dbo_it.first != "CAT021" && dbo_it.first != "RefTraj")  // check metas specific track vars
        {
            if (!dbcontent_man.metaVariable(DBContent::meta_var_track_num_.name()).existsIn(dbo_it.first)
                    || !dbcontent_man.metaVariable(DBContent::meta_var_track_end_.name()).existsIn(dbo_it.first))
            {
                return false;
            }
        }
    }

    logdbg << "CreateAssociationsTask: canRun: ok";

    return true;
}

void CreateAssociationsTask::run()
{
    assert(canRun());

    loginf << "CreateAssociationsTask: run: started";

    start_time_ = boost::posix_time::microsec_clock::local_time();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    assert(!status_dialog_);
    status_dialog_.reset(new CreateAssociationsStatusDialog(*this));
    connect(status_dialog_.get(), &CreateAssociationsStatusDialog::closeSignal, this,
            &CreateAssociationsTask::closeStatusDialogSlot);
    status_dialog_->markStartTime();

    status_dialog_->setStatus("Deleting Previously Calculated RefTraj");
    status_dialog_->show();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    dbcontent_man.clearData();

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    for (auto& ds_it : ds_man.dbDataSources())
    {
        if (ds_it->isCalculatedReferenceSource())
        {
            status_dialog_->setStatus("Deleting From Data Source " + ds_it->name());

            dbcontent_man.dbContent("RefTraj").deleteDBContentData(ds_it->sac(), ds_it->sic());

            while (dbcontent_man.dbContent("RefTraj").isDeleting())
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                QThread::msleep(10);
            }

            ds_it->clearNumInserted("RefTraj");
        }
    }

    COMPASS::instance().viewManager().disableDataDistribution(true);

    status_dialog_->setStatus("Loading Data");

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &CreateAssociationsTask::loadedDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &CreateAssociationsTask::loadingDoneSlot);

    for (auto& dbo_it : dbcontent_man)
    {
        if (!dbo_it.second->hasData())
            continue;

        VariableSet read_set = getReadSetFor(dbo_it.first);

        dbo_it.second->load(read_set, false, false);
    }

    status_dialog_->show();
}

double CreateAssociationsTask::maxTimeDiffTracker() const
{
    return max_time_diff_tracker_;
}

void CreateAssociationsTask::maxTimeDiffTracker(double value)
{
    loginf << "CreateAssociationsTask: maxTimeDiffTracker: value " << value;
    max_time_diff_tracker_ = value;
}

double CreateAssociationsTask::maxTimeDiffSensor() const
{
    return max_time_diff_sensor_;
}

void CreateAssociationsTask::maxTimeDiffSensor(double value)
{
    loginf << "CreateAssociationsTask: maxTimeDiffSensor: value " << value;
    max_time_diff_sensor_ = value;
}

double CreateAssociationsTask::maxDistanceQuitTracker() const
{
    return max_distance_quit_tracker_;
}

void CreateAssociationsTask::maxDistanceQuitTracker(double value)
{
    loginf << "CreateAssociationsTask: maxDistanceQuitTracker: value " << value;
    max_distance_quit_tracker_ = value;
}

double CreateAssociationsTask::maxDistanceDubiousTracker() const
{
    return max_distance_dubious_tracker_;
}

void CreateAssociationsTask::maxDistanceDubiousTracker(double value)
{
    loginf << "CreateAssociationsTask: maxDistanceDubiousTracker: value " << value;
    max_distance_dubious_tracker_ = value;
}

unsigned int CreateAssociationsTask::maxPositionsDubiousTracker() const
{
    return max_positions_dubious_tracker_;
}

void CreateAssociationsTask::maxPositionsDubiousTracker(unsigned int value)
{
    loginf << "CreateAssociationsTask: maxPositionsDubiousTracker: value " << value;
    max_positions_dubious_tracker_ = value;
}

double CreateAssociationsTask::maxDistanceAcceptableTracker() const
{
    return max_distance_acceptable_tracker_;
}

void CreateAssociationsTask::maxDistanceAcceptableTracker(double value)
{
    loginf << "CreateAssociationsTask: maxDistanceAcceptableTracker: value " << value;
    max_distance_acceptable_tracker_ = value;
}

double CreateAssociationsTask::maxDistanceAcceptableSensor() const
{
    return max_distance_acceptable_sensor_;
}

void CreateAssociationsTask::maxDistanceAcceptableSensor(double value)
{
    loginf << "CreateAssociationsTask: maxDistanceAcceptableSensor: value " << value;
    max_distance_acceptable_sensor_ = value;
}

double CreateAssociationsTask::maxAltitudeDiffTracker() const
{
    return max_altitude_diff_tracker_;
}

void CreateAssociationsTask::maxAltitudeDiffTracker(double value)
{
    loginf << "CreateAssociationsTask: maxAltitudeDiffTracker: value " << value;
    max_altitude_diff_tracker_ = value;
}

double CreateAssociationsTask::maxAltitudeDiffSensor() const
{
    return max_altitude_diff_sensor_;
}

void CreateAssociationsTask::maxAltitudeDiffSensor(double value)
{
    loginf << "CreateAssociationsTask: maxAltitudeDiffSensor: value " << value;
    max_altitude_diff_sensor_ = value;
}

double CreateAssociationsTask::probMinTimeOverlapTracker() const
{
    return prob_min_time_overlap_tracker_;
}

void CreateAssociationsTask::probMinTimeOverlapTracker(double value)
{
    loginf << "CreateAssociationsTask: probMinTimeOverlapTracker: value " << value;
    prob_min_time_overlap_tracker_ = value;
}

unsigned int CreateAssociationsTask::minUpdatesTracker() const
{
    return min_updates_tracker_;
}

void CreateAssociationsTask::minUpdatesTracker(unsigned int value)
{
    loginf << "CreateAssociationsTask: minUpdatesTracker: value " << value;
    min_updates_tracker_ = value;
}

bool CreateAssociationsTask::associateNonModeS() const
{
    return associate_non_mode_s_;
}

void CreateAssociationsTask::associateNonModeS(bool value)
{
    loginf << "CreateAssociationsTask: associateNonModeS: value " << value;
    associate_non_mode_s_ = value;
}

double CreateAssociationsTask::maxSpeedTrackerKts() const
{
    return max_speed_tracker_kts_;
}

void CreateAssociationsTask::maxSpeedTrackerKts(double value)
{
    loginf << "CreateAssociationsTask: maxSpeedTrackerKts: value " << value;
    max_speed_tracker_kts_ = value;
}

bool CreateAssociationsTask::cleanDubiousUtns() const
{
    return clean_dubious_utns_;
}

void CreateAssociationsTask::cleanDubiousUtns(bool value)
{
    loginf << "CreateAssociationsTask: cleanDubiousUtns: value " << value;
    clean_dubious_utns_ = value;
}

bool CreateAssociationsTask::markDubiousUtnsUnused() const
{
    return mark_dubious_utns_unused_;
}

void CreateAssociationsTask::markDubiousUtnsUnused(bool value)
{
    loginf << "CreateAssociationsTask: markDubiousUtnsUnused: value " << value;
    mark_dubious_utns_unused_ = value;
}

bool CreateAssociationsTask::commentDubiousUtns() const
{
    return comment_dubious_utns_;
}

void CreateAssociationsTask::commentDubiousUtns(bool value)
{
    loginf << "CreateAssociationsTask: commentDubiousUtns: value " << value;
    comment_dubious_utns_ = value;
}

double CreateAssociationsTask::contMaxTimeDiffTracker() const
{
    return cont_max_time_diff_tracker_;
}

void CreateAssociationsTask::contMaxTimeDiffTracker(double cont_max_time_diff_tracker)
{
    cont_max_time_diff_tracker_ = cont_max_time_diff_tracker;
}

double CreateAssociationsTask::contMaxDistanceAcceptableTracker() const
{
    return cont_max_distance_acceptable_tracker_;
}

void CreateAssociationsTask::contMaxDistanceAcceptableTracker(double cont_max_distance_acceptable_tracker)
{
    cont_max_distance_acceptable_tracker_ = cont_max_distance_acceptable_tracker;
}

const std::set<unsigned int>& CreateAssociationsTask::modeAConspicuityCodes() const
{
    return mode_a_conspicuity_codes_;
}

void CreateAssociationsTask::loadedDataSlot(
        const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    data_ = data;

    assert (status_dialog_);
    status_dialog_->updateTime();
}

void CreateAssociationsTask::loadingDoneSlot()
{
    loginf << "CreateAssociationsTask: loadingDoneSlot";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    if (!cache_)
        cache_ = std::make_shared<dbContent::Cache> (dbcontent_man);

    cache_->add(data_);

    disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
               this, &CreateAssociationsTask::loadedDataSlot);
    disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
               this, &CreateAssociationsTask::loadingDoneSlot);

    assert(status_dialog_);
    status_dialog_->setStatus("Loading done, starting association");

    dbcontent_man.clearData();

    COMPASS::instance().viewManager().disableDataDistribution(false);

    loginf << "CreateAssociationsTask: loadingDoneSlot: data loading done";

    //assert(!create_job_);

    create_job_ = std::make_shared<CreateAssociationsJob>(
                *this, COMPASS::instance().interface(), cache_);

    connect(create_job_.get(), &CreateAssociationsJob::doneSignal, this,
            &CreateAssociationsTask::createDoneSlot, Qt::QueuedConnection);
    connect(create_job_.get(), &CreateAssociationsJob::obsoleteSignal, this,
            &CreateAssociationsTask::createObsoleteSlot, Qt::QueuedConnection);
    connect(create_job_.get(), &CreateAssociationsJob::statusSignal, this,
            &CreateAssociationsTask::associationStatusSlot, Qt::QueuedConnection);

    JobManager::instance().addDBJob(create_job_);

}

void CreateAssociationsTask::dialogRunSlot()
{
    loginf << "CreateAssociationsTask: dialogRunSlot";

    assert (dialog_);
    dialog_->hide();

    assert (canRun());
    run ();
}

void CreateAssociationsTask::dialogCancelSlot()
{
    loginf << "CreateAssociationsTask: dialogCancelSlot";

    assert (dialog_);
    dialog_->hide();
}

void CreateAssociationsTask::createDoneSlot()
{
    loginf << "CreateAssociationsTask: createDoneSlot";

    create_job_done_ = true;

    status_dialog_->setStatus("Done");

    assert (create_job_);

    status_dialog_->setAssociationsCounts(create_job_->associationCounts());
    status_dialog_->setDone();

    if (!show_done_summary_)
        status_dialog_->close();

    create_job_ = nullptr;

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string time_str = String::timeStringFromDouble(diff.total_milliseconds() / 1000.0, false);

    COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");
    COMPASS::instance().dbContentManager().setAssociationsIdentifier("All");
    COMPASS::instance().dbContentManager().resizeTargetListWidget();

    COMPASS::instance().interface().saveProperties();

    cache_ = nullptr;
    data_.clear();

    done_ = true;

    QApplication::restoreOverrideCursor();

    emit doneSignal(name_);
}

void CreateAssociationsTask::createObsoleteSlot()
{
    create_job_ = nullptr;
}

void CreateAssociationsTask::associationStatusSlot(QString status)
{
    assert(status_dialog_);
    status_dialog_->setStatus(status.toStdString());
}

void CreateAssociationsTask::closeStatusDialogSlot()
{
    assert(status_dialog_);
    status_dialog_->close();
    status_dialog_ = nullptr;
}

VariableSet CreateAssociationsTask::getReadSetFor(const std::string& dbcontent_name)
{
    VariableSet read_set;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    // ds id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_));

    // line id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_line_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_));

    // timestamp
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));

    // aircraft address
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ta_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ta_));

    // aircraft id
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ti_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ti_));

    // track num
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_num_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_num_));

    // track end
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_end_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_end_));

    // mode 3a
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_));

    // mode c
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_));

    if (dbcontent_name == "CAT062")
    {
        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat062_fl_measured_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat062_fl_measured_));
    }

    // latitude
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_));

    // longitude
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_));

    // assoc
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_utn_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_));

    // rec num, must be last for update process
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));

    // adsb mops
    if (dbcontent_name == "CAT021")
    {
        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_mops_version_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_mops_version_));
    }

    return read_set;
}
