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
#include "createartasassociationstaskdialog.h"
#include "createartasassociationsstatusdialog.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "datasourcemanager.h"
#include "jobmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "sqliteconnection.h"
#include "viewmanager.h"

#include <QApplication>
#include <QMessageBox>
#include <sstream>

using namespace std;
using namespace Utils;
using namespace dbContent;

const std::string CreateARTASAssociationsTask::DONE_PROPERTY_NAME = "artas_associations_created";

CreateARTASAssociationsTask::CreateARTASAssociationsTask(const std::string& class_id,
                                                         const std::string& instance_id,
                                                         TaskManager& task_manager)
    : Task("CreateARTASAssociationsTask", "Associate ARTAS TRIs", task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_calc_artas_assoc.json")
{
    tooltip_ =
            "Allows creation of UTNs and target report association based on ARTAS tracks and the TRI "
        "information.";

    registerParameter("current_data_source_name", &current_data_source_name_, "");

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

CreateARTASAssociationsTaskDialog* CreateARTASAssociationsTask::dialog()
{
    if (!dialog_)
    {
        dialog_.reset(new CreateARTASAssociationsTaskDialog(*this));

        connect(dialog_.get(), &CreateARTASAssociationsTaskDialog::runSignal,
                this, &CreateARTASAssociationsTask::dialogRunSlot);

        connect(dialog_.get(), &CreateARTASAssociationsTaskDialog::cancelSignal,
                this, &CreateARTASAssociationsTask::dialogCancelSlot);
    }

    assert(dialog_);
    return dialog_.get();

}

bool CreateARTASAssociationsTask::canRun()
{
    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    logdbg << "CreateARTASAssociationsTask: canRun: tracker " << dbcontent_man.existsDBContent("CAT062");

    if (!dbcontent_man.existsDBContent("CAT062"))
        return false;

    DBContent& tracker_object = dbcontent_man.dbContent("CAT062");

    // tracker stuff
    logdbg << "CreateARTASAssociationsTask: canRun: tracker loadable " << tracker_object.loadable();

    if (!tracker_object.loadable())
        return false;

    logdbg << "CreateARTASAssociationsTask: canRun: tracker count " << tracker_object.count();
    if (!tracker_object.count())
        return false;

    // no data sources


    logdbg << "CreateARTASAssociationsTask: canRun: num tracker data sources "
               << ds_man.hasDataSourcesOfDBContent("CAT062");

    if (!ds_man.hasDataSourcesOfDBContent("CAT062"))
        return false;

    bool ds_found{false};
    unsigned int current_ds_id {0};

    for (auto& ds_it : ds_man.dbDataSources())
    {
        if (!ds_it->numInsertedMap().count("CAT062")) // check if track data exists
            continue;

        if ((ds_it->hasShortName() &&
             ds_it->shortName() == current_data_source_name_) ||
                (ds_it->name() == current_data_source_name_))
        {
            ds_found = true;
            current_ds_id = ds_it->id();
            break;
        }
    }

    logdbg << "CreateARTASAssociationsTask: canRun: tracker ds_found " << ds_found << " id " << current_ds_id;

    if (!ds_found)
        return false;

    loginf << "CreateARTASAssociationsTask: canRun: tracker vars";

    if (!tracker_object.hasVariable(DBContent::var_cat062_tris_.name()) ||
            !tracker_object.hasVariable(DBContent::var_cat062_track_begin_.name()) ||
            !tracker_object.hasVariable(DBContent::var_cat062_coasting_.name()) ||
            !tracker_object.hasVariable(DBContent::var_cat062_track_end_.name()))
        return false;

    // meta var stuff
    loginf << "CreateARTASAssociationsTask: canRun: meta vars "
        << !dbcontent_man.existsMetaVariable(DBContent::meta_var_rec_num_.name()) << " "
        << !dbcontent_man.existsMetaVariable(DBContent::meta_var_datasource_id_.name()) << " "
        << !dbcontent_man.existsMetaVariable(DBContent::meta_var_timestamp_.name()) << " "
        << !dbcontent_man.existsMetaVariable(DBContent::meta_var_track_num_.name()) << " "
        << !dbcontent_man.existsMetaVariable(DBContent::meta_var_artas_hash_.name()) << " "
        << !dbcontent_man.existsMetaVariable(DBContent::meta_var_utn_.name());

    if (!dbcontent_man.existsMetaVariable(DBContent::meta_var_rec_num_.name()) ||
            !dbcontent_man.existsMetaVariable(DBContent::meta_var_datasource_id_.name()) ||
            !dbcontent_man.existsMetaVariable(DBContent::meta_var_timestamp_.name()) ||
            !dbcontent_man.existsMetaVariable(DBContent::meta_var_track_num_.name()) ||
            !dbcontent_man.existsMetaVariable(DBContent::meta_var_artas_hash_.name()) ||
            !dbcontent_man.existsMetaVariable(DBContent::meta_var_utn_.name()))
        return false;

    loginf << "CreateARTASAssociationsTask: canRun: metas in objects";
    for (auto& dbo_it : dbcontent_man)
    {
        if (dbo_it.first == "RefTraj") // not covered by ARTAS
            continue;

        if (dbo_it.first == "CAT062")  // check metas specific to tracker
        {
            if (!dbcontent_man.metaVariable(DBContent::meta_var_track_num_.name()).existsIn(dbo_it.first))
            {
                logwrn << "CreateARTASAssociationsTask: canRun: no track number in " << dbo_it.first;
                return false;
            }
        }
        else // check metas specific not to tracker
        {
            if (!dbcontent_man.metaVariable(DBContent::meta_var_artas_hash_.name()).existsIn(dbo_it.first))
            {
                logwrn << "CreateARTASAssociationsTask: canRun: no ARTAS hash in " << dbo_it.first;
                return false;
            }
        }

        loginf << "CreateARTASAssociationsTask: canRun: metas in object " << dbo_it.first << " "
            << !dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).existsIn(dbo_it.first) << " "
            << !dbcontent_man.metaVariable(DBContent::meta_var_datasource_id_.name()).existsIn(dbo_it.first) << " "
            << !dbcontent_man.metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(dbo_it.first) << " "
            << !dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).existsIn(dbo_it.first);

        if (!dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_datasource_id_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(dbo_it.first)
                || !dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).existsIn(dbo_it.first))
            return false;
    }

    loginf << "CreateARTASAssociationsTask: canRun: ok";
    return true;
}

