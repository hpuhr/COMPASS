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

#include "createartasassociationstask.h"

#include "compass.h"
#include "createartasassociationsstatusdialog.h"
#include "createartasassociationstaskwidget.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbodatasource.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "jobmanager.h"
#include "metadbovariable.h"
#include "postprocesstask.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "sqliteconnection.h"

#include <QApplication>
#include <QMessageBox>
#include <sstream>

using namespace std;
using namespace Utils;

const std::string CreateARTASAssociationsTask::DONE_PROPERTY_NAME = "artas_associations_created";

CreateARTASAssociationsTask::CreateARTASAssociationsTask(const std::string& class_id,
                                                         const std::string& instance_id,
                                                         TaskManager& task_manager)
    : Task("CreateARTASAssociationsTask", "Associate ARTAS TRIs", true, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_artas_assoc.json")
{
    tooltip_ =
        "Allows creation of UTNs and target report association based on ARTAS tracks and the TRI "
        "information.";

    registerParameter("current_data_source_name", &current_data_source_name_, "");

    // tracker vars
    registerParameter("tracker_ds_id_var_str", &tracker_ds_id_var_str_, "ds_id");
    registerParameter("tracker_track_num_var_str", &tracker_track_num_var_str_, "track_num");
    registerParameter("tracker_track_begin_var_str", &tracker_track_begin_var_str_,
                      "track_created");
    registerParameter("tracker_track_end_var_str", &tracker_track_end_var_str_, "track_end");
    registerParameter("tracker_track_coasting_var_str", &tracker_track_coasting_var_str_,
                      "track_coasted");

    // meta vars
    registerParameter("key_var_str", &key_var_str_, "rec_num");
    registerParameter("hash_var_str", &hash_var_str_, "hash_code");
    registerParameter("tod_var_str", &tod_var_str_, "tod");

    // time stuff
    registerParameter("end_track_time", &end_track_time_, 300.0);

    registerParameter("association_time_past", &association_time_past_, 60.0);
    registerParameter("association_time_future", &association_time_future_, 2.0);

    registerParameter("misses_acceptable_time", &misses_acceptable_time_, 60.0);

    registerParameter("associations_dubious_distant_time", &associations_dubious_distant_time_,
                      30.0);
    registerParameter("association_dubious_close_time_past", &association_dubious_close_time_past_,
                      20.0);
    registerParameter("association_dubious_close_time_future",
                      &association_dubious_close_time_future_, 1.0);

    // track flag stuff
    registerParameter("ignore_track_end_associations", &ignore_track_end_associations_, true);
    registerParameter("mark_track_end_associations_dubious", &mark_track_end_associations_dubious_,
                      false);
    registerParameter("ignore_track_coasting_associations", &ignore_track_coasting_associations_,
                      true);
    registerParameter("mark_track_coasting_associations_dubious",
                      &mark_track_coasting_associations_dubious_, false);
}

CreateARTASAssociationsTask::~CreateARTASAssociationsTask() {}

TaskWidget* CreateARTASAssociationsTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new CreateARTASAssociationsTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &CreateARTASAssociationsTaskWidget::expertModeChangedSlot);
    }

    assert(widget_);
    return widget_.get();
}

void CreateARTASAssociationsTask::deleteWidget() { widget_.reset(nullptr); }

