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

#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/label/labelgenerator.h"
#include "compass.h"
#include "mainwindow.h"
#include "configurationmanager.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanagerwidget.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/target/target.h"
#include "dbcontent/target/targetlistwidget.h"
#include "logger.h"
#include "dbcontent/variable/metavariable.h"
#include "datasourcemanager.h"
#include "stringconv.h"
#include "number.h"
#include "viewmanager.h"
#include "jobmanager.h"
#include "evaluationmanager.h"
#include "filtermanager.h"
#include "util/number.h"
#include "util/system.h"
#include "util/timeconv.h"
#include "dbcontent/variable/metavariableconfigurationdialog.h"
#include "dbcontentdeletedbjob.h"
#include "taskmanager.h"
#include "asteriximporttask.h"
#include "dbcontent_commands.h"

#include "util/tbbhack.h"

#include <QApplication>
#include <QMessageBox>

#include <algorithm>
#include <string>

using namespace std;
using namespace Utils;
using namespace dbContent;



DBContentManager::DBContentManager(const std::string& class_id, const std::string& instance_id,
                                   COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "db_content.json"), compass_(*compass)
{
    logdbg << "DBContentManager: constructor: creating subconfigurables";

    registerParameter("max_live_data_age_cache", &max_live_data_age_cache_, 5);
    registerParameter("max_live_data_age_db", &max_live_data_age_db_, 60);

    createSubConfigurables();

    // check uniqueness of dbcontent ids
    set<unsigned int> dbcont_ids;

    for (auto& object_it : dbcontent_)
    {
        assert (object_it.second->id() < 256);
        assert (dbcont_ids.count(object_it.second->id()) == 0);
        dbcont_ids.insert(object_it.second->id());
    }

    assert (label_generator_);
    label_generator_->checkLabelConfig(); // here because references meta variables

    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>"); // for dbo read job
    // for signal about new data
    qRegisterMetaType<std::map<std::string, std::shared_ptr<Buffer>>>("std::map<std::string, std::shared_ptr<Buffer>>");

    dbContent::init_dbcontent_commands();
}

DBContentManager::~DBContentManager()
{
    loginf << "DBContentManager: dtor";

    data_.clear();

    for (auto it : dbcontent_)
        delete it.second;
    dbcontent_.clear();

    meta_variables_.clear();

    widget_ = nullptr;

    loginf << "DBContentManager: dtor: done";
}

dbContent::LabelGenerator& DBContentManager::labelGenerator()
{
    assert (label_generator_);
    return *label_generator_;
}

