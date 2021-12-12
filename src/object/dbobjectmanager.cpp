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

#include "dbobjectmanager.h"

#include "compass.h"
#include "configurationmanager.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanagerloadwidget.h"
#include "dbobjectmanagerwidget.h"
#include "dbovariable.h"
#include "dbovariableset.h"
#include "logger.h"
#include "metadbovariable.h"
#include "stringconv.h"
#include "number.h"
#include "viewmanager.h"
#include "jobmanager.h"
#include "evaluationmanager.h"
#include "filtermanager.h"
#include "util/number.h"
#include "metadbovariableconfigurationdialog.h"
#include "json.hpp"

#include <QApplication>
#include <QMessageBox>

#include <algorithm>
#include <chrono>

using namespace std;
using namespace Utils;
using namespace DBContent;
using namespace nlohmann;

// move to somewhere else?
double secondsSinceMidnighUTC ()
{
    using namespace boost::posix_time;

    auto p_time = microsec_clock::universal_time (); // UTC.
    return (p_time.time_of_day().total_milliseconds() / 1000.0) - 3600.0;

////    auto now = std::chrono::system_clock::now(); // system_clock
////    time_t tnow = std::chrono::system_clock::to_time_t(now);

////    loginf << " now is " << std::ctime(&tnow);

////    //tm *date = std::localtime(&tnow); // local
////    tm *date = std::gmtime(&tnow); // utc
////    date->tm_hour = 0;
////    date->tm_min = 0;
////    date->tm_sec = 0;
////    auto midnight = std::chrono::system_clock::from_time_t(std::mktime(date));

////    double tod = std::chrono::duration<double>(now-midnight).count();

//////    auto now = std::chrono::system_clock::now();
//////    auto today = floor<days>(now);
//////    auto tod = duration_cast<seconds>(now - today);

////    using namespace std::chrono;
////    using namespace std;
////    using days = duration<int, ratio<86400>>;
////    seconds last_midnight =
////        time_point_cast<days>(system_clock::now()).time_since_epoch();
////    loginf << "UGA " <<  last_midnight.count() << String::timeStringFromDouble(last_midnight.count());

//    return tod;
}

const std::vector<std::string> DBObjectManager::data_source_types_ {"Radar", "MLAT", "ADSB", "Tracker", "RefTraj"};

DBObjectManager::DBObjectManager(const std::string& class_id, const std::string& instance_id,
                                 COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "db_object.json"), compass_(*compass)
{
    logdbg << "DBObjectManager: constructor: creating subconfigurables";

    registerParameter("use_order", &use_order_, false);
    registerParameter("use_order_ascending", &use_order_ascending_, false);
    registerParameter("order_variable_dbo_name", &order_variable_dbo_name_, "");
    registerParameter("order_variable_name", &order_variable_name_, "");

    registerParameter("use_limit", &use_limit_, false);
    registerParameter("limit_min", &limit_min_, 0);
    registerParameter("limit_max", &limit_max_, 100000);

    createSubConfigurables();

    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>"); // for dbo read job
    // for signal about new data
    qRegisterMetaType<std::map<std::string, std::shared_ptr<Buffer>>>("std::map<std::string, std::shared_ptr<Buffer>>");
}

DBObjectManager::~DBObjectManager()
{
    data_.clear();

    for (auto it : objects_)
        delete it.second;
    objects_.clear();

    meta_variables_.clear();

    widget_ = nullptr;
    load_widget_ = nullptr;
}

