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

#include "compass.h"
#include "mainwindow.h"
#include "configurationmanager.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontentmanagerloadwidget.h"
#include "dbcontent/dbcontentmanagerwidget.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variableset.h"
#include "logger.h"
#include "dbcontent/variable/metavariable.h"
#include "stringconv.h"
#include "number.h"
#include "viewmanager.h"
#include "jobmanager.h"
#include "evaluationmanager.h"
#include "filtermanager.h"
#include "util/number.h"
#include "dbcontent/variable/metavariableconfigurationdialog.h"
#include "json.hpp"

#include <QApplication>
#include <QMessageBox>

#include <algorithm>
#include <chrono>

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace dbContent;

// move to somewhere else?
double secondsSinceMidnightUTC ()
{
    using namespace boost::posix_time;

    auto p_time = microsec_clock::universal_time (); // UTC.
    return (p_time.time_of_day().total_milliseconds() / 1000.0);

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

const std::vector<std::string> DBContentManager::data_source_types_ {"Radar", "MLAT", "ADSB", "Tracker", "RefTraj"};

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

    createSubConfigurables();

    for (auto& dbo_it : dbcontent_)
        dbo_it.second->checkLabelDefinitions();

    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>"); // for dbo read job
    // for signal about new data
    qRegisterMetaType<std::map<std::string, std::shared_ptr<Buffer>>>("std::map<std::string, std::shared_ptr<Buffer>>");
}

DBContentManager::~DBContentManager()
{
    data_.clear();

    for (auto it : dbcontent_)
        delete it.second;
    dbcontent_.clear();

    meta_variables_.clear();

    widget_ = nullptr;
    load_widget_ = nullptr;
}