void DBContentManager::generateSubConfigurable(const std::string& class_id,
                                               const std::string& instance_id)
{
    logdbg << "DBContentManager: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;

    if (class_id == "DBContentLabelGenerator")
    {
        assert (!label_generator_);
        label_generator_.reset(new dbContent::LabelGenerator(class_id, instance_id, *this));
    }
    else if (class_id == "DBContentTargetModel")
    {
        assert (!target_model_);
        target_model_.reset(new dbContent::TargetModel(class_id, instance_id, *this));
    }
    else if (class_id == "DBContent")
    {
        DBContent* object = new DBContent(compass_, class_id, instance_id, this);
        loginf << "DBContentManager: generateSubConfigurable: adding content " << object->name()
               << " id " << object->id();
        assert(!dbcontent_.count(object->name()));
        assert(!dbcontent_ids_.count(object->id()));

        dbcontent_[object->name()] = object;
        dbcontent_ids_[object->id()] = object;
    }
    else if (class_id == "MetaVariable")
    {
        MetaVariable* meta_var = new MetaVariable(class_id, instance_id, this);
        logdbg << "DBContentManager: generateSubConfigurable: adding meta var type "
               << meta_var->name();

        assert(!existsMetaVariable(meta_var->name()));
        //meta_variables_.emplace(meta_var->name(), meta_var);
        //meta_variables_.emplace_back(meta_var);

        meta_variables_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(meta_var->name()),   // args for key
                    std::forward_as_tuple(meta_var));  // args for mapped value
    }
    else
        throw std::runtime_error("DBContentManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void DBContentManager::checkSubConfigurables()
{
    if (!label_generator_)
    {
        generateSubConfigurable("DBContentLabelGenerator", "DBContentLabelGenerator0");
        assert (label_generator_);
    }

    if (!target_model_)
    {
        generateSubConfigurable("DBContentTargetModel", "DBContentTargetModel0");
        assert (target_model_);
    }
}

bool DBContentManager::existsDBContent(const std::string& dbcontent_name)
{
    logdbg << "DBContentManager: existsDBContent: '" << dbcontent_name << "'";

    return (dbcontent_.find(dbcontent_name) != dbcontent_.end());
}

DBContent& DBContentManager::dbContent(const std::string& dbcontent_name)
{
    logdbg << "DBContentManager: dbContent: name " << dbcontent_name;

    assert(dbcontent_.find(dbcontent_name) != dbcontent_.end());

    return *dbcontent_.at(dbcontent_name);
}

void DBContentManager::deleteDBContent(const std::string& dbcontent_name)
{
    logdbg << "DBContentManager: deleteDBContent: name " << dbcontent_name;
    assert(existsDBContent(dbcontent_name));
    delete dbcontent_.at(dbcontent_name);
    dbcontent_.erase(dbcontent_name);

    emit dbObjectsChangedSignal();
}

void DBContentManager::deleteDBContentData(boost::posix_time::ptime before_timestamp)
{
    loginf << "DBContentManager: deleteDBContentData";

    assert (!delete_job_);

    delete_job_ = make_shared<DBContentDeleteDBJob>(COMPASS::instance().interface());
    delete_job_->setBeforeTimestamp(before_timestamp);

    connect(delete_job_.get(), &DBContentDeleteDBJob::doneSignal, this, &DBContentManager::deleteJobDoneSlot,
            Qt::QueuedConnection);

    JobManager::instance().addDBJob(delete_job_);
}

bool DBContentManager::hasData()
{
    for (auto& object_it : dbcontent_)
        if (object_it.second->hasData())
            return true;

    return false;
}

unsigned int DBContentManager::getMaxDBContentID()
{
    unsigned int ret = 0;

    for (auto& object_it : dbcontent_)
        ret = max(ret, object_it.second->id());

    return ret;
}

bool DBContentManager::existsDBContentWithId (unsigned int id)
{
    return dbcontent_ids_.count(id);
}

const std::string& DBContentManager::dbContentWithId (unsigned int id)
{
    assert (dbcontent_ids_.count(id));
    return dbcontent_ids_.at(id)->name();
}

bool DBContentManager::existsMetaVariable(const std::string& var_name)
{
//    return std::find_if(meta_variables_.begin(), meta_variables_.end(),
//                        [var_name](const std::unique_ptr<MetaVariable>& var) -> bool { return var->name() == var_name; })
//            != meta_variables_.end();

    return meta_variables_.count(var_name);
}

MetaVariable& DBContentManager::metaVariable(const std::string& var_name)
{
    logdbg << "DBContentManager: metaVariable: name " << var_name;

//    assert(existsMetaVariable(var_name));

//    auto it = std::find_if(meta_variables_.begin(), meta_variables_.end(),
//                           [var_name](const std::unique_ptr<MetaVariable>& var) -> bool { return var->name() == var_name; });

//    assert (it != meta_variables_.end());

//    return *it->get();

    assert (meta_variables_.count(var_name));
    return *(meta_variables_.at(var_name).get());
}

void DBContentManager::renameMetaVariable(const std::string& old_var_name, const std::string& new_var_name)
{
    assert(existsMetaVariable(old_var_name));

    std::unique_ptr<dbContent::MetaVariable> meta_var = std::move(meta_variables_.at(old_var_name));
    meta_variables_.erase(old_var_name);
    meta_var->name(new_var_name);
    meta_variables_.emplace(new_var_name, std::move(meta_var));


    if (meta_cfg_dialog_)
    {
        meta_cfg_dialog_->updateList();
        meta_cfg_dialog_->selectMetaVariable(new_var_name);
    }
}

void DBContentManager::deleteMetaVariable(const std::string& var_name)
{
    logdbg << "DBContentManager: deleteMetaVariable: name " << var_name;
    assert(existsMetaVariable(var_name));

//    auto it = std::find_if(meta_variables_.begin(), meta_variables_.end(),
//                           [var_name](const std::unique_ptr<MetaVariable>& var) -> bool { return var->name() == var_name; });

//    assert (it != meta_variables_.end());

    meta_variables_.erase(var_name);

    if (meta_cfg_dialog_)
    {
        meta_cfg_dialog_->updateList();
        meta_cfg_dialog_->clearDetails();
    }
}

bool DBContentManager::usedInMetaVariable(const Variable& variable)
{
    for (auto& meta_it : meta_variables_)
        if (meta_it.second->uses(variable))
            return true;

    return false;
}

DBContentManagerWidget* DBContentManager::widget()
{
    if (!widget_)
    {
        widget_.reset(new DBContentManagerWidget(*this));
    }

    assert(widget_);
    return widget_.get();
}


void DBContentManager::load(const std::string& custom_filter_clause)
{
    logdbg << "DBContentManager: loadSlot: custom_filter_clause '" << custom_filter_clause << "'";

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (load_in_progress_)
    {
        loginf << "DBContentManager: loadSlot: quitting previous load";

        for (auto& object : dbcontent_)
        {
            if (object.second->isLoading())
                object.second->quitLoading();
        }

        while (load_in_progress_) // JobManager::instance().hasDBJobs()
        {
            loginf << "DBContentManager: loadSlot: previous load to finish";

            QCoreApplication::processEvents();
            QThread::msleep(1);
        }
    }

    loginf << "DBContentManager: loadSlot: starting loading";

    //data_.clear();
    clearData();

    load_in_progress_ = true;

    bool load_job_created = false;

    DataSourceManager& ds_man =  COMPASS::instance().dataSourceManager();
    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();
    ViewManager& view_man = COMPASS::instance().viewManager();

    assert (label_generator_);

    for (auto& object : dbcontent_)
    {
        loginf << "DBContentManager: loadSlot: object " << object.first
               << " loadable " << object.second->loadable()
               << " loading wanted " << ds_man.loadingWanted(object.first);

        if (object.second->loadable() && ds_man.loadingWanted(object.first))
        {
            loginf << "DBContentManager: loadSlot: loading object " << object.first;
            VariableSet read_set = view_man.getReadSet(object.first);

            // add required vars for processing
            addStandardVariables(object.first, read_set);

            label_generator_->addVariables(object.first, read_set);

            if (eval_man.needsAdditionalVariables())
                eval_man.addVariables(object.first, read_set);

            if (read_set.getSize() == 0)
            {
                logwrn << "DBContentManager: loadSlot: skipping loading of object " << object.first
                       << " since an empty read list was detected";
                continue;
            }

            // load(dbContent::VariableSet& read_set, bool use_datasrc_filters, bool use_filters,
            // const std::string& custom_filter_clause="")
            object.second->load(read_set, true, COMPASS::instance().filterManager().useFilters(),
                                custom_filter_clause);

            load_job_created = true;
        }
    }
    emit loadingStartedSignal();

    if (!load_job_created)
        finishLoading();
}

void DBContentManager::addLoadedData(std::map<std::string, std::shared_ptr<Buffer>> data)
{
    logdbg << "DBContentManager: addLoadedData";

    // newest data batch has been finalized, ready to be added

    // add buffers to data

    bool something_changed = false;

    for (auto& buf_it : data)
    {
        if (!buf_it.second->size()) // empty buffer
            continue;

        if (data_.count(buf_it.first))
        {
            logdbg << "DBContentManager: addLoadedData: adding buffer dbo " << buf_it.first
                   << " adding size " << buf_it.second->size() << " current size " << data_.at(buf_it.first)->size();

            data_.at(buf_it.first)->seizeBuffer(*buf_it.second.get());

            logdbg << "DBContentManager: addLoadedData: new buffer dbo " << buf_it.first
                   << " size " << data_.at(buf_it.first)->size();
        }
        else
        {
            data_[buf_it.first] = move(buf_it.second);

            logdbg << "DBContentManager: addLoadedData: created buffer dbo " << buf_it.first
                   << " size " << data_.at(buf_it.first)->size();
        }

        something_changed = true;
    }

    if (something_changed)
    {
        updateNumLoadedCounts();

        logdbg << "DBContentManager: addLoadedData: emitting signal";

        emit loadedDataSignal(data_, false);
    }
}

std::map<std::string, std::shared_ptr<Buffer>> DBContentManager::loadedData()
{
    return data_;
}

void DBContentManager::quitLoading()
{
    loginf << "DBContentManager: quitLoading";

    assert (load_in_progress_);

    for (auto& object : dbcontent_)
    {
        if (object.second->isLoading())
            object.second->quitLoading();
    }

    //load_in_progress_ = true;  // TODO
}

void DBContentManager::databaseOpenedSlot()
{
    loginf << "DBContentManager: databaseOpenedSlot";

    loadMaxRecordNumber();
    loadMaxRefTrajTrackNum();

    DBInterface& db_interface = COMPASS::instance().interface();

    if (db_interface.hasProperty("associations_generated"))
    {
        assert(db_interface.hasProperty("associations_id"));

        has_associations_ =
                db_interface.getProperty("associations_generated") == "1";
        associations_id_ = db_interface.getProperty("associations_id");
    }
    else
    {
        has_associations_ = false;
        associations_id_ = "";
    }

    // load min max values
    if (db_interface.hasProperty(PROP_TIMESTAMP_MIN_NAME))
        timestamp_min_ = Time::fromLong(stol(db_interface.getProperty(PROP_TIMESTAMP_MIN_NAME)));
    if (db_interface.hasProperty(PROP_TIMESTAMP_MAX_NAME))
        timestamp_max_ = Time::fromLong(stol(db_interface.getProperty(PROP_TIMESTAMP_MAX_NAME)));

    if (db_interface.hasProperty(PROP_LATITUDE_MIN_NAME))
        latitude_min_ = stod(db_interface.getProperty(PROP_LATITUDE_MIN_NAME));
    if (db_interface.hasProperty(PROP_LATITUDE_MAX_NAME))
        latitude_max_ = stod(db_interface.getProperty(PROP_LATITUDE_MAX_NAME));

    if (db_interface.hasProperty(PROP_LONGITUDE_MIN_NAME))
        longitude_min_ = stod(db_interface.getProperty(PROP_LONGITUDE_MIN_NAME));
    if (db_interface.hasProperty(PROP_LONGITUDE_MAX_NAME))
        longitude_max_ = stod(db_interface.getProperty(PROP_LONGITUDE_MAX_NAME));

    for (auto& object : dbcontent_)
        object.second->databaseOpenedSlot();

    loadTargets();

    emit associationStatusChangedSignal();

    loginf << "DBContentManager: databaseOpenedSlot: done";
}

void DBContentManager::databaseClosedSlot()
{
    loginf << "DBContentManager: databaseClosedSlot";

    max_rec_num_ = 0;
    has_max_rec_num_ = false;

    max_reftraj_track_num_ = 0;
    has_max_reftraj_track_num_ = false;

    has_associations_ = false;
    associations_id_ = "";

    for (auto& object : dbcontent_)
        object.second->databaseClosedSlot();

    target_model_->clear();

    timestamp_min_.reset();
    timestamp_max_.reset();
    latitude_min_.reset();
    latitude_max_.reset();
    longitude_min_.reset();
    longitude_max_.reset();

    associationStatusChangedSignal();
}

void DBContentManager::loadingDone(DBContent& object)
{
    bool done = true;

    for (auto& object_it : dbcontent_)
    {
        if (object_it.second->isLoading())
        {
            logdbg << "DBContentManager: loadingDoneSlot: " << object_it.first << " still loading";
            done = false;
            break;
        }
    }

    if (done)
    {
        finishLoading();
    }
    else
        logdbg << "DBContentManager: loadingDoneSlot: not done";
}

void DBContentManager::deleteJobDoneSlot()
{
    loginf << "DBContentManager: deleteJobDoneSlot";

    assert (delete_job_);

    delete_job_ = nullptr;
}

void DBContentManager::metaDialogOKSlot()
{
    assert (meta_cfg_dialog_);
    meta_cfg_dialog_->hide();
}

void DBContentManager::finishLoading()
{
    loginf << "DBContentManager: finishLoading: all done";
    load_in_progress_ = false;

    COMPASS::instance().viewManager().doViewPointAfterLoad();

    emit loadingDoneSignal();

    COMPASS::instance().dbContentManager().labelGenerator().updateAvailableLabelLines(); // update available lines

    QApplication::restoreOverrideCursor();
}


bool DBContentManager::hasAssociations() const
{
    return has_associations_;
}

void DBContentManager::setAssociationsIdentifier(const std::string& assoc_id)
{
    COMPASS::instance().interface().setProperty("associations_generated", "1");
    COMPASS::instance().interface().setProperty("associations_id", assoc_id);

    has_associations_ = true;
    associations_id_ = assoc_id;

    COMPASS::instance().dataSourceManager().updateWidget();

    emit associationStatusChangedSignal();
}

std::string DBContentManager::associationsID() const { return associations_id_; }

bool DBContentManager::loadInProgress() const
{
    return load_in_progress_;
}

void DBContentManager::clearData()
{
    loginf << "DBContentManager: clearData";

    data_.clear();

    COMPASS::instance().viewManager().clearDataInViews();
}

void DBContentManager::insertData(std::map<std::string, std::shared_ptr<Buffer>> data)
{
    loginf << "DBContentManager: insertData";

    while (load_in_progress_) // pending insert
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }

    assert (!insert_in_progress_);
    assert (!insert_data_.size());

    insert_in_progress_ = true;
    logdbg << "DBContentManager: insertData: insert in progress " << insert_in_progress_;

    insert_data_ = data;

    for (auto& buf_it : insert_data_)
    {
        assert(existsDBContent(buf_it.first));
        dbContent(buf_it.first).insertData(buf_it.second);
    }
}

