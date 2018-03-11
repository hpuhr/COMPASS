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

#include <algorithm>

#include "dbtable.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbobject.h"
#include "dbobjectwidget.h"
#include "dbobjectinfowidget.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "buffer.h"
#include "filtermanager.h"
//#include "StructureDescriptionManager.h"
#include "propertylist.h"
#include "metadbtable.h"
#include "dboreaddbjob.h"
#include "finalizedboreadjob.h"
#include "atsdb.h"
#include "dbinterface.h"
#include "jobmanager.h"
#include "dbtableinfo.h"
#include "dbolabeldefinition.h"
#include "dbolabeldefinitionwidget.h"
#include "data.h"
#include "updatebufferdbjob.h"

/**
 * Registers parameters, creates sub configurables
 */
DBObject::DBObject(const std::string& class_id, const std::string& instance_id, Configurable* parent)
    : Configurable (class_id, instance_id, parent)
{
    registerParameter ("name" , &name_, "Undefined");
    registerParameter ("info" , &info_, "");

    createSubConfigurables ();

    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>");

    logdbg  << "DBObject: constructor: created with instance_id " << instance_id_ << " name " << name_;
}

/**
 * Deletes data source definitions, schema & meta table definitions and variables
 */
DBObject::~DBObject()
{
    current_meta_table_ = nullptr;

    data_sources_.clear();

    if (label_definition_)
    {
        delete label_definition_;
        label_definition_=nullptr;
    }

    for (auto it : data_source_definitions_)
        delete it.second;
    data_source_definitions_.clear();

    for (auto it : meta_table_definitions_)
        delete it;
    meta_table_definitions_.clear();

    for (auto it : variables_)
        delete it.second;
    variables_.clear();

    if (widget_)
    {
        delete widget_;
        widget_=nullptr;
    }

    if (info_widget_)
    {
        delete info_widget_;
        info_widget_=nullptr;
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
        DBOVariable* variable = new DBOVariable (class_id, instance_id, this);
        assert (variables_.find (variable->name()) == variables_.end());
        variables_.insert (std::pair <std::string, DBOVariable*> (variable->name(), variable));
    }
    else if (class_id.compare ("DBOSchemaMetaTableDefinition") == 0)
    {
        DBOSchemaMetaTableDefinition* def = new DBOSchemaMetaTableDefinition (class_id, instance_id, this);
        meta_table_definitions_.push_back (def);

        logdbg  << "DBObject "<< name() << ": generateSubConfigurable: schema " << def->schema() << " meta "
                << def->metaTable();

        assert (meta_tables_.find (def->schema()) == meta_tables_.end());
        meta_tables_[def->schema()] = def->metaTable();
    }
    else if (class_id.compare ("DBODataSourceDefinition") == 0)
    {
        DBODataSourceDefinition* def = new DBODataSourceDefinition (class_id, instance_id, this);
        assert (data_source_definitions_.find(def->schema()) == data_source_definitions_.end());
        connect (def, SIGNAL(definitionChangedSignal()), this, SLOT(dataSourceDefinitionChanged()));

        data_source_definitions_.insert (std::pair<std::string, DBODataSourceDefinition*> (def->schema(), def));
    }
    else if (class_id.compare ("DBOLabelDefinition") == 0)
    {
        DBOLabelDefinition* def = new DBOLabelDefinition (class_id, instance_id, this);
        assert (!label_definition_);
        //connect (def, SIGNAL(definitionChangedSignal()), this, SLOT(dataSourceDefinitionChanged()));
        label_definition_ = def;
    }
    else
        throw std::runtime_error ("DBObject: generateSubConfigurable: unknown class_id "+class_id );
}

void DBObject::checkSubConfigurables ()
{
    //nothing to see here

    if (!label_definition_)
    {
        generateSubConfigurable ("DBOLabelDefinition", "DBOLabelDefinition0");
        assert (label_definition_);
    }
}

bool DBObject::hasVariable (const std::string& name) const
{
    return (variables_.find (name) != variables_.end());
}