bool CreateARTASAssociationsTask::checkPrerequisites()
{
    logdbg << "CreateARTASAssociationsTask: checkPrerequisites: ready "
           << COMPASS::instance().interface().ready();

    if (!COMPASS::instance().interface().ready())
        return false;

    if (COMPASS::instance().interface().connection().type() != SQLITE_IDENTIFIER)
        return false;

    logdbg << "CreateARTASAssociationsTask: checkPrerequisites: done "
           << COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME);

    if (COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = COMPASS::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    if (!canRun())
        return false;

    // check if was post-processed
    logdbg << "CreateARTASAssociationsTask: checkPrerequisites: post "
           << COMPASS::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME);

    if (!COMPASS::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME))
        return false;

    logdbg << "CreateARTASAssociationsTask: checkPrerequisites: post2 "
           << COMPASS::instance().interface().hasProperty(PostProcessTask::DONE_PROPERTY_NAME);

    if (COMPASS::instance().interface().getProperty(PostProcessTask::DONE_PROPERTY_NAME) != "1")
        return false;

    // check if hash var exists in all data
    DBObjectManager& object_man = COMPASS::instance().objectManager();

    logdbg << "CreateARTASAssociationsTask: checkPrerequisites: tracker hashes";
    assert (object_man.existsObject("Tracker"));

    DBObject& tracker_obj = object_man.object("Tracker");
    if (!tracker_obj.hasData()) // check if tracker data exists
        return false;

    DBOVariable& tracker_hash_var = object_man.metaVariable(hash_var_str_).getFor("Tracker");
    if (tracker_hash_var.getMinString() == NULL_STRING || tracker_hash_var.getMaxString() == NULL_STRING)
        return false;  // tracker needs hash info no hashes


    logdbg << "CreateARTASAssociationsTask: checkPrerequisites: sensor hashes";
    bool any_data_found = false;
    for (auto& dbo_it : object_man)
    {
        if (dbo_it.first == "Tracker" ||
            !dbo_it.second->hasData())  // DBO other than tracker no data is acceptable
            continue;

        DBOVariable& hash_var = object_man.metaVariable(hash_var_str_).getFor(dbo_it.first);
        if (hash_var.getMinString() != NULL_STRING && hash_var.getMaxString() != NULL_STRING)
            any_data_found = true;  // has data and hashes
    }
    if (!any_data_found)
        return false; // sensor data needed

    logdbg << "CreateARTASAssociationsTask: checkPrerequisites: ok";
    return true;
}

bool CreateARTASAssociationsTask::isRecommended()
{
    //if (!checkPrerequisites())
    return false;

    //return !done_;
}

bool CreateARTASAssociationsTask::canRun()
{
    DBObjectManager& object_man = COMPASS::instance().objectManager();

    logdbg << "CreateARTASAssociationsTask: canRun: tracker " << object_man.existsObject("Tracker");

    if (!object_man.existsObject("Tracker"))
        return false;

    DBObject& tracker_object = object_man.object("Tracker");

    // tracker stuff
    logdbg << "CreateARTASAssociationsTask: canRun: tracker loadable " << tracker_object.loadable();
    if (!tracker_object.loadable())
        return false;

    logdbg << "CreateARTASAssociationsTask: canRun: tracker count " << tracker_object.count();
    if (!tracker_object.count())
        return false;

    // no data sources
    logdbg << "CreateARTASAssociationsTask: canRun: no tracker data sources "
           << (tracker_object.dsBegin() == tracker_object.dsEnd());
    if (tracker_object.dsBegin() == tracker_object.dsEnd())
        return false;

    bool ds_found{false};
    for (auto ds_it = tracker_object.dsBegin(); ds_it != tracker_object.dsEnd(); ++ds_it)
    {
        if ((ds_it->second.hasShortName() &&
             ds_it->second.shortName() == current_data_source_name_) ||
            (ds_it->second.name() == current_data_source_name_))
        {
            ds_found = true;
            break;
        }
    }

    logdbg << "CreateARTASAssociationsTask: canRun: tracker ds_found " << ds_found;
    if (!ds_found)
    {
        logdbg << "CreateARTASAssociationsTask: canRun: resetting current source to "
               << tracker_object.dsBegin()->second.name();
        current_data_source_name_ = tracker_object.dsBegin()->second.name();
    }

    loginf << "CreateARTASAssociationsTask: canRun: tracker vars";
    if (!tracker_object.hasVariable(tracker_track_num_var_str_) ||
        !tracker_object.hasVariable(tracker_track_begin_var_str_) ||
        !tracker_object.hasVariable(tracker_track_end_var_str_) ||
        !tracker_object.hasVariable(tracker_track_coasting_var_str_))
        return false;

    // meta var stuff
    logdbg << "CreateARTASAssociationsTask: canRun: meta vars";
    if (!key_var_str_.size() || !hash_var_str_.size() || !tod_var_str_.size())
        return false;

    logdbg << "CreateARTASAssociationsTask: canRun: metas in tracker";
    if (!object_man.existsMetaVariable(key_var_str_) ||
        !object_man.existsMetaVariable(hash_var_str_) ||
        !object_man.existsMetaVariable(tod_var_str_))
        return false;

    logdbg << "CreateARTASAssociationsTask: canRun: metas in objects";
    for (auto& dbo_it : object_man)
    {
        if (dbo_it.first == "RefTraj") // not set in references
            continue;

        if (!object_man.metaVariable(key_var_str_).existsIn(dbo_it.first) ||
            !object_man.metaVariable(hash_var_str_).existsIn(dbo_it.first) ||
            !object_man.metaVariable(tod_var_str_).existsIn(dbo_it.first))
            return false;
    }

    logdbg << "CreateARTASAssociationsTask: canRun: ok";
    return true;
}