void DBContentManager::insertDone(DBContent& object)
{
    bool done = true;

    for (auto& object_it : dbcontent_)
    {
        if (object_it.second->isInserting())
        {
            logdbg << "DBContentManager: insertDone: " << object_it.first << " still inserting";
            done = false;
            break;
        }
    }

    if (done)
        finishInserting();
    else
        loginf << "DBContentManager: insertDone: not done";
}

void DBContentManager::finishInserting()
{
    loginf << "DBContentManager: finishInserting";

    using namespace boost::posix_time;

    ptime start_time = microsec_clock::local_time();
    ptime tmp_time = microsec_clock::local_time();

    unsigned int insert_cnt = 0;
    for (auto& buf_it : insert_data_)
        insert_cnt += buf_it.second->size();

    logdbg << "DBContentManager: finishInserting: size " << insert_cnt;

    assert (existsMetaVariable(DBContent::meta_var_timestamp_.name()));

    insert_in_progress_ = false;
    logdbg << "DBContentManager: finishInserting: insert in progress " << insert_in_progress_;
    emit insertDoneSignal();

    logdbg << "DBContentManager: finishInserting: done signal took "
           << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
           << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);

    // calculate min/max values

    tmp_time = microsec_clock::local_time();

    // ts calc for resume action

    boost::posix_time::ptime min_time_found; // for max latency calculation

    for (auto& buf_it : insert_data_)
    {
        string dbcont_name = buf_it.first;

        assert (metaCanGetVariable(dbcont_name, DBContent::meta_var_timestamp_));
        assert (metaCanGetVariable(dbcont_name, DBContent::meta_var_latitude_));
        assert (metaCanGetVariable(dbcont_name, DBContent::meta_var_longitude_));

        unsigned int buffer_size = buf_it.second->size();

        //auto updateMinMaxRange = [ & ] (Property prop, const std::string& dbcont_name, )
        // TODO template function this

        // use dbcolum name since buffer has been transformed during insert

        // timestamp
        {
            Variable& var = metaGetVariable(dbcont_name, DBContent::meta_var_timestamp_);
            if (buf_it.second->has<boost::posix_time::ptime>(var.dbColumnName()))
            {
                NullableVector<boost::posix_time::ptime>& data_vec = buf_it.second->get<boost::posix_time::ptime>(
                            var.dbColumnName());

                bool has_vec_min_max;
                ptime ts_vec_min, ts_vec_max;

                tie(has_vec_min_max, ts_vec_min, ts_vec_max) = data_vec.minMaxValues();

                if (has_vec_min_max)
                {
                    // set min time found
                    if (min_time_found.is_not_a_date_time())
                        min_time_found = ts_vec_min;
                    else
                        min_time_found = min(min_time_found, ts_vec_min);

                    if (hasMinMaxTimestamp())
                    {
                        timestamp_min_ = std::min(timestamp_min_.get(), ts_vec_min);
                        timestamp_max_ = std::max(timestamp_max_.get(), ts_vec_max);
                    }
                    else
                    {
                        timestamp_min_ = ts_vec_min;
                        timestamp_max_ = ts_vec_max;
                    }
                }
            }
        }

        if (hasMinMaxTimestamp())
        {
            COMPASS::instance().interface().setProperty(PROP_TIMESTAMP_MIN_NAME,
                                                        to_string(Time::toLong(timestamp_min_.get())));
            COMPASS::instance().interface().setProperty(PROP_TIMESTAMP_MAX_NAME,
                                                        to_string(Time::toLong(timestamp_max_.get())));

            logdbg << "DBContentManager: finishInserting: tod min " << timestamp_min_.get()
                   << " max " << timestamp_max_.get();
        }

        // lat & long
        {
            Variable& lat_var = metaGetVariable(dbcont_name, DBContent::meta_var_latitude_);
            Variable& lon_var = metaGetVariable(dbcont_name, DBContent::meta_var_longitude_);

            if (buf_it.second->has<double>(lat_var.dbColumnName())
                    && buf_it.second->has<double>(lon_var.dbColumnName()))
            {
                NullableVector<double>& lat_vec = buf_it.second->get<double>(lat_var.dbColumnName());
                NullableVector<double>& lon_vec = buf_it.second->get<double>(lon_var.dbColumnName());

                bool has_min_max = hasMinMaxPosition();

                for (unsigned int cnt=0; cnt < buffer_size; cnt++)
                {
                    if (!lat_vec.isNull(cnt) && !lon_vec.isNull(cnt))
                    {
                        if (has_min_max)
                        {
                            latitude_min_ = std::min(latitude_min_.get(), lat_vec.get(cnt));
                            latitude_max_ = std::max(latitude_max_.get(), lat_vec.get(cnt));

                            longitude_min_ = std::min(longitude_min_.get(), lon_vec.get(cnt));
                            longitude_max_ = std::max(longitude_max_.get(), lon_vec.get(cnt));
                        }
                        else
                        {
                            latitude_min_ = lat_vec.get(cnt);
                            latitude_max_ = lat_vec.get(cnt);

                            longitude_min_ = lon_vec.get(cnt);
                            longitude_max_ = lon_vec.get(cnt);

                            has_min_max = true;
                        }
                    }
                }

                if (has_min_max)
                {
                    COMPASS::instance().interface().setProperty(PROP_LATITUDE_MIN_NAME, to_string(latitude_min_.get()));
                    COMPASS::instance().interface().setProperty(PROP_LATITUDE_MAX_NAME, to_string(latitude_max_.get()));

                    COMPASS::instance().interface().setProperty(PROP_LONGITUDE_MIN_NAME, to_string(longitude_min_.get()));
                    COMPASS::instance().interface().setProperty(PROP_LONGITUDE_MAX_NAME, to_string(longitude_max_.get()));

                    logdbg << "DBContentManager: finishInserting: lat min " << latitude_min_.get()
                           << " max " << latitude_max_.get()
                           << " lon min " << longitude_min_.get()
                           << " max " << longitude_max_.get();
                }
            }
        }
    }

    logdbg << "DBContentManager: finishInserting: min/max took "
           << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
           << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);

    tmp_time = microsec_clock::local_time();

    if (COMPASS::instance().appMode() == AppMode::Offline || COMPASS::instance().appMode() == AppMode::LivePaused)
        insert_data_.clear();

    // start clearing old data

    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
    {
        using namespace boost::posix_time;

        ptime old_time = Time::currentUTCTime() - minutes(max_live_data_age_db_);

        logdbg << "DBContentManager: finishInserting: deleting data before " << Time::toString(old_time);

        deleteDBContentData(old_time);
    }

    logdbg << "DBContentManager: finishInserting: clear old took "
           << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
           << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);

    tmp_time = microsec_clock::local_time();

    // add inserted to loaded data

    bool had_data = data_.size();

    if (COMPASS::instance().appMode() == AppMode::LiveRunning) // do tod cleanup
    {
        addInsertedDataToChache();

        logdbg << "DBContentManager: finishInserting: insert cache took "
               << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
               << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);

        tmp_time = microsec_clock::local_time();

        insert_cnt = 0;
        for (auto& buf_it : data_)
            insert_cnt += buf_it.second->size();

        logdbg << "DBContentManager: finishInserting: before cut data size " << insert_cnt;

        cutCachedData();

        logdbg << "DBContentManager: finishInserting: cut cache took "
               << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
               << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);

        tmp_time = microsec_clock::local_time();

        insert_cnt = 0;
        for (auto& buf_it : data_)
            insert_cnt += buf_it.second->size();

        logdbg << "DBContentManager: finishInserting: after cut data size " << insert_cnt;

        // INFO] DBContentManager: finishInserting: size 220692
        // filter ds took 00:00:13.266 full 00:00:13.395
        filterDataSources();

        loginf << "DBContentManager: finishInserting: filterDataSources took "
               << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
               << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);

        tmp_time = microsec_clock::local_time();

        if (COMPASS::instance().filterManager().useFilters())
        {
            COMPASS::instance().filterManager().filterBuffers(data_);

            loginf << "DBContentManager: finishInserting: filter buffs took "
                   << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
                   << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);

            tmp_time = microsec_clock::local_time();
        }

        logdbg << "DBContentManager: finishInserting: distributing data";

        if (data_.size())
            emit loadedDataSignal(data_, true);
        else if (had_data)
            COMPASS::instance().viewManager().clearDataInViews();

        logdbg << "DBContentManager: finishInserting: distribute took "
               << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
               << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);

        tmp_time = microsec_clock::local_time();

        updateNumLoadedCounts();

        logdbg << "DBContentManager: finishInserting: update cnts took "
               << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
               << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);

        tmp_time = microsec_clock::local_time();

        if (!min_time_found.is_not_a_date_time())
            loginf << "DBContentManager: finishInserting: max latency "
                   << Time::toString(Time::currentUTCTime() - min_time_found);
    }

    COMPASS::instance().dataSourceManager().updateWidget();

    COMPASS::instance().dbContentManager().labelGenerator().updateAvailableLabelLines(); // update available lines

    logdbg << "DBContentManager: finishInserting: update lines took "
           << String::timeStringFromDouble((microsec_clock::local_time() - tmp_time).total_milliseconds() / 1000.0, true)
           << " full " << String::timeStringFromDouble((microsec_clock::local_time() - start_time).total_milliseconds() / 1000.0, true);
}

