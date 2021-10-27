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
#include "createassociationstaskwidget.h"
#include "createassociationsjob.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbodatasource.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "jobmanager.h"
#include "metadbovariable.h"
//#include "postprocesstask.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "buffer.h"
#include "sqliteconnection.h"

#include <QApplication>
#include <QMessageBox>
#include <sstream>

using namespace std;
using namespace Utils;

const std::string CreateAssociationsTask::DONE_PROPERTY_NAME = "associations_created";

CreateAssociationsTask::CreateAssociationsTask(const std::string& class_id,
                                               const std::string& instance_id,
                                               TaskManager& task_manager)
    : Task("CreateAssociationsTask", "Associate Target Reports", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_assoc.json")
{
    tooltip_ =
            "Allows creation of UTNs and target report association based on Mode S Addresses.";

    registerParameter("key_var_str", &key_var_str_, "rec_num");
    registerParameter("ds_id_var_str", &ds_id_var_str_, "ds_id");
    registerParameter("tod_var_str", &tod_var_str_, "tod");
    registerParameter("target_addr_var_str", &target_addr_var_str_, "target_addr");
    registerParameter("target_id_var_str", &target_id_var_str_, "callsign");
    registerParameter("track_num_var_str", &track_num_var_str_, "track_num");
    registerParameter("track_end_var_str", &track_end_var_str_, "track_end");
    registerParameter("mode_3a_var_str", &mode_3a_var_str_, "mode3a_code");
    registerParameter("mode_c_var_str", &mode_c_var_str_, "modec_code_ft");
    registerParameter("latitude_var_str", &latitude_var_str_, "pos_lat_deg");
    registerParameter("longitude_var_str", &longitude_var_str_, "pos_long_deg");

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


TaskWidget* CreateAssociationsTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new CreateAssociationsTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &CreateAssociationsTaskWidget::expertModeChangedSlot);
    }

    assert(widget_);
    return widget_.get();
}

void CreateAssociationsTask::deleteWidget() { widget_.reset(nullptr); }