void DBObjectManager::generateSubConfigurable(const std::string& class_id,
                                              const std::string& instance_id)
{
    logdbg << "DBObjectManager: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;
    if (class_id.compare("DBObject") == 0)
    {
        DBObject* object = new DBObject(compass_, class_id, instance_id, this);
        loginf << "DBObjectManager: generateSubConfigurable: adding object type " << object->name();
        assert(!objects_.count(object->name()));
        objects_[object->name()] = object;
    }
    else if (class_id.compare("MetaDBOVariable") == 0)
    {
        MetaDBOVariable* meta_var = new MetaDBOVariable(class_id, instance_id, this);
        logdbg << "DBObjectManager: generateSubConfigurable: adding meta var type "
               << meta_var->name();

        assert(!existsMetaVariable(meta_var->name()));
        meta_variables_.emplace_back(meta_var);
    }
    else if (class_id.compare("DBContentConfigurationDataSource") == 0)
    {
        unique_ptr<ConfigurationDataSource> ds {new ConfigurationDataSource(class_id, instance_id, *this)};
        loginf << "DBObjectManager: generateSubConfigurable: adding config ds "
               << ds->name() << " sac/sic " <<  ds->sac() << "/" << ds->sic();

        assert (!hasConfigDataSource(Number::dsIdFrom(ds->sac(), ds->sic())));
        config_data_sources_.emplace_back(move(ds));
    }
    else
        throw std::runtime_error("DBObjectManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void DBObjectManager::checkSubConfigurables()
{
    // nothing to do, must be defined in configuration
}

bool DBObjectManager::existsObject(const std::string& dbo_name)
{
    logdbg << "DBObjectManager: existsObject: '" << dbo_name << "'";

    return (objects_.find(dbo_name) != objects_.end());
}

DBObject& DBObjectManager::object(const std::string& dbo_name)
{
    logdbg << "DBObjectManager: object: name " << dbo_name;

    assert(objects_.find(dbo_name) != objects_.end());

    return *objects_.at(dbo_name);
}

void DBObjectManager::deleteObject(const std::string& dbo_name)
{
    logdbg << "DBObjectManager: deleteObject: name " << dbo_name;
    assert(existsObject(dbo_name));
    delete objects_.at(dbo_name);
    objects_.erase(dbo_name);

    emit dbObjectsChangedSignal();
}

bool DBObjectManager::hasData()
{
    for (auto& object_it : objects_)
        if (object_it.second->hasData())
            return true;

    return false;
}

bool DBObjectManager::existsMetaVariable(const std::string& var_name)
{
    return std::find_if(meta_variables_.begin(), meta_variables_.end(),
                        [var_name](const std::unique_ptr<MetaDBOVariable>& var) -> bool { return var->name() == var_name; })
            != meta_variables_.end();
}

MetaDBOVariable& DBObjectManager::metaVariable(const std::string& var_name)
{
    logdbg << "DBObjectManager: metaVariable: name " << var_name;

    assert(existsMetaVariable(var_name));

    auto it = std::find_if(meta_variables_.begin(), meta_variables_.end(),
                           [var_name](const std::unique_ptr<MetaDBOVariable>& var) -> bool { return var->name() == var_name; });

    assert (it != meta_variables_.end());

    return *it->get();
}

void DBObjectManager::renameMetaVariable(const std::string& old_var_name, const std::string& new_var_name)
{
    assert(existsMetaVariable(old_var_name));
    metaVariable(old_var_name).name(new_var_name);

    if (meta_cfg_dialog_)
    {
        meta_cfg_dialog_->updateList();
        meta_cfg_dialog_->selectMetaVariable(new_var_name);
    }
}

void DBObjectManager::deleteMetaVariable(const std::string& var_name)
{
    logdbg << "DBObjectManager: deleteMetaVariable: name " << var_name;
    assert(existsMetaVariable(var_name));

    auto it = std::find_if(meta_variables_.begin(), meta_variables_.end(),
                           [var_name](const std::unique_ptr<MetaDBOVariable>& var) -> bool { return var->name() == var_name; });

    assert (it != meta_variables_.end());

    meta_variables_.erase(it);

    if (meta_cfg_dialog_)
    {
        meta_cfg_dialog_->updateList();
        meta_cfg_dialog_->clearDetails();
    }
}

bool DBObjectManager::usedInMetaVariable(const DBOVariable& variable)
{
    for (auto& meta_it : meta_variables_)
        if (meta_it->uses(variable))
            return true;

    return false;
}

DBObjectManagerWidget* DBObjectManager::widget()
{
    if (!widget_)
    {
        widget_.reset(new DBObjectManagerWidget(*this));
    }

    assert(widget_);
    return widget_.get();
}

DBObjectManagerLoadWidget* DBObjectManager::loadWidget()
{
    if (!load_widget_)
    {
        load_widget_.reset(new DBObjectManagerLoadWidget(*this));
    }

    assert(load_widget_);
    return load_widget_.get();
}

bool DBObjectManager::useLimit() const { return use_limit_; }

void DBObjectManager::useLimit(bool use_limit) { use_limit_ = use_limit; }

unsigned int DBObjectManager::limitMin() const { return limit_min_; }

void DBObjectManager::limitMin(unsigned int limit_min)
{
    limit_min_ = limit_min;
    loginf << "DBObjectManager: limitMin: " << limit_min_;
}

unsigned int DBObjectManager::limitMax() const { return limit_max_; }

void DBObjectManager::limitMax(unsigned int limit_max)
{
    limit_max_ = limit_max;
    loginf << "DBObjectManager: limitMax: " << limit_max_;
}

bool DBObjectManager::useOrder() const { return use_order_; }

void DBObjectManager::useOrder(bool use_order) { use_order_ = use_order; }

bool DBObjectManager::useOrderAscending() const { return use_order_ascending_; }

void DBObjectManager::useOrderAscending(bool use_order_ascending)
{
    use_order_ascending_ = use_order_ascending;
}

bool DBObjectManager::hasOrderVariable()
{
    if (existsObject(order_variable_dbo_name_))
        if (object(order_variable_dbo_name_).hasVariable(order_variable_name_))
            return true;
    return false;
}

DBOVariable& DBObjectManager::orderVariable()
{
    assert(hasOrderVariable());
    return object(order_variable_dbo_name_).variable(order_variable_name_);
}

void DBObjectManager::orderVariable(DBOVariable& variable)
{
    order_variable_dbo_name_ = variable.dboName();
    order_variable_name_ = variable.name();
}

bool DBObjectManager::hasOrderMetaVariable()
{
    if (order_variable_dbo_name_ == META_OBJECT_NAME)
        return existsMetaVariable(order_variable_name_);

    return false;
}

MetaDBOVariable& DBObjectManager::orderMetaVariable()
{
    assert(hasOrderMetaVariable());
    return metaVariable(order_variable_name_);
}

void DBObjectManager::orderMetaVariable(MetaDBOVariable& variable)
{
    order_variable_dbo_name_ = META_OBJECT_NAME;
    order_variable_name_ = variable.name();
}

void DBObjectManager::clearOrderVariable()
{
    order_variable_dbo_name_ = "";
    order_variable_name_ = "";
}

void DBObjectManager::startLoading()
{
    logdbg << "DBObjectManager: loadSlot";

    data_.clear();

    load_in_progress_ = true;

    bool load_job_created = false;

    loginf << "DBObjectManager: loadSlot: loading associations";

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QMessageBox* msg_box = new QMessageBox();
    msg_box->setWindowTitle("Loading Associations");
    msg_box->setStandardButtons(QMessageBox::NoButton);

    bool shown = false;

    for (auto& object : objects_)
    {
        //object.second->clearData();  // clear previous data

        msg_box->setText(("Processing DBObject "+object.first).c_str());

        if (has_associations_ && object.second->loadable() && !object.second->associationsLoaded())
        {
            if (!shown)
            {
                msg_box->show();
                shown = true;
            }

            object.second->loadAssociationsIfRequired();

            while (!object.second->associationsLoaded())
            {
                QCoreApplication::processEvents();
                QThread::msleep(1);
            }
        }
    }

    msg_box->close();
    delete msg_box;

    while (JobManager::instance().hasDBJobs())
    {
        logdbg << "DBObjectManager: loadSlot: waiting on association loading";

        QCoreApplication::processEvents();
        QThread::msleep(5);
    }

    loginf << "DBObjectManager: loadSlot: starting loading";

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();
    ViewManager& view_man = COMPASS::instance().viewManager();

    for (auto& object : objects_)
    {
        loginf << "DBObjectManager: loadSlot: object " << object.first
               << " loadable " << object.second->loadable()
               << " wanted " << object.second->loadingWanted();

        if (object.first == "RefTraj")
            continue;

        if (object.second->loadable() && object.second->loadingWanted())
        {
            loginf << "DBObjectManager: loadSlot: loading object " << object.first;
            DBOVariableSet read_set = view_man.getReadSet(object.first);

            if (eval_man.needsAdditionalVariables())
                eval_man.addVariables(object.first, read_set);

            if (read_set.getSize() == 0)
            {
                loginf << "DBObjectManager: loadSlot: skipping loading of object " << object.first
                       << " since an empty read list was detected";
                continue;
            }

            std::string limit_str = "";
            if (use_limit_)
            {
                limit_str = std::to_string(limit_min_) + "," + std::to_string(limit_max_);
                logdbg << "DBObjectManager: loadSlot: use limit str " << limit_str;
            }

            DBOVariable* variable = nullptr;

            assert (hasOrderVariable() || hasOrderMetaVariable());

            if (hasOrderVariable())
                variable = &orderVariable();
            if (hasOrderMetaVariable())
                variable = &orderMetaVariable().getFor(object.first);

            // load (DBOVariableSet &read_set, bool use_filters, bool use_order, DBOVariable
            // *order_variable, bool use_order_ascending, const std::string &limit_str="")
            object.second->load(read_set, COMPASS::instance().filterManager().useFilters(),
                                use_order_, variable, use_order_ascending_,
                                limit_str);

            load_job_created = true;
        }
    }
    emit loadingStartedSignal();

    if (!load_job_created)
        finishLoading();
}

void DBObjectManager::addLoadedData(std::map<std::string, std::shared_ptr<Buffer>> data)
{
    loginf << "DBObjectManager: addLoadedData";

    // newest data batch has been finalized, ready to be added

    // add buffers to data

    bool something_changed = false;

    for (auto& buf_it : data)
    {
        if (!buf_it.second->size())
        {
            logerr << "DBObjectManager: addLoadedData: buffer dbo " << buf_it.first << " with 0 size";
            continue;
        }

        if (data_.count(buf_it.first))
            data_.at(buf_it.first)->seizeBuffer(*buf_it.second.get());
        else
            data_[buf_it.first] = move(buf_it.second);

        something_changed = true;
    }

    if (something_changed)
        emit loadedDataSignal(data_, false);
}

void DBObjectManager::quitLoading()
{
    loginf << "DBObjectManager: quitLoading";

    for (auto& object : objects_)
        object.second->quitLoading();

    load_in_progress_ = true;  // TODO
}

void DBObjectManager::databaseOpenedSlot()
{
    loginf << "DBObjectManager: databaseOpenedSlot";

    loadDBDataSources();
    loadMaxRecordNumber();

    for (auto& object : objects_)
        object.second->databaseOpenedSlot();

    if (load_widget_)
        load_widget_->update();
}

void DBObjectManager::databaseClosedSlot()
{
    db_data_sources_.clear();

    max_rec_num_ = 0;
    has_max_rec_num_ = false;

    for (auto& object : objects_)
        object.second->databaseClosedSlot();

    if (load_widget_)
        load_widget_->update();
}

void DBObjectManager::databaseContentChangedSlot()
{
    loginf << "DBObjectManager: databaseContentChangedSlot";

    // emit databaseContentChangedSignal();

    //    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    //    loginf << "DBObjectManager: databaseContentChangedSlot";

    //    if (COMPASS::instance().interface().hasProperty("associations_generated"))
    //    {
    //        assert(COMPASS::instance().interface().hasProperty("associations_dbo"));
    //        assert(COMPASS::instance().interface().hasProperty("associations_ds"));

    //        has_associations_ =
    //                COMPASS::instance().interface().getProperty("associations_generated") == "1";
    //        associations_dbo_ = COMPASS::instance().interface().getProperty("associations_dbo");
    //        associations_ds_ = COMPASS::instance().interface().getProperty("associations_ds");
    //    }
    //    else
    //    {
    //        has_associations_ = false;
    //        associations_dbo_ = "";
    //        associations_ds_ = "";
    //    }

    //    for (auto& object : objects_)
    //        object.second->updateToDatabaseContent();

    //    QApplication::restoreOverrideCursor();

    if (load_widget_)
        load_widget_->update();

    //emit dbObjectsChangedSignal();

}

void DBObjectManager::loadingDone(DBObject& object)
{
    bool done = true;

    for (auto& object_it : objects_)
    {
        if (object_it.second->isLoading())
        {
            logdbg << "DBObjectManager: loadingDoneSlot: " << object_it.first << " still loading";
            done = false;
            break;
        }
    }

    if (done)
        finishLoading();
    else
        logdbg << "DBObjectManager: loadingDoneSlot: not done";
}

void DBObjectManager::metaDialogOKSlot()
{
    assert (meta_cfg_dialog_);
    meta_cfg_dialog_->hide();
}

void DBObjectManager::finishLoading()
{
    loginf << "DBObjectManager: finishLoading: all done";
    load_in_progress_ = false;

    COMPASS::instance().viewManager().doViewPointAfterLoad();

    emit loadingDoneSignal();

    if (load_widget_)
        load_widget_->loadingDone();

    QApplication::restoreOverrideCursor();
}

bool DBObjectManager::hasConfigDataSource (unsigned int ds_id)
{
    unsigned int sac = Number::sacFromDsId(ds_id);
    unsigned int sic = Number::sicFromDsId(ds_id);

    return find_if(config_data_sources_.begin(), config_data_sources_.end(),
                   [sac,sic] (const std::unique_ptr<DBContent::ConfigurationDataSource>& s)
    { return s->sac() == sac && s->sic() == sic; } ) != config_data_sources_.end();

}

DBContent::ConfigurationDataSource& DBObjectManager::configDataSource (unsigned int ds_id)
{
    assert (hasConfigDataSource(ds_id));

    unsigned int sac = Number::sacFromDsId(ds_id);
    unsigned int sic = Number::sicFromDsId(ds_id);

    return *find_if(config_data_sources_.begin(), config_data_sources_.end(),
                    [sac,sic] (const std::unique_ptr<DBContent::ConfigurationDataSource>& s)
    { return s->sac() == sac && s->sic() == sic; } )->get();
}

void DBObjectManager::loadDBDataSources()
{
    assert (!db_data_sources_.size());

    DBInterface& db_interface = COMPASS::instance().interface();

    if (db_interface.existsDataSourcesTable())
    {
        db_data_sources_ = db_interface.getDataSources();
        sortDBDataSources();
    }
}

void DBObjectManager::sortDBDataSources()
{
    sort(db_data_sources_.begin(), db_data_sources_.end(),
         [](const std::unique_ptr<DBContent::DBDataSource>& a, const std::unique_ptr<DBContent::DBDataSource>& b) -> bool
    {
        return a->name() < b->name();
    });
}

void DBObjectManager::saveDBDataSources()
{
    DBInterface& db_interface = COMPASS::instance().interface();

    assert(db_interface.dbOpen());
    db_interface.saveDataSources(db_data_sources_);
}

bool DBObjectManager::canAddNewDataSourceFromConfig (unsigned int ds_id)
{
    if (hasDataSource(ds_id))
        return false;

    return hasConfigDataSource(ds_id);
}

bool DBObjectManager::hasDataSource(unsigned int ds_id)
{
    return find_if(db_data_sources_.begin(), db_data_sources_.end(),
                   [ds_id] (const std::unique_ptr<DBContent::DBDataSource>& s)
    { return Number::dsIdFrom(s->sac(), s->sic()) == ds_id; } ) != db_data_sources_.end();
}

void DBObjectManager::addNewDataSource (unsigned int ds_id)
{
    loginf << "DBObjectManager: addNewDataSource: ds_id " << ds_id;

    assert (!hasDataSource(ds_id));

    if (hasConfigDataSource(ds_id))
    {
        loginf << "DBObjectManager: addNewDataSource: ds_id " << ds_id << " from config";

        DBContent::ConfigurationDataSource& cfg_ds = configDataSource(ds_id);

        db_data_sources_.emplace_back(move(cfg_ds.getAsNewDBDS()));
        sortDBDataSources();
    }
    else
    {
        loginf << "DBObjectManager: addNewDataSource: ds_id " << ds_id << " create new";

        DBContent::DBDataSource* new_ds = new DBContent::DBDataSource();
        new_ds->id(ds_id);
        new_ds->sac(Number::sacFromDsId(ds_id));
        new_ds->sic(Number::sicFromDsId(ds_id));
        new_ds->name(to_string(ds_id));

        db_data_sources_.emplace_back(move(new_ds));
        sortDBDataSources();
    }

    assert (hasDataSource(ds_id));

    loginf << "DBObjectManager: addNewDataSource: ds_id " << ds_id << " done";
}

DBContent::DBDataSource& DBObjectManager::dataSource(unsigned int ds_id)
{
    assert (hasDataSource(ds_id));

    return *find_if(db_data_sources_.begin(), db_data_sources_.end(),
                    [ds_id] (const std::unique_ptr<DBContent::DBDataSource>& s)
    { return Number::dsIdFrom(s->sac(), s->sic()) == ds_id; } )->get();
}



bool DBObjectManager::hasAssociations() const { return has_associations_; }

void DBObjectManager::setAssociationsDataSource(const std::string& dbo, const std::string& data_source_name)
{
    COMPASS::instance().interface().setProperty("associations_generated", "1");
    COMPASS::instance().interface().setProperty("associations_dbo", dbo);
    COMPASS::instance().interface().setProperty("associations_ds", data_source_name);

    has_associations_ = true;
    associations_dbo_ = dbo;
    assert(existsObject(associations_dbo_));
    associations_ds_ = data_source_name;

    if (load_widget_)
        loadWidget()->update();
}

void DBObjectManager::setAssociationsByAll()
{
    COMPASS::instance().interface().setProperty("associations_generated", "1");
    COMPASS::instance().interface().setProperty("associations_dbo", "");
    COMPASS::instance().interface().setProperty("associations_ds", "");

    has_associations_ = true;
    associations_dbo_ = "";
    associations_ds_ = "";

    if (load_widget_)
        loadWidget()->update();
}

void DBObjectManager::removeAssociations()
{
    COMPASS::instance().interface().setProperty("associations_generated", "0");
    COMPASS::instance().interface().setProperty("associations_dbo", "");
    COMPASS::instance().interface().setProperty("associations_ds", "");

    has_associations_ = false;
    associations_dbo_ = "";
    associations_ds_ = "";

    for (auto& dbo_it : objects_)
        dbo_it.second->clearAssociations();

    if (load_widget_)
        loadWidget()->update();
}

bool DBObjectManager::hasAssociationsDataSource() const
{
    return associations_dbo_.size() && associations_ds_.size();
}

std::string DBObjectManager::associationsDBObject() const { return associations_dbo_; }

std::string DBObjectManager::associationsDataSourceName() const { return associations_ds_; }

bool DBObjectManager::isOtherDBObjectPostProcessing(DBObject& object)
{
    for (auto& dbo_it : objects_)
        if (dbo_it.second != &object && dbo_it.second->isPostProcessing())
            return true;

    return false;
}

bool DBObjectManager::loadInProgress() const
{
    return load_in_progress_;
}

void DBObjectManager::insertData(std::map<std::string, std::shared_ptr<Buffer>> data)
{
    loginf << "DBObjectManager: insertData";

    assert (!load_in_progress_);
    assert (!insert_in_progress_);
    assert (!insert_data_.size());

    insert_in_progress_ = true;

    insert_data_ = data;

    for (auto& buf_it : insert_data_)
    {
        assert(existsObject(buf_it.first));
        object(buf_it.first).insertData(buf_it.second);
    }
}

void DBObjectManager::insertDone(DBObject& object)
{
    bool done = true;

    for (auto& object_it : objects_)
    {
        if (object_it.second->isInserting())
        {
            logdbg << "DBObjectManager: insertDone: " << object_it.first << " still inserting";
            done = false;
            break;
        }
    }

    if (done)
        finishInserting();
    else
        logdbg << "DBObjectManager: insertDone: not done";
}

void DBObjectManager::finishInserting()
{
    loginf << "DBObjectManager: finishInserting: all done";

    insert_in_progress_ = false;

    emit insertDoneSignal();

    // add inserted to loaded data

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    assert (existsMetaVariable(DBObject::meta_var_tod_id_.name()));

    {
        for (auto& buf_it : insert_data_)
        {

            DBOVariableSet read_set = COMPASS::instance().viewManager().getReadSet(buf_it.first);
            vector<Property> buffer_properties_to_be_removed;

            // remove all unused
            for (const auto& prop_it : buf_it.second->properties().properties())
            {
                if (!read_set.hasDBColumnName(prop_it.name()))
                    buffer_properties_to_be_removed.push_back(prop_it); // remove it later
            }

            for (auto& prop_it : buffer_properties_to_be_removed)
            {
                logdbg << "DBObjectManager: finishInserting: deleting property " << prop_it.name();
                buf_it.second->deleteProperty(prop_it);
            }

            // change db column names to dbo var names
            buf_it.second->transformVariables(read_set, true);

            // add selection flags
            buf_it.second->addProperty(DBObject::selected_var);

            // add buffer to be able to distribute to views
            if (!data_.count(buf_it.first))
                data_[buf_it.first] = buf_it.second;
            else
            {
                data_.at(buf_it.first)->seizeBuffer(*buf_it.second.get());

                // sort by tod
                assert (metaVariable(DBObject::meta_var_tod_id_.name()).existsIn(buf_it.first));

                DBOVariable& tod_var = metaVariable(DBObject::meta_var_tod_id_.name()).getFor(buf_it.first);

                Property tod_prop {tod_var.name(), tod_var.dataType()};

                assert (data_.at(buf_it.first)->hasProperty(tod_prop));

                data_.at(buf_it.first)->sortByProperty(tod_prop);
            }
        }
    }

    insert_data_.clear();

    if (COMPASS::instance().liveMode()) // do tod cleanup
    {
        unsigned int buffer_size;

//        bool max_time_set = false;
//        float min_tod_found, max_tod_found;

        float max_time = secondsSinceMidnighUTC();

//        for (auto& buf_it : data_)
//        {
//            assert (metaVariable(DBObject::meta_var_tod_id_.name()).existsIn(buf_it.first));

//            DBOVariable& tod_var = metaVariable(DBObject::meta_var_tod_id_.name()).getFor(buf_it.first);

//            Property tod_prop {tod_var.name(), tod_var.dataType()};

//            assert (data_.at(buf_it.first)->hasProperty(tod_prop));

//            NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

//            auto minmax = tod_vec.minMaxValues();
//            assert (get<0>(minmax)); // there is minmax

//            if (max_time_set)
//            {
//                min_tod_found = min(min_tod_found, get<1>(minmax));
//                max_tod_found = max(max_tod_found, get<2>(minmax));
//            }
//            else
//            {
//                min_tod_found = get<1>(minmax);
//                max_tod_found = get<2>(minmax);
//                max_time_set = true;
//            }
//        }

//        if (max_time_set) // cut to size
//        {
            float min_tod = max_time - 300.0; // max - 5min
            assert (min_tod > 0); // does not work for midnight crossings

            logdbg << "DBObjectManager: finishInserting: min_tod " << String::timeStringFromDouble(min_tod)
                   //<< " data min " << String::timeStringFromDouble(min_tod_found)
                   << " data max " << String::timeStringFromDouble(max_time);
                   //<< " utc " << String::timeStringFromDouble(secondsSinceMidnighUTC());

//            if (min_tod > min_tod_found) // cut indexes
//            {
                for (auto& buf_it : data_)
                {
                    buffer_size = buf_it.second->size();

                    assert (metaVariable(DBObject::meta_var_tod_id_.name()).existsIn(buf_it.first));

                    DBOVariable& tod_var = metaVariable(DBObject::meta_var_tod_id_.name()).getFor(buf_it.first);

                    Property tod_prop {tod_var.name(), tod_var.dataType()};

                    assert (data_.at(buf_it.first)->hasProperty(tod_prop));

                    NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

                    unsigned int index=0;

                    for (; index < buffer_size; ++index)
                    {
                        if (!tod_vec.isNull(index) && tod_vec.get(index) > min_tod)
                        {
                            logdbg << "DBObjectManager: finishInserting: found " << buf_it.first
                                   << " cutoff tod index " << index
                                   << " tod " << String::timeStringFromDouble(tod_vec.get(index));
                            break;
                        }
                    }

                    if (index) // index found
                    {
                        index--; // cut at previous

                        logdbg << "DBObjectManager: finishInserting: cutting " << buf_it.first
                               << " up to index " << index
                               << " total size " << buffer_size;
                        assert (index < buffer_size);
                        buf_it.second->cutUpToIndex(index);
                    }
                }
//            }
//        }
//        else
//            logwrn << "DBObjectManager: finishInserting: no viable time found in live mode";

        // remove empty buffers

        std::map<std::string, std::shared_ptr<Buffer>> tmp_data = data_;

        for (auto& buf_it : tmp_data)
            if (!buf_it.second->size())
                data_.erase(buf_it.first);

    }

    if (load_widget_)
        load_widget_->update();

    logdbg << "DBObjectManager: finishInserting: distributing data";
    emit loadedDataSignal(data_, true);

    boost::posix_time::time_duration time_diff = boost::posix_time::microsec_clock::local_time() - start_time;
    loginf << "DBObjectManager: finishInserting: processing took "
        << String::timeStringFromDouble(time_diff.total_milliseconds() / 1000.0, true);
}


unsigned int DBObjectManager::maxRecordNumber() const
{
    assert (has_max_rec_num_);
    return max_rec_num_;
}

void DBObjectManager::maxRecordNumber(unsigned int value)
{
    logdbg << "DBObjectManager: maxRecordNumber: " << value;

    max_rec_num_ = value;
    has_max_rec_num_ = true;
}

const std::map<std::string, std::shared_ptr<Buffer>>& DBObjectManager::data() const
{
    return data_;
}

bool DBObjectManager::insertInProgress() const
{
    return insert_in_progress_;
}

void DBObjectManager::loadMaxRecordNumber()
{
    assert (COMPASS::instance().interface().dbOpen());

    max_rec_num_ = 0;

    for (auto& obj_it : objects_)
    {
        if (obj_it.second->existsInDB())
            max_rec_num_ = max(COMPASS::instance().interface().getMaxRecordNumber(*obj_it.second), max_rec_num_);
    }

    has_max_rec_num_ = true;

    loginf << "DBObjectManager: loadMaxRecordNumber: " << max_rec_num_;
}

const std::vector<std::unique_ptr<DBContent::DBDataSource>>& DBObjectManager::dataSources() const
{
    return db_data_sources_;
}

std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>> DBObjectManager::getNetworkLines()
{
    //ds_id -> (ip, port)
    std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>> lines;

    string line_address;
    string ip;
    unsigned int port;

    for (auto& ds_it : config_data_sources_)
    {
        if (ds_it->info().contains("network_lines"))
        {
            json& network_lines = ds_it->info().at("network_lines");
            assert (network_lines.is_array());

            for (auto& line_it : network_lines.get<json::array_t>())  // iterate over array
            {
                assert (line_it.is_primitive());
                assert (line_it.is_string());

                line_address = line_it;

                ip = String::ipFromString(line_address);
                port = String::portFromString(line_address);

                lines[Number::dsIdFrom(ds_it->sac(), ds_it->sic())].push_back({ip, port});
            }
        }
    }

    for (auto& ds_it : db_data_sources_) // should be same
    {
        if (ds_it->info().contains("network_lines"))
        {
            json& network_lines = ds_it->info().at("network_lines");
            assert (network_lines.is_array());

            for (auto& line_it : network_lines.get<json::array_t>())  // iterate over array
            {
                assert (line_it.is_primitive());
                assert (line_it.is_string());

                line_address = line_it;

                ip = String::ipFromString(line_address);
                port = String::portFromString(line_address);

                lines[Number::dsIdFrom(ds_it->sac(), ds_it->sic())].push_back({ip, port});
            }
        }
    }

    return lines;
}



MetaDBOVariableConfigurationDialog* DBObjectManager::metaVariableConfigdialog()
{
    if (!meta_cfg_dialog_)
    {
        meta_cfg_dialog_.reset(new MetaDBOVariableConfigurationDialog(*this));

        connect(meta_cfg_dialog_.get(), &MetaDBOVariableConfigurationDialog::okSignal,
                this, &DBObjectManager::metaDialogOKSlot);
    }

    assert(meta_cfg_dialog_);
    return meta_cfg_dialog_.get();
}