void DBContentManager::addInsertedDataToChache()
{
    loginf << "DBContentManager: addInsertedDataToChache";

    assert (label_generator_);

    //tbb::parallel_for(uint(0), num_targets, [&](unsigned int cnt)

    //for (auto& buf_it : insert_data_)
    unsigned int num_buffers = insert_data_.size();
    boost::mutex data_mutex;

    tbb::parallel_for(uint(0), num_buffers, [&](unsigned int buffer_cnt)
    {
        std::map<std::string, std::shared_ptr<Buffer>>::iterator buf_it = insert_data_.begin();
        std::advance(buf_it, buffer_cnt);

        VariableSet read_set = COMPASS::instance().viewManager().getReadSet(buf_it->first);
        addStandardVariables(buf_it->first, read_set);
        label_generator_->addVariables(buf_it->first, read_set);

        vector<Property> buffer_properties_to_be_removed;

        // remove all unused
        for (const auto& prop_it : buf_it->second->properties().properties())
        {
            if (!read_set.hasDBColumnName(prop_it.name()))
                buffer_properties_to_be_removed.push_back(prop_it); // remove it later
        }

        for (auto& prop_it : buffer_properties_to_be_removed)
        {
            logdbg << "DBContentManager: addInsertedDataToChache: deleting property " << prop_it.name();
            buf_it->second->deleteProperty(prop_it);
        }

        // add assoc property if required
        Variable& utn_var = metaGetVariable(buf_it->first, DBContent::meta_var_utn_);
        Property utn_prop (utn_var.dbColumnName(), utn_var.dataType());

        if (!buf_it->second->hasProperty(utn_prop))
            buf_it->second->addProperty(utn_prop);

        // change db column names to dbo var names
        buf_it->second->transformVariables(read_set, true);

        // add selection flags
        buf_it->second->addProperty(DBContent::selected_var);

        // add buffer to be able to distribute to views
        if (!data_.count(buf_it->first))
        {
            boost::mutex::scoped_lock locker(data_mutex);
            data_[buf_it->first] = buf_it->second;
        }
        else
        {
            data_.at(buf_it->first)->seizeBuffer(*buf_it->second.get());

            // sort by tod
            assert (metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(buf_it->first));

            Variable& ts_var = metaVariable(DBContent::meta_var_timestamp_.name()).getFor(buf_it->first);

            Property ts_prop {ts_var.name(), ts_var.dataType()};

            assert (data_.at(buf_it->first)->hasProperty(ts_prop));

            data_.at(buf_it->first)->sortByProperty(ts_prop);
        }
    });


    insert_data_.clear();
}