bool CreateAssociationsTask::checkPrerequisites()
{
    logdbg << "CreateAssociationsTask: checkPrerequisites: ready "
           << COMPASS::instance().interface().ready();

    if (!COMPASS::instance().interface().ready())
        return false;

    if (COMPASS::instance().interface().connection().type() != SQLITE_IDENTIFIER)
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
    DBObjectManager& object_man = COMPASS::instance().objectManager();

    logdbg << "CreateAssociationsTask: checkPrerequisites: tracker hashes";
    assert (object_man.existsObject("Tracker"));

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
    DBObjectManager& object_man = COMPASS::instance().objectManager();

    logdbg << "CreateAssociationsTask: canRun: tracker " << object_man.existsObject("Tracker");

    // meta var stuff
    logdbg << "CreateAssociationsTask: canRun: meta vars";
    if (!key_var_str_.size()
            || !ds_id_var_str_.size()
            || !tod_var_str_.size()
            || !target_addr_var_str_.size()
            || !target_id_var_str_.size()
            || !track_num_var_str_.size()
            || !track_end_var_str_.size()
            || !mode_3a_var_str_.size()
            || !mode_c_var_str_.size()
            || !latitude_var_str_.size()
            || !longitude_var_str_.size())
        return false;

    logdbg << "CreateAssociationsTask: canRun: metas ";
    if (!object_man.existsMetaVariable(key_var_str_)
            || !object_man.existsMetaVariable(ds_id_var_str_)
            || !object_man.existsMetaVariable(tod_var_str_)
            || !object_man.existsMetaVariable(target_addr_var_str_)
            || !object_man.existsMetaVariable(target_id_var_str_)
            || !object_man.existsMetaVariable(track_num_var_str_)
            || !object_man.existsMetaVariable(track_end_var_str_)
            || !object_man.existsMetaVariable(mode_3a_var_str_)
            || !object_man.existsMetaVariable(mode_c_var_str_)
            || !object_man.existsMetaVariable(latitude_var_str_)
            || !object_man.existsMetaVariable(longitude_var_str_))
        return false;

    logdbg << "CreateAssociationsTask: canRun: metas in objects";
    for (auto& dbo_it : object_man)
    {
        if (!object_man.metaVariable(key_var_str_).existsIn(dbo_it.first)
                || !object_man.metaVariable(ds_id_var_str_).existsIn(dbo_it.first)
                || !object_man.metaVariable(tod_var_str_).existsIn(dbo_it.first)
                || !object_man.metaVariable(target_addr_var_str_).existsIn(dbo_it.first)
                || !object_man.metaVariable(target_id_var_str_).existsIn(dbo_it.first)
                //|| !object_man.metaVariable(track_num_var_str_).existsIn(dbo_it.first) // not in adsb
                //|| !object_man.metaVariable(track_end_var_str_).existsIn(dbo_it.first) // not in adsb
                || !object_man.metaVariable(mode_3a_var_str_).existsIn(dbo_it.first)
                || !object_man.metaVariable(mode_c_var_str_).existsIn(dbo_it.first)
                || !object_man.metaVariable(latitude_var_str_).existsIn(dbo_it.first)
                || !object_man.metaVariable(longitude_var_str_).existsIn(dbo_it.first)
                )
            return false;
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

    checkAndSetMetaVariable(key_var_str_, &key_var_);
    checkAndSetMetaVariable(ds_id_var_str_, &ds_id_var_);
    checkAndSetMetaVariable(tod_var_str_, &tod_var_);
    checkAndSetMetaVariable(target_addr_var_str_, &target_addr_var_);
    checkAndSetMetaVariable(target_id_var_str_, &target_id_var_);
    checkAndSetMetaVariable(track_num_var_str_, &track_num_var_);
    checkAndSetMetaVariable(track_end_var_str_, &track_end_var_);
    checkAndSetMetaVariable(mode_3a_var_str_, &mode_3a_var_);
    checkAndSetMetaVariable(mode_c_var_str_, &mode_c_var_);
    checkAndSetMetaVariable(latitude_var_str_, &latitude_var_);
    checkAndSetMetaVariable(longitude_var_str_, &longitude_var_);

    DBObjectManager& object_man = COMPASS::instance().objectManager();

    for (auto& dbo_it : object_man)
    {
        if (!dbo_it.second->hasData())
            continue;

        DBOVariableSet read_set = getReadSetFor(dbo_it.first);
        connect(dbo_it.second, &DBObject::newDataSignal, this,
                &CreateAssociationsTask::newDataSlot);
        connect(dbo_it.second, &DBObject::loadingDoneSignal, this,
                &CreateAssociationsTask::loadingDoneSlot);

        dbo_it.second->load(read_set, false, true, &tod_var_->getFor(dbo_it.first), true);

        dbo_loading_done_flags_[dbo_it.first] = false;
    }

    status_dialog_->setDBODoneFlags(dbo_loading_done_flags_);
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

void CreateAssociationsTask::newDataSlot(DBObject& object)
{
}

void CreateAssociationsTask::loadingDoneSlot(DBObject& object)
{
    loginf << "CreateAssociationsTask: loadingDoneSlot: object " << object.name();

    disconnect(&object, &DBObject::newDataSignal, this, &CreateAssociationsTask::newDataSlot);
    disconnect(&object, &DBObject::loadingDoneSignal, this,
               &CreateAssociationsTask::loadingDoneSlot);

    dbo_loading_done_flags_.at(object.name()) = true;

    assert(status_dialog_);
    status_dialog_->setDBODoneFlags(dbo_loading_done_flags_);

    dbo_loading_done_ = true;

    for (auto& done_it : dbo_loading_done_flags_)
        if (!done_it.second)
            dbo_loading_done_ = false;

    if (dbo_loading_done_)
    {
        task_manager_.appendInfo("CreateAssociationsTask: data loading done");
        loginf << "CreateAssociationsTask: loadingDoneSlot: data loading done";

        //assert(!create_job_);

        std::map<std::string, std::shared_ptr<Buffer>> buffers;

        DBObjectManager& object_man = COMPASS::instance().objectManager();

        for (auto& dbo_it : object_man)
        {
            if (!dbo_it.second->hasData())
                continue;

            buffers[dbo_it.first] = dbo_it.second->data();

            loginf << "CreateAssociationsTask: loadingDoneSlot: object " << object.name()
                   << " data " << buffers[dbo_it.first]->size();

            dbo_it.second->clearData();
        }

        create_job_ = std::make_shared<CreateAssociationsJob>(
                    *this, COMPASS::instance().interface(), buffers);

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

MetaDBOVariable* CreateAssociationsTask::keyVar() const
{
    assert (key_var_);
    return key_var_;
}

MetaDBOVariable* CreateAssociationsTask::dsIdVar() const
{
    assert (key_var_);
    return ds_id_var_;
}

MetaDBOVariable* CreateAssociationsTask::targetAddrVar() const
{
    assert (target_addr_var_);
    return target_addr_var_;
}

MetaDBOVariable* CreateAssociationsTask::todVar() const
{
    assert (tod_var_);
    return tod_var_;
}

MetaDBOVariable* CreateAssociationsTask::targetIdVar() const
{
    assert (target_id_var_);
    return target_id_var_;
}

MetaDBOVariable* CreateAssociationsTask::trackNumVar() const
{
    assert (track_num_var_);
    return track_num_var_;
}

MetaDBOVariable* CreateAssociationsTask::trackEndVar() const
{
    assert (track_end_var_);
    return track_end_var_;
}

MetaDBOVariable* CreateAssociationsTask::mode3AVar() const
{
    assert (mode_3a_var_);
    return mode_3a_var_;
}

MetaDBOVariable* CreateAssociationsTask::modeCVar() const
{
    assert (mode_c_var_);
    return mode_c_var_;
}

MetaDBOVariable* CreateAssociationsTask::latitudeVar() const
{
    assert (latitude_var_);
    return latitude_var_;
}

MetaDBOVariable* CreateAssociationsTask::longitudeVar() const
{
    assert (longitude_var_);
    return longitude_var_;
}

void CreateAssociationsTask::checkAndSetMetaVariable(std::string& name_str,
                                                     MetaDBOVariable** var)
{
    DBObjectManager& object_man = COMPASS::instance().objectManager();

    if (!object_man.existsMetaVariable(name_str))
    {
        loginf << "CreateAssociationsTask: checkAndSetMetaVariable: var " << name_str
               << " does not exist";
        name_str = "";
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

DBOVariableSet CreateAssociationsTask::getReadSetFor(const std::string& dbo_name)
{
    DBOVariableSet read_set;

    assert(key_var_);
    assert(key_var_->existsIn(dbo_name));
    read_set.add(key_var_->getFor(dbo_name));

    assert(ds_id_var_);
    assert(ds_id_var_->existsIn(dbo_name));
    read_set.add(ds_id_var_->getFor(dbo_name));

    assert(tod_var_);
    assert(tod_var_->existsIn(dbo_name));
    read_set.add(tod_var_->getFor(dbo_name));

    assert(target_addr_var_);
    assert(target_addr_var_->existsIn(dbo_name));
    read_set.add(target_addr_var_->getFor(dbo_name));

    assert(target_id_var_);
    assert(target_id_var_->existsIn(dbo_name));
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

    return read_set;
}