void CreateARTASAssociationsTask::run()
{
    assert(canRun());

    loginf << "CreateARTASAssociationsTask: run: started";

    task_manager_.appendInfo("CreateARTASAssociationsTask: started");

    save_associations_ = true;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    assert(!status_dialog_);
    status_dialog_.reset(new CreateARTASAssociationsStatusDialog(*this));
    connect(status_dialog_.get(), &CreateARTASAssociationsStatusDialog::closeSignal, this,
            &CreateARTASAssociationsTask::closeStatusDialogSlot);
    status_dialog_->markStartTime();

    checkAndSetVariable(tracker_ds_id_var_str_, &tracker_ds_id_var_);
    checkAndSetVariable(tracker_track_num_var_str_, &tracker_track_num_var_);
    checkAndSetVariable(tracker_track_begin_var_str_, &tracker_track_begin_var_);
    checkAndSetVariable(tracker_track_end_var_str_, &tracker_track_end_var_);
    checkAndSetVariable(tracker_track_coasting_var_str_, &tracker_track_coasting_var_);

    checkAndSetMetaVariable(key_var_str_, &key_var_);
    checkAndSetMetaVariable(hash_var_str_, &hash_var_);
    checkAndSetMetaVariable(tod_var_str_, &tod_var_);

    DBObjectManager& object_man = COMPASS::instance().objectManager();

    for (auto& dbo_it : object_man)
    {
        if (!dbo_it.second->hasData())
            continue;

        if (dbo_it.first == "RefTraj") // not set in references
            continue;

        DBOVariableSet read_set = getReadSetFor(dbo_it.first);
        connect(dbo_it.second, &DBObject::newDataSignal, this,
                &CreateARTASAssociationsTask::newDataSlot);
        connect(dbo_it.second, &DBObject::loadingDoneSignal, this,
                &CreateARTASAssociationsTask::loadingDoneSlot);

        if (dbo_it.first == "Tracker")
        {
            DBObject& tracker_object = object_man.object("Tracker");

            bool ds_found{false};
            int ds_id{-1};
            for (auto ds_it = tracker_object.dsBegin(); ds_it != tracker_object.dsEnd(); ++ds_it)
            {
                if ((ds_it->second.hasShortName() &&
                     ds_it->second.shortName() == current_data_source_name_) ||
                    (!ds_it->second.hasShortName() &&
                     ds_it->second.name() == current_data_source_name_))
                {
                    ds_found = true;
                    ds_id = ds_it->first;
                    break;
                }
            }

            assert(ds_found);
            std::string custom_filter_clause{tracker_ds_id_var_str_ + " in (" +
                                             std::to_string(ds_id) + ")"};

            assert(tracker_ds_id_var_);

            //        void DBObject::load (DBOVariableSet& read_set,  std::string
            //        custom_filter_clause,
            //                             std::vector <DBOVariable*> filtered_variables, bool
            //                             use_order, DBOVariable* order_variable, bool
            //                             use_order_ascending, const std::string &limit_str)

            dbo_it.second->load(read_set, custom_filter_clause, {tracker_ds_id_var_}, false,
                                &tod_var_->getFor("Tracker"), false);
        }
        else
            dbo_it.second->load(read_set, false, false, nullptr, false);

        dbo_loading_done_flags_[dbo_it.first] = false;
    }

    status_dialog_->setDBODoneFlags(dbo_loading_done_flags_);
    status_dialog_->show();
}