void DBContentManager::filterDataSources()
{
    logdbg << "DBContentManager: filterDataSources";

    std::map<unsigned int, std::set<unsigned int>> wanted_data_sources =
            COMPASS::instance().dataSourceManager().getLoadDataSources();

    unsigned int num_buffers = data_.size();

    tbb::parallel_for(uint(0), num_buffers, [&](unsigned int buffer_cnt)
    {
        std::map<std::string, std::shared_ptr<Buffer>>::iterator buf_it = data_.begin();
        std::advance(buf_it, buffer_cnt);

        // remove unwanted data sources
        assert (metaVariable(DBContent::meta_var_datasource_id_.name()).existsIn(buf_it->first));
        assert (metaVariable(DBContent::meta_var_line_id_.name()).existsIn(buf_it->first));

        Variable& ds_id_var = metaVariable(DBContent::meta_var_datasource_id_.name()).getFor(buf_it->first);
        Variable& line_id_var = metaVariable(DBContent::meta_var_line_id_.name()).getFor(buf_it->first);

        Property ds_id_prop {ds_id_var.name(), ds_id_var.dataType()};
        assert (buf_it->second->hasProperty(ds_id_prop));

        Property line_id_prop {line_id_var.name(), line_id_var.dataType()};
        assert (buf_it->second->hasProperty(ds_id_prop));

        NullableVector<unsigned int>& ds_id_vec = buf_it->second->get<unsigned int>(ds_id_var.name());
        NullableVector<unsigned int>& line_id_vec = buf_it->second->get<unsigned int>(line_id_var.name());

        unsigned int buffer_size = buf_it->second->size();

        vector<size_t> indexes_to_remove;
        //assert (ds_id_vec.isNeverNull()); TODO why asserts?

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            assert (!ds_id_vec.isNull(index));
            assert (!line_id_vec.isNull(index));

            if (!wanted_data_sources.count(ds_id_vec.get(index)) // unwanted ds
                    || !wanted_data_sources.at(ds_id_vec.get(index)).count(line_id_vec.get(index))) // unwanted line
                indexes_to_remove.push_back(index);
        }

        logdbg << "DBContentManager: filterDataSources: in " << buf_it->first << " remove "
               << indexes_to_remove.size() << " of " << buffer_size;

        // remove unwanted indexes
        if (indexes_to_remove.size())
        {
            buf_it->second->removeIndexes(indexes_to_remove); // huge cost here
        }
        //buffer_size = buf_it.second->size();
    });

    // remove empty buffers
    std::map<std::string, std::shared_ptr<Buffer>> tmp_data = data_;

    for (auto& buf_it : tmp_data)
        if (!buf_it.second->size())
            data_.erase(buf_it.first);
}

