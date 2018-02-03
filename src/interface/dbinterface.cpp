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

#include "boost/date_time/posix_time/posix_time.hpp"

#include <QMutexLocker>
#include <QMessageBox>
#include <QThread>
#include <QProgressDialog>

#include "atsdb.h"
#include "buffer.h"
#include "bufferwriter.h"
#include "config.h"
#include "dbobject.h"
#include "dbodatasource.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "mysqlserver.h"
#include "dbconnection.h"
#include "mysqlppconnection.h"
#include "sqliteconnection.h"
#include "dbinterfacewidget.h"
#include "dbinterfaceinfowidget.h"
#include "dbinterface.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "dbresult.h"
#include "metadbtable.h"
#include "dbschemamanager.h"
#include "dbschema.h"
//#include "StructureDescriptionManager.h"
#include "jobmanager.h"
#include "dboactivedatasourcesdbjob.h"
#include "dbominmaxdbjob.h"
#include "dimension.h"
#include "unit.h"
#include "unitmanager.h"
#include "dbtableinfo.h"
#include "dbtable.h"

#include "string.h"

using namespace Utils;


/**
 * Creates SQLGenerator, several containers based in DBOs (prepared_, reading_done_, exists_, count_), creates
 * write_table_names_,
 */
DBInterface::DBInterface(std::string class_id, std::string instance_id, ATSDB *atsdb)
    : Configurable (class_id, instance_id, atsdb), current_connection_(nullptr), sql_generator_(*this), widget_(nullptr), info_widget_(nullptr)
{
    QMutexLocker locker(&connection_mutex_);

    //registerParameter ("database_name", &database_name_, "");
    registerParameter ("read_chunk_size", &read_chunk_size_, 50000);
    registerParameter ("used_connection", &used_connection_, "");

    //TODO writing process should be different.
    //    write_table_names_[DBO_PLOTS] = "Plot";
    //    write_table_names_[DBO_SYSTEM_TRACKS] = "SystemTrack";
    //    write_table_names_[DBO_MLAT] = "MLAT";
    //    write_table_names_[DBO_ADS_B] = "ADSB";
    //    write_table_names_[DBO_REFERENCE_TRAJECTORIES] = "ReferenceTrajectory";
    //    write_table_names_[DBO_SENSOR_INFORMATION] = "PlotSensor";

    createSubConfigurables();
}

/**
 * If required, deletes connection, sql_generator_ and buffer_writer_.
 */
DBInterface::~DBInterface()
{
    logdbg  << "DBInterface: desctructor: start";

    QMutexLocker locker(&connection_mutex_);

    for (auto it : connections_)
        delete it.second;

    connections_.clear();

    assert (!widget_);

    if (buffer_writer_)
    {
        delete buffer_writer_;
        buffer_writer_=0;
    }

    logdbg  << "DBInterface: desctructor: end";
}

/**
 * Generates connection based on the DB_CONNECTION_TYPE of info, calls init on it. If a new database will be created, creates
 * the buffer_writer_, else calls updateExists and updateCount.
 */
void DBInterface::useConnection (const std::string &connection_type)
{
    logdbg << "DBInterface: useConnection: '" << connection_type << "'";
    if (current_connection_)
        assert (!current_connection_->ready());

    current_connection_ = connections_.at(connection_type);
    used_connection_ = connection_type;

    assert (current_connection_);
}

void DBInterface::databaseContentChanged ()
{
    updateTableInfo();

    if (!existsPropertiesTable())
        createPropertiesTable();

    loginf << "DBInterface: databaseOpened: post-processed " << isPostProcessed ();

    emit databaseContentChangedSignal();
}

void DBInterface::closeConnection ()
{
    QMutexLocker locker(&connection_mutex_);

    logdbg  << "DBInterface: closeConnection";
    for (auto it : connections_)
        it.second->disconnect ();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    if (info_widget_)
    {
        delete info_widget_;
        info_widget_ = nullptr;
    }

    table_info_.clear();
    logdbg  << "DBInterface: closeConnection: done";
}

void DBInterface::updateTableInfo ()
{
    QMutexLocker locker(&connection_mutex_);
    table_info_.clear();

    assert (current_connection_);
    table_info_ = current_connection_->getTableInfo();

    loginf << "DBInterface::updateTableInfo: found " << table_info_.size() << " tables";
}

DBInterfaceWidget *DBInterface::widget()
{
    if (!widget_)
    {
        widget_ = new DBInterfaceWidget (*this);
    }

    assert (widget_);
    return widget_;
}

DBInterfaceInfoWidget *DBInterface::infoWidget()
{
    if (!info_widget_)
    {
        info_widget_ = new DBInterfaceInfoWidget (*this);
    }

    assert (info_widget_);
    return info_widget_;
}

QWidget *DBInterface::connectionWidget()
{
    assert (current_connection_);
    return current_connection_->widget();
}

std::vector <std::string> DBInterface::getDatabases ()
{
    assert (current_connection_);
    return current_connection_->getDatabases();
}

bool DBInterface::ready ()
{
    if (!current_connection_)
        return false;

    return current_connection_->ready();
}

DBConnection &DBInterface::connection ()
{
    assert (ready());
    return *current_connection_;
}

void DBInterface::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "DBInterface: generateSubConfigurable: generating variable " << instance_id;
    if (class_id == "MySQLppConnection")
    {
        MySQLppConnection *connection = new MySQLppConnection (class_id, instance_id, this);
        assert (connections_.count (connection->getInstanceId()) == 0);
        connections_.insert (std::pair <std::string, DBConnection*> (connection->getInstanceId(), dynamic_cast<DBConnection*>(connection)));
    }
    else if (class_id == "SQLiteConnection")
    {
        SQLiteConnection *connection = new SQLiteConnection (class_id, instance_id, this);
        assert (connections_.count (connection->getInstanceId()) == 0);
        connections_.insert (std::pair <std::string, DBConnection*> (connection->getInstanceId(), dynamic_cast<DBConnection*>(connection)));
    }
    else
        throw std::runtime_error ("DBInterface: generateSubConfigurable: unknown class_id "+class_id );
}