void CreateARTASAssociationsTask::newDataSlot(DBObject& object)
{
}

void CreateARTASAssociationsTask::loadingDoneSlot(DBObject& object)
{
    loginf << "CreateARTASAssociationsTask: loadingDoneSlot: object " << object.name();

    disconnect(&object, &DBObject::newDataSignal, this, &CreateARTASAssociationsTask::newDataSlot);
    disconnect(&object, &DBObject::loadingDoneSignal, this,
               &CreateARTASAssociationsTask::loadingDoneSlot);

    dbo_loading_done_flags_.at(object.name()) = true;

    assert(status_dialog_);
    status_dialog_->setDBODoneFlags(dbo_loading_done_flags_);

    dbo_loading_done_ = true;

    for (auto& done_it : dbo_loading_done_flags_)
        if (!done_it.second)
            dbo_loading_done_ = false;

    if (dbo_loading_done_)
    {
        assert(!create_job_);

        std::map<std::string, std::shared_ptr<Buffer>> buffers;

        DBObjectManager& object_man = COMPASS::instance().objectManager();

        for (auto& dbo_it : object_man)
        {
            buffers[dbo_it.first] = dbo_it.second->data();
            dbo_it.second->clearData();
        }

        create_job_ = std::make_shared<CreateARTASAssociationsJob>(
            *this, COMPASS::instance().interface(), buffers);

        connect(create_job_.get(), &CreateARTASAssociationsJob::doneSignal, this,
                &CreateARTASAssociationsTask::createDoneSlot, Qt::QueuedConnection);
        connect(create_job_.get(), &CreateARTASAssociationsJob::obsoleteSignal, this,
                &CreateARTASAssociationsTask::createObsoleteSlot, Qt::QueuedConnection);
        connect(create_job_.get(), &CreateARTASAssociationsJob::statusSignal, this,
                &CreateARTASAssociationsTask::associationStatusSlot, Qt::QueuedConnection);
        connect(create_job_.get(), &CreateARTASAssociationsJob::saveAssociationsQuestionSignal,
                this, &CreateARTASAssociationsTask::saveAssociationsQuestionSlot,
                Qt::QueuedConnection);

        JobManager::instance().addDBJob(create_job_);

        status_dialog_->setAssociationStatus("In Progress");
    }
}

void CreateARTASAssociationsTask::createDoneSlot()
{
    loginf << "CreateARTASAssociationsTask: createDoneSlot";

    create_job_done_ = true;

    status_dialog_->setAssociationStatus("Done");
    status_dialog_->setFoundHashes(create_job_->foundHashes());
    status_dialog_->setMissingHashesAtBeginning(create_job_->missingHashesAtBeginning());
    status_dialog_->setMissingHashes(create_job_->missingHashes());
    status_dialog_->setDubiousAssociations(create_job_->dubiousAssociations());
    status_dialog_->setFoundDuplicates(create_job_->foundHashDuplicates());

    status_dialog_->setDone();

    if (!show_done_summary_)
        status_dialog_->close();

    create_job_ = nullptr;

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string time_str = String::timeStringFromDouble(diff.total_milliseconds() / 1000.0, false);

    if (save_associations_)
    {
        COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

        task_manager_.appendSuccess("CreateARTASAssociationsTask: done after " + time_str);
        done_ = true;
    }
    else
    {
        task_manager_.appendWarning("CreateARTASAssociationsTask: done after " + time_str +
                                    " without saving");
    }

    QApplication::restoreOverrideCursor();

    emit doneSignal(name_);
}