void DBContentManager::cutCachedData()
{
    unsigned int buffer_size;

    boost::posix_time::ptime min_ts = Time::currentUTCTime() - boost::posix_time::minutes(max_live_data_age_cache_);
    // max - x minutes

    loginf << "DBContentManager: cutCachedData: current ts " << Time::toString(Time::currentUTCTime())
           << " min_ts " << Time::toString(min_ts);

    for (auto& buf_it : data_)
    {
        buffer_size = buf_it.second->size();

        assert (metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(buf_it.first));

        Variable& ts_var = metaVariable(DBContent::meta_var_timestamp_.name()).getFor(buf_it.first);

        Property ts_prop {ts_var.name(), ts_var.dataType()};

        if (buf_it.second->hasProperty(ts_prop))
        {
            NullableVector<boost::posix_time::ptime>& ts_vec = buf_it.second->get<boost::posix_time::ptime>(
                        ts_var.name());

            unsigned int index=0;

            for (; index < buffer_size; ++index)
            {
                if (!ts_vec.isNull(index) && ts_vec.get(index) > min_ts)
                {
                    logdbg << "DBContentManager: cutCachedData: found " << buf_it.first
                           << " cutoff tod index " << index
                           << " ts " << Time::toString(ts_vec.get(index));
                    break;
                }
            }
            // index == buffer_size if none bigger than min_ts

            if (index) // index found
            {
                index--; // cut at previous

                logdbg << "DBContentManager: cutCachedData: cutting " << buf_it.first
                       << " up to index " << index
                       << " total size " << buffer_size
                       << " index time " << (ts_vec.isNull(index) ? "null" : Time::toString(ts_vec.get(index)));
                assert (index < buffer_size);
                buf_it.second->cutUpToIndex(index);
            }
        }
        else
            logwrn << "DBContentManager: cutCachedData: buffer " << buf_it.first << " has not tod for cutoff";
    }

    // remove empty buffers
    std::map<std::string, std::shared_ptr<Buffer>> tmp_data = data_;

    for (auto& buf_it : tmp_data)
        if (!buf_it.second->size())
            data_.erase(buf_it.first);
}

void DBContentManager::updateNumLoadedCounts()
{
    logdbg << "DBContentManager: updateNumLoadedCounts";

    // ds id->dbcont->line->cnt
    std::map<unsigned int, std::map<std::string,
            std::map<unsigned int, unsigned int>>> loaded_counts;

    for (auto& buf_it : data_)
    {
        assert (metaCanGetVariable(buf_it.first, DBContent::meta_var_datasource_id_));
        assert (metaCanGetVariable(buf_it.first, DBContent::meta_var_line_id_));

        Variable& ds_id_var = metaGetVariable(buf_it.first, DBContent::meta_var_datasource_id_);
        Variable& line_id_var = metaGetVariable(buf_it.first, DBContent::meta_var_line_id_);

        NullableVector<unsigned int>& ds_id_vec = buf_it.second->get<unsigned int>(ds_id_var.name());
        NullableVector<unsigned int>& line_id_vec = buf_it.second->get<unsigned int>(line_id_var.name());

        assert (ds_id_vec.isNeverNull());
        assert (line_id_vec.isNeverNull());

        unsigned int buffer_size = buf_it.second->size();

        for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
            loaded_counts[ds_id_vec.get(cnt)][buf_it.first][line_id_vec.get(cnt)] += 1;
    }

    COMPASS::instance().dataSourceManager().setLoadedCounts(loaded_counts);
}