DBOVariable& DBObject::variable (const std::string& name) const
{
    assert (hasVariable (name));
    return *variables_.at(name);
}

void DBObject::renameVariable (const std::string& name, const std::string& new_name)
{
    loginf << "DBObject: renameVariable: name " << name << " new_name " << new_name;

    assert (hasVariable (name));
    assert (!hasVariable (new_name));

    DBOVariable* variable = variables_.at(name);

    variables_.erase(name);
    assert (!hasVariable (name));

    variable->name(new_name);
    variables_[new_name] = variable;

    assert (hasVariable (new_name));
}

void DBObject::deleteVariable (const std::string& name)
{
    assert (hasVariable (name));

    DBOVariable* variable = variables_.at(name);
    variables_.erase(variables_.find (name));
    assert (!hasVariable (name));
    delete variable;
}

const std::map<std::string, DBOVariable*>& DBObject::variables () const
{
    return variables_;
}

bool DBObject::hasMetaTable (const std::string& schema) const
{
    return meta_tables_.find(schema) != meta_tables_.end();
}

const std::string& DBObject::metaTable (const std::string& schema) const
{
    assert (hasMetaTable(schema));
    return meta_tables_.at(schema);
}

/**
 * Returns if the current schema name exists in the data source definitions
 */
bool DBObject::hasCurrentDataSourceDefinition () const
{
    if (!ATSDB::instance().schemaManager().hasCurrentSchema())
        return false;

    return (data_source_definitions_.find(ATSDB::instance().schemaManager().getCurrentSchema().name())
            != data_source_definitions_.end());
}

const DBODataSourceDefinition& DBObject::currentDataSourceDefinition () const
{
    assert (hasCurrentDataSourceDefinition());
    return *data_source_definitions_.at(ATSDB::instance().schemaManager().getCurrentSchema().name());
}

void DBObject::deleteDataSourceDefinition (const std::string& schema)
{
    assert (data_source_definitions_.count(schema) == 1);
    delete data_source_definitions_.at(schema);
    data_source_definitions_.erase(schema);
}

void DBObject::dataSourceDefinitionChanged ()
{
    logdbg << "DBObject: " << name_ << " dataSourceDefinitionChanged";
}

void DBObject::buildDataSources()
{
    logdbg << "DBObject: buildDataSources: start";
    data_sources_.clear();

    logdbg  << "DBObject: buildDataSources: building dbo " << name_;
    if (!is_loadable_ || !hasCurrentDataSourceDefinition ())
    {
        logdbg << "DBObject: buildDataSources: not processed is loadable " << is_loadable_
               << " has data source " << hasCurrentDataSourceDefinition ();
        return;
    }

    if (!ATSDB::instance().interface().hasDataSourceTables(*this))
    {
        logwrn << "DBObject: buildDataSources: object " << name_ << " has data sources but no respective tables";
        return;
    }

    logdbg  << "DBObject: buildDataSources: building data sources for " << name_;

    try
    {
        data_sources_ = ATSDB::instance().interface().getDataSources(*this);
    }
    catch (std::exception& e)
    {
        logerr << "DBObject: buildDataSources: failed with '" << e.what() << "', deleting entry";
        deleteDataSourceDefinition (ATSDB::instance().schemaManager().getCurrentSchema().name());
        assert (!hasCurrentDataSourceDefinition());
    }

    logdbg << "DBObject: buildDataSources: end";
}


/**
 * Returns true if current_meta_table_ is not null, else gets the current schema, checks and get the meta_table_
 * entry for the current schema, and returns if the meta table for the current schema exists in the current schema.
 */
bool DBObject::hasCurrentMetaTable () const
{
    return current_meta_table_ != nullptr;
}

/**
 * If current_meta_table_ is not set, it is set be getting the current schema, and getting the current meta table from
 * the schema by its identifier. Then current_meta_table_ is returned.
 */
const MetaDBTable& DBObject::currentMetaTable () const
{
    assert (current_meta_table_);
    return *current_meta_table_;
}


//void DBObject::checkVariables ()
//{
//    assert (hasCurrentMetaTable());