void CreateARTASAssociationsTask::createObsoleteSlot() { create_job_ = nullptr; }

std::string CreateARTASAssociationsTask::currentDataSourceName() const
{
    return current_data_source_name_;
}

void CreateARTASAssociationsTask::currentDataSourceName(const std::string& current_data_source_name)
{
    loginf << "CreateARTASAssociationsTask: currentDataSourceName: " << current_data_source_name;

    current_data_source_name_ = current_data_source_name;
}

DBOVariable* CreateARTASAssociationsTask::trackerDsIdVar() const { return tracker_ds_id_var_; }

std::string CreateARTASAssociationsTask::trackerDsIdVarStr() const
{
    return tracker_ds_id_var_str_;
}

void CreateARTASAssociationsTask::trackerDsIdVarStr(const std::string& var_str)
{
    loginf << "CreateARTASAssociationsTask: trackerDsIdVarStr: '" << var_str << "'";
    tracker_ds_id_var_str_ = var_str;
}

std::string CreateARTASAssociationsTask::trackerTrackNumVarStr() const
{
    return tracker_track_num_var_str_;
}

void CreateARTASAssociationsTask::trackerTrackNumVarStr(const std::string& var_str)
{
    loginf << "CreateARTASAssociationsTask: trackerTrackNumVarStr: '" << var_str << "'";
    tracker_track_num_var_str_ = var_str;
}

std::string CreateARTASAssociationsTask::trackerTrackBeginVarStr() const
{
    return tracker_track_begin_var_str_;
}

void CreateARTASAssociationsTask::trackerTrackBeginVarStr(const std::string& var_str)
{
    loginf << "CreateARTASAssociationsTask: trackerTrackBeginVarStr: '" << var_str << "'";
    tracker_track_begin_var_str_ = var_str;
}

std::string CreateARTASAssociationsTask::trackerTrackEndVarStr() const
{
    return tracker_track_end_var_str_;
}

void CreateARTASAssociationsTask::trackerTrackEndVarStr(const std::string& var_str)
{
    loginf << "CreateARTASAssociationsTask: trackerTrackEndVarStr: '" << var_str << "'";
    tracker_track_end_var_str_ = var_str;
}

std::string CreateARTASAssociationsTask::trackerTrackCoastingVarStr() const
{
    return tracker_track_coasting_var_str_;
}

void CreateARTASAssociationsTask::trackerTrackCoastingVarStr(const std::string& var_str)
{
    loginf << "CreateARTASAssociationsTask: trackerTrackCoastingVarStr: '" << var_str << "'";
    tracker_track_coasting_var_str_ = var_str;
}

std::string CreateARTASAssociationsTask::keyVarStr() const { return key_var_str_; }

void CreateARTASAssociationsTask::keyVarStr(const std::string& key_var_str)
{
    loginf << "CreateARTASAssociationsTask: keyVarStr: '" << key_var_str << "'";

    key_var_str_ = key_var_str;
}

std::string CreateARTASAssociationsTask::hashVarStr() const { return hash_var_str_; }

void CreateARTASAssociationsTask::hashVarStr(const std::string& hash_var_str)
{
    loginf << "CreateARTASAssociationsTask: hashVarStr: '" << hash_var_str << "'";

    hash_var_str_ = hash_var_str;
}

std::string CreateARTASAssociationsTask::todVarStr() const { return tod_var_str_; }

