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
#include "viewmanager.h"
#include "jobmanager.h"
#include "evaluationmanager.h"
#include "filtermanager.h"
#include "util/number.h"
#include "metadbovariableconfigurationdialog.h"

#include <QApplication>
#include <QMessageBox>

#include <algorithm>

using namespace std;
using namespace Utils;
using namespace DBContent;

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
}

DBObjectManager::~DBObjectManager()
{
    for (auto it : objects_)
        delete it.second;
    objects_.clear();

    for (auto it : meta_variables_)
        delete it.second;
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

        connect(object, &DBObject::loadingDoneSignal, this, &DBObjectManager::loadingDoneSlot);
        // TODO what if generation after db opening?
    }
    else if (class_id.compare("MetaDBOVariable") == 0)
    {
        MetaDBOVariable* meta_var = new MetaDBOVariable(class_id, instance_id, this);
        logdbg << "DBObjectManager: generateSubConfigurable: adding meta var type "
               << meta_var->name();
        assert(meta_variables_.find(meta_var->name()) == meta_variables_.end());
        meta_variables_.insert(
                    std::pair<std::string, MetaDBOVariable*>(meta_var->name(), meta_var));
    }
    else if (class_id.compare("DBContentConfigurationDataSource") == 0)
    {
        unique_ptr<ConfigurationDataSource> ds {new ConfigurationDataSource(class_id, instance_id, *this)};
        loginf << "DBObjectManager: generateSubConfigurable: adding config ds "
               << ds->name() << " sac/sic " <<  ds->sac() << "/" << ds->sic();

        assert (!hasConfigDataSource(ds->sac(), ds->sic()));
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
    return (meta_variables_.find(var_name) != meta_variables_.end());
}

MetaDBOVariable& DBObjectManager::metaVariable(const std::string& var_name)
{
    logdbg << "DBObjectManager: metaVariable: name " << var_name;

    assert(meta_variables_.find(var_name) != meta_variables_.end());

    return *meta_variables_.at(var_name);
}

void DBObjectManager::deleteMetaVariable(const std::string& var_name)
{
    logdbg << "DBObjectManager: deleteMetaVariable: name " << var_name;
    assert(existsMetaVariable(var_name));
    delete meta_variables_.at(var_name);
    meta_variables_.erase(var_name);
}

bool DBObjectManager::usedInMetaVariable(const DBOVariable& variable)
{
    for (auto& meta_it : meta_variables_)
        if (meta_it.second->uses(variable))
            return true;

    return false;
}

DBObjectManagerWidget* DBObjectManager::widget()
{
    if (!widget_)
    {
        widget_.reset(new DBObjectManagerWidget(*this));
        // connect (this, SIGNAL(databaseContentChangedSignal), widget_, SLOT(updateDBOsSlot));

        //        if (locked_)
        //            widget_->lock ();
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

void DBObjectManager::loadSlot()
{
    logdbg << "DBObjectManager: loadSlot";

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
        object.second->clearData();  // clear previous data

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

void DBObjectManager::quitLoading()
{
    loginf << "DBObjectManager: quitLoading";

    for (auto& object : objects_)
        object.second->quitLoading();

    load_in_progress_ = true;  // TODO
}

void DBObjectManager::updateSchemaInformationSlot()
{
    emit schemaChangedSignal();
}

void DBObjectManager::databaseOpenedSlot()
{
    loginf << "DBObjectManager: databaseOpenedSlot";

    loadDBDataSources();

    if (load_widget_)
        load_widget_->update();
}

void DBObjectManager::databaseContentChangedSlot()
{
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

    emit dbObjectsChangedSignal();

    loginf << "DBObjectManager: databaseContentChangedSlot: done";
}

void DBObjectManager::loadingDoneSlot(DBObject& object)
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
    loginf << "DBObjectManager: loadingDoneSlot: all done";
    load_in_progress_ = false;

    COMPASS::instance().viewManager().doViewPointAfterLoad();

    emit allLoadingDoneSignal();

    if (load_widget_)
        load_widget_->loadingDone();

    QApplication::restoreOverrideCursor();
}

bool DBObjectManager::hasConfigDataSource (unsigned int sac, unsigned int sic)
{
    return find_if(config_data_sources_.begin(), config_data_sources_.end(),
                   [sac,sic] (const std::unique_ptr<DBContent::ConfigurationDataSource>& s)
    { return s->sac() == sac && s->sic() == sic; } ) != config_data_sources_.end();

}

DBContent::ConfigurationDataSource& DBObjectManager::getConfigDataSource (unsigned int sac, unsigned int sic)
{
    assert (hasConfigDataSource(sac, sic));

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

    return hasConfigDataSource(Number::sacFromDsId(ds_id), Number::sicFromDsId(ds_id));
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

    if (hasConfigDataSource(Number::sacFromDsId(ds_id), Number::sicFromDsId(ds_id)))
    {
        loginf << "DBObjectManager: addNewDataSource: ds_id " << ds_id << " from config";

        DBContent::ConfigurationDataSource& cfg_ds = getConfigDataSource(
                    Number::sacFromDsId(ds_id), Number::sicFromDsId(ds_id));

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


//void DBObjectManager::removeDependenciesForSchema(const std::string& schema_name)
//{
//    loginf << "DBObjectManager: removeDependenciesForSchema: " << schema_name;

//    loginf << "DBObjectManager: removeDependenciesForSchema: cleaning dbos";

//    for (auto obj_it = objects_.begin(); obj_it != objects_.end();)
//    {
//        const std::map<std::string, DBOSchemaMetaTableDefinition>& meta_tables =
//            obj_it->second->metaTables();

//        if (meta_tables.size() == 1 && meta_tables.count(schema_name) == 1)
//        {
//            loginf << "DBObjectManager: removeDependenciesForSchema: dbo " << obj_it->first
//                   << " exists only in schema to be removed, deleting";
//            delete obj_it->second;
//            objects_.erase(obj_it++);
//        }
//        else
//        {
//            obj_it->second->removeDependenciesForSchema(schema_name);
//            ++obj_it;
//        }
//    }
//    loginf << "DBObjectManager: removeDependenciesForSchema: cleaning meta variables";

//    for (auto meta_it = meta_variables_.begin(); meta_it != meta_variables_.end();)
//    {
//        meta_it->second->removeOutdatedVariables();

//        if (!meta_it->second->hasVariables())
//        {
//            loginf << "DBObjectManager: removeDependenciesForSchema: removing meta var "
//                   << meta_it->first;
//            delete meta_it->second;
//            meta_variables_.erase(meta_it++);
//        }
//        else
//            ++meta_it;
//    }

//    if (widget_)
//    {
//        widget_->updateDBOsSlot();
//        widget_->updateMetaVariablesSlot();
//    }

//    emit updateSchemaInformationSlot();
//}

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

bool DBObjectManager::loadInProgress() const { return load_in_progress_; }

const std::vector<std::unique_ptr<DBContent::DBDataSource>>& DBObjectManager::dataSources() const
{
    return db_data_sources_;
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
