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
#include "dbcontent/target.h"
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

#include <QApplication>
#include <QMessageBox>

#include <algorithm>
#include <string>

using namespace std;
using namespace Utils;
using namespace dbContent;

const string timestamp_min_name {"timestamp_min"};
const string timestamp_max_name {"timestamp_max"};
const string latitude_min_name {"latitude_min"};
const string latitude_max_name {"latitude_max"};
const string longitude_min_name {"longitude_min"};
const string longitude_max_name {"longitude_max"};

DBContentManager::DBContentManager(const std::string& class_id, const std::string& instance_id,
                                   COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "db_content.json"), compass_(*compass)
{
    logdbg << "DBContentManager: constructor: creating subconfigurables";

    registerParameter("use_order", &use_order_, false);
    registerParameter("use_order_ascending", &use_order_ascending_, false);
    registerParameter("order_variable_dbcontent_name", &order_variable_dbcontent_name_, "");
    registerParameter("order_variable_name", &order_variable_name_, "");

    registerParameter("use_limit", &use_limit_, false);
    registerParameter("limit_min", &limit_min_, 0);
    registerParameter("limit_max", &limit_max_, 100000);

    registerParameter("max_live_data_age", &max_live_data_age_, 10);

    createSubConfigurables();

    assert (label_generator_);
    label_generator_->checkLabelConfig(); // here because references meta variables

    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>"); // for dbo read job
    // for signal about new data
    qRegisterMetaType<std::map<std::string, std::shared_ptr<Buffer>>>("std::map<std::string, std::shared_ptr<Buffer>>");
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

    if (class_id.compare("DBContentLabelGenerator") == 0)
    {
        assert (!label_generator_);
        label_generator_.reset(new dbContent::LabelGenerator(class_id, instance_id, *this));
    }
    else if (class_id.compare("DBContent") == 0)
    {
        DBContent* object = new DBContent(compass_, class_id, instance_id, this);
        loginf << "DBContentManager: generateSubConfigurable: adding content " << object->name();
        assert(!dbcontent_.count(object->name()));
        dbcontent_[object->name()] = object;
    }
    else if (class_id.compare("MetaVariable") == 0)
    {
        MetaVariable* meta_var = new MetaVariable(class_id, instance_id, this);
        logdbg << "DBContentManager: generateSubConfigurable: adding meta var type "
               << meta_var->name();

        assert(!existsMetaVariable(meta_var->name()));
        meta_variables_.emplace_back(meta_var);
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

void DBContentManager::deleteDBContent(boost::posix_time::ptime before_timestamp)
{
    loginf << "DBContentManager: deleteDBContent";

    assert (!delete_job_);

    delete_job_ = make_shared<DBContentDeleteDBJob>(COMPASS::instance().interface(), before_timestamp);

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

bool DBContentManager::existsMetaVariable(const std::string& var_name)
{
    return std::find_if(meta_variables_.begin(), meta_variables_.end(),
                        [var_name](const std::unique_ptr<MetaVariable>& var) -> bool { return var->name() == var_name; })
            != meta_variables_.end();
}

MetaVariable& DBContentManager::metaVariable(const std::string& var_name)
{
    logdbg << "DBContentManager: metaVariable: name " << var_name;

    assert(existsMetaVariable(var_name));

    auto it = std::find_if(meta_variables_.begin(), meta_variables_.end(),
                           [var_name](const std::unique_ptr<MetaVariable>& var) -> bool { return var->name() == var_name; });

    assert (it != meta_variables_.end());

    return *it->get();
}

void DBContentManager::renameMetaVariable(const std::string& old_var_name, const std::string& new_var_name)
{
    assert(existsMetaVariable(old_var_name));
    metaVariable(old_var_name).name(new_var_name);

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

    auto it = std::find_if(meta_variables_.begin(), meta_variables_.end(),
                           [var_name](const std::unique_ptr<MetaVariable>& var) -> bool { return var->name() == var_name; });

    assert (it != meta_variables_.end());

    meta_variables_.erase(it);

    if (meta_cfg_dialog_)
    {
        meta_cfg_dialog_->updateList();
        meta_cfg_dialog_->clearDetails();
    }
}

bool DBContentManager::usedInMetaVariable(const Variable& variable)
{
    for (auto& meta_it : meta_variables_)
        if (meta_it->uses(variable))
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

bool DBContentManager::useLimit() const { return use_limit_; }

void DBContentManager::useLimit(bool use_limit) { use_limit_ = use_limit; }

unsigned int DBContentManager::limitMin() const { return limit_min_; }

void DBContentManager::limitMin(unsigned int limit_min)
{
    limit_min_ = limit_min;
    loginf << "DBContentManager: limitMin: " << limit_min_;
}

unsigned int DBContentManager::limitMax() const { return limit_max_; }

void DBContentManager::limitMax(unsigned int limit_max)
{
    limit_max_ = limit_max;
    loginf << "DBContentManager: limitMax: " << limit_max_;
}

bool DBContentManager::useOrder() const { return use_order_; }

void DBContentManager::useOrder(bool use_order) { use_order_ = use_order; }

bool DBContentManager::useOrderAscending() const { return use_order_ascending_; }

void DBContentManager::useOrderAscending(bool use_order_ascending)
{
    use_order_ascending_ = use_order_ascending;
}

bool DBContentManager::hasOrderVariable()
{
    if (existsDBContent(order_variable_dbcontent_name_))
        if (dbContent(order_variable_dbcontent_name_).hasVariable(order_variable_name_))
            return true;
    return false;
}

Variable& DBContentManager::orderVariable()
{
    assert(hasOrderVariable());
    return dbContent(order_variable_dbcontent_name_).variable(order_variable_name_);
}

void DBContentManager::orderVariable(Variable& variable)
{
    order_variable_dbcontent_name_ = variable.dbContentName();
    order_variable_name_ = variable.name();
}

bool DBContentManager::hasOrderMetaVariable()
{
    if (order_variable_dbcontent_name_ == META_OBJECT_NAME)
        return existsMetaVariable(order_variable_name_);

    return false;
}

MetaVariable& DBContentManager::orderMetaVariable()
{
    assert(hasOrderMetaVariable());
    return metaVariable(order_variable_name_);
}

void DBContentManager::orderMetaVariable(MetaVariable& variable)
{
    order_variable_dbcontent_name_ = META_OBJECT_NAME;
    order_variable_name_ = variable.name();
}

void DBContentManager::clearOrderVariable()
{
    order_variable_dbcontent_name_ = "";
    order_variable_name_ = "";
}


void DBContentManager::load()
{
    logdbg << "DBContentManager: loadSlot";

    data_.clear();

    load_in_progress_ = true;

    bool load_job_created = false;

    loginf << "DBContentManager: loadSlot: loading associations";

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    while (JobManager::instance().hasDBJobs())
    {
        logdbg << "DBContentManager: loadSlot: waiting on association loading";

        QCoreApplication::processEvents();
        QThread::msleep(5);
    }

    loginf << "DBContentManager: loadSlot: starting loading";

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
            assert (metaCanGetVariable(object.first, DBContent::meta_var_rec_num_));
            read_set.add(metaGetVariable(object.first, DBContent::meta_var_rec_num_));

            assert (metaCanGetVariable(object.first, DBContent::meta_var_datasource_id_));
            read_set.add(metaGetVariable(object.first, DBContent::meta_var_datasource_id_));

            assert (metaCanGetVariable(object.first, DBContent::meta_var_line_id_));
            read_set.add(metaGetVariable(object.first, DBContent::meta_var_line_id_));

            assert (metaCanGetVariable(object.first, DBContent::meta_var_associations_));
            read_set.add(metaGetVariable(object.first, DBContent::meta_var_associations_));

            label_generator_->addVariables(object.first, read_set);

            if (eval_man.needsAdditionalVariables())
                eval_man.addVariables(object.first, read_set);

            if (read_set.getSize() == 0)
            {
                logwrn << "DBContentManager: loadSlot: skipping loading of object " << object.first
                       << " since an empty read list was detected";
                continue;
            }

            std::string limit_str = "";
            if (use_limit_)
            {
                limit_str = std::to_string(limit_min_) + "," + std::to_string(limit_max_);
                logdbg << "DBContentManager: loadSlot: use limit str " << limit_str;
            }

            Variable* variable = nullptr;

            assert (hasOrderVariable() || hasOrderMetaVariable());

            if (hasOrderVariable())
                variable = &orderVariable();
            if (hasOrderMetaVariable())
                variable = &orderMetaVariable().getFor(object.first);

            // load (DBOVariableSet &read_set, bool use_filters, bool use_order, DBOVariable
            // *order_variable, bool use_order_ascending, const std::string &limit_str="")
            object.second->load(read_set, true, COMPASS::instance().filterManager().useFilters(),
                                use_order_, variable, use_order_ascending_,
                                limit_str);

            load_job_created = true;
        }
    }
    emit loadingStartedSignal();

    if (!load_job_created)
        finishLoading();
}

void DBContentManager::addLoadedData(std::map<std::string, std::shared_ptr<Buffer>> data)
{
    loginf << "DBContentManager: addLoadedData";

    // newest data batch has been finalized, ready to be added

    // add buffers to data

    bool something_changed = false;

    for (auto& buf_it : data)
    {
        if (!buf_it.second->size()) // empty buffer
            continue;

        if (data_.count(buf_it.first))
        {
            loginf << "DBContentManager: addLoadedData: adding buffer dbo " << buf_it.first
                   << " adding size " << buf_it.second->size() << " current size " << data_.at(buf_it.first)->size();

            data_.at(buf_it.first)->seizeBuffer(*buf_it.second.get());

            loginf << "DBContentManager: addLoadedData: new buffer dbo " << buf_it.first
                   << " size " << data_.at(buf_it.first)->size();
        }
        else
        {
            data_[buf_it.first] = move(buf_it.second);

            loginf << "DBContentManager: addLoadedData: created buffer dbo " << buf_it.first
                   << " size " << data_.at(buf_it.first)->size();
        }

        something_changed = true;
    }

    if (something_changed)
    {
        updateNumLoadedCounts();
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

    for (auto& object : dbcontent_)
        object.second->quitLoading();

    load_in_progress_ = true;  // TODO
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
    if (db_interface.hasProperty(timestamp_min_name))
        timestamp_min_ = Time::fromLong(stol(db_interface.getProperty(timestamp_min_name)));
    if (db_interface.hasProperty(timestamp_max_name))
        timestamp_max_ = Time::fromLong(stol(db_interface.getProperty(timestamp_max_name)));

    if (db_interface.hasProperty(latitude_min_name))
        latitude_min_ = stod(db_interface.getProperty(latitude_min_name));
    if (db_interface.hasProperty(latitude_max_name))
        latitude_max_ = stod(db_interface.getProperty(latitude_max_name));

    if (db_interface.hasProperty(longitude_min_name))
        longitude_min_ = stod(db_interface.getProperty(longitude_min_name));
    if (db_interface.hasProperty(longitude_max_name))
        longitude_max_ = stod(db_interface.getProperty(longitude_max_name));

    for (auto& object : dbcontent_)
        object.second->databaseOpenedSlot();

    targets_ = COMPASS::instance().interface().loadTargets();

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

    targets_.clear();

    timestamp_min_.reset();
    timestamp_max_.reset();
    latitude_min_.reset();
    latitude_max_.reset();
    longitude_min_.reset();
    longitude_max_.reset();

    associationStatusChangedSignal();
}

//void DBContentManager::databaseContentChangedSlot()
//{
//    loginf << "DBContentManager: databaseContentChangedSlot";

//    // emit databaseContentChangedSignal();

//    //    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

//    //    loginf << "DBContentManager: databaseContentChangedSlot";

//    //    if (COMPASS::instance().interface().hasProperty("associations_generated"))
//    //    {
//    //        assert(COMPASS::instance().interface().hasProperty("associations_dbo"));
//    //        assert(COMPASS::instance().interface().hasProperty("associations_ds"));

//    //        has_associations_ =
//    //                COMPASS::instance().interface().getProperty("associations_generated") == "1";
//    //        associations_dbo_ = COMPASS::instance().interface().getProperty("associations_dbo");
//    //        associations_ds_ = COMPASS::instance().interface().getProperty("associations_ds");
//    //    }
//    //    else
//    //    {
//    //        has_associations_ = false;
//    //        associations_dbo_ = "";
//    //        associations_ds_ = "";
//    //    }

//    //    for (auto& object : objects_)
//    //        object.second->updateToDatabaseContent();

//    //    QApplication::restoreOverrideCursor();

//    if (load_widget_)
//        load_widget_->update();

//    //emit dbObjectsChangedSignal();

//}

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
    logdbg << "DBContentManager: insertData";

    assert (!load_in_progress_);
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
        logdbg << "DBContentManager: insertDone: not done";
}

void DBContentManager::finishInserting()
{
    logdbg << "DBContentManager: finishInserting: all done";

    // calculate min/max values

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

                bool has_min_max = hasMinMaxTimestamp();

                for (unsigned int cnt=0; cnt < buffer_size; cnt++)
                {
                    if (!data_vec.isNull(cnt))
                    {
                        if (has_min_max)
                        {
                            timestamp_min_ = std::min(timestamp_min_.get(), data_vec.get(cnt));
                            timestamp_max_ = std::max(timestamp_max_.get(), data_vec.get(cnt));
                        }
                        else
                        {
                            timestamp_min_ = data_vec.get(cnt);
                            timestamp_max_ = data_vec.get(cnt);

                            has_min_max = true;
                        }
                    }
                }

                if (has_min_max)
                {
                    COMPASS::instance().interface().setProperty(timestamp_min_name,
                                                                to_string(Time::toLong(timestamp_min_.get())));
                    COMPASS::instance().interface().setProperty(timestamp_max_name,
                                                                to_string(Time::toLong(timestamp_max_.get())));

                    logdbg << "DBContentManager: finishInserting: tod min " << timestamp_min_.get()
                           << " max " << timestamp_max_.get();
                }
            }
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
                    COMPASS::instance().interface().setProperty(latitude_min_name, to_string(latitude_min_.get()));
                    COMPASS::instance().interface().setProperty(latitude_max_name, to_string(latitude_max_.get()));

                    COMPASS::instance().interface().setProperty(longitude_min_name, to_string(longitude_min_.get()));
                    COMPASS::instance().interface().setProperty(longitude_max_name, to_string(longitude_max_.get()));

                    logdbg << "DBContentManager: finishInserting: lat min " << latitude_min_.get()
                           << " max " << latitude_max_.get()
                           << " lon min " << longitude_min_.get()
                           << " max " << longitude_max_.get();
                }
            }
        }
    }

    if (COMPASS::instance().appMode() == AppMode::Offline || COMPASS::instance().appMode() == AppMode::LivePaused)
        insert_data_.clear();

    insert_in_progress_ = false;
    logdbg << "DBContentManager: finishInserting: insert in progress " << insert_in_progress_;
    emit insertDoneSignal();

    // start clearing old data

    if (COMPASS::instance().appMode() == AppMode::LiveRunning)
    {
        using namespace boost::posix_time;

        ptime old_time = Time::currentUTCTime() - minutes(max_live_data_age_);

        loginf << "DBContentManager: finishInserting: deleting data before " << Time::toString(old_time);

        deleteDBContent(old_time);
    }

    // add inserted to loaded data

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    assert (existsMetaVariable(DBContent::meta_var_timestamp_.name()));

    bool had_data = data_.size();

    if (COMPASS::instance().appMode() == AppMode::LiveRunning) // do tod cleanup
    {
        addInsertedDataToChache();

        cutCachedData();
        filterDataSources();

        if (COMPASS::instance().filterManager().useFilters())
            COMPASS::instance().filterManager().filterBuffers(data_);

        logdbg << "DBContentManager: finishInserting: distributing data";

        if (data_.size())
            emit loadedDataSignal(data_, true);
        else if (had_data)
            COMPASS::instance().viewManager().clearDataInViews();

        updateNumLoadedCounts();
    }

    COMPASS::instance().dataSourceManager().updateWidget();

    boost::posix_time::time_duration time_diff = boost::posix_time::microsec_clock::local_time() - start_time;
    logdbg << "DBContentManager: finishInserting: processing took "
        << String::timeStringFromDouble(time_diff.total_milliseconds() / 1000.0, true);
}

