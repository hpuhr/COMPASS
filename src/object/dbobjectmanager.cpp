/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "atsdb.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "metadbovariable.h"
#include "dbovariableset.h"
#include "dbobjectmanager.h"
#include "logger.h"
#include "dbobjectmanagerwidget.h"
#include "dbobjectmanagerloadwidget.h"
//#include "structureDescriptionManager.h"
#include "stringconv.h"
#include "viewmanager.h"

using namespace Utils::String;

/**
 * Creates sub-configurables.
 */
DBObjectManager::DBObjectManager(const std::string& class_id, const std::string& instance_id, ATSDB* atsdb)
: Configurable (class_id, instance_id, atsdb, "db_object.xml")
{
    logdbg  << "DBObjectManager: constructor: creating subconfigurables";

    registerParameter("use_filters", &use_filters_, false);

    registerParameter("use_order", &use_order_, false);
    registerParameter("use_order_ascending", &use_order_ascending_, false);
    registerParameter("order_variable_dbo_name", &order_variable_dbo_name_, "");
    registerParameter("order_variable__name", &order_variable_name_, "");

    registerParameter("use_limit", &use_limit_, false);
    registerParameter("limit_min", &limit_min_, 0);
    registerParameter("limit_max", &limit_max_, 100000);

    createSubConfigurables ();

    lock();
}
/**
 * Deletes all DBOs.
 */
DBObjectManager::~DBObjectManager()
{
    for (auto it : objects_)
        delete it.second;
    objects_.clear();

    for (auto it : meta_variables_)
        delete it.second;
    meta_variables_.clear();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    if (load_widget_)
    {
        delete load_widget_;
        load_widget_ = nullptr;
    }
}

/**
 * Can create DBOs.
 */
void DBObjectManager::generateSubConfigurable (const std::string& class_id, const std::string& instance_id)
{
    logdbg  << "DBObjectManager: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
    if (class_id.compare ("DBObject") == 0)
    {
        DBObject* object = new DBObject (class_id, instance_id, this);
        logdbg  << "DBObjectManager: generateSubConfigurable: adding object type " << object->name();
        assert (objects_.find(object->name()) == objects_.end());
        objects_.insert(std::pair <std::string, DBObject*> (object->name(), object));
        connect (this, &DBObjectManager::schemaChangedSignal, object, &DBObject::schemaChangedSlot);
        connect (this, &DBObjectManager::databaseContentChangedSignal, object, &DBObject::databaseContentChangedSlot);
        connect (object, &DBObject::loadingDoneSignal, this, &DBObjectManager::loadingDoneSlot);
        // TODO what if generation after db opening?
    }
    else if (class_id.compare ("MetaDBOVariable") == 0)
    {
        MetaDBOVariable* meta_var = new MetaDBOVariable (class_id, instance_id, this);
        logdbg  << "DBObjectManager: generateSubConfigurable: adding meta var type " << meta_var->name();
        assert (meta_variables_.find(meta_var->name()) == meta_variables_.end());
        meta_variables_.insert(std::pair <std::string, MetaDBOVariable*> (meta_var->name(), meta_var));
    }
    else
        throw std::runtime_error ("DBObjectManager: generateSubConfigurable: unknown class_id "+class_id );
}

void DBObjectManager::checkSubConfigurables ()
{
    // nothing to do, must be defined in configuration
}

bool DBObjectManager::existsObject (const std::string& dbo_name)
{
    return (objects_.find(dbo_name) != objects_.end());
}

void DBObjectManager::schemaLockedSlot()
{
    loginf << "DBObjectManager: schemaLockedSlot";
    unlock ();
}

DBObject& DBObjectManager::object (const std::string& dbo_name)
{
    logdbg  << "DBObjectManager: object: name " << dbo_name;

    assert (objects_.find(dbo_name) != objects_.end());

    return *objects_.at(dbo_name);
}

void DBObjectManager::deleteObject (const std::string& dbo_name)
{
    logdbg  << "DBObjectManager: deleteObject: name " << dbo_name;
    assert (existsObject(dbo_name));
    delete objects_.at(dbo_name);
    objects_.erase(dbo_name);

    emit dbObjectsChangedSignal();
}

bool DBObjectManager::existsMetaVariable (const std::string& var_name)
{
    return (meta_variables_.find(var_name) != meta_variables_.end());
}

MetaDBOVariable& DBObjectManager::metaVariable (const std::string& var_name)
{
    logdbg  << "DBObjectManager: metaVariable: name " << var_name;

    assert (meta_variables_.find(var_name) != meta_variables_.end());

    return *meta_variables_.at(var_name);
}

void DBObjectManager::deleteMetaVariable (const std::string& var_name)
{
    logdbg  << "DBObjectManager: deleteMetaVariable: name " << var_name;
    assert (existsMetaVariable(var_name));
    delete meta_variables_.at(var_name);
    meta_variables_.erase(var_name);
}

DBObjectManagerWidget* DBObjectManager::widget()
{
    if (!widget_)
    {
        widget_ = new DBObjectManagerWidget (*this);
        //connect (this, SIGNAL(databaseContentChangedSignal), widget_, SLOT(updateDBOsSlot));

        if (locked_)
            widget_->lock ();
    }

    assert (widget_);
    return widget_;
}

DBObjectManagerLoadWidget* DBObjectManager::loadWidget()
{
    if (!load_widget_)
    {
        load_widget_ = new DBObjectManagerLoadWidget (*this);
    }

    assert (load_widget_);
    return load_widget_;
}

bool DBObjectManager::useLimit() const
{
    return use_limit_;
}