void DBInterface::checkSubConfigurables ()
{
    if (connections_.count("MySQL++ Connection") == 0)
    {
        addNewSubConfiguration ("MySQLppConnection", "MySQL++ Connection");
        generateSubConfigurable ("MySQLppConnection", "MySQL++ Connection");
    }

    if (connections_.count("SQLite Connection") == 0)
    {
        addNewSubConfiguration ("SQLiteConnection", "SQLite Connection");
        generateSubConfigurable ("SQLiteConnection", "SQLite Connection");
    }
}

bool DBInterface::existsTable (const std::string& table_name)
{
    return table_info_.count (table_name) == 1;
}

/**
 * Returns existsTable for table name.
 */
bool DBInterface::existsMinMaxTable ()
{
    return table_info_.count (TABLE_NAME_MINMAX) == 1;
}

/**
 * Returns existsTable for table name.
 */
bool DBInterface::existsPropertiesTable ()
{
    return table_info_.count (TABLE_NAME_PROPERTIES) == 1;
}

///**
// * Gets SQL command for data sources list and packs the resulting buffer into a set, which is returned.
// */
std::set<int> DBInterface::queryActiveSensorNumbers(const DBObject &object)
{
    logdbg  << "DBInterface: queryActiveSensorNumbers: start";

    QMutexLocker locker(&connection_mutex_);

    assert (object.hasCurrentDataSourceDefinition());

    std::string local_key_dbovar = object.currentDataSourceDefinition().localKey();
    assert (object.hasVariable(local_key_dbovar));
    const DBTableColumn& local_key_col = object.variable(local_key_dbovar).currentDBColumn();

    std::set<int> data;

    std::shared_ptr<DBCommand> command = sql_generator_.getDistinctDataSourcesSelectCommand(object);

    std::shared_ptr<DBResult> result = current_connection_->execute(*command);

    assert (result->containsData());

    std::shared_ptr<Buffer> buffer = result->buffer();
    for (unsigned int cnt=0; cnt < buffer->size(); cnt++)
    {
        int tmp = buffer->getInt(local_key_col.name()).get(cnt);
        data.insert (tmp);
    }

    logdbg << "DBInterface: queryActiveSensorNumbers: done";
    return data;
}