unsigned long DBContentManager::maxRecordNumber() const
{
    assert (has_max_rec_num_);
    return max_rec_num_;
}

void DBContentManager::maxRecordNumber(unsigned long value)
{
    logdbg << "DBContentManager: maxRecordNumber: " << value;

    max_rec_num_ = value;
    has_max_rec_num_ = true;
}

unsigned int DBContentManager::maxRefTrajTrackNum() const
{
    assert (has_max_reftraj_track_num_);
    return max_reftraj_track_num_;
}

void DBContentManager::maxRefTrajTrackNum(unsigned int value)
{
    logdbg << "DBContentManager: maxRefTrajTrackNum: " << value;

    max_reftraj_track_num_ = value;
    has_max_reftraj_track_num_ = true;
}


bool DBContentManager::hasMinMaxInfo() const
{
    return timestamp_min_.has_value() || timestamp_max_.has_value()
            || latitude_min_.has_value() || latitude_max_.has_value()
            || longitude_min_.has_value() || longitude_max_.has_value();
}

bool DBContentManager::hasMinMaxTimestamp() const
{
    return timestamp_min_.has_value() && timestamp_max_.has_value();
}

void DBContentManager::setMinMaxTimestamp(boost::posix_time::ptime min, boost::posix_time::ptime max)
{
    timestamp_min_ = min;
    timestamp_max_ = max;

    COMPASS::instance().interface().setProperty("timestamp_min", to_string(Time::toLong(timestamp_min_.get())));
    COMPASS::instance().interface().setProperty("timestamp_max", to_string(Time::toLong(timestamp_max_.get())));
}

std::pair<boost::posix_time::ptime , boost::posix_time::ptime> DBContentManager::minMaxTimestamp() const
{
    assert (hasMinMaxTimestamp());
    return {timestamp_min_.get(), timestamp_max_.get()};
}

bool DBContentManager::hasMinMaxPosition() const
{
    return latitude_min_.has_value() || latitude_max_.has_value()
            || longitude_min_.has_value() || longitude_max_.has_value();
}

void DBContentManager::setMinMaxLatitude(double min, double max)
{
    latitude_min_ = min;
    latitude_max_ = max;

    COMPASS::instance().interface().setProperty("latitude_min", to_string(latitude_min_.get()));
    COMPASS::instance().interface().setProperty("latitude_max", to_string(latitude_max_.get()));
}

std::pair<double, double> DBContentManager::minMaxLatitude() const
{
    assert (hasMinMaxPosition());
    return {latitude_min_.get(), latitude_max_.get()};
}

void DBContentManager::setMinMaxLongitude(double min, double max)
{
    longitude_min_ = min;
    longitude_max_ = max;

    COMPASS::instance().interface().setProperty("longitude_min", to_string(longitude_min_.get()));
    COMPASS::instance().interface().setProperty("longitude_max", to_string(longitude_max_.get()));
}

std::pair<double, double> DBContentManager::minMaxLongitude() const
{
    assert (hasMinMaxPosition());
    return {longitude_min_.get(), longitude_max_.get()};
}

const std::map<std::string, std::shared_ptr<Buffer>>& DBContentManager::data() const
{
    return data_;
}

bool DBContentManager::canGetVariable (const std::string& dbcont_name, const Property& property)
{
    assert (dbcontent_.count(dbcont_name));

    return dbcontent_.at(dbcont_name)->hasVariable(property.name());
}

dbContent::Variable& DBContentManager::getVariable (const std::string& dbcont_name, const Property& property)
{
    assert (canGetVariable(dbcont_name, property));
    assert (dbcontent_.at(dbcont_name)->hasVariable(property.name()));

    Variable& variable = dbcontent_.at(dbcont_name)->variable(property.name());

    assert (variable.dataType() == property.dataType());

    return variable;
}

bool DBContentManager::metaCanGetVariable (const std::string& dbcont_name, const Property& meta_property)
{
    assert (dbcontent_.count(dbcont_name));

    if (!existsMetaVariable(meta_property.name()))
        return false;

    return metaVariable(meta_property.name()).existsIn(dbcont_name);
}

dbContent::Variable& DBContentManager::metaGetVariable (const std::string& dbcont_name, const Property& meta_property)
{
    assert (metaCanGetVariable(dbcont_name, meta_property));

    return metaVariable(meta_property.name()).getFor(dbcont_name);
}

bool DBContentManager::hasTargetsInfo() const
{
    return target_model_->hasTargetsInfo();
}

void DBContentManager::clearTargetsInfo()
{
    target_model_->clearTargetsInfo();
}

bool DBContentManager::existsTarget(unsigned int utn)
{
    return target_model_->existsTarget(utn);
}

void DBContentManager::createNewTarget(unsigned int utn)
{
    target_model_->createNewTarget(utn);
}

dbContent::Target& DBContentManager::target(unsigned int utn)
{
    assert (existsTarget(utn));
    return target_model_->target(utn);
}

void DBContentManager::removeDBContentFromTargets(const std::string& dbcont_name)
{
    target_model_->removeDBContentFromTargets(dbcont_name);
    saveTargets();
}

void DBContentManager::loadTargets()
{
    loginf << "DBContentManager: loadTargets";

    target_model_->loadFromDB();

    if (target_list_widget_)
        target_list_widget_->resizeColumnsToContents();
}

void DBContentManager::saveTargets()
{
    loginf << "DBContentManager: saveTargets";

    target_model_->saveToDB();
}

nlohmann::json DBContentManager::targetsInfoAsJSON() const
{
    assert (hasAssociations());
    assert (hasTargetsInfo());

    return target_model_->asJSON();
}

nlohmann::json DBContentManager::targetInfoAsJSON(unsigned int utn) const
{
    assert (hasAssociations());
    assert (hasTargetsInfo());

    return target_model_->targetAsJSON(utn);
}

nlohmann::json DBContentManager::utnsAsJSON() const
{
    assert (hasAssociations());
    assert (hasTargetsInfo());

    return target_model_->utnsAsJSON();
}

unsigned int DBContentManager::maxLiveDataAgeCache() const
{
    return max_live_data_age_cache_;
}