//    for (auto it : variables_)
//    {
//        bool found = current_meta_table_->hasColumn(it.first);

//        if (!found)
//        {
//            logwrn  << "DBObject: checkVariables: erasing non-existing variable '"<<it.first<<"'";
//            //      delete it->second;
//            variables_.erase(it.first);
//        }
//    }
//    variables_checked_=true;
//}

const std::string& DBObject::getNameOfSensor (int num)
{
    assert (data_sources_.count(num) > 0);

    return data_sources_.at(num).name();
}

bool DBObject::hasActiveDataSourcesInfo ()
{
    return ATSDB::instance().interface().hasActiveDataSources(*this);
}

const std::set<int> DBObject::getActiveDataSources () const
{
    return ATSDB::instance().interface().getActiveDataSources(*this);
}

std::string DBObject::status ()
{
    if (read_job_)
    {
        if (loadedCount())
            return "Loading";
        else
        {
            if (read_job_->started())
                return "Started";
            else
                return "Queued";
        }
    }
    else if (finalize_jobs_.size() > 0)
        return "Post-processing";
    else
        return "Idle";

}

DBObjectWidget* DBObject::widget ()
{
    if (!widget_)
    {
        widget_ = new DBObjectWidget (this, ATSDB::instance().schemaManager());
        assert (widget_);

        if (locked_)
            widget_->lock();
    }

    return widget_;
}

DBObjectInfoWidget *DBObject::infoWidget ()
{
    if (!info_widget_)
    {
        info_widget_ = new DBObjectInfoWidget (*this);
        assert (info_widget_);
    }

    return info_widget_;
}

DBOLabelDefinitionWidget* DBObject::labelDefinitionWidget()
{
    assert (label_definition_);
    return label_definition_->widget();
}

void DBObject::lock ()
{
    locked_ = true;

    for (auto& var_it : variables_)
        var_it.second->lock();

    if (widget_)
        widget_->lock();
}

void DBObject::unlock ()
{
    locked_ = false;

    for (auto& var_it : variables_)
        var_it.second->unlock();

    if (widget_)
        widget_->unlock();
}


void DBObject::schemaChangedSlot ()
{
    loginf << "DBObject: schemaChangedSlot";

    if (ATSDB::instance().schemaManager().hasCurrentSchema())
    {
        DBSchema& schema = ATSDB::instance().schemaManager().getCurrentSchema();

        if (meta_tables_.find(schema.name()) == meta_tables_.end())
        {
            logwrn << "DBObject: schemaChangedSlot: object " << name_ << " has not main meta table for current schema";
            current_meta_table_ = nullptr;
            return;
        }

        std::string meta_table_name = meta_tables_ .at(schema.name());
        assert (schema.hasMetaTable (meta_table_name));
        current_meta_table_ = &schema.metaTable (meta_table_name);
    }
    else
        current_meta_table_ = nullptr;

    databaseContentChangedSlot ();
}

void DBObject::load (DBOVariableSet& read_set, bool use_filters, bool use_order, DBOVariable* order_variable,
                     bool use_order_ascending, const std::string &limit_str)
{
    assert (is_loadable_);

    if (read_job_)
    {
        JobManager::instance().cancelJob(read_job_);
        read_job_ = nullptr;
    }
    read_job_data_.clear();

    for (auto job_it : finalize_jobs_)
        JobManager::instance().cancelJob(job_it);
    finalize_jobs_.clear();

    clearData ();

    std::string custom_filter_clause;
    std::vector <DBOVariable *> filtered_variables;

    if (use_filters)
    {
        custom_filter_clause = ATSDB::instance().filterManager().getSQLCondition (name_, filtered_variables);
    }

    //    DBInterface &db_interface, DBObject &dbobject, DBOVariableSet read_list, std::string custom_filter_clause,
    //    DBOVariable *order, const std::string &limit_str

    read_job_ = std::shared_ptr<DBOReadDBJob> (new DBOReadDBJob (ATSDB::instance().interface(), *this,
                                                                 read_set, custom_filter_clause,
                                                                 filtered_variables, use_order, order_variable,
                                                                 use_order_ascending, limit_str));

    connect (read_job_.get(), SIGNAL(intermediateSignal(std::shared_ptr<Buffer>)),
             this, SLOT(readJobIntermediateSlot(std::shared_ptr<Buffer>)), Qt::QueuedConnection);
    connect (read_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(readJobObsoleteSlot()), Qt::QueuedConnection);
    connect (read_job_.get(), SIGNAL(doneSignal()), this, SLOT(readJobDoneSlot()), Qt::QueuedConnection);

    if (info_widget_)
        info_widget_->updateSlot();

    JobManager::instance().addDBJob(read_job_);
}