void DBContentManager::addInsertedDataToChache()
{
    loginf << "DBContentManager: addInsertedDataToChache";

    assert (label_generator_);

    for (auto& buf_it : insert_data_)
    {

        VariableSet read_set = COMPASS::instance().viewManager().getReadSet(buf_it.first);
        label_generator_->addVariables(buf_it.first, read_set);

        vector<Property> buffer_properties_to_be_removed;

        // remove all unused
        for (const auto& prop_it : buf_it.second->properties().properties())
        {
            if (!read_set.hasDBColumnName(prop_it.name()))
                buffer_properties_to_be_removed.push_back(prop_it); // remove it later
        }

        for (auto& prop_it : buffer_properties_to_be_removed)
        {
            logdbg << "DBContentManager: addInsertedDataToChache: deleting property " << prop_it.name();
            buf_it.second->deleteProperty(prop_it);
        }

        // change db column names to dbo var names
        buf_it.second->transformVariables(read_set, true);

        // add selection flags
        buf_it.second->addProperty(DBContent::selected_var);

        // add buffer to be able to distribute to views
        if (!data_.count(buf_it.first))
            data_[buf_it.first] = buf_it.second;
        else
        {
            data_.at(buf_it.first)->seizeBuffer(*buf_it.second.get());

            // sort by tod
            assert (metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(buf_it.first));

            Variable& ts_var = metaVariable(DBContent::meta_var_timestamp_.name()).getFor(buf_it.first);

            Property ts_prop {ts_var.name(), ts_var.dataType()};

            assert (data_.at(buf_it.first)->hasProperty(ts_prop));

            data_.at(buf_it.first)->sortByProperty(ts_prop);
        }
    }

    insert_data_.clear();
}