void DBContentManager::resetToStartupConfiguration()
{
    if (label_generator_)
    {
        label_generator_->setTmpDisableRemoveConfigOnDelete(true);

        label_generator_ = nullptr;

        generateSubConfigurable("DBContentLabelGenerator", "DBContentLabelGenerator0");
        assert (label_generator_);
    }
}

dbContent::TargetListWidget* DBContentManager::targetListWidget()
{
    if (!target_list_widget_)
    {
        assert (target_model_);
        target_list_widget_.reset (new dbContent::TargetListWidget(*target_model_, *this));
    }

    return target_list_widget_.get();
}

void DBContentManager::resizeTargetListWidget()
{
    if (target_list_widget_)
        target_list_widget_->resizeColumnsToContents();
}

//void DBContentManager::updateMetaVarNames()
//{
//    loginf << "DBContentManager::updateMetaVarNames";

//    //std::map<std::string, std::unique_ptr<dbContent::MetaVariable>> tmp_meta_variables = std::move(meta_variables_);
//    std::map<std::string, std::unique_ptr<dbContent::MetaVariable>> tmp_meta_variables;

//    tmp_meta_variables.insert(make_move_iterator(std::begin(meta_variables_)),
//                              make_move_iterator(std::end(meta_variables_)));
//    assert (!meta_variables_.size());

//    for (auto it = tmp_meta_variables.begin(); it != tmp_meta_variables.end() /* not hoisted */; /* no increment */)
//    {
//        meta_variables_.emplace(it->second->name(), std::move(it->second));
//        it = tmp_meta_variables.erase(it);
//    }

//    meta_variables_.insert(make_move_iterator(std::begin(tmp_meta_variables)),
//                              make_move_iterator(std::end(tmp_meta_variables)));
//}

bool DBContentManager::insertInProgress() const
{
    return insert_in_progress_;
}

void DBContentManager::loadMaxRecordNumber()
{
    assert (COMPASS::instance().interface().dbOpen());

    max_rec_num_ = 0;

    for (auto& obj_it : dbcontent_)
    {
        if (obj_it.second->existsInDB())
            max_rec_num_ = max(COMPASS::instance().interface().getMaxRecordNumber(*obj_it.second), max_rec_num_);
    }

    has_max_rec_num_ = true;

    loginf << "DBContentManager: loadMaxRecordNumber: " << max_rec_num_;
}

void DBContentManager::loadMaxRefTrajTrackNum()
{
    assert (COMPASS::instance().interface().dbOpen());

    max_reftraj_track_num_ = COMPASS::instance().interface().getMaxRefTrackTrackNum();
    has_max_reftraj_track_num_ = true;

    loginf << "DBContentManager: loadMaxRefTrajTrackNum: " << max_reftraj_track_num_;
}

void DBContentManager::addStandardVariables(std::string dbcont_name, dbContent::VariableSet& read_set)
{
    assert (metaCanGetVariable(dbcont_name, DBContent::meta_var_rec_num_));
    read_set.add(metaGetVariable(dbcont_name, DBContent::meta_var_rec_num_));

    assert (metaCanGetVariable(dbcont_name, DBContent::meta_var_datasource_id_));
    read_set.add(metaGetVariable(dbcont_name, DBContent::meta_var_datasource_id_));

    assert (metaCanGetVariable(dbcont_name, DBContent::meta_var_line_id_));
    read_set.add(metaGetVariable(dbcont_name, DBContent::meta_var_line_id_));

    assert (metaCanGetVariable(dbcont_name, DBContent::meta_var_timestamp_));
    read_set.add(metaGetVariable(dbcont_name, DBContent::meta_var_timestamp_));

    assert (metaCanGetVariable(dbcont_name, DBContent::meta_var_utn_));
    read_set.add(metaGetVariable(dbcont_name, DBContent::meta_var_utn_));
}

MetaVariableConfigurationDialog* DBContentManager::metaVariableConfigdialog()
{
    if (!meta_cfg_dialog_)
    {
        meta_cfg_dialog_.reset(new MetaVariableConfigurationDialog(*this));

        connect(meta_cfg_dialog_.get(), &MetaVariableConfigurationDialog::okSignal,
                this, &DBContentManager::metaDialogOKSlot);
    }

    assert(meta_cfg_dialog_);
    return meta_cfg_dialog_.get();
}

void DBContentManager::setViewableDataConfig (const nlohmann::json::object_t& data)
{
    viewable_data_cfg_.reset(new ViewableDataConfig(data));

    COMPASS::instance().viewManager().setCurrentViewPoint(viewable_data_cfg_.get());
}

bool DBContentManager::utnUseEval (unsigned int utn)
{
    assert (target_model_);
    assert (target_model_->existsTarget(utn));
    return target_model_->target(utn).useInEval();
}

void DBContentManager::utnUseEval (unsigned int utn, bool value)
{
    assert (target_model_);
    assert (target_model_->existsTarget(utn));
    target_model_->setUseTargetData(utn, value);

    // eval man
    //    data_.setUseTargetData(utn, value);
    //    updateResultsToUseChangeOf(utn);
}

std::string DBContentManager::utnComment (unsigned int utn)
{
    assert (target_model_);
    assert (target_model_->existsTarget(utn));
    return target_model_->target(utn).comment();
}

void DBContentManager::utnComment (unsigned int utn, std::string value)
{
    loginf << "DBContentManager: utnComment: utn " << utn << " comment '" << value << "'";

    assert (target_model_);
    assert (target_model_->existsTarget(utn));
    target_model_->setTargetDataComment(utn, value);
}

void DBContentManager::autoFilterUTNS()
{
    assert (target_model_);
    target_model_->setUseByFilter();

    //    data_.setUseAllTargetData(true);
    //    data_.clearComments();
    //    data_.setUseByFilter();
}

void DBContentManager::showUTN (unsigned int utn)
{
    loginf << "DBContentManager: showUTN: utn " << utn;

    nlohmann::json data;
    data[VP_FILTERS_KEY]["UTNs"]["utns"] = to_string(utn);

    loginf << "DBContentManager: showUTN: showing";
    setViewableDataConfig(data);
}