void DBObject::quitLoading ()
{
    if (read_job_)
    {
        read_job_->setObsolete();
    }
}

void DBObject::clearData ()
{
    if (data_)
        data_ = nullptr;
}

void DBObject::updateData (DBOVariable &key_var, std::shared_ptr<Buffer> buffer)
{
    assert (!update_job_);

    update_job_ = std::shared_ptr<UpdateBufferDBJob> (new UpdateBufferDBJob(ATSDB::instance().interface(),
                                                                            *this, key_var, buffer));

    connect (update_job_.get(), SIGNAL(doneSignal()), this, SLOT(updateDoneSlot()), Qt::QueuedConnection);
    connect (update_job_.get(), SIGNAL(updateProgressSignal(float)), this, SLOT(updateProgressSlot(float)),
             Qt::QueuedConnection);

    JobManager::instance().addDBJob(update_job_);
}

void DBObject::updateProgressSlot (float percent)
{
    emit updateProgressSignal(percent);
}

void DBObject::updateDoneSlot ()
{
    update_job_ = nullptr;

    emit updateDoneSignal (*this);
}

std::map<int, std::string> DBObject::loadLabelData (std::vector<int> rec_nums, int break_item_cnt)
{
    std::string custom_filter_clause;
    bool first=true;

    assert (hasVariable("rec_num"));

    custom_filter_clause = variable("rec_num").currentDBColumn().identifier()+" in (";
    for (auto& rec_num : rec_nums)
    {
        if (first)
            first=false;
        else
            custom_filter_clause += ",";

        custom_filter_clause += std::to_string(rec_num);
    }
    custom_filter_clause += ")";

    DBOVariableSet read_list = label_definition_->readList();
    if (!read_list.hasVariable(variable("rec_num")))
        read_list.add(variable("rec_num"));

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    DBInterface& db_interface = ATSDB::instance().interface ();

    db_interface.prepareRead (*this, read_list, custom_filter_clause, {}, false, nullptr, false, "");
    std::shared_ptr<Buffer> buffer = db_interface.readDataChunk(*this);
    db_interface.finalizeReadStatement(*this);

    if (buffer->size() != rec_nums.size())
        throw std::runtime_error ("DBObject "+name_+": loadLabelData: failed to load label for "+custom_filter_clause);

    assert (buffer->size() == rec_nums.size());

    Utils::Data::finalizeBuffer(read_list, buffer);

    std::map<int, std::string> labels = label_definition_->generateLabels (rec_nums, buffer, break_item_cnt);

    boost::posix_time::ptime stop_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time - start_time;

    loginf  << "DBObject: loadLabelData: done after " << diff.total_milliseconds() << " ms";

    return labels;
}

