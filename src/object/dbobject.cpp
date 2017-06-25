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

/*
 * DBObject.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: sk
 */

#include <algorithm>

#include "dbtable.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbobject.h"
#include "dbobjectwidget.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
//#include "StructureDescriptionManager.h"
#include "propertylist.h"
#include "metadbtable.h"
//#include "ActiveSourcesObserver.h"
#include "dboreaddbjob.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "workerthreadmanager.h"
#include "dbtableinfo.h"

/**
 * Registers parameters, creates sub configurables
 */
DBObject::DBObject(std::string class_id, std::string instance_id, Configurable *parent)
    : Configurable (class_id, instance_id, parent), is_loadable_(false), count_(0), current_meta_table_(nullptr), variables_checked_(false),
      has_active_data_sources_info_(false), widget_(nullptr)
{
    registerParameter ("name" , &name_, "Undefined");
    registerParameter ("info" , &info_, "");
    //registerParameter ("is_meta" , &is_meta_, false);

    createSubConfigurables ();

    logdbg  << "DBObject: constructor: created with instance_id " << instance_id_ << " name " << name_;
}

/**
 * Deletes data source definitions, schema & meta table definitions and variables
 */
DBObject::~DBObject()
{
    current_meta_table_ = nullptr;

    for (auto it = data_source_definitions_.begin(); it != data_source_definitions_.end(); it++)
        delete it->second;
    data_source_definitions_.clear();

    for (auto it = meta_table_definitions_.begin(); it != meta_table_definitions_.end(); it++)
        delete *it;
    meta_table_definitions_.clear();

    for (auto it = variables_.begin(); it !=  variables_.end(); it++)
        delete it->second;
    variables_.clear();

    if (widget_)
    {
        delete widget_;
        widget_=nullptr;
    }
}

/**
 * Can generate DBOVariables, DBOSchemaMetaTableDefinitions and DBODataSourceDefinitions.
 */
void DBObject::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "DBObject: generateSubConfigurable: generating variable " << instance_id;
    if (class_id.compare ("DBOVariable") == 0)
    {
        DBOVariable *variable = new DBOVariable (class_id, instance_id, this);
        assert (variables_.find (variable->name()) == variables_.end());
        variables_.insert (std::pair <std::string, DBOVariable*> (variable->name(), variable));
    }
    else if (class_id.compare ("DBOSchemaMetaTableDefinition") == 0)
    {
        DBOSchemaMetaTableDefinition *def = new DBOSchemaMetaTableDefinition (class_id, instance_id, this);
        meta_table_definitions_.push_back (def);

        logdbg  << "DBObject "<< name() << ": generateSubConfigurable: schema " << def->schema() << " meta " << def->metaTable();

        assert (meta_tables_.find (def->schema()) == meta_tables_.end());
        meta_tables_[def->schema()] = def->metaTable();
    }
    else if (class_id.compare ("DBODataSourceDefinition") == 0)
    {
        DBODataSourceDefinition *def = new DBODataSourceDefinition (class_id, instance_id, this);
        assert (data_source_definitions_.find(def->schema()) == data_source_definitions_.end());

        data_source_definitions_.insert (std::pair<std::string, DBODataSourceDefinition*> (def->schema(), def));
    }
    else
        throw std::runtime_error ("DBObject: generateSubConfigurable: unknown class_id "+class_id );
}

void DBObject::checkSubConfigurables ()
{
    //nothing to see here
}

bool DBObject::hasVariable (const std::string &id) const
{
    //  if (!variables_checked_)
    //    checkVariables ();

    return (variables_.find (id) != variables_.end());
}

DBOVariable &DBObject::variable (std::string variable_id)
{
    //  if (!variables_checked_)
    //    checkVariables ();

    assert (hasVariable (variable_id));
    return *variables_.at(variable_id);
}

void DBObject::deleteVariable (std::string id)
{
    //  if (!variables_checked_)
    //    checkVariables ();

    assert (hasVariable (id));
    //std::map<std::string, DBOVariable*>::iterator it;
    //it = variables_.find (id);
    DBOVariable *variable = variables_[id];
    variables_.erase(variables_.find (id));
    assert (!hasVariable (id));
    delete variable;
}