///**
// * Gets SQL command, executes it and returns resulting buffer.
// */
std::map <int, DBODataSource> DBInterface::getDataSources (const DBObject &object)
{
    logdbg  << "DBInterface: getDataSourceDescription: start";

    QMutexLocker locker(&connection_mutex_);

    std::shared_ptr<DBCommand> command = sql_generator_.getDataSourcesSelectCommand(object);

    logdbg << "DBInterface: getDataSourceDescription: sql '" << command->get() << "'";

    std::shared_ptr <DBResult> result = current_connection_->execute(*command);
    assert (result->containsData());
    std::shared_ptr <Buffer> buffer = result->buffer();

    const DBODataSourceDefinition &ds = object.currentDataSourceDefinition ();
    const DBSchema &schema = ATSDB::instance().schemaManager().getCurrentSchema();
    assert (schema.hasMetaTable(ds.metaTableName()));

    const MetaDBTable& meta =  schema.metaTable(ds.metaTableName());

    const DBTableColumn& foreign_key_col = meta.column(ds.foreignKey());
    const DBTableColumn& name_col = meta.column(ds.nameColumn());

    assert (buffer->properties().hasProperty(foreign_key_col.name()));
    assert (buffer->properties().get(foreign_key_col.name()).dataType() == PropertyDataType::INT);
    assert (buffer->properties().hasProperty(name_col.name()));
    assert (buffer->properties().get(name_col.name()).dataType() == PropertyDataType::STRING);

    bool has_short_name = ds.hasShortNameColumn();
    std::string short_name_col_name;
    if (has_short_name)
    {
        short_name_col_name = meta.column(ds.shortNameColumn()).name();
        assert (buffer->properties().hasProperty(short_name_col_name) && buffer->properties().get(short_name_col_name).dataType() == PropertyDataType::STRING);
    }

    bool has_sac = ds.hasSacColumn();
    std::string sac_col_name;
    if (has_sac)
    {
        sac_col_name = meta.column(ds.sacColumn()).name();
        assert (buffer->properties().hasProperty(sac_col_name) && buffer->properties().get(sac_col_name).dataType() == PropertyDataType::CHAR);
    }

    bool has_sic = ds.hasSicColumn();
    std::string sic_col_name;
    if (has_sic)
    {
        sic_col_name = meta.column(ds.sicColumn()).name();
        assert (buffer->properties().hasProperty(sic_col_name) && buffer->properties().get(sic_col_name).dataType() == PropertyDataType::CHAR);
    }

    bool has_latitude = ds.hasLatitudeColumn();
    std::string latitude_col_name;
    if (has_latitude)
    {
        latitude_col_name = meta.column(ds.latitudeColumn()).name();
        assert (buffer->properties().hasProperty(latitude_col_name) && buffer->properties().get(latitude_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_longitude = ds.hasLongitudeColumn();
    std::string longitude_col_name;
    if (has_longitude)
    {
        longitude_col_name = meta.column(ds.longitudeColumn()).name();
        assert (buffer->properties().hasProperty(longitude_col_name) && buffer->properties().get(longitude_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_altitude = ds.hasAltitudeColumn();
    std::string altitude_col_name;
    if (has_altitude)
    {
        altitude_col_name = meta.column(ds.altitudeColumn()).name();
        assert (buffer->properties().hasProperty(altitude_col_name) || buffer->properties().get(altitude_col_name).dataType() == PropertyDataType::DOUBLE);
    }


    std::map <int, DBODataSource> sources;

    for (unsigned cnt = 0; cnt < buffer->size(); cnt++)
    {
        int key = buffer->getInt(foreign_key_col.name()).get(cnt);
        std::string name = buffer->getString(name_col.name()).get(cnt);
        assert (sources.count(key) == 0);
        loginf << "DBInterface: getDataSources: object " << object.name() << " key " << key << " name " << name;
        sources.insert(std::pair<int, DBODataSource>(key, DBODataSource(key, name)));

        if (has_short_name && !buffer->getString(short_name_col_name).isNone(cnt))
            sources.at(key).shortName(buffer->getString(short_name_col_name).get(cnt));

        if (has_sac && !buffer->getChar(sac_col_name).isNone(cnt))
            sources.at(key).sac(buffer->getChar(sac_col_name).get(cnt));

        if (has_sic && !buffer->getChar(sic_col_name).isNone(cnt))
            sources.at(key).sic(buffer->getChar(sic_col_name).get(cnt));

        if (has_latitude && !buffer->getDouble(latitude_col_name).isNone(cnt))
            sources.at(key).latitude(buffer->getDouble(latitude_col_name).get(cnt));

        if (has_longitude && !buffer->getDouble(longitude_col_name).isNone(cnt))
            sources.at(key).longitude(buffer->getDouble(longitude_col_name).get(cnt));

        if (has_altitude && !buffer->getDouble(altitude_col_name).isNone(cnt))
            sources.at(key).altitude(buffer->getDouble(altitude_col_name).get(cnt));
    }

    return sources;
}

size_t DBInterface::count (const std::string &table)
{
    logdbg  << "DBInterface: count: table " << table;
    assert (table_info_.count(table) > 0);

    QMutexLocker locker(&connection_mutex_);
    assert (current_connection_);

    std::string sql = sql_generator_.getCountStatement(table);

    logdbg  << "DBInterface: count: sql '" << sql << "'";

    DBCommand command;
    command.set(sql);

    PropertyList list;
    list.addProperty("count", PropertyDataType::INT);
    command.list(list);

    std::shared_ptr <DBResult> result = current_connection_->execute(command);

    assert (result->containsData());
    int tmp = result->buffer()->getInt("count").get(0);

    logdbg  << "DBInterface: count: " << table << ": "<< tmp <<" end";
    return static_cast<size_t> (tmp);
}

void DBInterface::setProperty (const std::string& id, const std::string& value)
{
    QMutexLocker locker(&connection_mutex_);
    assert (current_connection_);

    std::string str = sql_generator_.getInsertPropertyStatement(id, value);
    current_connection_->executeSQL (str);
}

std::string DBInterface::getProperty (const std::string& id)
{
    logdbg  << "DBInterface: getProperty: start";

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectPropertyStatement(id));

    PropertyList list;
    list.addProperty("property", PropertyDataType::STRING);
    command.list(list);

    std::shared_ptr <DBResult> result = current_connection_->execute(command);

    assert (result->containsData());

    std::string text;
    std::shared_ptr <Buffer> buffer = result->buffer();

    assert (buffer);

    if (buffer->firstWrite())
        throw std::invalid_argument ("DBInterface: getProperty: id "+id+" does not exist");

    assert (buffer->size() == 1);

    text = buffer->getString("property").get(0);

    logdbg  << "DBInterface: getProperty: end id " << id << " value " << text;
    return text;
}

bool DBInterface::hasProperty (const std::string& id)
{
    logdbg  << "DBInterface: hasProperty: start";

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectPropertyStatement(id));

    PropertyList list;
    list.addProperty("property", PropertyDataType::STRING);
    command.list(list);

    std::shared_ptr <DBResult> result = current_connection_->execute(command);

    assert (result->containsData());

    std::shared_ptr <Buffer> buffer = result->buffer();

    assert (buffer);

    return buffer->size() == 1;
}

void DBInterface::insertMinMax (const std::string& id, const std::string& object_name, const std::string& min, const std::string& max)
{
    QMutexLocker locker(&connection_mutex_);

    std::string str = sql_generator_.getInsertMinMaxStatement(id, object_name, min, max);
    current_connection_->executeSQL (str);
}

/**
 * If variable is a not meta variable, min/max values just for the variable. If it is, gets min/max values for all subvariables
 * and calculates the min/max for all subvariables. If the variable needs a unit transformation, it is performed (locally
 * in this thread).
 */
std::pair<std::string, std::string> DBInterface::getMinMaxString (const DBOVariable& var)
{
    if (!var.dbObject().count()) // object doesn't exist in this database
        return std::pair<std::string, std::string> (NULL_STRING, NULL_STRING);

    QMutexLocker locker(&connection_mutex_);

    PropertyList list;
    list.addProperty("min", PropertyDataType::STRING);
    list.addProperty("max", PropertyDataType::STRING);

    // get min max as strings

    //    if (!var->isMetaVariable())
    //    {
    logdbg  << "DBInterface: getMinMax: is not meta";
    DBCommand command;
    command.set(sql_generator_.getSelectMinMaxStatement(var.currentDBColumn().name(), var.dboName()));
    command.list(list);

    loginf << "DBInterface: getMinMaxString: sql '" << command.get() << "'";

    std::shared_ptr<DBResult> result = current_connection_->execute(command);

    assert (result);
    assert (result->containsData());
    std::shared_ptr<Buffer> buffer = result->buffer();

    assert (buffer);
    if (buffer->size() != 1)
    {
        throw std::invalid_argument ("DBInterface: getMinMaxString: string buffer for variable " + var.name() + " empty");
    }

    std::string min = buffer->getString("min").get(0);
    std::string max = buffer->getString("max").get(0);

    const DBTableColumn &column = var.currentDBColumn ();
    if (column.unit() != var.unitConst()) // do unit conversion stuff
    {
        const Dimension &dimension = UnitManager::instance().dimension (var.dimensionConst());
        double factor = dimension.getFactor (column.unit(), var.unitConst());

        min = String::multiplyString(min, factor, var.dataType());
        max = String::multiplyString(max, factor, var.dataType());
    }

    loginf << "DBInterface: getMinMaxString: var " << var.name() << " min " << min << " max " << max;
    return std::pair <std::string, std::string> (min, max);



    // TODO factor in units && representation?


    //    else
    //    {
    //        logdbg  << "DBInterface: getMinMax: is meta";
    //        DBCommandList command_list;

    //        const std::map <std::string, std::string> &subvars = var->getSubVariables ();
    //        std::map <std::string, std::string>::const_iterator it;

    //        for (it =subvars.begin(); it != subvars.end(); it++)
    //        {
    //            if (exists_[it->first])
    //                command_list.addCommandString(sql_generator_->getSelectMinMaxStatement(it->second, it->first));
    //        }

    //        command_list.setPropertyList(list);

    //        DBResult *result = connection_->execute(&command_list);

    //        assert (result->containsData());
    //        string_buffer = result->getBuffer();
    //        assert (string_buffer);
    //        delete result;
    //    }

    // got final (possibly multiple) min max in a buffer

    //    for (unsigned int cnt=0; cnt < string_buffer->size(); cnt++)
    //    {
    //        logdbg << "DBInterface: getMinMax: var " << var->getName() << " cnt " << cnt
    //                << " string min " << *((std::string*)string_buffer->get(cnt,0))
    //                << " max " << *((std::string*)string_buffer->get(cnt,1));
    //    }

    //    Buffer *data_buffer = createFromMinMaxStringBuffer (string_buffer, var->getDBOType());

    //    // only 1 minmax line
    //    assert (data_buffer->size() == 1);

    //    // check unit transformation

    //    DBOVariable *tmpvar = var->getFirst();

    //    std::string meta_tablename = tmpvar->getCurrentMetaTable ();
    //    std::string table_varname = tmpvar->getCurrentVariableName ();

    //    DBTableColumn *table_column = DBSchemaManager::getInstance().getCurrentSchema ()->getMetaTable(meta_tablename)->getTableColumn(table_varname);

    //    if (tmpvar->hasUnit () || table_column->hasUnit())
    //    {
    //        if (tmpvar->hasUnit () != table_column->hasUnit())
    //        {
    //            logerr << "DBInterface: getMinMax: unit transformation inconsistent: var " << tmpvar->getName () << " has unit " << tmpvar->hasUnit ()
    //                                                      << " table column " << table_column->getName() << " has unit " << table_column->hasUnit();
    //            throw std::runtime_error ("DBInterface: getMinMax: unit transformation error 1");
    //        }

    //        if (tmpvar->getUnitDimension().compare(table_column->getUnitDimension()) != 0)
    //        {
    //            logerr << "DBInterface: getMinMax: unit transformation inconsistent: var " << tmpvar->getName () << " has dimension " << tmpvar->getUnitDimension ()
    //                                                      << " table column " << table_column->getName() << " has dimension " << table_column->getUnitDimension();
    //            throw std::runtime_error ("DBInterface: getMinMax: unit transformation error 2");
    //        }

    //        Unit *unit = UnitManager::getInstance().getUnit (tmpvar->getUnitDimension());
    //        double factor = unit->getFactor (table_column->getUnitUnit(), tmpvar->getUnitUnit());
    //        logdbg  << "DBInterface: getMinMax: adapting " << tmpvar->getName () << " unit transformation with factor " << factor;

    //        multiplyData (data_buffer->get(0,0), (PROPERTY_DATA_TYPE) tmpvar->data_type_int_, factor);
    //        multiplyData (data_buffer->get(0,1), (PROPERTY_DATA_TYPE) tmpvar->data_type_int_, factor);
    //    }
    //    // write values back to string buffer
    //    *((std::string *)(string_buffer->get(0,0))) = tmpvar->getValueFrom (data_buffer->get(0,0));
    //    *((std::string *)(string_buffer->get(0,1))) = tmpvar->getValueFrom (data_buffer->get(0,1));

    //    delete data_buffer;

    //    return string_buffer;
}

//std::map <std::pair<std::string, std::string>, std::pair<std::string, std::string> > DBInterface::getMinMaxInfo ()
//{
//    std::scoped_lock l(mutex_);

//    std::map <std::pair<std::string, std::string>, std::pair<std::string, std::string> > min_max_values;

//    PropertyList list;
//    list.addProperty("id", PropertyDataType::STRING);
//    list.addProperty("dbo_type", PropertyDataType::STRING);
//    list.addProperty("min", PropertyDataType::STRING);
//    list.addProperty("max", PropertyDataType::STRING);

//    // get min max as strings

//    logdbg  << "DBInterface: getMinMax: is not meta";
//    DBCommand command;
//    command.setCommandString(sql_generator_->getSelectMinMaxStatement());

//    command.setPropertyList(list);

//    DBResult *result = connection_->execute(&command);

//    assert (result->containsData());
//    Buffer *min_max_buffer = result->getBuffer();
//    assert (min_max_buffer);

//    if (min_max_buffer->firstWrite())
//    {
//        logerr << "DBInterface: getMinMaxInfo: no minmax values defined";
//        delete min_max_buffer;
//        delete result;
//        return min_max_values;
//    }

//    assert (false);
//    // TODO FIXMEE

////    for (unsigned int cnt=0; cnt < min_max_buffer->getSize(); cnt ++)
////    {
////        std::string id = *(std::string*)min_max_buffer->get(cnt, 0);
////        DB_OBJECT_TYPE dbo_type = (DB_OBJECT_TYPE) *(int*)min_max_buffer->get(cnt, 1);
////        std::string min = *(std::string*)min_max_buffer->get(cnt, 2);
////        std::string max = *(std::string*)min_max_buffer->get(cnt, 3);

////        if (min.size() == 0 && max.size() == 0)
////        {
////            logwrn << "DBInterace: getMinMaxInfo: var " << id << " dbo " << dbo_type << " min '" << min << "' max '"
////                    << max << "' unusable";
////            continue;
////        }
////        assert (min_max_values.find(std::pair <DB_OBJECT_TYPE, std::string> (dbo_type, id)) == min_max_values.end());
////        min_max_values[std::pair <DB_OBJECT_TYPE, std::string> (dbo_type, id)] = std::pair<std::string, std::string> (min, max);
////    }


//    delete min_max_buffer;
//    delete result;

//    return min_max_values;
//}

bool DBInterface::isPostProcessed ()
{
    return hasProperty("postProcessed") && getProperty("postProcessed") == "Yes";

    //return existsMinMaxTable ();// && existsPropertiesTable();
}

void DBInterface::setPostProcessed (bool value)
{
    setProperty("postProcessed", value ? "Yes" : "Nope");
}

void DBInterface::postProcess ()
{
    loginf << "DBInterface: postProcess: creating jobs";

    if (!existsMinMaxTable())
        createMinMaxTable();
    else
        clearTableContent (TABLE_NAME_MINMAX);

    for (auto obj_it : ATSDB::instance().objectManager().objects())
    {
        if (!obj_it.second->hasData())
            continue;

        {
            DBOActiveDataSourcesDBJob* job = new DBOActiveDataSourcesDBJob (ATSDB::instance().interface(), *obj_it.second);

            std::shared_ptr<Job> shared_job = std::shared_ptr<Job> (job);
            connect (job, SIGNAL(doneSignal()), this, SLOT(postProcessingJobDoneSlot()), Qt::QueuedConnection);
            JobManager::instance().addDBJob(shared_job);
            postprocess_jobs_.push_back(shared_job);
        }
        {
            DBOMinMaxDBJob* job = new DBOMinMaxDBJob (ATSDB::instance().interface(), *obj_it.second);
            std::shared_ptr<Job> shared_job = std::shared_ptr<Job> (job);
            connect (job, SIGNAL(doneSignal()), this, SLOT(postProcessingJobDoneSlot()), Qt::QueuedConnection);
            JobManager::instance().addDBJob(shared_job);
            postprocess_jobs_.push_back(shared_job);
        }
    }

    assert (!postprocess_dialog_);
    postprocess_dialog_ = new QProgressDialog (tr("Post-Processing"), tr(""), 0, static_cast<int>(postprocess_jobs_.size()));
    postprocess_dialog_->setCancelButton(0);
    postprocess_dialog_->setWindowModality(Qt::ApplicationModal);
    postprocess_dialog_->show();

    postprocess_job_num_ = postprocess_jobs_.size();
}

void DBInterface::postProcessingJobDoneSlot()
{
    loginf << "DBInterface: postProcessingJobDoneSlot: " << postprocess_jobs_.size() << " active jobs" ;

    Job* job_sender = static_cast <Job*> (QObject::sender());
    assert (job_sender);
    assert (postprocess_jobs_.size() > 0);
    assert (postprocess_dialog_);

    bool found=false;
    for (auto job_it = postprocess_jobs_.begin(); job_it != postprocess_jobs_.end(); job_it++)
    {
        Job *current = job_it->get();
        if (current == job_sender)
        {
            postprocess_jobs_.erase(job_it);
            found = true;
            break;
        }
    }
    assert (found);

    if (postprocess_jobs_.size() == 0)
    {
        loginf << "DBInterface: postProcessingJobDoneSlot: done";
        setPostProcessed(true);

        delete postprocess_dialog_;
        postprocess_dialog_=nullptr;

        emit postProcessingDoneSignal();
    }
    else
        postprocess_dialog_->setValue(postprocess_job_num_-postprocess_jobs_.size());
}


//DBResult *DBInterface::count (const std::string &type, unsigned int sensor_number)
//{
//    std::scoped_lock l(mutex_);

//    loginf << "DBInterface: count: for type " << type << " number " << sensor_number;

//    assert (DBObjectManager::getInstance().existsDBObject (type));
//    assert (DBObjectManager::getInstance().getDBObject (type)->hasCurrentDataSource());

//    DBCommand *command = sql_generator_->getCountStatement(type, sensor_number);

//    DBResult *result = connection_->execute(command);

//    delete command;

//    return result;
//}

bool DBInterface::hasActiveDataSources (const DBObject &object)
{
    if (!existsPropertiesTable())
        return false;

    return hasProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX+object.name());
}

/**
 * Gets active sensor numbers as property, splits it and packs it into a set.
 */
std::set<int> DBInterface::getActiveDataSources (const DBObject &object)
{
    logdbg  << "DBInterface: getActiveDataSources: start";

    std::string tmp = getProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX+object.name());

    std::set<int> ret;

    std::vector<std::string> tmp2 = String::split(tmp, ',');

    logdbg  << "DBInterface: getActiveDataSources: got "<< tmp2.size() << " parts from '" << tmp << "'" ;

    for (unsigned int cnt=0; cnt < tmp2.size(); cnt++)
    {
        ret.insert (std::stoi(tmp2.at(cnt)));
        logdbg  << "DBInterface: getActiveDataSources: got active radar "<< cnt << " '" << std::stoi(tmp2.at(cnt)) << "'" ;
    }

    logdbg  << "DBInterface: getActiveDataSources: end";
    return ret;
}

//void DBInterface::writeBuffer (Buffer *data)
//{
//    if (!buffer_writer_)
//        buffer_writer_ = new BufferWriter (connection_, sql_generator_);

//    std::scoped_lock l(mutex_);

//    assert (buffer_writer_);
//    assert (data);

//    std::string type = data->dboType();

//    //TODO FIXME
//    assert (false);

//    //assert (write_table_names_.find(type) != write_table_names_.end());

//    //buffer_writer_->write (data, write_table_names_[type]);
//}

//void DBInterface::writeBuffer (Buffer *data, std::string table_name)
//{
//    if (!buffer_writer_)
//        buffer_writer_ = new BufferWriter (connection_, sql_generator_);

//    std::scoped_lock l(mutex_);

//    assert (buffer_writer_);
//    assert (data);

//    buffer_writer_->write (data, table_name);
//}

void DBInterface::updateBuffer (DBObject &object, DBOVariable &key_var, std::shared_ptr<Buffer> buffer)
{
    QMutexLocker locker(&connection_mutex_);

    assert (current_connection_);
    assert (buffer);

    if (!buffer_writer_)
        buffer_writer_ = new BufferWriter (current_connection_, &sql_generator_);

    assert (buffer_writer_);

    const DBTable &table = object.currentMetaTable().mainTable();

    const PropertyList &properties = buffer->properties();

    for (unsigned int cnt=0; cnt < properties.size(); cnt++)
    {
        if (!table.hasColumn(properties.at(cnt).name()))
            throw std::runtime_error ("DBInterface: updateBuffer: column '"+properties.at(cnt).name()+"' does not exist in table "+table.name());
    }

    buffer_writer_->update (buffer, object, key_var, table.name());
}

void DBInterface::prepareRead (const DBObject &dbobject, DBOVariableSet read_list, std::string custom_filter_clause, std::vector <DBOVariable *> filtered_variables,
                               bool use_order, DBOVariable *order_variable, bool use_order_ascending, const std::string &limit)
{
    connection_mutex_.lock();
    assert (current_connection_);

    //    getSelectCommand (const DBObject &object, const PropertyList &variables
    //                                               const std::string &filter, const std::vector <std::string> &filtered_variable_names,  DBOVariable *order,
    //                                               const std::string &limit, bool left_join)

    std::shared_ptr<DBCommand> read = sql_generator_.getSelectCommand (dbobject.currentMetaTable(), read_list, custom_filter_clause, filtered_variables, use_order, order_variable,
                                                                       use_order_ascending, limit, true);
    loginf  << "DBInterface: prepareRead: dbo " << dbobject.name() << " sql '" << read->get() << "'";
    current_connection_->prepareCommand(read);
}

/**
 * Retrieves result from connection stepPreparedCommand, calls activateKeySearch on buffer and returns it.
 */
std::shared_ptr <Buffer> DBInterface::readDataChunk (const DBObject &dbobject, bool activate_key_search)
{
    // locked by prepareRead
    assert (current_connection_);

    std::shared_ptr <DBResult> result = current_connection_->stepPreparedCommand(read_chunk_size_);

    if (!result)
    {
        logerr  << "DBInterface: readDataChunk: connection returned error";
        //TODO inform object
        throw std::runtime_error ("DBInterface: readDataChunk: connection returned error");
    }

    if (!result->containsData())
    {
        logerr  << "DBInterface: readDataChunk: buffer does not contain data";
        //TODO inform object
        throw std::runtime_error ("DBInterface: readDataChunk: buffer does not contain data");
    }

    std::shared_ptr <Buffer> buffer = result->buffer();

    buffer->dboName(dbobject.name());

    assert (buffer);
    if (buffer->firstWrite())
    {
        return buffer; // HACK UGGGA WAS 0
    }

    bool last_one = current_connection_->getPreparedCommandDone();
    buffer->lastOne (last_one);

    assert (!activate_key_search); // TODO FIXXXXME

    //    if (activate_key_search)
    //    {
    //        assert (DBObjectManager::getInstance().existsDBOVariable (DBO_UNDEFINED, "id"));
    //        assert (DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id")->existsIn (type));
    //        std::string id_name = DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id")->getFor(type)->getName();
    //        assert (buffer->getPropertyList()->hasProperty(id_name));
    //        logdbg << "DBInterface: readDataChunk: key search id " << id_name << " index " << buffer->getPropertyList()->getPropertyIndex(id_name)
    //                                                    << " buffer first " << buffer->getFirstWrite() << " size " << buffer->getSize();
    //        buffer->activateKeySearch(buffer->getPropertyList()->getPropertyIndex(id_name));
    //    }

    return buffer;
}


void DBInterface::finalizeReadStatement (const DBObject &dbobject)
{
    connection_mutex_.unlock();
    assert (current_connection_);

    logdbg  << "DBInterface: finishReadSystemTracks: start ";
    //prepared_.at(dbobject.name())=false;
    current_connection_->finalizeCommand();
}

void DBInterface::createPropertiesTable ()
{
    assert (!existsPropertiesTable());
    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getTablePropertiesCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo ();
}
void DBInterface::createMinMaxTable ()
{
    assert (!existsMinMaxTable());
    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getTableMinMaxCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo ();
}

///**
// * Gets SQL command from SQLGenerator, executes it, transforms units on results if required, returns buffer.
// *
// * \param type DBO type
// * \param ids Vector with key ids of elements of interest
// * \param list Property list of all data columns of interest
// * \param use_filters Whether to use filter configuration or not
// * \param order_by_varaible Name of data column to be ordered by
// * \param ascending True is ascending, false descending, only used if order_by_varaible set
// * \param limit_min For SQL Limit statement
// * \param limit_max For SQL Limit statement
// */
//Buffer *DBInterface::getInfo (const std::string &type, std::vector<unsigned int> ids, DBOVariableSet read_list,
//        bool use_filters, std::string order_by_variable, bool ascending, unsigned int limit_min,
//        unsigned int limit_max, bool finalize)
//{
//    logdbg  << "DBInterface: getInfo: start";

//    std::scoped_lock l(mutex_);

//    DBCommand *command = sql_generator_->getSelectInfoCommand(type, ids, read_list, use_filters, order_by_variable, ascending, limit_min, limit_max);

//    loginf  << "DBInterface: getInfo: sql '" << command->getCommandString() << "'";

//    DBResult *result = connection_->execute(command);
//    assert (result->containsData());
//    Buffer *buffer = result->getBuffer();
//    buffer->setDBOType (type);
//    delete result;
//    delete command;

//    // TODO FIXME
//    assert (false);

////    if (finalize)
////        finalizeDBData (type, buffer, read_list);

//    return buffer;
//}

//void DBInterface::setContextReferencePoint (bool defined, float latitude, float longitude)
//{
//    loginf  << "DBInterface: setContextReferencePoint: start";

//    if (!defined)
//        return;

//    createPropertiesTable();

//    logdbg  << "DBInterface: setContextReferencePoint: defined";
//    insertProperty("reference_point_defined", String::intToString(defined));
//    logdbg  << "DBInterface: setContextReferencePoint: lat";
//    insertProperty("reference_point_latitude", String::doubleToString(latitude));
//    logdbg  << "DBInterface: setContextReferencePoint: long";
//    insertProperty("reference_point_longitude", String::doubleToString(longitude));
//    logdbg  << "DBInterface: setContextReferencePoint: end";
//}
//bool DBInterface::getContextReferencePointDefined ()
//{
//    if (!hasProperty ("reference_point_defined"))
//        return false;
//    else
//        return (bool) String::intFromString(getProperty ("reference_point_defined"));
//}
//std::pair<float, float> DBInterface::getContextReferencePoint ()
//{
//    std::pair<float, float> point;
//    point.first = String::doubleFromString(getProperty ("reference_point_latitude"));
//    point.second = String::doubleFromString(getProperty ("reference_point_longitude"));
//    return point;
//}



void DBInterface::clearTableContent (const std::string& table_name)
{
    QMutexLocker locker(&connection_mutex_);
    //DELETE FROM tablename;
    current_connection_->executeSQL("DELETE FROM "+table_name+";");
}

std::shared_ptr<DBResult> DBInterface::queryMinMaxNormalForTable (const DBTable& table)
{
    QMutexLocker locker(&connection_mutex_);
    logdbg  << "DBInterface: queryMinMaxForTable: getting command";
    std::shared_ptr <DBCommand> command = sql_generator_.getTableSelectMinMaxNormalStatement (table);

    //loginf  << "DBInterface: queryMinMaxForTable: executing command '" << command->getCommandString() << "'";
    std::shared_ptr<DBResult> result = current_connection_->execute(*command);
    return result;
}

//DBResult *DBInterface::queryMinMaxForColumn (DBTableColumn *column, std::string table)
//{
//    assert (column);
//    assert (table.size() > 0);

//    std::scoped_lock l(mutex_);
//    logdbg  << "DBInterface: queryMinMaxForColumn: getting command";
//    DBCommand *command = sql_generator_->getColumnSelectMinMaxStatement(column, table);

//    logdbg  << "DBInterface: queryMinMaxForTable: executing command";
//    DBResult *result = connection_->execute(command);
//    return result;
//}

//DBResult *DBInterface::getDistinctStatistics (const std::string &type, DBOVariable *variable, unsigned int sensor_number)
//{
//    std::scoped_lock l(mutex_);

//    assert (DBObjectManager::getInstance().existsDBObject(type));
//    assert (variable->existsIn(type));

//    DBCommand *command = sql_generator_->getDistinctStatistics(type, variable, sensor_number);
//    DBResult *result = connection_->execute(command);
//    return result;
//}

//Buffer *DBInterface::createFromMinMaxStringBuffer (Buffer *string_buffer, PropertyDataType type)
//{
//    assert (string_buffer);
//    assert (string_buffer->size() >= 1);

//    PropertyList result_list;
//    result_list.addProperty ("min", type);
//    result_list.addProperty ("max", type);


//    //TODO FIXME
//    assert (false);
//    return 0;

////    Buffer *result_buffer = new Buffer (result_list, DBO_UNDEFINED);

////    bool first=true;

////    logdbg  << "DBInterface: createDataFromStringBuffer: result size " << string_buffer->getSize();

////    for (unsigned int cnt=0; cnt < string_buffer->getSize(); cnt ++)
////    {
////        logdbg  << "DBInterface: createDataFromStringBuffer: checking result " << cnt;
////        if (type == P_TYPE_BOOL)
////        {
////            bool min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            bool max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: bool min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: bool first " << min << " max " << max;
////                *((bool*) result_buffer->get(cnt,0)) = min;
////                *((bool*) result_buffer->get(cnt,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: bool other min " << min << " max " << max;
////                if (min < *((bool*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: bool new min " << min;
////                    *((bool*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((bool*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: bool new max " << max;
////                    *((bool*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_CHAR)
////        {
////            int min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            int max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: char min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: char first " << min << " max " << max;
////                *((char*) result_buffer->get(cnt,0)) = min;
////                *((char*) result_buffer->get(cnt,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: char other min " << min << " max " << max;
////                if (min < *((char*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: char new min " << min;
////                    *((char*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((char*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: char new max " << max;
////                    *((char*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_INT)
////        {
////            int min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            int max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: int min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: int first " << min << " max " << max;
////                *((int*) result_buffer->get(0,0)) = min;
////                *((int*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: int other min " << min << " max " << max;
////                if (min < *((int*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: int new min " << min;
////                    *((int*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((int*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: int new max " << max;
////                    *((int*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_UCHAR)
////        {
////            unsigned int min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            unsigned int max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: uchar min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: uchar first " << min << " max " << max;
////                *((unsigned char*) result_buffer->get(0,0)) = min;
////                *((unsigned char*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: uchar other min " << min << " max " << max;
////                if (min < *((unsigned char*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: uchar new min " << min;
////                    *((unsigned char*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((unsigned char*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: uchar new max " << max;
////                    *((unsigned char*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_UINT)
////        {
////            unsigned int min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            unsigned int max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: uint min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: uint first " << min << " max " << max;
////                *((unsigned int*) result_buffer->get(0,0)) = min;
////                *((unsigned int*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: uint other min " << min << " max " << max;
////                if (min < *((unsigned int*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: uint new min " << min;
////                    *((unsigned int*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((unsigned int*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: uint new max " << max;
////                    *((unsigned int*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_STRING)
////        {
////            std::string min = *((std::string*) string_buffer->get(cnt,0));
////            std::string max = *((std::string*) string_buffer->get(cnt,1));

////            logdbg  << "DBInterface: createDataFromStringBuffer: string min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: string first " << min << " max " << max;
////                *((std::string*) result_buffer->get(0,0)) = min;
////                *((std::string*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: string other min " << min << " max " << max;
////                if (min.compare(*((std::string*) result_buffer->get(0,0))) < 0)
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: string new min " << min;
////                    *((std::string*) result_buffer->get(0,0)) = min;
////                }
////                if (max.compare(*((std::string*) result_buffer->get(0,1))) > 0)
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: string new max " << max;
////                    *((std::string*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_FLOAT)
////        {
////            float min = doubleFromString (*((std::string*) string_buffer->get(cnt,0)));
////            float max = doubleFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: float min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: float first " << min << " max " << max;
////                *((float*) result_buffer->get(0,0)) = min;
////                *((float*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: float other min " << min << " max " << max;
////                if (min < *((float*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: float new min " << min;
////                    *((float*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((float*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: float new max " << max;
////                    *((float*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_DOUBLE)
////        {
////            double min = doubleFromString (*((std::string*) string_buffer->get(cnt,0)));
////            double max = doubleFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: double min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer:double first min " << min << " max " << max;
////                *((double*) result_buffer->get(0,0)) = min;
////                *((double*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: double other min " << min << " max " << max;
////                if (min < *((double*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: double new min " << min;
////                    *((double*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((double*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: double new max " << max;
////                    *((double*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else
////            throw std::runtime_error ("DBInterface: createDataFromStringBuffer: unknown property type");
////    }
////    //delete string_buffer;

////    return result_buffer;
//}

//void DBInterface::deleteAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string filter)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    std::scoped_lock l(mutex_);
//    connection_->executeSQL(sql_generator_->getDeleteStatement(variable->getCurrentDBColumn(), value, filter));
//}

//void DBInterface::updateAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string new_value, std::string filter)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    connection_->executeSQL(sql_generator_->getUpdateStatement(variable->getCurrentDBColumn(), value, new_value, filter));
//}

//void DBInterface::getMinMaxOfVariable (DBOVariable *variable, std::string filter_condition, std::string &min, std::string &max)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    DBCommand command;
//    command.setCommandString(sql_generator_->getMinMaxSelectStatement(variable->getCurrentDBColumn()->getName(),
//            variable->getCurrentDBColumn()->getDBTableName(), filter_condition));

//    PropertyList list;
//    list.addProperty("min", PropertyDataType::STRING);
//    list.addProperty("max", PropertyDataType::STRING);
//    command.setPropertyList(list);

//    DBResult *result = connection_->execute (&command);
//    assert (result->containsData());
//    Buffer *buffer = result->getBuffer();
//    assert (!buffer->firstWrite());
//    assert (buffer->size() == 1);
//    min = buffer->getString("min").get(0);
//    max = buffer->getString("max").get(0);

//    delete result;
//    delete buffer;

//    loginf << "DBInterface: getMinMaxOfVariable: variable " << variable->getName() << " min " << min << " max " << max;
//}

////void DBInterface::getDistinctValues (DBOVariable *variable, std::string filter_condition, std::vector<std::string> &values)
////{
////    assert (sql_generator_);
////
////    assert (!variable->isMetaVariable());
////    assert (variable->hasCurrentDBColumn());
////
////    DBCommand command;
////    command.setCommandString(sql_generator_->getMinMaxSelectStatement(variable->getCurrentDBColumn()->getName(),
////            variable->getCurrentDBColumn()->getDBTableName(), filter_condition));
////
////    PropertyList list;
////    list.addProperty("value", P_TYPE_STRING);
////    command.setPropertyList(list);
////
////    DBResult *result = connection_->execute (&command);
////    assert (result->containsData());
////    Buffer *buffer = result->getBuffer();
////    assert (!buffer->getFirstWrite());
////
////    for (unsigned int cnt=0; cnt < buffer->getSize(); cnt++)
////    {
////        values.push_back(*(std::string *) (buffer->get(0, cnt)));
////        loginf << "DBInterface: getDistinctValues: variable " << variable->getName() << " value: " << *(std::string *) (buffer->get(0, cnt));
////    }
////
////
////}

//Buffer *DBInterface::getTrackMatches (bool has_mode_a, unsigned int mode_a, bool has_ta, unsigned int ta, bool has_ti, std::string ti,
//        bool has_tod, double tod_min, double tod_max)
//{
//    assert (sql_generator_);
//    assert (has_mode_a || has_ta || has_ti);


//    std::stringstream ss;

//    ss << "select track_num, min(tod), max(tod) from sd_track where mode3a_code";

//    if (has_mode_a)
//        ss << "=" << String::intToString(mode_a);
//    else
//        ss << " IS NULL";

//    ss << " AND target_addr";

//    if (has_ta)
//        ss << "=" << String::intToString(ta);
//    else
//        ss << " IS NULL";

//    ss << " AND callsign";

//    if (has_ti)
//        ss << "='" << ti <<"'";
//    else
//        ss << "='        '";

//    if (has_tod)
//        ss << " AND TOD>"+String::doubleToStringNoScientific(tod_min)+" AND TOD<"+String::doubleToStringNoScientific(tod_max);

//    ss << " group by track_num;";

//    loginf << "DBInterface: getTrackMatches: sql " << ss.str() << "'";

//    DBCommand command;
//    command.setCommandString(ss.str());

//    PropertyList list;
//    list.addProperty("track_num", PropertyDataType::INT);
//    list.addProperty("tod_min", PropertyDataType::DOUBLE);
//    list.addProperty("tod_max", PropertyDataType::DOUBLE);
//    command.setPropertyList(list);

//    DBResult *result = connection_->execute (&command);
//    assert (result->containsData());
//    Buffer *buffer = result->getBuffer();

//    delete result;

//    loginf << "DBInterface: getTrackMatches: done";

//    return buffer;
//}

//void DBInterface::updateDBObjectInformationSlot ()
//{
//    logdbg << "DBInterface: updateDBObjectInformationSlot";
//    std::scoped_lock l(reading_done_mutex_);
//    prepared_.clear();
//    reading_done_.clear();
//    exists_.clear();
//    count_.clear();

//    auto objects = ATSDB::instance().objectManager().objects();

//    for (auto it = objects.begin(); it != objects.end(); it++)
//    {
//        if (!it->second->loadable())
//            continue;

//        std::string type = it->first;

//        prepared_[type] = false;
//        reading_done_[type] = true;
//        exists_[type]=false;
//        count_[type]=0;
//    }
//}

//void DBInterface::testReading ()
//{
//    loginf << "DBInterface: testReading";

//    boost::posix_time::ptime start_time;
//    boost::posix_time::ptime stop_time;

//    DBObject &object = ATSDB::instance().objectManager().object("MLAT");
//    DBOVariableSet read_list;

//    loginf << "DBInterface: testReading: adding all variables";
//    for (auto variable_it : object.variables())
//        read_list.add(variable_it.second);

//    loginf << "DBInterface: testReading: preparing reading";
//    prepareRead (object, read_list); //, std::string custom_filter_clause="", DBOVariable *order=0);

//    start_time = boost::posix_time::microsec_clock::local_time();

//    loginf << "DBInterface: testReading: starting reading";
//    std::vector<std::shared_ptr <Buffer>> buffer_vector;

//    unsigned int num_rows=0;

//    while (!getReadingDone(object))
//    {
//        std::shared_ptr <Buffer> buffer = readDataChunk (object, false);
//        buffer_vector.push_back(buffer);

//        stop_time = boost::posix_time::microsec_clock::local_time();
//        boost::posix_time::time_duration diff = stop_time - start_time;

//        num_rows += buffer->size();
//        if (diff.total_seconds() > 0)
//            loginf << "DBInterface: testReading: got buffer size " << buffer->size() << " all " << num_rows << " elapsed " << diff << " #el/sec " << num_rows/diff.total_seconds();
//    }

//    boost::posix_time::time_duration diff = stop_time - start_time;
//    loginf << "DBInterface: testReading: reading done: all " << num_rows << " elapsed " << diff << " #el/sec " << num_rows/diff.total_seconds();
//    finalizeReadStatement (object);


//    loginf << "DBInterface: testReading: clearing buffers";
//    buffer_vector.clear();

//    loginf << "DBInterface: testReading: done";
//}
