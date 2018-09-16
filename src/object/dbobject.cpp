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
#include <memory>

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
#include "insertbufferdbjob.h"
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

    logdbg  << "DBObject: constructor: created with instance_id " << instanceId() << " name " << name_;
}

/**
 * Deletes data source definitions, schema & meta table definitions and variables
 */
DBObject::~DBObject()
{
    current_meta_table_ = nullptr;
}

/**
 * Can generate DBOVariables, DBOSchemaMetaTableDefinitions and DBODataSourceDefinitions.
 */
void DBObject::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "DBObject: generateSubConfigurable: generating variable " << instance_id;
    if (class_id.compare ("DBOVariable") == 0)
    {
        std::string var_name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("name");

        assert (variables_.find (var_name) == variables_.end());

        loginf  << "DBObject: generateSubConfigurable: generating variable " << instance_id << " with name " << var_name;

        variables_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(var_name),  // args for key
                     std::forward_as_tuple(class_id, instance_id, this));  // args for mapped value
    }
    else if (class_id.compare ("DBOSchemaMetaTableDefinition") == 0)
    {
        logdbg << "DBObject: generateSubConfigurable: creating DBOSchemaMetaTableDefinition";
        std::string schema_name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("schema");

        assert (meta_table_definitions_.find(schema_name) == meta_table_definitions_.end());

        meta_table_definitions_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(schema_name),  // args for key
                     std::forward_as_tuple(class_id, instance_id, this));  // args for mapped value
    }
    else if (class_id.compare ("DBODataSourceDefinition") == 0)
    {
        std::string schema_name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("schema");

        assert (data_source_definitions_.find(schema_name) == data_source_definitions_.end());

        data_source_definitions_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(schema_name),  // args for key
                     std::forward_as_tuple(class_id, instance_id, this));  // args for mapped value

        connect (&data_source_definitions_.at(schema_name), SIGNAL(definitionChangedSignal()),
                 this, SLOT(dataSourceDefinitionChanged()));
    }
    else if (class_id.compare ("DBOLabelDefinition") == 0)
    {
        assert (!label_definition_);
        label_definition_.reset (new DBOLabelDefinition (class_id, instance_id, this));
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
    if (variables_.find (name) == variables_.end())
    {
        logwrn << "DBObject: hasVariable: name " << name_ << " has no variable " << name;
        return false;
    }
    else
        return true;
}

DBOVariable& DBObject::variable (const std::string& name)
{
    assert (hasVariable (name));
    return variables_.at(name);
}

void DBObject::renameVariable (const std::string& name, const std::string& new_name)
{
    loginf << "DBObject: renameVariable: name " << name << " new_name " << new_name;

    assert (hasVariable (name));
    assert (!hasVariable (new_name));

    // UGA TODO
    variables_[new_name] = std::move(variables_.at(name));
    variables_.erase(name);

    assert (hasVariable (new_name));
    variables_.at(new_name).name(new_name);
}

void DBObject::deleteVariable (const std::string& name)
{
    assert (hasVariable (name));
    variables_.erase(name);
    assert (!hasVariable (name));
}

bool DBObject::hasMetaTable (const std::string& schema) const
{
    return meta_table_definitions_.find(schema) != meta_table_definitions_.end();
}

const std::string& DBObject::metaTable (const std::string& schema) const
{
    assert (hasMetaTable(schema));
    return meta_table_definitions_.at(schema).metaTable();
}

void DBObject::deleteMetaTable (const std::string& schema)
{
    assert (hasMetaTable(schema));

    std::string meta_table_name = metaTable(schema);
    meta_table_definitions_.erase(schema);
    assert (!hasMetaTable(schema));

    if (current_meta_table_->name() == meta_table_name)
        current_meta_table_ = nullptr;

    if (widget_)
        widget_->updateMetaTablesGridSlot();
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
    return data_source_definitions_.at(ATSDB::instance().schemaManager().getCurrentSchema().name());
}