void DBContentManager::generateSubConfigurable(const std::string& class_id,
                                              const std::string& instance_id)
{
    logdbg << "DBContentManager: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;
    if (class_id.compare("DBContent") == 0)
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
    else if (class_id.compare("ConfigurationDataSource") == 0)
    {
        unique_ptr<dbContent::ConfigurationDataSource> ds {
            new dbContent::ConfigurationDataSource(class_id, instance_id, *this)};
        loginf << "DBContentManager: generateSubConfigurable: adding config ds "
               << ds->name() << " sac/sic " <<  ds->sac() << "/" << ds->sic();

        assert (!hasConfigDataSource(Number::dsIdFrom(ds->sac(), ds->sic())));
        config_data_sources_.emplace_back(move(ds));
    }
    else
        throw std::runtime_error("DBContentManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void DBContentManager::checkSubConfigurables()
{
    // nothing to do, must be defined in configuration
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

DBContentManagerDataSourcesWidget* DBContentManager::loadWidget()
{
    if (!load_widget_)
    {
        load_widget_.reset(new DBContentManagerDataSourcesWidget(*this));
    }

    assert(load_widget_);
    return load_widget_.get();
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
    order_variable_dbcontent_name_ = variable.dboName();
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

bool DBContentManager::loadingWanted (const std::string& dbo_name)
{
    for (auto& ds_it : db_data_sources_)
    {
        if (dsTypeLoadingWanted(ds_it->dsType()) && ds_it->loadingWanted() && ds_it->hasNumInserted(dbo_name))
            return true;
    }

    return false;
}

bool DBContentManager::hasDSFilter (const std::string& dbo_name)
{
    for (auto& ds_it : db_data_sources_)
    {
        if (dsTypeLoadingWanted(ds_it->dsType()) && !ds_it->loadingWanted() && ds_it->hasNumInserted(dbo_name))
            return true;
    }

    return false;
}

std::vector<unsigned int> DBContentManager::unfilteredDS (const std::string& dbo_name)
{
    assert (hasDSFilter(dbo_name));

    std::vector<unsigned int> ds_ids;

    for (auto& ds_it : db_data_sources_)
    {
        if (dsTypeLoadingWanted(ds_it->dsType()) && ds_it->loadingWanted() && ds_it->hasNumInserted(dbo_name))
            ds_ids.push_back(ds_it->id());
    }

    assert (ds_ids.size());

    return ds_ids;
}

void DBContentManager::setLoadOnlyDataSources (std::set<unsigned int> ds_ids)
{
    loginf << "DBContentManager: setLoadOnlyDataSources";

    // deactivate all loading
    for (auto& ds_it : db_data_sources_)
        ds_it->loadingWanted(false);

    for (auto ds_id_it : ds_ids)
    {
        assert (hasDataSource(ds_id_it));
        dataSource(ds_id_it).loadingWanted(true);
    }

    if (load_widget_)
        load_widget_->update();

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

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();
    ViewManager& view_man = COMPASS::instance().viewManager();

    for (auto& object : dbcontent_)
    {
        loginf << "DBContentManager: loadSlot: object " << object.first
               << " loadable " << object.second->loadable()
               << " loading wanted " << loadingWanted(object.first);

        if (object.first == "RefTraj")
            continue;

        if (object.second->loadable() && loadingWanted(object.first))
        {
            loginf << "DBContentManager: loadSlot: loading object " << object.first;
            VariableSet read_set = view_man.getReadSet(object.first); // TODO add required vars for processing

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

void DBContentManager::addLoadedData(std::map<std::string, std::shared_ptr<Buffer>> data)
{
    loginf << "DBContentManager: addLoadedData";

    // newest data batch has been finalized, ready to be added

    // add buffers to data

    bool something_changed = false;

    for (auto& buf_it : data)
    {
        if (!buf_it.second->size())
        {
            logerr << "DBContentManager: addLoadedData: buffer dbo " << buf_it.first << " with 0 size";
            continue;
        }

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
        emit loadedDataSignal(data_, false);
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

    loadDBDataSources();
    loadMaxRecordNumber();

    if (COMPASS::instance().interface().hasProperty("associations_generated"))
    {
        assert(COMPASS::instance().interface().hasProperty("associations_id"));

        has_associations_ =
                COMPASS::instance().interface().getProperty("associations_generated") == "1";
        associations_id_ = COMPASS::instance().interface().getProperty("associations_id");
    }
    else
    {
        has_associations_ = false;
        associations_id_ = "";
    }

    for (auto& object : dbcontent_)
        object.second->databaseOpenedSlot();

    if (load_widget_)
        load_widget_->update();

    emit associationStatusChangedSignal();
}

void DBContentManager::databaseClosedSlot()
{
    db_data_sources_.clear();

    max_rec_num_ = 0;
    has_max_rec_num_ = false;

    has_associations_ = false;
    associations_id_ = "";

    for (auto& object : dbcontent_)
        object.second->databaseClosedSlot();

    if (load_widget_)
        load_widget_->update();

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
        finishLoading();
    else
        logdbg << "DBContentManager: loadingDoneSlot: not done";
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

bool DBContentManager::hasConfigDataSource (unsigned int ds_id)
{
    unsigned int sac = Number::sacFromDsId(ds_id);
    unsigned int sic = Number::sicFromDsId(ds_id);

    return find_if(config_data_sources_.begin(), config_data_sources_.end(),
                   [sac,sic] (const std::unique_ptr<dbContent::ConfigurationDataSource>& s)
    { return s->sac() == sac && s->sic() == sic; } ) != config_data_sources_.end();

}

dbContent::ConfigurationDataSource& DBContentManager::configDataSource (unsigned int ds_id)
{
    assert (hasConfigDataSource(ds_id));

    unsigned int sac = Number::sacFromDsId(ds_id);
    unsigned int sic = Number::sicFromDsId(ds_id);

    return *find_if(config_data_sources_.begin(), config_data_sources_.end(),
                    [sac,sic] (const std::unique_ptr<dbContent::ConfigurationDataSource>& s)
    { return s->sac() == sac && s->sic() == sic; } )->get();
}

void DBContentManager::loadDBDataSources()
{
    assert (!db_data_sources_.size());

    DBInterface& db_interface = COMPASS::instance().interface();

    if (db_interface.existsDataSourcesTable())
    {
        db_data_sources_ = db_interface.getDataSources();
        sortDBDataSources();
    }
}

void DBContentManager::sortDBDataSources()
{
    sort(db_data_sources_.begin(), db_data_sources_.end(),
         [](const std::unique_ptr<dbContent::DBDataSource>& a,
         const std::unique_ptr<dbContent::DBDataSource>& b) -> bool
    {
        return a->name() < b->name();
    });
}

void DBContentManager::saveDBDataSources()
{
    DBInterface& db_interface = COMPASS::instance().interface();

    assert(db_interface.dbOpen());
    db_interface.saveDataSources(db_data_sources_);
}

bool DBContentManager::canAddNewDataSourceFromConfig (unsigned int ds_id)
{
    if (hasDataSource(ds_id))
        return false;

    return hasConfigDataSource(ds_id);
}

bool DBContentManager::hasDataSource(unsigned int ds_id)
{
    return find_if(db_data_sources_.begin(), db_data_sources_.end(),
                   [ds_id] (const std::unique_ptr<dbContent::DBDataSource>& s)
    { return Number::dsIdFrom(s->sac(), s->sic()) == ds_id; } ) != db_data_sources_.end();
}

bool DBContentManager::hasDataSourcesOfDBContent(const std::string dbcontent_name)
{
    return find_if(db_data_sources_.begin(), db_data_sources_.end(),
                   [dbcontent_name] (const std::unique_ptr<dbContent::DBDataSource>& s)
    { return s->numInsertedMap().count(dbcontent_name) > 0; } ) != db_data_sources_.end();
}


void DBContentManager::addNewDataSource (unsigned int ds_id)
{
    loginf << "DBContentManager: addNewDataSource: ds_id " << ds_id;

    assert (!hasDataSource(ds_id));

    if (hasConfigDataSource(ds_id))
    {
        loginf << "DBContentManager: addNewDataSource: ds_id " << ds_id << " from config";

        dbContent::ConfigurationDataSource& cfg_ds = configDataSource(ds_id);

        db_data_sources_.emplace_back(move(cfg_ds.getAsNewDBDS()));
        sortDBDataSources();
    }
    else
    {
        loginf << "DBContentManager: addNewDataSource: ds_id " << ds_id << " create new";

        dbContent::DBDataSource* new_ds = new dbContent::DBDataSource();
        new_ds->id(ds_id);
        new_ds->sac(Number::sacFromDsId(ds_id));
        new_ds->sic(Number::sicFromDsId(ds_id));
        new_ds->name(to_string(ds_id));

        db_data_sources_.emplace_back(move(new_ds));
        sortDBDataSources();
    }

    assert (hasDataSource(ds_id));

    loginf << "DBContentManager: addNewDataSource: ds_id " << ds_id << " done";
}

dbContent::DBDataSource& DBContentManager::dataSource(unsigned int ds_id)
{
    assert (hasDataSource(ds_id));

    return *find_if(db_data_sources_.begin(), db_data_sources_.end(),
                    [ds_id] (const std::unique_ptr<dbContent::DBDataSource>& s)
    { return Number::dsIdFrom(s->sac(), s->sic()) == ds_id; } )->get();
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

    if (load_widget_)
        loadWidget()->update();
}

//void DBContentManager::setAssociationsByAll()
//{
////    COMPASS::instance().interface().setProperty("associations_generated", "1");
////    COMPASS::instance().interface().setProperty("associations_dbo", "");
////    COMPASS::instance().interface().setProperty("associations_ds", "");

////    has_associations_ = true;
////    associations_dbo_ = "";
////    associations_ds_ = "";

////    if (load_widget_)
////        loadWidget()->update();
//}

//void DBContentManager::removeAssociations()
//{
////    COMPASS::instance().interface().setProperty("associations_generated", "0");
////    COMPASS::instance().interface().setProperty("associations_dbo", "");
////    COMPASS::instance().interface().setProperty("associations_ds", "");

////    has_associations_ = false;
////    associations_dbo_ = "";
////    associations_ds_ = "";

////    for (auto& dbo_it : objects_)
////        dbo_it.second->clearAssociations();

////    if (load_widget_)
////        loadWidget()->update();
//}

//bool DBContentManager::hasAssociationsDataSource() const
//{
//    return associations_dbo_.size() && associations_ds_.size();
//}

std::string DBContentManager::associationsID() const { return associations_id_; }

//std::string DBContentManager::associationsDataSourceName() const { return associations_ds_; }

bool DBContentManager::isOtherDBObjectPostProcessing(DBContent& object)
{
    for (auto& dbo_it : dbcontent_)
        if (dbo_it.second != &object && dbo_it.second->isPostProcessing())
            return true;

    return false;
}

bool DBContentManager::loadInProgress() const
{
    return load_in_progress_;
}

void DBContentManager::clearData()
{
    data_.clear();
}

void DBContentManager::insertData(std::map<std::string, std::shared_ptr<Buffer>> data)
{
    loginf << "DBContentManager: insertData";

    assert (!load_in_progress_);
    assert (!insert_in_progress_);
    assert (!insert_data_.size());

    insert_in_progress_ = true;

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
    loginf << "DBContentManager: finishInserting: all done";

    insert_in_progress_ = false;

    emit insertDoneSignal();

    // add inserted to loaded data

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();
    assert (existsMetaVariable(DBContent::meta_var_tod_.name()));

    if (COMPASS::instance().appMode() == AppMode::LiveRunning) // do tod cleanup
    {
        addInsertedDataToChache();

        cutCachedData();
        filterDataSources();

        logdbg << "DBContentManager: finishInserting: distributing data";

        if (data_.size())
            emit loadedDataSignal(data_, true);
    }
    else
        insert_data_.clear();

    if (load_widget_)
        load_widget_->update();

    boost::posix_time::time_duration time_diff = boost::posix_time::microsec_clock::local_time() - start_time;
    loginf << "DBContentManager: finishInserting: processing took "
        << String::timeStringFromDouble(time_diff.total_milliseconds() / 1000.0, true);
}

void DBContentManager::addInsertedDataToChache()
{
    loginf << "DBContentManager: addInsertedDataToChache";

    for (auto& buf_it : insert_data_)
    {

        VariableSet read_set = COMPASS::instance().viewManager().getReadSet(buf_it.first);
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
            assert (metaVariable(DBContent::meta_var_tod_.name()).existsIn(buf_it.first));

            Variable& tod_var = metaVariable(DBContent::meta_var_tod_.name()).getFor(buf_it.first);

            Property tod_prop {tod_var.name(), tod_var.dataType()};

            assert (data_.at(buf_it.first)->hasProperty(tod_prop));

            data_.at(buf_it.first)->sortByProperty(tod_prop);
        }
    }

    insert_data_.clear();
}

void DBContentManager::filterDataSources()
{
    set<unsigned int> wanted_data_sources;

    for (auto& ds_it : db_data_sources_)
    {
        if (dsTypeLoadingWanted(ds_it->dsType()) && ds_it->loadingWanted())
           wanted_data_sources.insert(ds_it->id());
    }

    unsigned int buffer_size;
    vector<size_t> indexes_to_remove;

    for (auto& buf_it : data_)
    {
        assert (metaVariable(DBContent::meta_var_datasource_id_.name()).existsIn(buf_it.first));

        Variable& ds_id_var = metaVariable(DBContent::meta_var_datasource_id_.name()).getFor(buf_it.first);

        Property ds_id_prop {ds_id_var.name(), ds_id_var.dataType()};
        assert (buf_it.second->hasProperty(ds_id_prop));

        NullableVector<unsigned int>& ds_id_vec = buf_it.second->get<unsigned int>(ds_id_var.name());

        buffer_size = buf_it.second->size();

        indexes_to_remove.clear();
        //assert (ds_id_vec.isNeverNull()); TODO why asserts?

        for (unsigned int index=0; index < buffer_size; ++index)
        {
            assert (!ds_id_vec.isNull(index));

            if (!wanted_data_sources.count(ds_id_vec.get(index)))
                indexes_to_remove.push_back(index);
        }

        loginf << "DBContentManager: filterDataSources: in " << buf_it.first << " remove "
               << indexes_to_remove.size() << " of " << buffer_size;

        buf_it.second->removeIndexes(indexes_to_remove);
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
    float min_tod_found, max_tod_found;

    float max_time = secondsSinceMidnightUTC();

    float time_offset = COMPASS::instance().mainWindow().importASTERIXFromNetworkTimeOffset();

    loginf << "DBContentManager: cutCachedData: max_time " << String::timeStringFromDouble(max_time)
           << " time offset " << String::timeStringFromDouble(time_offset);

    max_time += time_offset;

    for (auto& buf_it : data_)
    {
        assert (metaVariable(DBContent::meta_var_tod_.name()).existsIn(buf_it.first));

        Variable& tod_var = metaVariable(DBContent::meta_var_tod_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};

        if(buf_it.second->hasProperty(tod_prop))
        {
            NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

            auto minmax = tod_vec.minMaxValues();
            assert (get<0>(minmax)); // there is minmax

            if (max_time_set)
            {
                min_tod_found = min(min_tod_found, get<1>(minmax));
                max_tod_found = max(max_tod_found, get<2>(minmax));
            }
            else
            {
                min_tod_found = get<1>(minmax);
                max_tod_found = get<2>(minmax);
                max_time_set = true;
            }
        }
        else
            logwrn << "DBContentManager: cutCachedData: buffer " << buf_it.first << " has not tod for min/max";
    }

    if (max_time_set)
        loginf << "DBContentManager: cutCachedData: data time min " << String::timeStringFromDouble(min_tod_found)
               << " max " << String::timeStringFromDouble(max_tod_found);

    float min_tod = max_time - 300.0; // max - 5min
    assert (min_tod > 0); // does not work for midnight crossings

    loginf << "DBContentManager: cutCachedData: min_tod " << String::timeStringFromDouble(min_tod)
              //<< " data min " << String::timeStringFromDouble(min_tod_found)
           << " data max " << String::timeStringFromDouble(max_time);
    //<< " utc " << String::timeStringFromDouble(secondsSinceMidnighUTC());

    for (auto& buf_it : data_)
    {
        buffer_size = buf_it.second->size();

        assert (metaVariable(DBContent::meta_var_tod_.name()).existsIn(buf_it.first));

        Variable& tod_var = metaVariable(DBContent::meta_var_tod_.name()).getFor(buf_it.first);

        Property tod_prop {tod_var.name(), tod_var.dataType()};

        if (buf_it.second->hasProperty(tod_prop))
        {
            NullableVector<float>& tod_vec = buf_it.second->get<float>(tod_var.name());

            unsigned int index=0;

            for (; index < buffer_size; ++index)
            {
                if (!tod_vec.isNull(index) && tod_vec.get(index) > min_tod)
                {
                    logdbg << "DBContentManager: cutCachedData: found " << buf_it.first
                           << " cutoff tod index " << index
                           << " tod " << String::timeStringFromDouble(tod_vec.get(index));
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

const std::map<std::string, std::shared_ptr<Buffer>>& DBContentManager::data() const
{
    return data_;
}

void DBContentManager::dsTypeLoadingWanted (const std::string& ds_type, bool wanted)
{
    loginf << "DBContentManager: dsTypeLoadingWanted: ds_type " << ds_type << " wanted " << wanted;

    ds_type_loading_wanted_[ds_type] = wanted;
}

bool DBContentManager::dsTypeLoadingWanted (const std::string& ds_type)
{
    if (!ds_type_loading_wanted_.count(ds_type))
        return true;

    return ds_type_loading_wanted_.at(ds_type);
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

const std::vector<std::unique_ptr<dbContent::DBDataSource>>& DBContentManager::dataSources() const
{
    return db_data_sources_;
}

std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>> DBContentManager::getNetworkLines()
{
    //ds_id -> (ip, port)
    std::map<unsigned int, std::vector <std::pair<std::string, unsigned int>>> lines;

    string line_address;
    string ip;
    unsigned int port;

    set<string> existing_lines; // to check

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

                if (existing_lines.count(ip+":"+to_string(port)))
                {
                    logwrn << "DBContentManager: getNetworkLines: source " << ds_it->name()
                           << " line " << ip << ":" << port
                           << " already in use";
                }
                else
                    lines[Number::dsIdFrom(ds_it->sac(), ds_it->sic())].push_back({ip, port});

                existing_lines.insert(ip+":"+to_string(port));

                //break; // TODO only parse one for now
            }
        }
    }

//    for (auto& ds_it : db_data_sources_) // should be same
//    {
//        if (ds_it->info().contains("network_lines"))
//        {
//            json& network_lines = ds_it->info().at("network_lines");
//            assert (network_lines.is_array());

//            for (auto& line_it : network_lines.get<json::array_t>())  // iterate over array
//            {
//                assert (line_it.is_primitive());
//                assert (line_it.is_string());

//                line_address = line_it;

//                ip = String::ipFromString(line_address);
//                port = String::portFromString(line_address);

//                lines[Number::dsIdFrom(ds_it->sac(), ds_it->sic())].push_back({ip, port});
//            }
//        }
//    }

    return lines;
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