const std::map<std::string, DBOVariable*> &DBObject::variables () const
{
    //  if (!variables_checked_)
    //    checkVariables ();

    return variables_;
}

/**
 * Iterates through all variables, and checks if the current meta table has a variable with the same name, if so it adds
 * the variable to the returned list.
 */
//std::vector <DBOVariable*> DBObject::getVariablesForTable (std::string table)
//{
////  if (!variables_checked_)
////    checkVariables ();

//  std::vector <DBOVariable*> variables;

//  std::map<std::string, DBOVariable*>::iterator it;

//  for (it = variables_.begin(); it != variables_.end(); it++)
//  {
//    if (getCurrentMetaTable()->getTableDBNameForVariable(it->second->getCurrentVariableName()).compare (table) != 0)
//      continue;
//    variables.push_back (it->second);
//  }

//  return variables;
//}

const std::string &DBObject::metaTable (const std::string &schema) const
{
    assert (meta_tables_.find(schema) != meta_tables_.end());
    return meta_tables_.at(schema);
}

/**
 * Returns if the current schema name exists in the data source definitions
 */
bool DBObject::hasCurrentDataSource () const
{
    if (!ATSDB::instance().schemaManager().hasCurrentSchema())
        return false;

    return (data_source_definitions_.find(ATSDB::instance().schemaManager().getCurrentSchema().name()) != data_source_definitions_.end());
}

const DBODataSourceDefinition &DBObject::currentDataSource () const
{
    assert (hasCurrentDataSource());
    return *data_source_definitions_.at(ATSDB::instance().schemaManager().getCurrentSchema().name());
}

/**
 * Returns true if current_meta_table_ is not null, else gets the current schema, checks and get the meta_table_ entry for
 * the current schema, and returns if the meta table for the current schema exists in the current schema.
 */
bool DBObject::hasCurrentMetaTable () const
{
    return current_meta_table_ != nullptr;
}

/**
 * If current_meta_table_ is not set, it is set be getting the current schema, and getting the current meta table from
 * the schema by its identifier. Then current_meta_table_ is returned.
 */
const MetaDBTable &DBObject::currentMetaTable () const
{
    assert (current_meta_table_);
    return *current_meta_table_;
}


void DBObject::checkVariables ()
{
    assert (hasCurrentMetaTable());

    for (auto it : variables_)
    {
        bool found = current_meta_table_->hasColumn(it.first);

        if (!found)
        {
            logwrn  << "DBObject: checkVariables: erasing non-existing variable '"<<it.first<<"'";
            //      delete it->second;
            variables_.erase(it.first);
        }
    }
    variables_checked_=true;
}

//void DBObject::addActiveSourcesObserver (ActiveSourcesObserver *observer)
//{
//    assert (find (active_sources_observers_.begin(), active_sources_observers_.end(), observer) ==
//            active_sources_observers_.end());
//    active_sources_observers_.push_back (observer);
//}

//void DBObject::removeActiveSourcesObserver (ActiveSourcesObserver *observer)
//{
//    assert (find (active_sources_observers_.begin(), active_sources_observers_.end(), observer) !=
//            active_sources_observers_.end());
//    active_sources_observers_.erase (find (active_sources_observers_.begin(), active_sources_observers_.end(), observer));
//}

//void DBObject::notifyActiveDataSourcesObservers ()
//{
//    std::vector <ActiveSourcesObserver *>::iterator it;
//    for (it=active_sources_observers_.begin(); it != active_sources_observers_.end(); it++)
//        (*it)->notifyActiveSources ();
//}

//bool DBObject::hasActiveDataSourcesInfo ()
//{
//    return ATSDB::getInstance().hasActiveDataSourcesInfo(dbo_type_);
//}

//void DBObject::buildActiveDataSourcesInfo ()
//{
//    assert (hasCurrentDataSource());
//    assert (!hasActiveDataSourcesInfo());
//    ATSDB::getInstance().buildActiveDataSourcesInfo(dbo_type_);
//}

//void DBObject::setActiveDataSources (std::set<int> active_data_sources)
//{
//    assert (hasCurrentDataSource());
//    active_data_sources_= active_data_sources;
//    notifyActiveDataSourcesObservers();
//}

/**
 * Bit of a hack, explanation at DBOVariable::registerAsParent ().
 */