void CreateARTASAssociationsTask::run()
{
    assert(canRun());

    loginf << "CreateARTASAssociationsTask: run: started";

    save_associations_ = true;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    assert(!status_dialog_);
    status_dialog_.reset(new CreateARTASAssociationsStatusDialog(*this));
    connect(status_dialog_.get(), &CreateARTASAssociationsStatusDialog::closeSignal, this,
            &CreateARTASAssociationsTask::closeStatusDialogSlot);
    status_dialog_->markStartTime();
    status_dialog_->setAssociationStatus("Loading Data");
    status_dialog_->show();

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    dbcontent_man.clearData();

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    COMPASS::instance().viewManager().disableDataDistribution(true);

    connect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &CreateARTASAssociationsTask::loadedDataDataSlot);
    connect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &CreateARTASAssociationsTask::loadingDoneSlot);

    for (auto& dbo_it : dbcontent_man)
    {
        if (!dbo_it.second->hasData())
            continue;

        if (dbo_it.first == "RefTraj") // not set in references
            continue;

        VariableSet read_set = getReadSetFor(dbo_it.first);

        if (dbo_it.first == "CAT062")
        {
            bool ds_found{false};
            unsigned int current_ds_id;

            for (auto& ds_it : ds_man.dbDataSources())
            {
                if (!ds_it->numInsertedMap().count("CAT062")) // check if track data exists
                    continue;

                if ((ds_it->hasShortName() &&
                     ds_it->shortName() == current_data_source_name_) ||
                        (ds_it->name() == current_data_source_name_))
                {
                    ds_found = true;
                    current_ds_id = ds_it->id();
                    break;
                }
            }

            assert(ds_found);
            std::string custom_filter_clause {
                dbcontent_man.metaGetVariable(dbo_it.first, DBContent::meta_var_datasource_id_).dbColumnName()
                        + " in (" + std::to_string(current_ds_id) + ")"};

            //        void DBContent::load (DBOVariableSet& read_set,  std::string
            //        custom_filter_clause,
            //                             std::vector <DBOVariable*> filtered_variables, bool
            //                             use_order, DBOVariable* order_variable, bool
            //                             use_order_ascending, const std::string &limit_str)

            dbo_it.second->loadFiltered(read_set, custom_filter_clause);
        }
        else
            dbo_it.second->load(read_set, false, false);

    }

    //status_dialog_->setDBODoneFlags(dbo_loading_done_flags_);
}

