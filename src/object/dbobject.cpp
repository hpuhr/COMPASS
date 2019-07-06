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
#include "dboeditdatasourceswidget.h"
#include "dboeditdatasourceactionoptionswidget.h"
#include "storeddbodatasourcewidget.h"

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
    logdbg  << "DBObject: dtor: " << name_;

    current_meta_table_ = nullptr;
}

/**
 * Can generate DBOVariables, DBOSchemaMetaTableDefinitions and DBODataSourceDefinitions.
 */
void DBObject::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "DBObject: generateSubConfigurable: generating variable " << instance_id;
    if (class_id == "DBOVariable")
    {
        std::string var_name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("name");

        assert (variables_.find (var_name) == variables_.end());

        logdbg << "DBObject: generateSubConfigurable: generating variable " << instance_id << " with name " << var_name;

        variables_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(var_name),  // args for key
                     std::forward_as_tuple(class_id, instance_id, this));  // args for mapped value
    }
    else if (class_id == "DBOSchemaMetaTableDefinition")
    {
        logdbg << "DBObject: generateSubConfigurable: creating DBOSchemaMetaTableDefinition";
        std::string schema_name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("schema");

        assert (meta_table_definitions_.find(schema_name) == meta_table_definitions_.end());

        meta_table_definitions_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(schema_name),  // args for key
                     std::forward_as_tuple(class_id, instance_id, this));  // args for mapped value
    }
    else if (class_id == "DBODataSourceDefinition")
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
    else if (class_id == "DBOLabelDefinition")
    {
        assert (!label_definition_);
        label_definition_.reset (new DBOLabelDefinition (class_id, instance_id, this));
    }
    else if (class_id == "StoredDBODataSource")
    {
        unsigned int id = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueUint("id");

        assert (stored_data_sources_.find (id) == stored_data_sources_.end());

        loginf << "DBObject: generateSubConfigurable: generating stored DS " << instance_id << " with id " << id;

        stored_data_sources_.emplace(std::piecewise_construct,
                                     std::forward_as_tuple(id),  // args for key
                                     std::forward_as_tuple(class_id, instance_id, this));  // args for mapped value
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
    return variables_.find (name) != variables_.end();
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

bool DBObject::uses (const DBTableColumn& column) const
{
    logdbg << "DBObject " << name_ << ": uses: column " << column.name();

    for (auto& dbovar_it : variables_)
    {
        const DBOVariable& var = dbovar_it.second;

        if (!var.hasCurrentDBColumn())
            continue;

        if (var.currentDBColumn() == column)
            return true;
    }

    return false;
}

bool DBObject::hasStoredDataSource (unsigned int id) const
{
    return stored_data_sources_.find (id) != stored_data_sources_.end();
}

StoredDBODataSource& DBObject::storedDataSource (unsigned int id)
{
    assert (hasStoredDataSource (id));
    return stored_data_sources_.at(id);
}

StoredDBODataSource& DBObject::addNewStoredDataSource ()
{
    unsigned int id = stored_data_sources_.size() ? stored_data_sources_.rbegin()->first+1 : 0;

    loginf << "DBObject: addNewStoredDataSource: new id " << id;

    assert (!hasStoredDataSource (id));

    Configuration& config = configuration().addNewSubConfiguration("StoredDBODataSource",
                                                                   "StoredDBODataSource"+std::to_string(id));
    config.addParameterUnsignedInt ("id", id);

    generateSubConfigurable("StoredDBODataSource", "StoredDBODataSource"+std::to_string(id));

    return storedDataSource(id);
}

//void DBObject::renameStoredDataSource (const std::string& name, const std::string& new_name)
//{
//    loginf << "DBObject: renameStoredDataSource: name " << name << " new_name " << new_name;

//    assert (hasStoredDataSource (name));
//    assert (!hasStoredDataSource (new_name));

//    stored_data_sources_[new_name] = std::move(stored_data_sources_.at(name));

//    stored_data_sources_.erase(name);

//    assert (hasStoredDataSource (new_name));
//    stored_data_sources_.at(new_name).name(new_name);
//}

void DBObject::deleteStoredDataSource (unsigned int id)
{
    assert (hasStoredDataSource (id));
    stored_data_sources_.erase(id);
    assert (!hasStoredDataSource (id));

    if (edit_ds_widget_)
        edit_ds_widget_->update();
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

    removeVariableInfoForSchema (schema);

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

//    if (!existsInDB() || !is_loadable_ || !hasCurrentDataSourceDefinition ())
//    {
//        logerr << "DBObject: buildDataSources: not processed, exists " << existsInDB()
//               << " is loadable " << is_loadable_
//               << " has data source " << hasCurrentDataSourceDefinition ();
//        return;
//    }

    if (!hasCurrentDataSourceDefinition ())
    {
        logerr << "DBObject: buildDataSources: has data source definition " << hasCurrentDataSourceDefinition ();
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
MetaDBTable& DBObject::currentMetaTable () const
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

bool DBObject::hasKeyVariable ()
{
    if (hasCurrentMetaTable())
        return currentMetaTable().mainTable().hasKey();

    for (auto& var_it : variables_)
        if (var_it.second.isKey())
            return true;

    return false;
}


DBOVariable& DBObject::getKeyVariable()
{
    assert (hasKeyVariable());

    if (hasCurrentMetaTable()) // search in main table
    {
         std::string key_col_name = currentMetaTable().mainTable().key();

         for (auto& var_it : variables_)
             if (var_it.second.hasCurrentDBColumn() && var_it.second.currentDBColumn().name() == key_col_name)
             {
                 logdbg << "DBObject " << name() << ": getKeyVariable: returning var " << var_it.second.name();
                 return var_it.second;
             }
    }


    for (auto& var_it : variables_) // search in any
        if (var_it.second.isKey())
        {
            loginf << "DBObject " << name() << ": getKeyVariable: returning first found var " << var_it.second.name();
            return var_it.second;
        }

    throw std::runtime_error ("DBObject: getKeyVariable: no key variable found");
}

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
    db_interface.insertBuffer(meta_table, buffer_ptr);

    logdbg << "DBObject: addDataSources: emitting signal";
    emit db_interface.databaseContentChangedSignal();
}

void DBObject::addDataSources (std::map <int, std::pair<int,int>>& sources)
{
    loginf << "DBObject " << name_ << ": addDataSources:  inserting " << sources.size() << " sources";
    assert (hasCurrentDataSourceDefinition());

    const DBODataSourceDefinition &mos_def = currentDataSourceDefinition ();
    std::string meta_table_name = mos_def.metaTableName();

    std::string key_col_name = mos_def.foreignKey();
    std::string name_col_name = mos_def.nameColumn();
    std::string short_name_col_name = mos_def.shortNameColumn();
    std::string sac_col_name = mos_def.sacColumn();
    std::string sic_col_name = mos_def.sicColumn();
    std::string latitude_col_name = mos_def.latitudeColumn();
    std::string longitude_col_name = mos_def.longitudeColumn();
    std::string altitude_col_name = mos_def.altitudeColumn();

    const DBSchema &schema = ATSDB::instance().schemaManager().getCurrentSchema();
    assert (schema.hasMetaTable(meta_table_name));

    const MetaDBTable& meta =  schema.metaTable(meta_table_name);
    assert (meta.hasColumn(key_col_name));
    assert (meta.hasColumn(name_col_name));
    assert (meta.hasColumn(short_name_col_name));
    assert (meta.hasColumn(sac_col_name));
    assert (meta.hasColumn(sic_col_name));

    bool has_lat_long = meta.hasColumn(latitude_col_name) && meta.hasColumn(longitude_col_name);
    bool has_altitude = meta.hasColumn(altitude_col_name);

    const DBTableColumn& foreign_key_col = meta.column(key_col_name);
    const DBTableColumn& name_col = meta.column(name_col_name);
    const DBTableColumn& short_name_col = meta.column(short_name_col_name);
    const DBTableColumn& sac_col = meta.column(sac_col_name);
    const DBTableColumn& sic_col = meta.column(sic_col_name);
    //const DBTableColumn& latitude_col = meta.column(latitude_col_name);
    //const DBTableColumn& longitude_col = meta.column(longitude_col_name);
    //const DBTableColumn& altitude_col = meta.column(altitude_col_name);

    PropertyList list;
    list.addProperty(foreign_key_col.name(), foreign_key_col.propertyType());
    list.addProperty(name_col.name(), name_col.propertyType());
    list.addProperty(short_name_col.name(), short_name_col.propertyType());
    list.addProperty(sac_col.name(), sac_col.propertyType());
    list.addProperty(sic_col.name(), sic_col.propertyType());

    if (has_lat_long)
    {
        list.addProperty(meta.column(latitude_col_name).name(), meta.column(latitude_col_name).propertyType());
        list.addProperty(meta.column(longitude_col_name).name(), meta.column(longitude_col_name).propertyType());
    }

    if (has_altitude)
        list.addProperty(meta.column(altitude_col_name).name(), meta.column(altitude_col_name).propertyType());

    loginf << "DBObject: addDataSources: " << name() << " has lat/long " << has_lat_long << " has alt " << has_altitude;

    std::shared_ptr<Buffer> buffer_ptr = std::shared_ptr<Buffer> (new Buffer (list, name_));

    unsigned int cnt=0;
    int sac, sic;
    bool has_sac_sic;
    std::string name;

    for (auto& src_it : sources)
    {
        sac = src_it.second.first;
        sic = src_it.second.second;
        has_sac_sic = (sac >= 0) && (sic >= 0);

        name = std::to_string(src_it.first);

        if (has_sac_sic)
        {
            bool stored_found = false;
            unsigned int stored_id;

            for (auto& stored_it : stored_data_sources_)
            {
                if (stored_it.second.sac() == sac && stored_it.second.sic() == sic)
                {
                    stored_found = true;
                    stored_id = stored_it.first;
                    break;
                }
            }

            if (stored_found)
            {
                assert (hasStoredDataSource(stored_id));
                StoredDBODataSource& src = storedDataSource(stored_id);

                name = src.name();
                if (src.hasShortName())
                    buffer_ptr->get<std::string>(short_name_col.name()).set(cnt, src.shortName());

                buffer_ptr->get<char>(sac_col.name()).set(cnt, src.sac());
                buffer_ptr->get<char>(sic_col.name()).set(cnt, src.sic());

                loginf << "DBObject " << name_ << ": addDataSources: id " << src_it.first
                       << " sac " << sac << " sic " << sic
                       << " found stored name " << name << " id " << stored_id
                       << " sac " << static_cast<int> (src.sac()) << " sic " << static_cast<int> (src.sic());

                if (has_lat_long)
                {
                    loginf << "DBObject: addDataSources: " << name << " stored found has lat/long";

                    if (src.hasLatitude())
                        buffer_ptr->get<double>(meta.column(latitude_col_name).name()).set(cnt, src.latitude());
                    else
                        buffer_ptr->get<double>(meta.column(latitude_col_name).name()).setNull(cnt);

                    if (src.hasLongitude())
                        buffer_ptr->get<double>(meta.column(longitude_col_name).name()).set(cnt, src.longitude());
                    else
                        buffer_ptr->get<double>(meta.column(longitude_col_name).name()).setNull(cnt);
                }
//                else
//                {
//                    buffer_ptr->get<double>(meta.column(latitude_col_name).name()).set(cnt, src.latitude());
//                    buffer_ptr->get<double>(meta.column(longitude_col_name).name()).set(cnt, src.longitude());
//                }

                if (has_altitude)
                {
                    loginf << "DBObject: addDataSources: " << name << " stored found has alt";
                    if (src.hasAltitude())
                        buffer_ptr->get<double>(meta.column(altitude_col_name).name()).set(cnt, src.altitude());
                    else
                        buffer_ptr->get<double>(meta.column(altitude_col_name).name()).setNull(cnt);
                }
            }
            else
            {
                buffer_ptr->get<std::string>(short_name_col.name()).setNull(cnt);
                buffer_ptr->get<char>(sac_col.name()).set(cnt, sac);
                buffer_ptr->get<char>(sic_col.name()).set(cnt, sic);

                if (has_lat_long)
                {
                    buffer_ptr->get<double>(meta.column(latitude_col_name).name()).setNull(cnt);
                    buffer_ptr->get<double>(meta.column(longitude_col_name).name()).setNull(cnt);
                }

                if (has_altitude)
                    buffer_ptr->get<double>(meta.column(altitude_col_name).name()).setNull(cnt);
            }
        }
        else
        {
            buffer_ptr->get<std::string>(short_name_col.name()).setNull(cnt);
            buffer_ptr->get<char>(sac_col.name()).setNull(cnt);
            buffer_ptr->get<char>(sic_col.name()).setNull(cnt);

            if (has_lat_long)
            {
                buffer_ptr->get<double>(meta.column(latitude_col_name).name()).setNull(cnt);
                buffer_ptr->get<double>(meta.column(longitude_col_name).name()).setNull(cnt);
            }

            if (has_altitude)
                buffer_ptr->get<double>(meta.column(altitude_col_name).name()).setNull(cnt);
        }

        buffer_ptr->get<int>(foreign_key_col.name()).set(cnt, src_it.first);
        buffer_ptr->get<std::string>(name_col.name()).set(cnt, name);
        cnt++;
    }

    assert (ATSDB::instance().schemaManager().getCurrentSchema().hasMetaTable(meta_table_name));
    MetaDBTable& meta_table = ATSDB::instance().schemaManager().getCurrentSchema().metaTable(meta_table_name);

    DBInterface& db_interface = ATSDB::instance().interface();
    db_interface.insertBuffer(meta_table, buffer_ptr);

    logdbg << "DBObject: addDataSources: emitting signal";
    emit db_interface.databaseContentChangedSignal();

}

bool DBObject::hasDataSource (int id)
{
    return data_sources_.count(id) > 0;
}

DBODataSource& DBObject::getDataSource (int id)
{
    assert (hasDataSource (id));
    return data_sources_.at(id);
}

void DBObject::updateDataSource (int id)
{
    assert (hasDataSource (id));
    ATSDB::instance().interface().updateDataSource(getDataSource(id));
}

const std::string& DBObject::getNameOfSensor (int id)
{
    assert (hasDataSource (id));

    return data_sources_.at(id).name();
}

DBOEditDataSourceActionOptionsCollection DBObject::getSyncOptionsFromDB ()
{
    DBOEditDataSourceActionOptionsCollection options_collection;

    for (auto& ds_it : data_sources_)
    {
        assert (ds_it.first >= 0); // todo refactor to uint?
        unsigned int id = ds_it.first;
        options_collection [id] = DBOEditDataSourceActionOptionsCreator::getSyncOptionsFromDB (*this, ds_it.second);
    }

    return options_collection;
}

DBOEditDataSourceActionOptionsCollection DBObject::getSyncOptionsFromCfg ()
{
    DBOEditDataSourceActionOptionsCollection options_collection;

    for (auto& ds_it : stored_data_sources_)
    {
        assert (ds_it.first >= 0); // todo refactor to uint?
        unsigned int id = ds_it.first;
        options_collection [id] = DBOEditDataSourceActionOptionsCreator::getSyncOptionsFromCfg (*this, ds_it.second);
    }

    return options_collection;
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

DBOEditDataSourcesWidget* DBObject::editDataSourcesWidget()
{
    if (!edit_ds_widget_)
    {
        edit_ds_widget_.reset(new DBOEditDataSourcesWidget (this));
    }

    return edit_ds_widget_.get();
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

void DBObject::insertData (DBOVariableSet& list, std::shared_ptr<Buffer> buffer, bool emit_change)
{
    loginf << "DBObject " << name_ << ": insertData";

    assert (!insert_job_);

    buffer->transformVariables(list, false); // back again

    insert_job_ = std::shared_ptr<InsertBufferDBJob> (new InsertBufferDBJob(ATSDB::instance().interface(),
                                                                            *this, buffer, emit_change));

    connect (insert_job_.get(), &InsertBufferDBJob::doneSignal, this, &DBObject::insertDoneSlot, Qt::QueuedConnection);
    connect (insert_job_.get(), &InsertBufferDBJob::insertProgressSignal, this, &DBObject::insertProgressSlot,
             Qt::QueuedConnection);

    JobManager::instance().addDBJob(insert_job_);

    logdbg << "DBObject: insertData: end";
}

void DBObject::insertProgressSlot (float percent)
{
    emit insertProgressSignal(percent);
}

void DBObject::insertDoneSlot ()
{
    assert (insert_job_);
    bool emit_change = insert_job_->emitChange();
    insert_job_ = nullptr;

    emit insertDoneSignal (*this);

    if (emit_change)
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

    JobManager::instance().addBlockingJob(job_ptr);

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
    {
        data_ = buffer;
    }
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
    logdbg << "DBObject: databaseContentChangedSlot";

    if (!current_meta_table_)
    {
        logdbg << "DBObject: databaseContentChangedSlot: object " << name_ << " has no current meta table";
        is_loadable_ = false;
        return;
    }

    assert (current_meta_table_);
    std::string table_name = current_meta_table_->mainTableName();

    is_loadable_ = current_meta_table_->existsInDB() && ATSDB::instance().interface().tableInfo().count(table_name) > 0;

    if (is_loadable_)
        count_ = ATSDB::instance().interface().count (table_name);

    logdbg << "DBObject: " << name_ << " databaseContentChangedSlot: exists in db "
           << current_meta_table_->existsInDB() << " count " << count_;

    data_sources_.clear();
    if (current_meta_table_->existsInDB())
        buildDataSources();

    if (info_widget_)
        info_widget_->updateSlot();

    logdbg << "DBObject: " << name_ << " databaseContentChangedSlot: loadable " << is_loadable_ << " count " << count_;
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

void DBObject::print ()
{
    assert (hasCurrentMetaTable());

    std::stringstream ss;

    ss << "COLID";
    ss << " && COLTYPE";
    ss << " && COLKEY";
    ss << " && COLCOMMENT";
    ss << " && COLCIM";
    ss << " && COLUNIT";
    ss << " && OBNAME";
    ss << " && OBDESC";
    ss << " && OBDIM";
    ss << " && OBUNIT";
    ss << " && OBREP";
    ss << std::endl;

    for (auto& col_it : currentMetaTable ().columns())
    {
        const DBTableColumn& col = col_it.second;
        ss << col.identifier();
        ss << " && " << col.type();
        ss << " && " << col.isKey();
        ss << " && " << col.comment();
        ss << " && " << col.dimension();
        ss << " && " << col.unit();

        for (auto& var_it : variables_)
        {
            DBOVariable& var = var_it.second;
            if (var.hasCurrentDBColumn() && var.currentDBColumn().identifier() == col.identifier())
            {
                ss << " && " << var.name();
                ss << " && " << var.description();
                ss << " && " << var.dimension();
                ss << " && " << var.unit();
                ss << " && " << var.representationString();
            }
        }

        ss << std::endl;
    }

    loginf << "DBObject " << name() << ":\n" << ss.str();

}

void DBObject::removeDependenciesForSchema (const std::string& schema_name)
{
    loginf << "DBObject " << name() << ": removeDependenciesForSchema: " << schema_name;

    if (meta_table_definitions_.count(schema_name))
    {
        loginf << "DBObject " << name() << ": removeDependenciesForSchema: removing meta-table";
        deleteMetaTable(schema_name);
    }
}

void DBObject::removeVariableInfoForSchema (const std::string& schema_name)
{
    loginf << "DBObject " << name() << ": removeVariableInfoForSchema: " << schema_name;

    for (auto var_it = variables_.begin(); var_it != variables_.end();)
    {
        if (var_it->second.onlyExistsInSchema(schema_name))
        {
            loginf << "DBObject " << name() << ": removeVariableInfoForSchema: variable" << var_it->first
                   << " exists only in schema to be removed, deleting";
            variables_.erase(var_it++);
        }
        else
        {
            var_it->second.removeInfoForSchema(schema_name);
            ++var_it;
        }
    }
}