void DBContentManager::filterDataSources()
{
    std::map<unsigned int, std::set<unsigned int>> wanted_data_sources =
            COMPASS::instance().dataSourceManager().getLoadDataSources();

    unsigned int buffer_size;
    vector<size_t> indexes_to_remove;

    for (auto& buf_it : data_)
    {
        // remove unwanted data sources
        assert (metaVariable(DBContent::meta_var_datasource_id_.name()).existsIn(buf_it.first));
        assert (metaVariable(DBContent::meta_var_line_id_.name()).existsIn(buf_it.first));

        Variable& ds_id_var = metaVariable(DBContent::meta_var_datasource_id_.name()).getFor(buf_it.first);
        Variable& line_id_var = metaVariable(DBContent::meta_var_line_id_.name()).getFor(buf_it.first);

        Property ds_id_prop {ds_id_var.name(), ds_id_var.dataType()};
        assert (buf_it.second->hasProperty(ds_id_prop));

        Property line_id_prop {line_id_var.name(), line_id_var.dataType()};
        assert (buf_it.second->hasProperty(ds_id_prop));

        NullableVector<unsigned int>& ds_id_vec = buf_it.second->get<unsigned int>(ds_id_var.name());
        NullableVector<unsigned int>& line_id_vec = buf_it.second->get<unsigned int>(line_id_var.name());

        buffer_size = buf_it.second->size();

        indexes_to_remove.clear();
        //assert (ds_id_vec.isNeverNull()); TODO why asserts?

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            assert (!ds_id_vec.isNull(index));
            assert (!line_id_vec.isNull(index));

            if (!wanted_data_sources.count(ds_id_vec.get(index)) // unwanted ds
                    || !wanted_data_sources.at(ds_id_vec.get(index)).count(line_id_vec.get(index))) // unwanted line
                indexes_to_remove.push_back(index);
        }

        loginf << "DBContentManager: filterDataSources: in " << buf_it.first << " remove "
               << indexes_to_remove.size() << " of " << buffer_size;

        buf_it.second->removeIndexes(indexes_to_remove);

        // remove unwanted lines
        indexes_to_remove.clear();
        buffer_size = buf_it.second->size();
    }

    // remove empty buffers
    std::map<std::string, std::shared_ptr<Buffer>> tmp_data = data_;

    for (auto& buf_it : tmp_data)
        if (!buf_it.second->size())
            data_.erase(buf_it.first);
}