void CreateARTASAssociationsTask::loadedDataDataSlot(
        const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset)
{
    data_ = data;
}

void CreateARTASAssociationsTask::loadingDoneSlot()
{
    loginf << "CreateARTASAssociationsTask: loadingDoneSlot";

    assert(status_dialog_);
    //status_dialog_->setDBODoneFlags(dbo_loading_done_flags_);

    dbo_loading_done_ = true;

    assert(!create_job_);

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    disconnect(&dbcontent_man, &DBContentManager::loadedDataSignal,
            this, &CreateARTASAssociationsTask::loadedDataDataSlot);
    disconnect(&dbcontent_man, &DBContentManager::loadingDoneSignal,
            this, &CreateARTASAssociationsTask::loadingDoneSlot);

    dbcontent_man.clearData();

    COMPASS::instance().viewManager().disableDataDistribution(false);

    create_job_ = std::make_shared<CreateARTASAssociationsJob>(
                *this, COMPASS::instance().interface(), data_);

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

void CreateARTASAssociationsTask::dialogRunSlot()
{
    loginf << "CreateARTASAssociationsTask: dialogRunSlot";

    assert (dialog_);
    dialog_->hide();

    assert (canRun());
    run ();
}

void CreateARTASAssociationsTask::dialogCancelSlot()
{
    loginf << "CreateARTASAssociationsTask: dialogCancelSlot";

    assert (dialog_);
    dialog_->hide();
}


void CreateARTASAssociationsTask::createDoneSlot()
{
    loginf << "CreateARTASAssociationsTask: createDoneSlot";

    assert (create_job_);

    create_job_done_ = true;

    status_dialog_->setAssociationCounts(create_job_->associationCounts());
    status_dialog_->setFoundHashes(create_job_->foundHashes());
    status_dialog_->setMissingHashesAtBeginning(create_job_->missingHashesAtBeginning());
    status_dialog_->setMissingHashes(create_job_->missingHashes());
    status_dialog_->setDubiousAssociations(create_job_->dubiousAssociations());
    status_dialog_->setFoundDuplicates(create_job_->foundHashDuplicates());
    status_dialog_->setAssociationStatus("Done");

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
        //COMPASS::instance().dbContentManager().setAssociationsIdentifier("ARTAS");

        COMPASS::instance().interface().saveProperties();

        done_ = true;
    }
    else
        logwrn << "CreateARTASAssociationsTask: done after " << time_str << " without saving";

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

float CreateARTASAssociationsTask::endTrackTime() const
{
    assert (end_track_time_);
    return end_track_time_;
}

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

VariableSet CreateARTASAssociationsTask::getReadSetFor(const std::string& dbcontent_name)
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    VariableSet read_set;

    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_));

    if (dbcontent_name == "CAT062")
    {
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_num_));
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_begin_));
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_end_));
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_coasting_));

        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat062_tris_));
    }
    else
    {
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_artas_hash_));
    }

    // must be last for update process
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));

    return read_set;
}

void CreateARTASAssociationsTask::associationStatusSlot(QString status)
{
    assert(status_dialog_);
    status_dialog_->setAssociationStatus(status.toStdString());
}

void CreateARTASAssociationsTask::saveAssociationsQuestionSlot(QString question_str)
{
    assert (status_dialog_);
    assert(create_job_);

    status_dialog_->setAssociationCounts(create_job_->associationCounts());
    status_dialog_->setFoundHashes(create_job_->foundHashes());
    status_dialog_->setMissingHashesAtBeginning(create_job_->missingHashesAtBeginning());
    status_dialog_->setMissingHashes(create_job_->missingHashes());
    status_dialog_->setDubiousAssociations(create_job_->dubiousAssociations());
    status_dialog_->setFoundDuplicates(create_job_->foundHashDuplicates());

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Malformed Associations", question_str,
                                  QMessageBox::Yes | QMessageBox::No);

    save_associations_ = reply == QMessageBox::Yes;

    create_job_->setSaveQuestionAnswer(save_associations_);
}


void CreateARTASAssociationsTask::closeStatusDialogSlot()
{
    assert(status_dialog_);
    status_dialog_->close();
    status_dialog_ = nullptr;
}