void DBObject::deleteDataSourceDefinition (const std::string& schema)
{
    assert (data_source_definitions_.count(schema) == 1);
    data_source_definitions_.erase(schema);

    if (widget_)
        widget_->updateDataSourcesGridSlot();

    buildDataSources();
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

    if (!existsInDB() || !is_loadable_ || !hasCurrentDataSourceDefinition ())
    {
        logerr << "DBObject: buildDataSources: not processed, exists " << existsInDB()
               << " is loadable " << is_loadable_
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

void DBObject::addDataSource (int key_value, const std::string& name)
{
    loginf << "DBObject: addDataSources: inserting source " << name;
    assert (hasCurrentDataSourceDefinition());

    const DBODataSourceDefinition &mos_def = currentDataSourceDefinition ();
    std::string meta_table_name = mos_def.metaTableName();
    std::string key_col_name = mos_def.foreignKey();
    std::string name_col_name = mos_def.nameColumn();

    const DBSchema &schema = ATSDB::instance().schemaManager().getCurrentSchema();
    assert (schema.hasMetaTable(meta_table_name));

    const MetaDBTable& meta =  schema.metaTable(meta_table_name);
    assert (meta.hasColumn(key_col_name));
    assert (meta.hasColumn(name_col_name));

    const DBTableColumn& foreign_key_col = meta.column(key_col_name);
    const DBTableColumn& name_col = meta.column(name_col_name);

    PropertyList list;
    list.addProperty(foreign_key_col.name(), PropertyDataType::INT);
    list.addProperty(name_col.name(), PropertyDataType::STRING);

    std::shared_ptr<Buffer> buffer_ptr = std::shared_ptr<Buffer> (new Buffer (list, name_));

    buffer_ptr->get<int>(foreign_key_col.name()).set(0, key_value);
    buffer_ptr->get<std::string>(name_col.name()).set(0, name);

    assert (ATSDB::instance().schemaManager().getCurrentSchema().hasMetaTable(meta_table_name));
    MetaDBTable& meta_table = ATSDB::instance().schemaManager().getCurrentSchema().metaTable(meta_table_name);

    DBInterface& db_interface = ATSDB::instance().interface();

    // create all needed tables
    DBTable& main_table = meta_table.mainTable();
    if (!main_table.existsInDB())
        // check for both since information might not be updated yet
        db_interface.createTable(main_table);

    // do sub table
    DBTable& db_table_name = meta_table.tableFor(name_col.identifier());
    db_interface.insertBuffer(db_table_name, buffer_ptr, 0, 0);

    if (main_table.name() != db_table_name.name()) // HACK of sub tables exist. should be rewritten
    {
        PropertyList main_list;
        main_list.addProperty(foreign_key_col.name(), PropertyDataType::INT);

        std::shared_ptr<Buffer> main_buffer_ptr = std::shared_ptr<Buffer> (new Buffer (main_list, name_));

        main_buffer_ptr->get<int>(foreign_key_col.name()).set(0, key_value);

        db_interface.insertBuffer(main_table, main_buffer_ptr, 0, 0);
    }

    emit db_interface.databaseContentChangedSignal();
}

void DBObject::addDataSources (std::map <int, std::string>& sources)
{
    loginf << "DBObject: addDataSources: inserting " << sources.size() << " sources";
    assert (hasCurrentDataSourceDefinition());

    const DBODataSourceDefinition &mos_def = currentDataSourceDefinition ();
    std::string meta_table_name = mos_def.metaTableName();
    std::string key_col_name = mos_def.foreignKey();
    std::string name_col_name = mos_def.nameColumn();

    const DBSchema &schema = ATSDB::instance().schemaManager().getCurrentSchema();
    assert (schema.hasMetaTable(meta_table_name));

    const MetaDBTable& meta =  schema.metaTable(meta_table_name);
    assert (meta.hasColumn(key_col_name));
    assert (meta.hasColumn(name_col_name));

    const DBTableColumn& foreign_key_col = meta.column(key_col_name);
    const DBTableColumn& name_col = meta.column(name_col_name);

    PropertyList list;
    list.addProperty(foreign_key_col.name(), PropertyDataType::INT);
    list.addProperty(name_col.name(), PropertyDataType::STRING);

    std::shared_ptr<Buffer> buffer_ptr = std::shared_ptr<Buffer> (new Buffer (list, name_));

    unsigned int cnt=0;
    for (auto& src_it : sources)
    {
        buffer_ptr->get<int>(foreign_key_col.name()).set(cnt, src_it.first);
        buffer_ptr->get<std::string>(name_col.name()).set(cnt, src_it.second);
        cnt++;
    }

    assert (ATSDB::instance().schemaManager().getCurrentSchema().hasMetaTable(meta_table_name));
    MetaDBTable& meta_table = ATSDB::instance().schemaManager().getCurrentSchema().metaTable(meta_table_name);

    DBInterface& db_interface = ATSDB::instance().interface();

    // create all needed tables
    DBTable& main_table = meta_table.mainTable();
    if (!main_table.existsInDB())
        // check for both since information might not be updated yet
        db_interface.createTable(main_table);

    // do sub table
    DBTable& db_table_name = meta_table.tableFor(name_col.identifier());
    db_interface.insertBuffer(db_table_name, buffer_ptr, 0, buffer_ptr->size()-1);

    if (main_table.name() != db_table_name.name()) // HACK of sub tables exist. should be rewritten
    {
        PropertyList main_list;
        main_list.addProperty(foreign_key_col.name(), PropertyDataType::INT);

        std::shared_ptr<Buffer> main_buffer_ptr = std::shared_ptr<Buffer> (new Buffer (main_list, name_));

        cnt=0;
        for (auto& src_it : sources)
        {
            main_buffer_ptr->get<int>(foreign_key_col.name()).set(cnt, src_it.first);
            cnt++;
        }

        db_interface.insertBuffer(main_table, main_buffer_ptr, 0, main_buffer_ptr->size()-1);
    }

    emit db_interface.databaseContentChangedSignal();
}

const std::string& DBObject::getNameOfSensor (int num)
{
    assert (data_sources_.count(num) > 0);

    return data_sources_.at(num).name();
}

bool DBObject::hasActiveDataSourcesInfo ()
{
    return ATSDB::instance().interface().hasActiveDataSources(*this);
}

const std::set<int> DBObject::getActiveDataSources ()
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
        widget_.reset(new DBObjectWidget (this, ATSDB::instance().schemaManager()));
        assert (widget_);

        if (locked_)
            widget_->lock();
    }

    return widget_.get(); // needed for qt integration, not pretty
}