void CreateARTASAssociationsTask::todVarStr(const std::string& tod_var_str)
{
    loginf << "CreateARTASAssociationsTask: todVarStr: '" << tod_var_str << "'";

    tod_var_str_ = tod_var_str;
}

MetaDBOVariable* CreateARTASAssociationsTask::keyVar() const { return key_var_; }

MetaDBOVariable* CreateARTASAssociationsTask::hashVar() const { return hash_var_; }

MetaDBOVariable* CreateARTASAssociationsTask::todVar() const { return tod_var_; }

float CreateARTASAssociationsTask::endTrackTime() const { return end_track_time_; }

void CreateARTASAssociationsTask::endTrackTime(float end_track_time)
{
    loginf << "CreateARTASAssociationsTask: endTrackTime: " << end_track_time;

    end_track_time_ = end_track_time;
}

float CreateARTASAssociationsTask::associationTimePast() const { return association_time_past_; }

void CreateARTASAssociationsTask::associationTimePast(float association_time_past)
{
    loginf << "CreateARTASAssociationsTask: associationTimePast: " << association_time_past;

    association_time_past_ = association_time_past;
}

float CreateARTASAssociationsTask::associationTimeFuture() const
{
    return association_time_future_;
}

void CreateARTASAssociationsTask::associationTimeFuture(float association_time_future)
{
    loginf << "CreateARTASAssociationsTask: associationTimeFuture: " << association_time_future;

    association_time_future_ = association_time_future;
}

float CreateARTASAssociationsTask::missesAcceptableTime() const { return misses_acceptable_time_; }

void CreateARTASAssociationsTask::missesAcceptableTime(float misses_acceptable_time)
{
    loginf << "CreateARTASAssociationsTask: missesAcceptableTime: " << misses_acceptable_time;

    misses_acceptable_time_ = misses_acceptable_time;
}

float CreateARTASAssociationsTask::associationsDubiousDistantTime() const
{
    return associations_dubious_distant_time_;
}

void CreateARTASAssociationsTask::associationsDubiousDistantTime(
    float associations_dubious_distant_time)
{
    loginf << "CreateARTASAssociationsTask: associationsDubiousDistantTime: "
           << associations_dubious_distant_time;

    associations_dubious_distant_time_ = associations_dubious_distant_time;
}

float CreateARTASAssociationsTask::associationDubiousCloseTimePast() const
{
    return association_dubious_close_time_past_;
}

void CreateARTASAssociationsTask::associationDubiousCloseTimePast(
    float association_dubious_close_time_past)
{
    loginf << "CreateARTASAssociationsTask:: associationDubiousCloseTimePast: "
           << association_dubious_close_time_past;

    association_dubious_close_time_past_ = association_dubious_close_time_past;
}

float CreateARTASAssociationsTask::associationDubiousCloseTimeFuture() const
{
    return association_dubious_close_time_future_;
}

void CreateARTASAssociationsTask::associationDubiousCloseTimeFuture(
    float association_dubious_close_time_future)
{
    loginf << "CreateARTASAssociationsTask: associationDubiousCloseTimeFuture: "
           << association_dubious_close_time_future;

    association_dubious_close_time_future_ = association_dubious_close_time_future;
}

bool CreateARTASAssociationsTask::ignoreTrackEndAssociations() const
{
    return ignore_track_end_associations_;
}

void CreateARTASAssociationsTask::ignoreTrackEndAssociations(bool value)
{
    loginf << "CreateARTASAssociationsTask: ignoreTrackEndAssociations: value " << value;
    ignore_track_end_associations_ = value;
}

bool CreateARTASAssociationsTask::markTrackEndAssociationsDubious() const
{
    return mark_track_end_associations_dubious_;
}

void CreateARTASAssociationsTask::markTrackEndAssociationsDubious(bool value)
{
    loginf << "CreateARTASAssociationsTask: markTrackEndAssociationsDubious: value " << value;
    mark_track_end_associations_dubious_ = value;
}

