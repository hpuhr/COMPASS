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
    : Task("CreateAssociationsTask", "Associate Target Reports", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_assoc.json")
{
    tooltip_ =
            "Allows creation of UTNs and target report association based on Mode S Addresses.";

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

bool CreateAssociationsTask::checkPrerequisites()
{
    logdbg << "CreateAssociationsTask: checkPrerequisites: ready "
           << COMPASS::instance().interface().ready();

    if (!COMPASS::instance().interface().ready())
        return false;

    logdbg << "CreateAssociationsTask: checkPrerequisites: done "
           << COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME);

    if (COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = COMPASS::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    if (!canRun())
        return false;

    // check if was post-processed
    //    logdbg << "CreateAssociationsTask: checkPrerequisites: post "
    //           << COMPASS::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME);

    //    if (!COMPASS::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME))
    //        return false;

    //    logdbg << "CreateAssociationsTask: checkPrerequisites: post2 "
    //           << COMPASS::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME);

    //    if (COMPASS::instance().interface().getProperty(PostProcessTask::DONE_PROPERTY_NAME) != "1")
    //        return false;

    // check if hash var exists in all data
    DBContentManager& object_man = COMPASS::instance().dbContentManager();

    if (!object_man.hasData())
        return false;

    logdbg << "CreateAssociationsTask: checkPrerequisites: ok";

    return true;
}

bool CreateAssociationsTask::isRecommended()
{
    //    if (!checkPrerequisites())
    //        return false;

    //    return !done_;

    return false;
}

bool CreateAssociationsTask::canRun()
{
    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    loginf << "CreateAssociationsTask: canRun: metas ";
    if (!dbcontent_man.existsMetaVariable(DBContent::meta_var_rec_num_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_datasource_id_.name())
            || !dbcontent_man.existsMetaVariable(DBContent::meta_var_tod_.name())
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
        if (dbo_it.first == "RefTraj") // TODO
            continue;

        loginf << "CreateAssociationsTask: canRun: metas in dbcontent " << dbo_it.first;

        if (!dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_datasource_id_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_tod_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_m3a_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_mc_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_latitude_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_longitude_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_associations_.name()).existsIn(dbo_it.first)
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

    task_manager_.appendInfo("CreateAssociationsTask: started");

    start_time_ = boost::posix_time::microsec_clock::local_time();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    assert(!status_dialog_);
    status_dialog_.reset(new CreateAssociationsStatusDialog(*this));
    connect(status_dialog_.get(), &CreateAssociationsStatusDialog::closeSignal, this,
            &CreateAssociationsTask::closeStatusDialogSlot);
    status_dialog_->markStartTime();
    status_dialog_->setAssociationStatus("Loading Data");
    status_dialog_->show();

    checkAndSetMetaVariable(DBContent::meta_var_rec_num_.name(), &rec_num_var_);
    checkAndSetMetaVariable(DBContent::meta_var_datasource_id_.name(), &ds_id_var_);
    checkAndSetMetaVariable(DBContent::meta_var_line_id_.name(), &line_id_var_);
    checkAndSetMetaVariable(DBContent::meta_var_tod_.name(), &tod_var_);
    checkAndSetMetaVariable(DBContent::meta_var_ta_.name(), &target_addr_var_);
    checkAndSetMetaVariable(DBContent::meta_var_ti_.name(), &target_id_var_);
    checkAndSetMetaVariable(DBContent::meta_var_track_num_.name(), &track_num_var_);
    checkAndSetMetaVariable(DBContent::meta_var_track_end_.name(), &track_end_var_);
    checkAndSetMetaVariable(DBContent::meta_var_m3a_.name(), &mode_3a_var_);
    checkAndSetMetaVariable(DBContent::meta_var_mc_.name(), &mode_c_var_);
    checkAndSetMetaVariable(DBContent::meta_var_latitude_.name(), &latitude_var_);
    checkAndSetMetaVariable(DBContent::meta_var_longitude_.name(), &longitude_var_);
    checkAndSetMetaVariable(DBContent::meta_var_associations_.name(), &associations_var_);


    DBContentManager& object_man = COMPASS::instance().dbContentManager();

    COMPASS::instance().viewManager().disableDataDistribution(true);

    connect(&object_man, &DBContentManager::loadedDataSignal,
            this, &CreateAssociationsTask::loadedDataDataSlot);
    connect(&object_man, &DBContentManager::loadingDoneSignal,
            this, &CreateAssociationsTask::loadingDoneSlot);

    for (auto& dbo_it : object_man)
    {
        if (!dbo_it.second->hasData())
            continue;

        if (dbo_it.first == "RefTraj") // TODO
            continue;

        VariableSet read_set = getReadSetFor(dbo_it.first);

        dbo_it.second->load(read_set, false, true, &tod_var_->getFor(dbo_it.first), true);
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

void CreateAssociationsTask::loadedDataDataSlot(
        const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    data_ = data;
}

void CreateAssociationsTask::loadingDoneSlot()
{
    loginf << "CreateAssociationsTask: loadingDoneSlot";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
               this, &CreateAssociationsTask::loadedDataDataSlot);
    disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
               this, &CreateAssociationsTask::loadingDoneSlot);

    assert(status_dialog_);

    dbcontent_man.clearData();

    COMPASS::instance().viewManager().disableDataDistribution(false);

    dbo_loading_done_ = true;

    task_manager_.appendInfo("CreateAssociationsTask: data loading done");
    loginf << "CreateAssociationsTask: loadingDoneSlot: data loading done";

    //assert(!create_job_);

    create_job_ = std::make_shared<CreateAssociationsJob>(
                *this, COMPASS::instance().interface(), data_);

    connect(create_job_.get(), &CreateAssociationsJob::doneSignal, this,
            &CreateAssociationsTask::createDoneSlot, Qt::QueuedConnection);
    connect(create_job_.get(), &CreateAssociationsJob::obsoleteSignal, this,
            &CreateAssociationsTask::createObsoleteSlot, Qt::QueuedConnection);
    connect(create_job_.get(), &CreateAssociationsJob::statusSignal, this,
            &CreateAssociationsTask::associationStatusSlot, Qt::QueuedConnection);
    //        connect(create_job_.get(), &CreateAssociationsJob::saveAssociationsQuestionSignal,
    //                this, &CreateAssociationsTask::saveAssociationsQuestionSlot,
    //                Qt::QueuedConnection);

    JobManager::instance().addDBJob(create_job_);

    status_dialog_->setAssociationStatus("In Progress");
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

    status_dialog_->setAssociationStatus("Done");

    status_dialog_->setDone();

    if (!show_done_summary_)
        status_dialog_->close();

    create_job_ = nullptr;

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string time_str = String::timeStringFromDouble(diff.total_milliseconds() / 1000.0, false);

    COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

    task_manager_.appendSuccess("CreateAssociationsTask: done after " + time_str);

    COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");
    COMPASS::instance().dbContentManager().setAssociationsIdentifier("All");

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
    status_dialog_->setAssociationStatus(status.toStdString());
}

void CreateAssociationsTask::closeStatusDialogSlot()
{
    assert(status_dialog_);
    status_dialog_->close();
    status_dialog_ = nullptr;
}

MetaVariable* CreateAssociationsTask::keyVar() const
{
    assert (rec_num_var_);
    return rec_num_var_;
}

MetaVariable* CreateAssociationsTask::dsIdVar() const
{
    assert (ds_id_var_);
    return ds_id_var_;
}

MetaVariable* CreateAssociationsTask::lineIdVar() const
{
    assert (line_id_var_);
    return line_id_var_;
}


MetaVariable* CreateAssociationsTask::targetAddrVar() const
{
    assert (target_addr_var_);
    return target_addr_var_;
}

MetaVariable* CreateAssociationsTask::todVar() const
{
    assert (tod_var_);
    return tod_var_;
}

MetaVariable* CreateAssociationsTask::targetIdVar() const
{
    assert (target_id_var_);
    return target_id_var_;
}

MetaVariable* CreateAssociationsTask::trackNumVar() const
{
    assert (track_num_var_);
    return track_num_var_;
}

MetaVariable* CreateAssociationsTask::trackEndVar() const
{
    assert (track_end_var_);
    return track_end_var_;
}

MetaVariable* CreateAssociationsTask::mode3AVar() const
{
    assert (mode_3a_var_);
    return mode_3a_var_;
}

MetaVariable* CreateAssociationsTask::modeCVar() const
{
    assert (mode_c_var_);
    return mode_c_var_;
}

MetaVariable* CreateAssociationsTask::latitudeVar() const
{
    assert (latitude_var_);
    return latitude_var_;
}

MetaVariable* CreateAssociationsTask::longitudeVar() const
{
    assert (longitude_var_);
    return longitude_var_;
}

void CreateAssociationsTask::checkAndSetMetaVariable(const std::string& name_str,
                                                     MetaVariable** var)
{
    DBContentManager& object_man = COMPASS::instance().dbContentManager();

    if (!object_man.existsMetaVariable(name_str))
    {
        loginf << "CreateAssociationsTask: checkAndSetMetaVariable: var " << name_str
               << " does not exist";
        var = nullptr;
    }
    else
    {
        *var = &object_man.metaVariable(name_str);
        loginf << "CreateAssociationsTask: checkAndSetMetaVariable: var " << name_str
               << " set";
        assert(var);
    }
}

VariableSet CreateAssociationsTask::getReadSetFor(const std::string& dbo_name)
{
    VariableSet read_set;

    assert(ds_id_var_);
    assert(ds_id_var_->existsIn(dbo_name));
    read_set.add(ds_id_var_->getFor(dbo_name));

    assert(line_id_var_);
    assert(line_id_var_->existsIn(dbo_name));
    read_set.add(line_id_var_->getFor(dbo_name));

    assert(tod_var_);
    assert(tod_var_->existsIn(dbo_name));
    read_set.add(tod_var_->getFor(dbo_name));

    assert(target_addr_var_);
    if(target_addr_var_->existsIn(dbo_name))
        read_set.add(target_addr_var_->getFor(dbo_name));

    assert(target_id_var_);
    if(target_id_var_->existsIn(dbo_name))
        read_set.add(target_id_var_->getFor(dbo_name));

    assert(track_num_var_);
    if(track_num_var_->existsIn(dbo_name))
        read_set.add(track_num_var_->getFor(dbo_name));

    assert(track_end_var_);
    if(track_end_var_->existsIn(dbo_name))
        read_set.add(track_end_var_->getFor(dbo_name));

    assert(mode_3a_var_);
    assert(mode_3a_var_->existsIn(dbo_name));
    read_set.add(mode_3a_var_->getFor(dbo_name));

    assert(mode_c_var_);
    assert(mode_c_var_->existsIn(dbo_name));
    read_set.add(mode_c_var_->getFor(dbo_name));

    assert(latitude_var_);
    assert(latitude_var_->existsIn(dbo_name));
    read_set.add(latitude_var_->getFor(dbo_name));

    assert(longitude_var_);
    assert(longitude_var_->existsIn(dbo_name));
    read_set.add(longitude_var_->getFor(dbo_name));

    assert(associations_var_);
    assert(associations_var_->existsIn(dbo_name));
    read_set.add(associations_var_->getFor(dbo_name));

    // must be last for update process
    assert(rec_num_var_);
    assert(rec_num_var_->existsIn(dbo_name));
    read_set.add(rec_num_var_->getFor(dbo_name));

    return read_set;
}