void DBObjectManager::useLimit(bool use_limit)
{
    use_limit_ = use_limit;
}

unsigned int DBObjectManager::limitMin() const
{
    return limit_min_;
}

void DBObjectManager::limitMin(unsigned int limit_min)
{
    limit_min_ = limit_min;
    loginf << "DBObjectManager: limitMin: " << limit_min_;
}

unsigned int DBObjectManager::limitMax() const
{
    return limit_max_;
}

void DBObjectManager::limitMax(unsigned int limit_max)
{
    limit_max_ = limit_max;
    loginf << "DBObjectManager: limitMax: " << limit_max_;
}

bool DBObjectManager::useFilters() const
{
    return use_filters_;
}

void DBObjectManager::useFilters(bool use_filters)
{
    use_filters_ = use_filters;
    loginf << "DBObjectManager: useFilters: " << use_filters_;
}

bool DBObjectManager::useOrder() const
{
    return use_order_;
}

void DBObjectManager::useOrder(bool use_order)
{
    use_order_ = use_order;
}

bool DBObjectManager::useOrderAscending() const
{
    return use_order_ascending_;
}

void DBObjectManager::useOrderAscending(bool use_order_ascending)
{
    use_order_ascending_ = use_order_ascending;
}

bool DBObjectManager::hasOrderVariable ()
{
    if (existsObject(order_variable_dbo_name_))
        if (object(order_variable_dbo_name_).hasVariable(order_variable_name_))
                return true;
    return false;
}

DBOVariable &DBObjectManager::orderVariable ()
{
    assert (hasOrderVariable());
    return object(order_variable_dbo_name_).variable(order_variable_name_);
}

void DBObjectManager::orderVariable(DBOVariable &variable)
{
    order_variable_dbo_name_=variable.dboName();
    order_variable_name_=variable.name();
}

bool DBObjectManager::hasOrderMetaVariable ()
{
    if (order_variable_dbo_name_ == META_OBJECT_NAME)
        return existsMetaVariable(order_variable_name_);

    return false;
}

MetaDBOVariable& DBObjectManager::orderMetaVariable ()
{
    assert (hasOrderMetaVariable());
    return metaVariable(order_variable_name_);
}

void DBObjectManager::orderMetaVariable(MetaDBOVariable &variable)
{
    order_variable_dbo_name_=META_OBJECT_NAME;
    order_variable_name_=variable.name();
}

void DBObjectManager::clearOrderVariable ()
{
    order_variable_dbo_name_="";
    order_variable_name_="";
}

void DBObjectManager::lock ()
{
    if (locked_)
        return;

    locked_ = true;

    for (auto& object_it : objects_)
        object_it.second->lock();

    for (auto& meta_it : meta_variables_)
        meta_it.second->lock();

    if (widget_)
        widget_->lock();
}

void DBObjectManager::unlock ()
{
    if (!locked_)
        return;

    locked_ = false;

    for (auto& object_it : objects_)
        object_it.second->unlock();

    for (auto& meta_it : meta_variables_)
        meta_it.second->unlock();


    if (widget_)
        widget_->unlock();
}

void DBObjectManager::loadSlot ()
{
    logdbg << "DBObjectManager: loadSlot";

    bool load_job_created = false;

    for (auto& object : objects_)
    {
        if (object.second->loadable() && object.second->loadingWanted())
        {
            loginf << "DBObjectManager: loadSlot: loading object " << object.first;
            DBOVariableSet read_set = ATSDB::instance().viewManager().getReadSet(object.first);

            if (read_set.getSize() == 0)
            {
                loginf << "DBObjectManager: loadSlot: skipping loading of object " << object.first
                       << " since an empty read list was detected";
                continue;
            }

            std::string limit_str = "";
            if (use_limit_)
            {
                limit_str = std::to_string(limit_min_)+","+std::to_string(limit_max_);
                logdbg << "DBObjectManager: loadSlot: use limit str " << limit_str;
            }

            DBOVariable *variable=nullptr;
            if (hasOrderVariable())
                variable = &orderVariable();
            if (hasOrderMetaVariable())
                variable = &orderMetaVariable().getFor(object.first);

            // load (DBOVariableSet &read_set, bool use_filters, bool use_order, DBOVariable *order_variable,
            // bool use_order_ascending, const std::string &limit_str="")
            object.second->load(read_set, use_filters_, use_order_, variable, use_order_ascending_, limit_str);

            load_job_created = true;
        }
    }
    emit loadingStartedSignal();

    if (!load_job_created)
    {
        if (load_widget_)
            load_widget_->loadingDone();
    }
}

void DBObjectManager::quitLoading ()
{
    loginf << "DBObjectManager: quitLoading";

    for (auto& object : objects_)
        object.second->quitLoading();
}

void DBObjectManager::updateSchemaInformationSlot ()
{
    emit schemaChangedSignal();
}

void DBObjectManager::databaseContentChangedSlot ()
{
    emit databaseContentChangedSignal();
}

void DBObjectManager::loadingDoneSlot (DBObject& object)
{
    bool done=true;

    for (auto& object_it : objects_)
        if (object_it.second->isLoading())
        {
            logdbg << "DBObjectManager: loadingDoneSlot: " << object_it.first << " still loading";
            done = false;
            break;
        }

    if (done)
    {
        loginf << "DBObjectManager: loadingDoneSlot: all done";
        emit allLoadingDoneSignal();

        if (load_widget_)
            load_widget_->loadingDone();
    }
    else
        logdbg << "DBObjectManager: loadingDoneSlot: not done";
}