bool CreateARTASAssociationsTask::ignoreTrackCoastingAssociations() const
{
    return ignore_track_coasting_associations_;
}

void CreateARTASAssociationsTask::ignoreTrackCoastingAssociations(bool value)
{
    loginf << "CreateARTASAssociationsTask: ignoreTrackCoastingAssociations: value " << value;
    ignore_track_coasting_associations_ = value;
}

bool CreateARTASAssociationsTask::markTrackCoastingAssociationsDubious() const
{
    return mark_track_coasting_associations_dubious_;
}

void CreateARTASAssociationsTask::markTrackCoastingAssociationsDubious(bool value)
{
    loginf << "CreateARTASAssociationsTask: markTrackCoastingAssociationsDubious: value " << value;
    mark_track_coasting_associations_dubious_ = value;
}

void CreateARTASAssociationsTask::checkAndSetVariable(std::string& name_str, DBOVariable** var)
{
    DBObjectManager& object_man = COMPASS::instance().objectManager();
    DBObject& object = object_man.object("Tracker");

    if (!object.hasVariable(name_str))
    {
        loginf << "CreateARTASAssociationsTask: checkAndSetVariable: var " << name_str
               << " does not exist";
        name_str = "";
        var = nullptr;
    }
    else
    {
        *var = &object.variable(name_str);
        loginf << "CreateARTASAssociationsTask: checkAndSetVariable: var " << name_str << " set";
        assert(var);
        assert((*var)->existsInDB());
    }
}

void CreateARTASAssociationsTask::checkAndSetMetaVariable(std::string& name_str,
                                                          MetaDBOVariable** var)
{
    DBObjectManager& object_man = COMPASS::instance().objectManager();

    if (!object_man.existsMetaVariable(name_str))
    {
        loginf << "CreateARTASAssociationsTask: checkAndSetMetaVariable: var " << name_str
               << " does not exist";
        name_str = "";
        var = nullptr;
    }
    else
    {
        *var = &object_man.metaVariable(name_str);
        loginf << "CreateARTASAssociationsTask: checkAndSetMetaVariable: var " << name_str
               << " set";
        assert(var);
    }
}

DBOVariableSet CreateARTASAssociationsTask::getReadSetFor(const std::string& dbo_name)
{
    DBOVariableSet read_set;

    assert(key_var_);
    assert(key_var_->existsIn(dbo_name));
    read_set.add(key_var_->getFor(dbo_name));

    assert(hash_var_);
    assert(hash_var_->existsIn(dbo_name));
    read_set.add(hash_var_->getFor(dbo_name));

    assert(tod_var_);
    assert(tod_var_->existsIn(dbo_name));
    read_set.add(tod_var_->getFor(dbo_name));

    if (dbo_name == "Tracker")
    {
        assert(tracker_track_num_var_);
        read_set.add(*tracker_track_num_var_);

        assert(tracker_track_begin_var_);
        read_set.add(*tracker_track_begin_var_);

        assert(tracker_track_end_var_);
        read_set.add(*tracker_track_end_var_);

        assert(tracker_track_coasting_var_);
        read_set.add(*tracker_track_coasting_var_);
    }

    return read_set;
}

void CreateARTASAssociationsTask::associationStatusSlot(QString status)
{
    assert(status_dialog_);
    status_dialog_->setAssociationStatus(status.toStdString());
}

void CreateARTASAssociationsTask::saveAssociationsQuestionSlot(QString question_str)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Malformed Associations", question_str,
                                  QMessageBox::Yes | QMessageBox::No);

    save_associations_ = reply == QMessageBox::Yes;

    assert(create_job_);
    create_job_->setSaveQuestionAnswer(save_associations_);
}

void CreateARTASAssociationsTask::closeStatusDialogSlot()
{
    assert(status_dialog_);
    status_dialog_->close();
    status_dialog_ = nullptr;
}