//void DBObject::registerParentVariables ()
//{
//    logdbg << "DBObject: registerParentVariables";
//    assert (isMeta());
//    std::map<std::string, DBOVariable*>::iterator it;

//    for (it = variables_.begin(); it != variables_.end(); it++)
//    {
//        assert (it->second->isMetaVariable());
//        it->second->registerAsParent();
//    }
//}

/**
 * Bit of a hack, explanation at DBOVariable::registerAsParent ().
 */
//void DBObject::unregisterParentVariables ()
//{
//    logdbg << "DBObject: unregisterParentVariables";
//    assert (isMeta());
//    std::map<std::string, DBOVariable*>::iterator it;

//    for (it = variables_.begin(); it != variables_.end(); it++)
//    {
//        assert (it->second->isMetaVariable());
//        it->second->unregisterAsParent();
//    }
//}

DBObjectWidget *DBObject::widget ()
{
    if (!widget_)
    {
        widget_ = new DBObjectWidget (this, ATSDB::instance().schemaManager());
    }

    assert (widget_);
    return widget_;
}

void DBObject::schemaChangedSlot ()
{
    if (ATSDB::instance().schemaManager().hasCurrentSchema())
    {
        DBSchema &schema = ATSDB::instance().schemaManager().getCurrentSchema();
        assert (meta_tables_.find(schema.name()) != meta_tables_.end());
        std::string meta_table_name = meta_tables_ .at(schema.name());
        assert (schema.hasMetaTable (meta_table_name));
        current_meta_table_ = &schema.metaTable (meta_table_name);
    }
    else
        current_meta_table_ = nullptr;
}

void DBObject::load ()
{
    assert (is_loadable_);

    assert (!read_job_);
    read_job_data_.clear();

    DBOVariableSet read_list;

    loginf << "DBInterface: testReading: adding all variables";
    read_list.add(variables_.at("REC_NUM"));
    read_list.add(variables_.at("TOD"));

    std::string custom_filter_clause;

    DBOReadDBJob *read_job = new DBOReadDBJob (ATSDB::instance().interface(), *this, read_list, custom_filter_clause, nullptr, false);

    read_job_ = std::shared_ptr<DBOReadDBJob> (read_job);
    connect (read_job, SIGNAL(intermediateSignal(std::shared_ptr<Buffer>)), this, SLOT(readJobIntermediateSlot(std::shared_ptr<Buffer>)));
    connect (read_job, SIGNAL(obsoleteSignal()), this, SLOT(readJobObsoleteSlot()));
    connect (read_job, SIGNAL(doneSignal()), this, SLOT(readJobDoneSlot()));

    WorkerThreadManager::instance().addJob(read_job_);
}

void DBObject::readJobIntermediateSlot (std::shared_ptr<Buffer> buffer)
{
    loginf << "DBObject: " << name_ << " readJobIntermediateSlot";

    DBOReadDBJob *sender = dynamic_cast <DBOReadDBJob*> (QObject::sender());
    assert (sender);
    assert (sender == read_job_.get());
    read_job_data_.push_back(buffer);
}

void DBObject::readJobObsoleteSlot ()
{
    loginf << "DBObject: " << name_ << " readJobObsoleteSlot";
    read_job_ = nullptr;
    read_job_data_.clear();
}

void DBObject::readJobDoneSlot()
{
    loginf << "DBObject: " << name_ << " readJobDoneSlot";
    read_job_ = nullptr;
}

void DBObject::databaseOpenedSlot ()
{
    loginf << "DBObject: " << name_ << " databaseOpenedSlot";

    assert (current_meta_table_);
    std::string table_name = current_meta_table_->mainTableName();

    is_loadable_ = ATSDB::instance().interface().tableInfo().count(table_name) > 0;

    if (is_loadable_)
        count_ = ATSDB::instance().interface().count (table_name);

    loginf << "DBObject: " << name_ << " databaseOpenedSlot: loadable " << is_loadable_ << " count " << count_;
}

bool DBObject::isLoading ()
{
    return read_job_ == nullptr;
}

bool DBObject::wasLoadingPerformed ()
{
    if (isLoading())
        return false;
    else
        return read_job_data_.size() > 0;

}

bool DBObject::hasData ()
{
    return count_ > 0;
}

size_t DBObject::count ()
{
    return count_;
}