void DBContentManager::cutCachedData()
{
    unsigned int buffer_size;

    bool max_time_set = false;
    boost::posix_time::ptime min_ts_found, max_ts_found;

    boost::posix_time::ptime max_ts = Time::currentUTCTime();

    float time_offset = COMPASS::instance().mainWindow().importASTERIXFromNetworkTimeOffset();

    loginf << "DBContentManager: cutCachedData: max_time " << Time::toString(max_ts)
           << " time offset " << String::timeStringFromDouble(time_offset);

    max_ts += boost::posix_time::milliseconds((unsigned int) (time_offset*1000.0));;

    for (auto& buf_it : data_)
    {
        assert (metaVariable(DBContent::meta_var_timestamp_.name()).existsIn(buf_it.first));

        Variable& ts_var = metaVariable(DBContent::meta_var_timestamp_.name()).getFor(buf_it.first);

        Property ts_prop {ts_var.name(), ts_var.dataType()};

        if(buf_it.second->hasProperty(ts_prop))
        {
            NullableVector<boost::posix_time::ptime>& timestamp_vec = buf_it.second->get<boost::posix_time::ptime>(
                        ts_var.name());

            auto minmax = timestamp_vec.minMaxValues();
            assert (get<0>(minmax)); // there is minmax

            if (max_time_set)
            {
                min_ts_found = min(min_ts_found, get<1>(minmax));
                max_ts_found = max(max_ts_found, get<2>(minmax));
            }
            else
            {
                min_ts_found = get<1>(minmax);
                max_ts_found = get<2>(minmax);
                max_time_set = true;
            }
        }
        else
            logwrn << "DBContentManager: cutCachedData: buffer " << buf_it.first << " has not timestamp for min/max";
    }

    if (max_time_set)
        loginf << "DBContentManager: cutCachedData: data time min " << Time::toString(min_ts_found)
               << " max " << Time::toString(max_ts_found);

    boost::posix_time::ptime min_ts = max_ts - boost::posix_time::minutes(5); // max - 5min


    loginf << "DBContentManager: cutCachedData: min_ts " << Time::toString(min_ts)
              //<< " data min " << String::timeStringFromDouble(min_tod_found)
           << " max_ts " << Time::toString(max_ts);
    //<< " utc " << String::timeStringFromDouble(secondsSinceMidnighUTC());

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

            if (index) // index found
            {
                index--; // cut at previous

                loginf << "DBContentManager: cutCachedData: cutting " << buf_it.first
                       << " up to index " << index
                       << " total size " << buffer_size;
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
    loginf << "DBContentManager: updateNumLoadedCounts";

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


unsigned int DBContentManager::maxRecordNumber() const
{
    assert (has_max_rec_num_);
    return max_rec_num_;
}

void DBContentManager::maxRecordNumber(unsigned int value)
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

bool DBContentManager::hasTargetsInfo()
{
    return targets_.size();
}

void DBContentManager::clearTargetsInfo()
{
    targets_.clear();
    COMPASS::instance().interface().clearTargetsTable();
}

bool DBContentManager::existsTarget(unsigned int utn)
{
    return targets_.count(utn);
}

void DBContentManager::createTarget(unsigned int utn)
{
    assert (!existsTarget(utn));

    targets_.emplace(utn, make_shared<dbContent::Target>(utn, nlohmann::json::object()));

    assert (existsTarget(utn));
}

std::shared_ptr<dbContent::Target> DBContentManager::target(unsigned int utn)
{
    assert (existsTarget(utn));
    return targets_.at(utn);
}

void DBContentManager::saveTargets()
{
    COMPASS::instance().interface().saveTargets(targets_);
}

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