DBObjectInfoWidget *DBObject::infoWidget ()
{
    if (!info_widget_)
    {
        info_widget_.reset (new DBObjectInfoWidget (*this));
        assert (info_widget_);
    }

    return info_widget_.get(); // needed for qt integration, not pretty
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
        var_it.second.lock();

    if (widget_)
        widget_->lock();
}

void DBObject::unlock ()
{
    locked_ = false;

    for (auto& var_it : variables_)
        var_it.second.unlock();

    if (widget_)
        widget_->unlock();
}


void DBObject::schemaChangedSlot ()
{
    loginf << "DBObject: schemaChangedSlot";

    if (ATSDB::instance().schemaManager().hasCurrentSchema())
    {
        DBSchema& schema = ATSDB::instance().schemaManager().getCurrentSchema();

        if (!hasMetaTable(schema.name()))
        {
            logwrn << "DBObject: schemaChangedSlot: object " << name_ << " has not main meta table for current schema";
            current_meta_table_ = nullptr;
            return;
        }

        std::string meta_table_name = meta_table_definitions_.at(schema.name()).metaTable();
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
    assert (existsInDB());

    for (auto& var_it : read_set.getSet())
        assert (var_it->existsInDB());

    if (order_variable)
        assert (order_variable->existsInDB());

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
    std::vector <DBOVariable*> filtered_variables;

    if (use_filters)
    {
        custom_filter_clause = ATSDB::instance().filterManager().getSQLCondition (name_, filtered_variables);
    }

    for (auto& var_it : filtered_variables)
        assert (var_it->existsInDB());

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

void DBObject::insertData (DBOVariableSet& list, std::shared_ptr<Buffer> buffer)
{
    assert (!insert_job_);

    buffer->transformVariables(list, false); // back again

    insert_job_ = std::shared_ptr<InsertBufferDBJob> (new InsertBufferDBJob(ATSDB::instance().interface(),
                                                                            *this, buffer));

    connect (insert_job_.get(), &InsertBufferDBJob::doneSignal, this, &DBObject::insertDoneSlot, Qt::QueuedConnection);
    connect (insert_job_.get(), &InsertBufferDBJob::insertProgressSignal, this, &DBObject::insertProgressSlot,
             Qt::QueuedConnection);

    JobManager::instance().addDBJob(insert_job_);
}

void DBObject::insertProgressSlot (float percent)
{
    emit insertProgressSignal(percent);
}

void DBObject::insertDoneSlot ()
{
    insert_job_ = nullptr;

    emit insertDoneSignal (*this);
    emit ATSDB::instance().interface().databaseContentChangedSignal();
}

void DBObject::updateData (DBOVariable &key_var, DBOVariableSet& list, std::shared_ptr<Buffer> buffer)
{
    assert (!update_job_);

    assert (existsInDB());
    assert (key_var.existsInDB());
    assert (ATSDB::instance().interface().checkUpdateBuffer(*this, key_var, list, buffer));

    buffer->transformVariables(list, false); // back again

    update_job_ = std::shared_ptr<UpdateBufferDBJob> (new UpdateBufferDBJob(ATSDB::instance().interface(),
                                                                            *this, key_var, buffer));

    connect (update_job_.get(), &UpdateBufferDBJob::doneSignal, this, &DBObject::updateDoneSlot, Qt::QueuedConnection);
    connect (update_job_.get(), &UpdateBufferDBJob::updateProgressSignal, this, &DBObject::updateProgressSlot,
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
    assert (is_loadable_);
    assert (existsInDB());

    std::string custom_filter_clause;
    bool first=true;

    // TODO rework to key variable
    assert (hasVariable("rec_num"));
    assert (variable("rec_num").existsInDB());

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

    for (auto& var_it : read_list.getSet())
        assert (var_it->existsInDB());

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    DBInterface& db_interface = ATSDB::instance().interface ();

    db_interface.prepareRead (*this, read_list, custom_filter_clause, {}, false, nullptr, false, "");
    std::shared_ptr<Buffer> buffer = db_interface.readDataChunk(*this);
    db_interface.finalizeReadStatement(*this);

    if (buffer->size() != rec_nums.size())
        throw std::runtime_error ("DBObject "+name_+": loadLabelData: failed to load label for "+custom_filter_clause);

    assert (buffer->size() == rec_nums.size());

    buffer->transformVariables(read_list, true);

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
    if (!current_meta_table_)
    {
        logwrn << "DBObject: databaseContentChangedSlot: object " << name_ << " has no current meta table";
        is_loadable_ = false;
        return;
    }

    assert (current_meta_table_);
    std::string table_name = current_meta_table_->mainTableName();

    is_loadable_ = current_meta_table_->existsInDB() && ATSDB::instance().interface().tableInfo().count(table_name) > 0;

    if (is_loadable_)
        count_ = ATSDB::instance().interface().count (table_name);

    loginf << "DBObject: " << name_ << " databaseContentChangedSlot: exists in db "
           << current_meta_table_->existsInDB() << " count " << count_;

    data_sources_.clear();
    if (current_meta_table_->existsInDB())
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

bool DBObject::existsInDB () const
{
    if (!hasCurrentMetaTable())
        return false;
    else
        return currentMetaTable().existsInDB();
}