void DBObject::readJobIntermediateSlot (std::shared_ptr<Buffer> buffer)
{
    assert (buffer);
    logdbg << "DBObject: " << name_ << " readJobIntermediateSlot: buffer size " << buffer->size();

    DBOReadDBJob* sender = dynamic_cast <DBOReadDBJob*> (QObject::sender());

    if (!sender)
    {
        logwrn << "DBObject: readJobIntermediateSlot: null sender, event on the loose";
        return;
    }
    assert (sender == read_job_.get());

    std::vector <DBOVariable*>& variables = sender->readList().getSet ();
    const PropertyList &properties = buffer->properties();

    for (auto var_it : variables)
    {
        const DBTableColumn &column = var_it->currentDBColumn ();
        assert (properties.hasProperty(column.name()));
        const Property &property = properties.get(column.name());
        assert (property.dataType() == var_it->dataType());
    }

    logdbg << "DBObject: " << name_ << " readJobIntermediateSlot: got buffer with size " << buffer->size();

    read_job_data_.push_back(buffer);

    FinalizeDBOReadJob* job = new FinalizeDBOReadJob (*this, sender->readList(), buffer);

    std::shared_ptr<FinalizeDBOReadJob> job_ptr = std::shared_ptr<FinalizeDBOReadJob> (job);
    connect (job, SIGNAL(doneSignal()), this, SLOT(finalizeReadJobDoneSlot()), Qt::QueuedConnection);
    finalize_jobs_.push_back(job_ptr);

    JobManager::instance().addJob(job_ptr);

    if (info_widget_)
        info_widget_->updateSlot();

}

void DBObject::readJobObsoleteSlot ()
{
    logdbg << "DBObject: " << name_ << " readJobObsoleteSlot";
    read_job_ = nullptr;
    read_job_data_.clear();

    if (info_widget_)
        info_widget_->updateSlot();

    emit loadingDoneSignal(*this);
}

void DBObject::readJobDoneSlot()
{
    loginf << "DBObject: " << name_ << " readJobDoneSlot";
    read_job_ = nullptr;

    if (info_widget_)
        info_widget_->updateSlot();

    if (!isLoading())
    {
        loginf << "DBObject: " << name_ << " readJobDoneSlot: no jobs left, done";
        emit loadingDoneSignal(*this);
    }
}

void DBObject::finalizeReadJobDoneSlot()
{
    logdbg << "DBObject: " << name_ << " finalizeReadJobDoneSlot";

    FinalizeDBOReadJob* sender = dynamic_cast <FinalizeDBOReadJob*> (QObject::sender());

    if (!sender)
    {
        logwrn << "DBObject: finalizeReadJobDoneSlot: null sender, event on the loose";
        return;
    }

    std::shared_ptr<Buffer> buffer = sender->buffer();

    bool found=false;
    for (auto final_it : finalize_jobs_)
    {
        if (final_it.get() == sender)
        {
            finalize_jobs_.erase(std::find(finalize_jobs_.begin(), finalize_jobs_.end(), final_it));
            found=true;
            break;
        }
    }
    assert (found);

    if (!data_)
        data_ = buffer;
    else
        data_->seizeBuffer (*buffer.get());

    logdbg << "DBObject: " << name_ << " finalizeReadJobDoneSlot: got buffer with size " << data_->size();

    if (info_widget_)
        info_widget_->updateSlot();

    emit newDataSignal(*this);

    if (!isLoading())
    {
        loginf << "DBObject: " << name_ << " finalizeReadJobDoneSlot: no jobs left, done";
        emit loadingDoneSignal(*this);
    }
}


void DBObject::databaseContentChangedSlot ()
{
    loginf << "DBObject: " << name_ << " databaseContentChangedSlot";

    if (!current_meta_table_)
    {
        logwrn << "DBObject: databaseContentChangedSlot: object " << name_ << " has no current meta table";
        is_loadable_ = false;
        return;
    }

    assert (current_meta_table_);
    std::string table_name = current_meta_table_->mainTableName();

    is_loadable_ = ATSDB::instance().interface().tableInfo().count(table_name) > 0;

    if (is_loadable_)
        count_ = ATSDB::instance().interface().count (table_name);

    buildDataSources();

    if (info_widget_)
        info_widget_->updateSlot();

    loginf << "DBObject: " << name_ << " databaseContentChangedSlot: loadable " << is_loadable_ << " count " << count_;
}

bool DBObject::isLoading ()
{
    return read_job_ || finalize_jobs_.size();
}

bool DBObject::hasData ()
{
    return count_ > 0;
}

size_t DBObject::count ()
{
    return count_;
}

size_t DBObject::loadedCount ()
{
    if (data_)
        return data_->size();
    else
        return 0;
}
