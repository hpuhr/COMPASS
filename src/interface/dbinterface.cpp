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

#include "dbinterface.h"

#include <QApplication>
#include <QMessageBox>
#include <QMutexLocker>
#include <QThread>

#include "atsdb.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "buffer.h"
#include "config.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "dbconnection.h"
#include "dbinterfaceinfowidget.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbodatasource.h"
#include "dbovariable.h"
#include "dbresult.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbtable.h"
#include "dbtableinfo.h"
#include "dimension.h"
#include "jobmanager.h"
#include "metadbtable.h"
#include "mysqlppconnection.h"
#include "mysqlserver.h"
#include "sqliteconnection.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"
#include "sector.h"

using namespace Utils;
using namespace std;

/**
 * Creates SQLGenerator, several containers based in DBOs (prepared_, reading_done_, exists_,
 * count_), creates write_table_names_,
 */
DBInterface::DBInterface(string class_id, string instance_id, ATSDB* atsdb)
    : Configurable(class_id, instance_id, atsdb), sql_generator_(*this)
{
    QMutexLocker locker(&connection_mutex_);

    registerParameter("read_chunk_size", &read_chunk_size_, 50000);
    registerParameter("used_connection", &used_connection_, "");

    createSubConfigurables();

    if (used_connection_.size())
    {
        if (!connections_.count(used_connection_))
        {
            logerr << "DBInterface: constructor: unknown connection '" << used_connection_ << "'";
            used_connection_ = "";
        }
        else
        {
            current_connection_ = connections_.at(used_connection_);
        }
    }
}

/**
 * If required, deletes connection, sql_generator_ and buffer_writer_.
 */
DBInterface::~DBInterface()
{
    logdbg << "DBInterface: desctructor: start";

    QMutexLocker locker(&connection_mutex_);

    for (auto it : connections_)
        delete it.second;

    connections_.clear();

    logdbg << "DBInterface: desctructor: end";
}

/**
 * Generates connection based on the DB_CONNECTION_TYPE of info, calls init on it. If a new database
 * will be created, creates the buffer_writer_, else calls updateExists and updateCount.
 */
void DBInterface::useConnection(const string& connection_type)
{
    logdbg << "DBInterface: useConnection: '" << connection_type << "'";
    if (current_connection_)
        assert(!current_connection_->ready());

    current_connection_ = connections_.at(connection_type);
    used_connection_ = connection_type;

    assert(current_connection_);
}

void DBInterface::databaseContentChanged()
{
    updateTableInfo();

    if (!existsPropertiesTable())
        createPropertiesTable();

    loadProperties();

    if (!existsSectorsTable())
        createSectorsTable();

    loadSectors();

    emit databaseContentChangedSignal();
}

void DBInterface::closeConnection()
{
    QMutexLocker locker(&connection_mutex_);

    saveProperties();

    logdbg << "DBInterface: closeConnection";
    for (auto it : connections_)
        it.second->disconnect();

    if (info_widget_)
    {
        delete info_widget_;
        info_widget_ = nullptr;
    }

    table_info_.clear();
    logdbg << "DBInterface: closeConnection: done";
}

void DBInterface::updateTableInfo()
{
    QMutexLocker locker(&connection_mutex_);
    loginf << "DBInterface: updateTableInfo";
    table_info_.clear();

    assert(current_connection_);
    table_info_ = current_connection_->getTableInfo();

    loginf << "DBInterface: updateTableInfo: found " << table_info_.size() << " tables";
}

DBInterfaceInfoWidget* DBInterface::infoWidget()
{
    if (!info_widget_)
    {
        info_widget_ = new DBInterfaceInfoWidget(*this);
    }

    assert(info_widget_);
    return info_widget_;
}

QWidget* DBInterface::connectionWidget()
{
    assert(current_connection_);
    return current_connection_->widget();
}

vector<string> DBInterface::getDatabases()
{
    assert(current_connection_);
    return current_connection_->getDatabases();
}

bool DBInterface::ready()
{
    if (!current_connection_)
    {
        logdbg << "DBInterface: ready: no connection";
        return false;
    }

    logdbg << "DBInterface: ready: connection ready " << current_connection_->ready();
    return current_connection_->ready();
}

DBConnection& DBInterface::connection()
{
    assert(ready());
    return *current_connection_;
}

void DBInterface::generateSubConfigurable(const string& class_id,
                                          const string& instance_id)
{
    logdbg << "DBInterface: generateSubConfigurable: generating variable " << instance_id;
    if (class_id == "MySQLppConnection")
    {
        MySQLppConnection* connection = new MySQLppConnection(class_id, instance_id, this);
        assert(connections_.count(connection->instanceId()) == 0);
        connections_.insert(pair<string, DBConnection*>(
                                connection->instanceId(), dynamic_cast<DBConnection*>(connection)));
    }
    else if (class_id == "SQLiteConnection")
    {
        SQLiteConnection* connection = new SQLiteConnection(class_id, instance_id, this);
        assert(connections_.count(connection->instanceId()) == 0);
        connections_.insert(pair<string, DBConnection*>(
                                connection->instanceId(), dynamic_cast<DBConnection*>(connection)));
    }
    else
        throw runtime_error("DBInterface: generateSubConfigurable: unknown class_id " +
                            class_id);
}

void DBInterface::checkSubConfigurables()
{
    if (connections_.count("MySQL++ Connection") == 0)
    {
        addNewSubConfiguration("MySQLppConnection", "MySQL++ Connection");
        generateSubConfigurable("MySQLppConnection", "MySQL++ Connection");
    }

    if (connections_.count("SQLite Connection") == 0)
    {
        addNewSubConfiguration("SQLiteConnection", "SQLite Connection");
        generateSubConfigurable("SQLiteConnection", "SQLite Connection");
    }
}

bool DBInterface::existsTable(const string& table_name)
{
    QMutexLocker locker(&connection_mutex_);  // TODO
    return table_info_.count(table_name) == 1;
}

void DBInterface::createTable(DBTable& table)
{
    loginf << "DBInterface: createTable: " << table.name();
    if (existsTable(table.name()))
    {
        logerr << "DBInterface: createTable: table " << table.name() << " already exists";
        return;
    }

    string statement = sql_generator_.getCreateTableStatement(table);

    QMutexLocker locker(&connection_mutex_);

    current_connection_->executeSQL(statement);

    locker.unlock();

    updateTableInfo();
    table.updateOnDatabase();

    loginf << "DBInterface: createTable: checking " << table.name();
    assert(existsTable(table.name()));
    assert(table.existsInDB());
}

/**
 * Returns existsTable for table name.
 */
bool DBInterface::existsMinMaxTable() { return existsTable(TABLE_NAME_MINMAX); }

/**
 * Returns existsTable for table name.
 */
bool DBInterface::existsPropertiesTable() { return existsTable(TABLE_NAME_PROPERTIES); }

///**
// * Gets SQL command for data sources list and packs the resulting buffer into a set, which is
// returned.
// */
set<int> DBInterface::queryActiveSensorNumbers(DBObject& object)
{
    logdbg << "DBInterface: queryActiveSensorNumbers: start";

    assert(object.existsInDB());
    assert(object.hasCurrentDataSourceDefinition());

    QMutexLocker locker(&connection_mutex_);

    string local_key_dbovar = object.currentDataSourceDefinition().localKey();
    assert(object.hasVariable(local_key_dbovar));
    const DBTableColumn& local_key_col = object.variable(local_key_dbovar).currentDBColumn();

    set<int> data;

    shared_ptr<DBCommand> command = sql_generator_.getDistinctDataSourcesSelectCommand(object);

    shared_ptr<DBResult> result = current_connection_->execute(*command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();
    for (unsigned int cnt = 0; cnt < buffer->size(); cnt++)
    {
        if (buffer->get<int>(local_key_col.name()).isNull(cnt))
        {
            logwrn << "DBInterface: queryActiveSensorNumbers: object " << object.name()
                   << " has NULL ds_id's, which will be omitted";
        }
        else
        {
            int tmp = buffer->get<int>(local_key_col.name()).get(cnt);
            data.insert(tmp);
        }
    }

    logdbg << "DBInterface: queryActiveSensorNumbers: done";
    return data;
}

bool DBInterface::hasDataSourceTables(DBObject& object)
{
    if (!object.hasCurrentDataSourceDefinition())
        return false;

    const DBODataSourceDefinition& ds = object.currentDataSourceDefinition();
    const DBSchema& schema = ATSDB::instance().schemaManager().getCurrentSchema();

    if (!schema.hasMetaTable(ds.metaTableName()))
        return false;

    const MetaDBTable& meta = schema.metaTable(ds.metaTableName());

    if (!meta.hasColumn(ds.foreignKey()))
        return false;

    if (!meta.hasColumn(ds.nameColumn()))
        return false;

    string main_table_name = meta.mainTableName();

    if (!existsTable(main_table_name))
        return false;

    return true;
}

void DBInterface::updateDataSource(DBODataSource& data_source)
{
    loginf << "DBInterface: updateDataSource: object " << data_source.object().name() << " source "
           << data_source.id();

    DBObject& object = data_source.object();
    shared_ptr<Buffer> buffer{new Buffer()};

    const DBODataSourceDefinition& ds_def = object.currentDataSourceDefinition();
    const DBSchema& schema = ATSDB::instance().schemaManager().getCurrentSchema();
    assert(schema.hasMetaTable(ds_def.metaTableName()));

    MetaDBTable& meta = schema.metaTable(ds_def.metaTableName());

    const DBTableColumn& foreign_key_col = meta.column(ds_def.foreignKey());

    const DBTableColumn& name_col = meta.column(ds_def.nameColumn());
    assert(name_col.propertyType() == PropertyDataType::STRING);
    buffer->addProperty(name_col.name(), name_col.propertyType());
    buffer->get<string>(name_col.name()).set(0, data_source.name());

    if (ds_def.hasShortNameColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.shortNameColumn());
        assert(col.propertyType() == PropertyDataType::STRING);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasShortName())
            buffer->get<string>(col.name()).set(0, data_source.shortName());
        else
            buffer->get<string>(col.name()).setNull(0);
    }

    if (ds_def.hasSacColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.sacColumn());
        assert(col.propertyType() == PropertyDataType::CHAR);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasSac())
            buffer->get<char>(col.name()).set(0, data_source.sac());
        else
            buffer->get<char>(col.name()).setNull(0);
    }

    if (ds_def.hasSicColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.sicColumn());
        assert(col.propertyType() == PropertyDataType::CHAR);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasSic())
            buffer->get<char>(col.name()).set(0, data_source.sic());
        else
            buffer->get<char>(col.name()).setNull(0);
    }

    if (ds_def.hasLatitudeColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.latitudeColumn());
        assert(col.propertyType() == PropertyDataType::DOUBLE);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasLatitude())
            buffer->get<double>(col.name()).set(0, data_source.latitude());
        else
            buffer->get<double>(col.name()).setNull(0);
    }

    if (ds_def.hasLongitudeColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.longitudeColumn());
        assert(col.propertyType() == PropertyDataType::DOUBLE);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasLongitude())
            buffer->get<double>(col.name()).set(0, data_source.longitude());
        else
            buffer->get<double>(col.name()).setNull(0);
    }

    if (ds_def.hasAltitudeColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.altitudeColumn());
        assert(col.propertyType() == PropertyDataType::DOUBLE);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasAltitude())
            buffer->get<double>(col.name()).set(0, data_source.altitude());
        else
            buffer->get<double>(col.name()).setNull(0);
    }

    // psr
    if (ds_def.hasPrimaryAzimuthStdDevColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.primaryAzimuthStdDevColumn());
        assert(col.propertyType() == PropertyDataType::DOUBLE);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasPrimaryAzimuthStdDev())
            buffer->get<double>(col.name()).set(0, data_source.primaryAzimuthStdDev());
        else
            buffer->get<double>(col.name()).setNull(0);
    }

    if (ds_def.hasPrimaryRangeStdDevColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.primaryRangeStdDevColumn());
        assert(col.propertyType() == PropertyDataType::DOUBLE);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasPrimaryRangeStdDev())
            buffer->get<double>(col.name()).set(0, data_source.primaryRangeStdDev());
        else
            buffer->get<double>(col.name()).setNull(0);
    }

    if (ds_def.hasPrimaryIRMinColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.primaryIRMinColumn());
        assert(col.propertyType() == PropertyDataType::CHAR);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasPrimaryRangeMin())
            buffer->get<char>(col.name()).set(0, data_source.primaryRangeMin());
        else
            buffer->get<char>(col.name()).setNull(0);
    }

    if (ds_def.hasPrimaryIRMaxColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.primaryIRMaxColumn());
        assert(col.propertyType() == PropertyDataType::INT);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasPrimaryRangeMax())
            buffer->get<int>(col.name()).set(0, data_source.primaryRangeMax());
        else
            buffer->get<int>(col.name()).setNull(0);
    }

    // ssr
    if (ds_def.hasSecondaryAzimuthStdDevColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.secondaryAzimuthStdDevColumn());
        assert(col.propertyType() == PropertyDataType::DOUBLE);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasSecondaryAzimuthStdDev())
            buffer->get<double>(col.name()).set(0, data_source.secondaryAzimuthStdDev());
        else
            buffer->get<double>(col.name()).setNull(0);
    }

    if (ds_def.hasSecondaryRangeStdDevColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.secondaryRangeStdDevColumn());
        assert(col.propertyType() == PropertyDataType::DOUBLE);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasSecondaryRangeStdDev())
            buffer->get<double>(col.name()).set(0, data_source.secondaryRangeStdDev());
        else
            buffer->get<double>(col.name()).setNull(0);
    }

    if (ds_def.hasSecondaryIRMinColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.secondaryIRMinColumn());
        assert(col.propertyType() == PropertyDataType::INT);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasSecondaryRangeMin())
            buffer->get<int>(col.name()).set(0, data_source.secondaryRangeMin());
        else
            buffer->get<int>(col.name()).setNull(0);
    }

    if (ds_def.hasSecondaryIRMaxColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.secondaryIRMaxColumn());
        assert(col.propertyType() == PropertyDataType::INT);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasSecondaryRangeMax())
            buffer->get<int>(col.name()).set(0, data_source.secondaryRangeMax());
        else
            buffer->get<int>(col.name()).setNull(0);
    }

    // mode s
    if (ds_def.hasModeSAzimuthStdDevColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.modeSAzimuthStdDevColumn());
        assert(col.propertyType() == PropertyDataType::DOUBLE);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasModeSAzimuthStdDev())
            buffer->get<double>(col.name()).set(0, data_source.modeSAzimuthStdDev());
        else
            buffer->get<double>(col.name()).setNull(0);
    }

    if (ds_def.hasModeSRangeStdDevColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.modeSRangeStdDevColumn());
        assert(col.propertyType() == PropertyDataType::DOUBLE);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasModeSRangeStdDev())
            buffer->get<double>(col.name()).set(0, data_source.modeSRangeStdDev());
        else
            buffer->get<double>(col.name()).setNull(0);
    }

    if (ds_def.hasModeSIRMinColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.modeSIRMinColumn());
        assert(col.propertyType() == PropertyDataType::CHAR);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasModeSRangeMin())
            buffer->get<char>(col.name()).set(0, data_source.modeSRangeMin());
        else
            buffer->get<char>(col.name()).setNull(0);
    }

    if (ds_def.hasModeSIRMaxColumn())
    {
        const DBTableColumn& col = meta.column(ds_def.modeSIRMaxColumn());
        assert(col.propertyType() == PropertyDataType::INT);
        buffer->addProperty(col.name(), col.propertyType());
        if (data_source.hasModeSRangeMax())
            buffer->get<int>(col.name()).set(0, data_source.modeSRangeMax());
        else
            buffer->get<int>(col.name()).setNull(0);
    }

    assert(foreign_key_col.propertyType() == PropertyDataType::INT);
    buffer->addProperty(foreign_key_col.name(), foreign_key_col.propertyType());
    buffer->get<int>(foreign_key_col.name()).set(0, data_source.id());

    loginf << "DBInterface: updateDataSource: updating";

    updateBuffer(meta, foreign_key_col, buffer);

    loginf << "DBInterface: updateDataSource: update done";
}

///**
// * Gets SQL command, executes it and returns resulting buffer.
// */
map<int, DBODataSource> DBInterface::getDataSources(DBObject& object)
{
    logdbg << "DBInterface: getDataSources: start";

    QMutexLocker locker(&connection_mutex_);

    shared_ptr<DBCommand> command = sql_generator_.getDataSourcesSelectCommand(object);

    logdbg << "DBInterface: getDataSources: sql '" << command->get() << "'";

    shared_ptr<DBResult> result = current_connection_->execute(*command);
    assert(result->containsData());
    shared_ptr<Buffer> buffer = result->buffer();

    logdbg << "DBInterface: getDataSources: json '" << buffer->asJSON().dump(4) << "'";

    const DBODataSourceDefinition& ds = object.currentDataSourceDefinition();
    const DBSchema& schema = ATSDB::instance().schemaManager().getCurrentSchema();
    assert(schema.hasMetaTable(ds.metaTableName()));

    const MetaDBTable& meta = schema.metaTable(ds.metaTableName());

    const DBTableColumn& foreign_key_col = meta.column(ds.foreignKey());
    const DBTableColumn& name_col = meta.column(ds.nameColumn());

    assert(buffer->properties().hasProperty(foreign_key_col.name()));
    assert(buffer->properties().get(foreign_key_col.name()).dataType() == PropertyDataType::INT);
    assert(buffer->properties().hasProperty(name_col.name()));
    assert(buffer->properties().get(name_col.name()).dataType() == PropertyDataType::STRING);

    bool has_short_name = ds.hasShortNameColumn();
    string short_name_col_name;
    if (has_short_name)
    {
        short_name_col_name = meta.column(ds.shortNameColumn()).name();
        assert(buffer->properties().hasProperty(short_name_col_name) &&
               buffer->properties().get(short_name_col_name).dataType() ==
               PropertyDataType::STRING);
    }

    bool has_sac = ds.hasSacColumn();
    bool has_int_sacsic = false;
    string sac_col_name;
    if (has_sac)
    {
        sac_col_name = meta.column(ds.sacColumn()).name();
        assert(buffer->properties().hasProperty(sac_col_name));

        if (buffer->properties().get(sac_col_name).dataType() == PropertyDataType::INT)
            has_int_sacsic = true;
        else
            assert(buffer->properties().get(sac_col_name).dataType() == PropertyDataType::CHAR);
    }

    bool has_sic = ds.hasSicColumn();
    string sic_col_name;
    if (has_sic)
    {
        sic_col_name = meta.column(ds.sicColumn()).name();
        assert(buffer->properties().hasProperty(sic_col_name));

        if (has_int_sacsic)
            assert(buffer->properties().get(sic_col_name).dataType() == PropertyDataType::INT);
        else
            assert(buffer->properties().get(sic_col_name).dataType() == PropertyDataType::CHAR);
    }

    bool has_latitude = ds.hasLatitudeColumn();
    string latitude_col_name;
    if (has_latitude)
    {
        latitude_col_name = meta.column(ds.latitudeColumn()).name();
        assert(buffer->properties().hasProperty(latitude_col_name) &&
               buffer->properties().get(latitude_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_longitude = ds.hasLongitudeColumn();
    string longitude_col_name;
    if (has_longitude)
    {
        longitude_col_name = meta.column(ds.longitudeColumn()).name();
        assert(buffer->properties().hasProperty(longitude_col_name) &&
               buffer->properties().get(longitude_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_altitude = ds.hasAltitudeColumn();
    string altitude_col_name;
    if (has_altitude)
    {
        altitude_col_name = meta.column(ds.altitudeColumn()).name();
        assert(buffer->properties().hasProperty(altitude_col_name) &&
               buffer->properties().get(altitude_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    // psr
    bool has_primary_azimuth_stddev = ds.hasPrimaryAzimuthStdDevColumn();
    string primary_azimuth_stddev_col_name;
    if (has_primary_azimuth_stddev)
    {
        primary_azimuth_stddev_col_name = meta.column(ds.primaryAzimuthStdDevColumn()).name();
        assert(buffer->properties().hasProperty(primary_azimuth_stddev_col_name) &&
               buffer->properties().get(primary_azimuth_stddev_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_primary_range_stddev = ds.hasPrimaryRangeStdDevColumn();
    string primary_range_stddev_col_name;
    if (has_primary_range_stddev)
    {
        primary_range_stddev_col_name = meta.column(ds.primaryRangeStdDevColumn()).name();
        assert(buffer->properties().hasProperty(primary_range_stddev_col_name) &&
               buffer->properties().get(primary_range_stddev_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_primary_ir_min = ds.hasPrimaryIRMinColumn();
    string primary_ir_min_col_name;
    if (has_primary_ir_min)
    {
        primary_ir_min_col_name = meta.column(ds.primaryIRMinColumn()).name();
        assert(buffer->properties().hasProperty(primary_ir_min_col_name) &&
               buffer->properties().get(primary_ir_min_col_name).dataType() == PropertyDataType::CHAR);
    }

    bool has_primary_ir_max = ds.hasPrimaryIRMaxColumn();
    string primary_ir_max_col_name;
    if (has_primary_ir_max)
    {
        primary_ir_max_col_name = meta.column(ds.primaryIRMaxColumn()).name();
        assert(buffer->properties().hasProperty(primary_ir_max_col_name) &&
               buffer->properties().get(primary_ir_max_col_name).dataType() == PropertyDataType::INT);
    }

    // ssr
    bool has_secondary_azimuth_stddev = ds.hasSecondaryAzimuthStdDevColumn();
    string secondary_azimuth_stddev_col_name;
    if (has_secondary_azimuth_stddev)
    {
        secondary_azimuth_stddev_col_name = meta.column(ds.secondaryAzimuthStdDevColumn()).name();
        assert(buffer->properties().hasProperty(secondary_azimuth_stddev_col_name) &&
               buffer->properties().get(secondary_azimuth_stddev_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_secondary_range_stddev = ds.hasSecondaryRangeStdDevColumn();
    string secondary_range_stddev_col_name;
    if (has_secondary_range_stddev)
    {
        secondary_range_stddev_col_name = meta.column(ds.secondaryRangeStdDevColumn()).name();
        assert(buffer->properties().hasProperty(secondary_range_stddev_col_name) &&
               buffer->properties().get(secondary_range_stddev_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_secondary_ir_min = ds.hasSecondaryIRMinColumn();
    string secondary_ir_min_col_name;
    if (has_secondary_ir_min)
    {
        secondary_ir_min_col_name = meta.column(ds.secondaryIRMinColumn()).name();
        assert(buffer->properties().hasProperty(secondary_ir_min_col_name) &&
               buffer->properties().get(secondary_ir_min_col_name).dataType() == PropertyDataType::INT);
    }

    bool has_secondary_ir_max = ds.hasSecondaryIRMaxColumn();
    string secondary_ir_max_col_name;
    if (has_secondary_ir_max)
    {
        secondary_ir_max_col_name = meta.column(ds.secondaryIRMaxColumn()).name();
        assert(buffer->properties().hasProperty(secondary_ir_max_col_name) &&
               buffer->properties().get(secondary_ir_max_col_name).dataType() == PropertyDataType::INT);
    }

    // mode s
    bool has_mode_s_azimuth_stddev = ds.hasModeSAzimuthStdDevColumn();
    string mode_s_azimuth_stddev_col_name;
    if (has_mode_s_azimuth_stddev)
    {
        mode_s_azimuth_stddev_col_name = meta.column(ds.modeSAzimuthStdDevColumn()).name();
        assert(buffer->properties().hasProperty(mode_s_azimuth_stddev_col_name) &&
               buffer->properties().get(mode_s_azimuth_stddev_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_mode_s_range_stddev = ds.hasModeSRangeStdDevColumn();
    string mode_s_range_stddev_col_name;
    if (has_mode_s_range_stddev)
    {
        mode_s_range_stddev_col_name = meta.column(ds.modeSRangeStdDevColumn()).name();
        assert(buffer->properties().hasProperty(mode_s_range_stddev_col_name) &&
               buffer->properties().get(mode_s_range_stddev_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_mode_s_ir_min = ds.hasModeSIRMinColumn();
    string mode_s_ir_min_col_name;
    if (has_mode_s_ir_min)
    {
        mode_s_ir_min_col_name = meta.column(ds.modeSIRMinColumn()).name();
        assert(buffer->properties().hasProperty(mode_s_ir_min_col_name) &&
               buffer->properties().get(mode_s_ir_min_col_name).dataType() == PropertyDataType::CHAR);
    }

    bool has_mode_s_ir_max = ds.hasModeSIRMaxColumn();
    string mode_s_ir_max_col_name;
    if (has_mode_s_ir_max)
    {
        mode_s_ir_max_col_name = meta.column(ds.modeSIRMaxColumn()).name();
        assert(buffer->properties().hasProperty(mode_s_ir_max_col_name) &&
               buffer->properties().get(mode_s_ir_max_col_name).dataType() == PropertyDataType::INT);
    }

    map<int, DBODataSource> sources;

    for (unsigned cnt = 0; cnt < buffer->size(); cnt++)
    {
        if (buffer->get<int>(foreign_key_col.name()).isNull(cnt))
        {
            loginf << "DBInterface: getDataSources: object " << object.name()
                   << " has NULL key, which will be omitted";
            continue;
        }

        if (buffer->get<string>(name_col.name()).isNull(cnt))
        {
            loginf << "DBInterface: getDataSources: object " << object.name()
                   << " has NULL name, which will be omitted";
            continue;
        }

        int key = buffer->get<int>(foreign_key_col.name()).get(cnt);
        string name = buffer->get<string>(name_col.name()).get(cnt);

        assert(sources.count(key) == 0);
        logdbg << "DBInterface: getDataSources: object " << object.name() << " key " << key
               << " name " << name;
        // sources.insert(make_pair(key, DBODataSource(key, name)));

        sources.emplace(piecewise_construct,
                        forward_as_tuple(key),                 // args for key
                        forward_as_tuple(object, key, name));  // args for mapped value

        if (has_short_name && !buffer->get<string>(short_name_col_name).isNull(cnt))
            sources.at(key).shortName(buffer->get<string>(short_name_col_name).get(cnt));

        if (has_sac)
        {
            if (has_int_sacsic)
            {
                if (!buffer->get<int>(sac_col_name).isNull(cnt))
                    sources.at(key).sac(buffer->get<int>(sac_col_name).get(cnt));
            }
            else
            {
                if (!buffer->get<char>(sac_col_name).isNull(cnt))
                    sources.at(key).sac(buffer->get<char>(sac_col_name).get(cnt));
            }
        }

        if (has_sic)
        {
            if (has_int_sacsic)
            {
                if (!buffer->get<int>(sic_col_name).isNull(cnt))
                    sources.at(key).sic(buffer->get<int>(sic_col_name).get(cnt));
            }
            else
            {
                if (!buffer->get<char>(sic_col_name).isNull(cnt))
                    sources.at(key).sic(buffer->get<char>(sic_col_name).get(cnt));
            }
        }

        if (has_latitude && !buffer->get<double>(latitude_col_name).isNull(cnt))
            sources.at(key).latitude(buffer->get<double>(latitude_col_name).get(cnt));

        if (has_longitude && !buffer->get<double>(longitude_col_name).isNull(cnt))
            sources.at(key).longitude(buffer->get<double>(longitude_col_name).get(cnt));

        if (has_altitude && !buffer->get<double>(altitude_col_name).isNull(cnt))
            sources.at(key).altitude(buffer->get<double>(altitude_col_name).get(cnt));

        // psr
        if (has_primary_azimuth_stddev && !buffer->get<double>(primary_azimuth_stddev_col_name).isNull(cnt))
            sources.at(key).primaryAzimuthStdDev(buffer->get<double>(primary_azimuth_stddev_col_name).get(cnt));

        if (has_primary_range_stddev && !buffer->get<double>(primary_range_stddev_col_name).isNull(cnt))
            sources.at(key).primaryRangeStdDev(buffer->get<double>(primary_range_stddev_col_name).get(cnt));

        if (has_primary_ir_min && !buffer->get<char>(primary_ir_min_col_name).isNull(cnt))
            sources.at(key).primaryRangeMin(buffer->get<char>(primary_ir_min_col_name).get(cnt));

        if (has_primary_ir_max && !buffer->get<int>(primary_ir_max_col_name).isNull(cnt))
            sources.at(key).primaryRangeMax(buffer->get<int>(primary_ir_max_col_name).get(cnt));

        // ssr
        if (has_secondary_azimuth_stddev && !buffer->get<double>(secondary_azimuth_stddev_col_name).isNull(cnt))
            sources.at(key).secondaryAzimuthStdDev(buffer->get<double>(secondary_azimuth_stddev_col_name).get(cnt));

        if (has_secondary_range_stddev && !buffer->get<double>(secondary_range_stddev_col_name).isNull(cnt))
            sources.at(key).secondaryRangeStdDev(buffer->get<double>(secondary_range_stddev_col_name).get(cnt));

        if (has_secondary_ir_min && !buffer->get<int>(secondary_ir_min_col_name).isNull(cnt))
            sources.at(key).secondaryRangeMin(buffer->get<int>(secondary_ir_min_col_name).get(cnt));

        if (has_secondary_ir_max && !buffer->get<int>(secondary_ir_max_col_name).isNull(cnt))
            sources.at(key).secondaryRangeMax(buffer->get<int>(secondary_ir_max_col_name).get(cnt));

        // mode s
        if (has_mode_s_azimuth_stddev && !buffer->get<double>(mode_s_azimuth_stddev_col_name).isNull(cnt))
            sources.at(key).modeSAzimuthStdDev(buffer->get<double>(mode_s_azimuth_stddev_col_name).get(cnt));

        if (has_mode_s_range_stddev && !buffer->get<double>(mode_s_range_stddev_col_name).isNull(cnt))
            sources.at(key).modeSRangeStdDev(buffer->get<double>(mode_s_range_stddev_col_name).get(cnt));

        if (has_mode_s_ir_min && !buffer->get<char>(mode_s_ir_min_col_name).isNull(cnt))
            sources.at(key).modeSRangeMin(buffer->get<char>(mode_s_ir_min_col_name).get(cnt));

        if (has_mode_s_ir_max && !buffer->get<int>(mode_s_ir_max_col_name).isNull(cnt))
            sources.at(key).modeSRangeMax(buffer->get<int>(mode_s_ir_max_col_name).get(cnt));

        // removed json content

        //sources.at(key).dbContent(buffer->asJSON());

        //sources.at(key).print();
    }

    return sources;
}

size_t DBInterface::count(const string& table)
{
    logdbg << "DBInterface: count: table " << table;
    assert(existsTable(table));

    QMutexLocker locker(&connection_mutex_);
    assert(current_connection_);

    string sql = sql_generator_.getCountStatement(table);

    logdbg << "DBInterface: count: sql '" << sql << "'";

    DBCommand command;
    command.set(sql);

    PropertyList list;
    list.addProperty("count", PropertyDataType::INT);
    command.list(list);

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result->containsData());
    int tmp = result->buffer()->get<int>("count").get(0);

    logdbg << "DBInterface: count: " << table << ": " << tmp << " end";
    return static_cast<size_t>(tmp);
}

void DBInterface::setProperty(const string& id, const string& value)
{
    properties_[id] = value;
}

string DBInterface::getProperty(const string& id)
{
    assert(hasProperty(id));
    return properties_.at(id);
}

bool DBInterface::hasProperty(const string& id) { return properties_.count(id); }

void DBInterface::loadProperties()
{
    loginf << "DBInterface: loadProperties";

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllPropertiesStatement());

    PropertyList list;
    list.addProperty("id", PropertyDataType::STRING);
    list.addProperty("value", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    assert(buffer->has<string>("id"));
    assert(buffer->has<string>("value"));

    NullableVector<string> id_vec = buffer->get<string>("id");
    NullableVector<string> value_vec = buffer->get<string>("value");

    for (size_t cnt = 0; cnt < buffer->size(); ++cnt)
    {
        assert(!id_vec.isNull(cnt));
        assert(!properties_.count(id_vec.get(cnt)));
        if (!value_vec.isNull(cnt))
            properties_[id_vec.get(cnt)] = value_vec.get(cnt);
    }

    for (auto& prop_it : properties_)
        loginf << "DBInterface: loadProperties: id '" << prop_it.first << "' value '"
               << prop_it.second << "'";
}

void DBInterface::saveProperties()
{
    loginf << "DBInterface: saveProperties";

    if (!current_connection_)
    {
        logwrn << "DBInterface: saveProperties: failed since no database connection exists";
        return;
    }

    // QMutexLocker locker(&connection_mutex_); // done in closeConnection
    assert(current_connection_);

    string str;

    for (auto& prop_it : properties_)
    {
        string str = sql_generator_.getInsertPropertyStatement(prop_it.first, prop_it.second);
        current_connection_->executeSQL(str);
    }

    loginf << "DBInterface: saveProperties: done";
}

void DBInterface::loadSectors()
{
    loginf << "DBInterface: loadSectors";

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllSectorsStatement());

    PropertyList list;
    list.addProperty("id", PropertyDataType::INT);
    list.addProperty("name", PropertyDataType::STRING);
    list.addProperty("layer_name", PropertyDataType::STRING);
    list.addProperty("json", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    assert(buffer->has<int>("id"));
    assert(buffer->has<string>("name"));
    assert(buffer->has<string>("layer_name"));
    assert(buffer->has<string>("json"));

    NullableVector<int> id_vec = buffer->get<int>("id");
    NullableVector<string> name_vec = buffer->get<string>("name");
    NullableVector<string> layer_name_vec = buffer->get<string>("layer_name");
    NullableVector<string> json_vec = buffer->get<string>("json");

    int id;
    string name;
    string layer_name;
    string json_str;

    for (size_t cnt = 0; cnt < buffer->size(); ++cnt)
    {
        assert(!id_vec.isNull(cnt));
        assert(!name_vec.isNull(cnt));
        assert(!layer_name_vec.isNull(cnt));
        assert(!json_vec.isNull(cnt));

        id = id_vec.get(cnt);
        name = name_vec.get(cnt);
        layer_name = layer_name_vec.get(cnt);
        json_str = json_vec.get(cnt);

        assert (!hasSector(name, layer_name));

        shared_ptr<Sector> new_sector = make_shared<Sector>(id, name, layer_name, json_str);
        string layer_name = new_sector->layerName();

        if (!hasSectorLayer(layer_name))
            sector_layers_.push_back(make_shared<SectorLayer>(layer_name));

        assert (hasSectorLayer(layer_name));

        sectorLayer(layer_name)->addSector(new_sector);

        assert (hasSector(name, layer_name));

        loginf << "DBInterface: loadSectors: loaded sector '" << name << "' in layer '"
               << layer_name << "' num points " << sector(name, layer_name)->size();
    }
}

void DBInterface::insertMinMax(const string& id, const string& object_name,
                               const string& min, const string& max)
{
    QMutexLocker locker(&connection_mutex_);

    string str = sql_generator_.getInsertMinMaxStatement(id, object_name, min, max);
    current_connection_->executeSQL(str);
}

/**
 * If variable is a not meta variable, min/max values just for the variable. If it is, gets min/max
 * values for all subvariables and calculates the min/max for all subvariables. If the variable
 * needs a unit transformation, it is performed (locally in this thread).
 */
pair<string, string> DBInterface::getMinMaxString(const DBOVariable& var)
{
    logdbg << "DBInterface: getMinMaxString: var " << var.name();

    if (!var.dbObject().existsInDB())  // object doesn't exist in this database
    {
        logerr << "DBInterface: getMinMaxString: parent object of var " << var.name()
               << " does not exist in db";
        return pair<string, string>(NULL_STRING, NULL_STRING);
    }

    if (!var.dbObject().count())  // object doesn't exist in this database
    {
        logerr << "DBInterface: getMinMaxString: parent object of var " << var.name()
               << " has no data in db";
        return pair<string, string>(NULL_STRING, NULL_STRING);
    }

    if (!var.existsInDB())  // variable doesn't exist in this database
    {
        logerr << "DBInterface: getMinMaxString: var " << var.name() << " does not exist in db";
        return pair<string, string>(NULL_STRING, NULL_STRING);
    }

    QMutexLocker locker(&connection_mutex_);

    PropertyList list;
    list.addProperty("min", PropertyDataType::STRING);
    list.addProperty("max", PropertyDataType::STRING);

    // get min max as strings

    DBCommand command;
    command.set(
                sql_generator_.getSelectMinMaxStatement(var.currentDBColumn().name(), var.dboName()));
    command.list(list);

    logdbg << "DBInterface: getMinMaxString: sql '" << command.get() << "'";

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result);
    assert(result->containsData());
    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    if (buffer->size() != 1)
    {
        logerr << "DBInterface: getMinMaxString: variable " << var.name() << " has "
               << buffer->size() << " minmax values";
        return pair<string, string>(NULL_STRING, NULL_STRING);
    }

    if (buffer->get<string>("min").isNull(0) || buffer->get<string>("max").isNull(0))
    {
        logerr << "DBInterface: getMinMaxString: variable " << var.name()
               << " has NULL minimum/maximum";
        return pair<string, string>(NULL_STRING, NULL_STRING);
    }

    string min = buffer->get<string>("min").get(0);
    string max = buffer->get<string>("max").get(0);

    logdbg << "DBInterface: getMinMaxString: minstr '" << min << " maxstr " << max;

    const DBTableColumn& column = var.currentDBColumn();
    if (column.unit() != var.unitConst())  // do unit conversion stuff
    {
        if (!UnitManager::instance().hasDimension(var.dimensionConst()))
        {
            logerr << "DBInterface: getMinMaxString: unknown dimension '" << var.dimensionConst()
                   << "'";
            throw runtime_error("DBInterface: getMinMaxString: unknown dimension '" +
                                var.dimensionConst() + "'");
        }

        const Dimension& dimension = UnitManager::instance().dimension(var.dimensionConst());

        if (!dimension.hasUnit(column.unit()))
            logerr << "DBInterface: getMinMaxString: dimension '" << var.dimensionConst()
                   << "' has unknown unit '" << column.unit() << "'";

        if (!dimension.hasUnit(var.unitConst()))
            logerr << "DBInterface: getMinMaxString: dimension '" << var.dimensionConst()
                   << "' has unknown unit '" << var.unitConst() << "'";

        double factor = dimension.getFactor(column.unit(), var.unitConst());

        min = var.multiplyString(min, factor);
        max = var.multiplyString(max, factor);
    }

    logdbg << "DBInterface: getMinMaxString: var " << var.name() << " min " << min << " max "
           << max;
    return pair<string, string>(min, max);
}

bool DBInterface::existsViewPointsTable()
{
    return existsTable(TABLE_NAME_VIEWPOINTS);
}

void DBInterface::createViewPointsTable()
{
    assert(!existsViewPointsTable());

    setProperty("view_points_version", "0.1");

    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getTableViewPointsCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

void DBInterface::setViewPoint(const unsigned int id, const string& value)
{
    if (!current_connection_)
    {
        logwrn << "DBInterface: setViewPoint: failed since no database connection exists";
        return;
    }

    // QMutexLocker locker(&connection_mutex_); // done in closeConnection
    assert(current_connection_);

    if (!existsViewPointsTable())
        createViewPointsTable();

    string str = sql_generator_.getInsertViewPointStatement(id, value);

    logdbg << "DBInterface: setViewPoint: cmd '" << str << "'";
    current_connection_->executeSQL(str);
}

map<unsigned int, string> DBInterface::viewPoints()
{
    loginf << "DBInterface: viewPoints";

    assert (existsViewPointsTable());

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllViewPointsStatement());

    PropertyList list;
    list.addProperty("id", PropertyDataType::UINT);
    list.addProperty("json", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    assert(buffer->has<unsigned int>("id"));
    assert(buffer->has<string>("json"));

    NullableVector<unsigned int> id_vec = buffer->get<unsigned int>("id");
    NullableVector<string> json_vec = buffer->get<string>("json");

    map<unsigned int, string> view_points;

    for (size_t cnt = 0; cnt < buffer->size(); ++cnt)
    {
        assert(!id_vec.isNull(cnt));
        assert(!view_points.count(id_vec.get(cnt)));
        if (!json_vec.isNull(cnt))
            view_points[id_vec.get(cnt)] = json_vec.get(cnt);
    }

    loginf << "DBInterface: loadViewPoints: loaded " << view_points.size() << " view points";

    return view_points;
}

void DBInterface::deleteViewPoint(const unsigned int id)
{
    QMutexLocker locker(&connection_mutex_);
    current_connection_->executeSQL(sql_generator_.getDeleteStatement(TABLE_NAME_VIEWPOINTS,
                                                                      "id="+to_string(id)));
}

void DBInterface::deleteAllViewPoints()
{
    clearTableContent(TABLE_NAME_VIEWPOINTS);
}


bool DBInterface::existsSectorsTable()
{
    return existsTable(TABLE_NAME_SECTORS);
}

void DBInterface::createSectorsTable()
{
    loginf << "DBInterface: createSectorsTable";

    assert(!existsSectorsTable());

    connection_mutex_.lock();

    loginf << "DBInterface: createSectorsTable: sql '" << sql_generator_.getTableSectorsCreateStatement() << "'";
    current_connection_->executeSQL(sql_generator_.getTableSectorsCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

bool DBInterface::hasSectorLayer (const std::string& layer_name)
{
    auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                             [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

    return iter != sector_layers_.end();
}

//void DBInterface::renameSectorLayer (const std::string& name, const std::string& new_name)
//{
//    // TODO
//}

std::shared_ptr<SectorLayer> DBInterface::sectorLayer (const std::string& layer_name)
{
    assert (hasSectorLayer(layer_name));

    auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                             [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});
    assert (iter != sector_layers_.end());

    return *iter;
}

unsigned int DBInterface::getMaxSectorId ()
{
    unsigned int id = 0;
    for (auto& sec_lay_it : sector_layers_)
        for (auto& sec_it : sec_lay_it->sectors())
            if (sec_it->id() > id)
                id = sec_it->id();

    return id;
}

void DBInterface::createNewSector (const std::string& name, const std::string& layer_name,
                                   std::vector<std::pair<double,double>> points)
{
    loginf << "DBInterface: createNewSector: name " << name << " layer_name " << layer_name
           << " num points " << points.size();

    assert (!hasSector(name, layer_name));

    unsigned int id = getMaxSectorId()+1;

    shared_ptr<Sector> sector = make_shared<Sector> (id, name, layer_name, points);

    // add to existing sectors
    if (!hasSectorLayer(layer_name))
        sector_layers_.push_back(make_shared<SectorLayer>(layer_name));

    assert (hasSectorLayer(layer_name));

    sectorLayer(layer_name)->addSector(sector);

    assert (hasSector(name, layer_name));
    sector->save();
}

bool DBInterface::hasSector (const string& name, const string& layer_name)
{
    if (!hasSectorLayer(layer_name))
        return false;

    return sectorLayer(layer_name)->hasSector(name);
}


std::shared_ptr<Sector> DBInterface::sector (const string& name, const string& layer_name)
{
    assert (hasSector(name, layer_name));
    return sectorLayer(layer_name)->sector(name);
}

std::vector<std::shared_ptr<SectorLayer>>& DBInterface::sectorsLayers()
{
    return sector_layers_;
}

void DBInterface::saveSector(unsigned int id)
{
    for (auto& sec_lay_it : sector_layers_)
    {
        for (auto& sec_it : sec_lay_it->sectors())
        {
            if (sec_it->id() == id)
            {
                saveSector(sec_it);
                return;
            }
        }
    }

    logerr << "DBInterface: setSector: id " << id << " not found";
}

void DBInterface::saveSector(shared_ptr<Sector> sector)
{
    loginf << "DBInterface: saveSector: sector " << sector->name() << " layer " << sector->layerName()
           << " id " << sector->id();

    if (!current_connection_)
    {
        logwrn << "DBInterface: saveSector: failed since no database connection exists";
        return;
    }

    assert(current_connection_);

    assert (hasSector(sector->name(), sector->layerName()));

    if (!existsSectorsTable())
        createSectorsTable();

    // insert and replace
    string str = sql_generator_.getReplaceSectorStatement(sector->id(), sector->name(), sector->layerName(),
                                                          sector->jsonData());

    logdbg << "DBInterface: saveSector: cmd '" << str << "'";
    {
        QMutexLocker locker(&connection_mutex_);
        current_connection_->executeSQL(str);
    }

    emit sectorsChangedSignal();
}

void DBInterface::deleteSector(shared_ptr<Sector> sector)
{
    assert (hasSector(sector->name(), sector->layerName()));

    string layer_name = sector->layerName();

    assert (hasSectorLayer(layer_name));

    sectorLayer(layer_name)->removeSector(sector);

    // remove sector layer if empty
    if (!sectorLayer(layer_name)->size())
    {
        auto iter = std::find_if(sector_layers_.begin(), sector_layers_.end(),
                                 [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

        assert (iter != sector_layers_.end());
        sector_layers_.erase(iter);
    }

    {
        QMutexLocker locker(&connection_mutex_);
        current_connection_->executeSQL(sql_generator_.getDeleteStatement(TABLE_NAME_SECTORS,
                                                                          "name="+sector->name()));
    }

    emit sectorsChangedSignal();
}

void DBInterface::deleteAllSectors()
{
    sector_layers_.clear();
    clearTableContent(TABLE_NAME_SECTORS);

    emit sectorsChangedSignal();
}

bool DBInterface::hasActiveDataSources(DBObject& object)
{
    if (!object.existsInDB())
        return false;

    if (!existsPropertiesTable())
        return false;

    return hasProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX + object.name());
}

/**
 * Gets active sensor numbers as property, splits it and packs it into a set.
 */
set<int> DBInterface::getActiveDataSources(DBObject& object)
{
    logdbg << "DBInterface: getActiveDataSources: start";

    assert(hasActiveDataSources(object));

    string tmp = getProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX + object.name());

    set<int> ret;

    vector<string> tmp2 = String::split(tmp, ',');

    loginf << "DBInterface: getActiveDataSources: got " << tmp2.size() << " parts from '" << tmp
           << "'";

    for (unsigned int cnt = 0; cnt < tmp2.size(); cnt++)
    {
        ret.insert(stoi(tmp2.at(cnt)));
        loginf << "DBInterface: getActiveDataSources: got active source " << cnt << " '"
               << stoi(tmp2.at(cnt)) << "'";
    }

    logdbg << "DBInterface: getActiveDataSources: end";
    return ret;
}

void DBInterface::insertBuffer(MetaDBTable& meta_table, shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: insertBuffer: meta " << meta_table.name() << " buffer size "
           << buffer->size();
    assert (buffer->size());

    logdbg << "DBInterface: insertBuffer: main table " << meta_table.mainTable().name();
    shared_ptr<Buffer> partial_buffer = getPartialBuffer(meta_table.mainTable(), buffer);
    assert(partial_buffer->size());
    insertBuffer(meta_table.mainTable(), partial_buffer);

    for (auto& sub_it : meta_table.subTables())
    {
        logdbg << "DBInterface: insertBuffer: sub table " << sub_it.second.name();

        partial_buffer = getPartialBuffer(sub_it.second, buffer);
        assert(partial_buffer->size());
        insertBuffer(sub_it.second, partial_buffer);
    }
}

void DBInterface::insertBuffer(DBTable& table, shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: insertBuffer: table " << table.name() << " buffer size "
           << buffer->size();

    assert(current_connection_);
    assert(buffer);

    const PropertyList& properties = buffer->properties();

    for (unsigned int cnt = 0; cnt < properties.size(); ++cnt)
    {
        logdbg << "DBInterface: insertBuffer: checking column '" << properties.at(cnt).name()
               << "'";

        if (!table.hasColumn(properties.at(cnt).name()))
            throw runtime_error("DBInterface: insertBuffer: column '" +
                                properties.at(cnt).name() + "' does not exist in table " +
                                table.name());
    }

    if (!table.existsInDB() &&
            !existsTable(table.name()))  // check for both since information might not be updated yet
        createTable(table);

    assert(table.existsInDB());

    string bind_statement = sql_generator_.insertDBUpdateStringBind(buffer, table.name());

    QMutexLocker locker(&connection_mutex_);

    logdbg << "DBInterface: insertBuffer: preparing bind statement";
    current_connection_->prepareBindStatement(bind_statement);
    current_connection_->beginBindTransaction();

    logdbg << "DBInterface: insertBuffer: starting inserts";
    size_t size = buffer->size();
    for (unsigned int cnt = 0; cnt < size; ++cnt)
    {
        insertBindStatementUpdateForCurrentIndex(buffer, cnt);
    }

    logdbg << "DBInterface: insertBuffer: ending bind transactions";
    current_connection_->endBindTransaction();
    logdbg << "DBInterface: insertBuffer: finalizing bind statement";
    current_connection_->finalizeBindStatement();
}

void DBInterface::insertBuffer(const string& table_name, shared_ptr<Buffer> buffer)
{
    loginf << "DBInterface: insertBuffer: table name " << table_name << " buffer size "
           << buffer->size();

    assert(current_connection_);
    assert(buffer);

    if (!existsTable(table_name))
        throw runtime_error("DBInterface: insertBuffer: table with name '" + table_name +
                            "' does not exist");

    const PropertyList& properties = buffer->properties();

    assert(table_info_.count(table_name));

    DBTableInfo& table_info = table_info_.at(table_name);

    for (unsigned int cnt = 0; cnt < properties.size(); ++cnt)
    {
        logdbg << "DBInterface: insertBuffer: checking column '" << properties.at(cnt).name()
               << "'";

        if (!table_info.hasColumn(properties.at(cnt).name()))
            throw runtime_error("DBInterface: insertBuffer: column '" +
                                properties.at(cnt).name() + "' does not exist in table " +
                                table_name);
    }

    string bind_statement = sql_generator_.insertDBUpdateStringBind(buffer, table_name);

    QMutexLocker locker(&connection_mutex_);

    logdbg << "DBInterface: insertBuffer: preparing bind statement";
    current_connection_->prepareBindStatement(bind_statement);
    current_connection_->beginBindTransaction();

    logdbg << "DBInterface: insertBuffer: starting inserts";
    size_t size = buffer->size();

    for (unsigned int cnt = 0; cnt < size; ++cnt)
    {
        insertBindStatementUpdateForCurrentIndex(buffer, cnt);
    }

    logdbg << "DBInterface: insertBuffer: ending bind transactions";
    current_connection_->endBindTransaction();
    logdbg << "DBInterface: insertBuffer: finalizing bind statement";
    current_connection_->finalizeBindStatement();
}

shared_ptr<Buffer> DBInterface::getPartialBuffer(DBTable& table,
                                                 shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: getPartialBuffer: table " << table.name() << " buffer size "
           << buffer->size();
    assert (buffer->size());

    PropertyList org_properties = buffer->properties();
    PropertyList partial_properties;

    for (unsigned int cnt = 0; cnt < org_properties.size(); ++cnt)
    {
        Property org_prop = org_properties.at(cnt);

        if (table.hasColumn(org_prop.name()))
        {
            logdbg << "DBInterface: getPartialBuffer: table " << table.name() << " adding property "
                   << org_prop.name();
            partial_properties.addProperty(org_prop);
        }
        else
            logdbg << "DBInterface: getPartialBuffer: table " << table.name()
                   << " skipping property " << org_prop.name();
    }

    shared_ptr<Buffer> tmp_buffer = buffer->getPartialCopy(partial_properties);

    logdbg << "DBInterface: getPartialBuffer: end with partial buffer size " << tmp_buffer->size();
    assert (tmp_buffer->size());

    return tmp_buffer;
}

bool DBInterface::checkUpdateBuffer(DBObject& object, DBOVariable& key_var, DBOVariableSet& list,
                                    shared_ptr<Buffer> buffer)
{
    if (!object.existsInDB())
        return false;

    if (!key_var.existsInDB())
        return false;

    const DBTable& table = object.currentMetaTable().mainTable();

    if (!table.existsInDB())  // might be redundant
        return false;

    const PropertyList& properties = buffer->properties();

    for (auto& var_it : list.getSet())
    {
        if (!properties.hasProperty(var_it->name()))
            return false;

        if (!var_it->hasCurrentDBColumn())
            return false;

        const DBTableColumn& col = var_it->currentDBColumn();

        if (!col.existsInDB())
            return false;
    }

    //    for (unsigned int cnt=0; cnt < properties.size(); cnt++)
    //    {
    //        if (!table.hasColumn(properties.at(cnt).name()))
    //            return false;

    //        if (!table.column(properties.at(cnt).name()).existsInDB())
    //            return false;
    //    }

    return true;
}

void DBInterface::updateBuffer(MetaDBTable& meta_table, const DBTableColumn& key_col,
                               shared_ptr<Buffer> buffer, int from_index, int to_index)
{
    logdbg << "DBInterface: updateBuffer: meta " << meta_table.name() << " buffer size "
           << buffer->size() << " key " << key_col.identifier();

    shared_ptr<Buffer> partial_buffer = getPartialBuffer(meta_table.mainTable(), buffer);
    assert(partial_buffer->size());
    updateBuffer(meta_table.mainTable(), key_col, partial_buffer, from_index, to_index);

    for (auto& sub_it : meta_table.subTables())
    {
        if (sub_it.second.hasColumn(key_col.name()))
        {
            const DBTableColumn& sub_key_col = sub_it.second.column(key_col.name());
            logdbg << "DBInterface: updateBuffer: got sub table " << sub_it.second.name()
                   << " key col " << sub_key_col.identifier();

            partial_buffer = getPartialBuffer(sub_it.second, buffer);
            if (partial_buffer->size())
            {
                logdbg << "DBInterface: updateBuffer: doing update for sub table "
                       << sub_it.second.name();
                updateBuffer(sub_it.second, sub_key_col, partial_buffer, from_index, to_index);
            }
            else
                logdbg << "DBInterface: updateBuffer: empty buffer for sub table "
                       << sub_it.second.name();
        }
        else
            logdbg << "DBInterface: updateBuffer: key not found in sub table "
                   << sub_it.second.name();
    }
}

void DBInterface::updateBuffer(DBTable& table, const DBTableColumn& key_col,
                               shared_ptr<Buffer> buffer, int from_index, int to_index)
{
    logdbg << "DBInterface: updateBuffer: table " << table.name() << " buffer size "
           << buffer->size() << " key " << key_col.identifier();

    // assert (checkUpdateBuffer(object, key_var, buffer));
    assert(current_connection_);
    assert(buffer);

    const PropertyList& properties = buffer->properties();

    for (unsigned int cnt = 0; cnt < properties.size(); cnt++)
    {
        if (!table.hasColumn(properties.at(cnt).name()))
            throw runtime_error("DBInterface: updateBuffer: column '" +
                                properties.at(cnt).name() + "' does not exist in table " +
                                table.name());
    }

    string bind_statement =
            sql_generator_.createDBUpdateStringBind(buffer, key_col, table.name());

    QMutexLocker locker(&connection_mutex_);

    logdbg << "DBInterface: updateBuffer: preparing bind statement '" << bind_statement << "'";
    current_connection_->prepareBindStatement(bind_statement);
    current_connection_->beginBindTransaction();

    if (from_index < 0)
        from_index = 0;
    if (to_index < 0)
        to_index = buffer->size() - 1;

    logdbg << "DBInterface: updateBuffer: starting inserts";
    for (int cnt = from_index; cnt <= to_index; cnt++)
    {
        logdbg << "DBInterface: updateBuffer: insert cnt " << cnt;
        insertBindStatementUpdateForCurrentIndex(buffer, cnt);
    }

    logdbg << "DBInterface: updateBuffer: ending bind transactions";
    current_connection_->endBindTransaction();
    logdbg << "DBInterface: update: finalizing bind statement";
    current_connection_->finalizeBindStatement();
}

void DBInterface::prepareRead(const DBObject& dbobject, DBOVariableSet read_list,
                              string custom_filter_clause,
                              vector<DBOVariable*> filtered_variables, bool use_order,
                              DBOVariable* order_variable, bool use_order_ascending,
                              const string& limit)
{
    assert(current_connection_);

    assert(dbobject.existsInDB());

    for (auto& var_it : read_list.getSet())
        assert(var_it->existsInDB());

    for (auto& var_it : filtered_variables)
        assert(var_it->existsInDB());

    if (order_variable)
        assert(order_variable->existsInDB());

    connection_mutex_.lock();

    shared_ptr<DBCommand> read = sql_generator_.getSelectCommand(
                dbobject.currentMetaTable(), read_list, custom_filter_clause, filtered_variables, use_order,
                order_variable, use_order_ascending, limit, true);

    logdbg << "DBInterface: prepareRead: dbo " << dbobject.name() << " sql '" << read->get() << "'";
    current_connection_->prepareCommand(read);
}

/**
 * Retrieves result from connection stepPreparedCommand, calls activateKeySearch on buffer and
 * returns it.
 */
shared_ptr<Buffer> DBInterface::readDataChunk(const DBObject& dbobject)
{
    // locked by prepareRead
    assert(current_connection_);

    shared_ptr<DBResult> result = current_connection_->stepPreparedCommand(read_chunk_size_);

    if (!result)
    {
        logerr << "DBInterface: readDataChunk: connection returned error";
        throw runtime_error("DBInterface: readDataChunk: connection returned error");
    }

    if (!result->containsData())
    {
        logerr << "DBInterface: readDataChunk: buffer does not contain data";
        throw runtime_error("DBInterface: readDataChunk: buffer does not contain data");
    }

    shared_ptr<Buffer> buffer = result->buffer();

    buffer->dboName(dbobject.name());

    assert(buffer);

    bool last_one = current_connection_->getPreparedCommandDone();
    buffer->lastOne(last_one);

    return buffer;
}

void DBInterface::finalizeReadStatement(const DBObject& dbobject)
{
    connection_mutex_.unlock();
    assert(current_connection_);

    logdbg << "DBInterface: finishReadSystemTracks: start ";
    current_connection_->finalizeCommand();
}

void DBInterface::createPropertiesTable()
{
    assert(!existsPropertiesTable());
    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getTablePropertiesCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

void DBInterface::createMinMaxTable()
{
    assert(!existsMinMaxTable());
    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getTableMinMaxCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

void DBInterface::clearTableContent(const string& table_name)
{
    QMutexLocker locker(&connection_mutex_);
    // DELETE FROM tablename;
    current_connection_->executeSQL("DELETE FROM " + table_name + ";");
}

shared_ptr<DBResult> DBInterface::queryMinMaxNormalForTable(const DBTable& table)
{
    QMutexLocker locker(&connection_mutex_);
    logdbg << "DBInterface: queryMinMaxForTable: getting command";
    shared_ptr<DBCommand> command = sql_generator_.getTableSelectMinMaxNormalStatement(table);

    // loginf  << "DBInterface: queryMinMaxForTable: executing command '" <<
    // command->getCommandString() << "'";
    shared_ptr<DBResult> result = current_connection_->execute(*command);
    return result;
}

void DBInterface::insertBindStatementUpdateForCurrentIndex(shared_ptr<Buffer> buffer,
                                                           unsigned int row)
{
    assert(buffer);
    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: start";
    const PropertyList& list = buffer->properties();
    unsigned int size = list.size();
    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: creating bind for " << size
           << " elements";

    string connection_type = current_connection_->type();

    assert(connection_type == MYSQL_IDENTIFIER || connection_type == SQLITE_IDENTIFIER);

    unsigned int index_cnt = 0;

    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: starting for loop";
    for (unsigned int cnt = 0; cnt < size; cnt++)
    {
        const Property& property = list.at(cnt);
        PropertyDataType data_type = property.dataType();

        logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: at cnt " << cnt << " id "
               << property.name() << " index cnt " << index_cnt;

        if (connection_type == SQLITE_IDENTIFIER)
            index_cnt = cnt + 1;
        else if (connection_type == MYSQL_IDENTIFIER)
            index_cnt = cnt + 1;
        else
            throw runtime_error(
                    "DBInterface: insertBindStatementForCurrentIndex: unknown db type");

        if (buffer->isNone(property, row))
        {
            current_connection_->bindVariableNull(index_cnt);
            logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: at " << cnt
                   << " is null";
            continue;
        }

        switch (data_type)
        {
            case PropertyDataType::BOOL:
                current_connection_->bindVariable(
                            index_cnt, static_cast<int>(buffer->get<bool>(property.name()).get(row)));
                break;
            case PropertyDataType::CHAR:
                current_connection_->bindVariable(
                            index_cnt, static_cast<int>(buffer->get<char>(property.name()).get(row)));
                break;
            case PropertyDataType::UCHAR:
                current_connection_->bindVariable(
                            index_cnt,
                            static_cast<int>(buffer->get<unsigned char>(property.name()).get(row)));
                break;
            case PropertyDataType::INT:
                logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: at " << cnt
                       << " is '" << buffer->get<int>(property.name()).get(row) << "'";
                current_connection_->bindVariable(
                            index_cnt, static_cast<int>(buffer->get<int>(property.name()).get(row)));
                break;
            case PropertyDataType::UINT:
                assert(false);
                break;
            case PropertyDataType::LONGINT:
                assert(false);
                break;
            case PropertyDataType::ULONGINT:
                assert(false);
                break;
            case PropertyDataType::FLOAT:
                current_connection_->bindVariable(
                            index_cnt, static_cast<double>(buffer->get<float>(property.name()).get(row)));
                break;
            case PropertyDataType::DOUBLE:
                current_connection_->bindVariable(index_cnt,
                                                  buffer->get<double>(property.name()).get(row));
                break;
            case PropertyDataType::STRING:
                if (connection_type == SQLITE_IDENTIFIER)
                    current_connection_->bindVariable(
                                index_cnt, buffer->get<string>(property.name()).get(row));
                else  // MYSQL assumed
                    current_connection_->bindVariable(
                                index_cnt, "'" + buffer->get<string>(property.name()).get(row) + "'");
                break;
            default:
                logerr << "Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type "
                       << Property::asString(data_type);
                throw runtime_error(
                            "Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type " +
                            Property::asString(data_type));
        }
    }

    current_connection_->stepAndClearBindings();

    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: done";
}

void DBInterface::createAssociationsTable(const string& table_name)
{
    assert(!existsTable(table_name));
    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getCreateAssociationTableStatement(table_name));
    connection_mutex_.unlock();

    updateTableInfo();
}

DBOAssociationCollection DBInterface::getAssociations(const string& table_name)
{
    assert(existsTable(table_name));

    DBOAssociationCollection associations;
    shared_ptr<DBCommand> command = sql_generator_.getSelectAssociationsCommand(table_name);

    connection_mutex_.lock();

    shared_ptr<DBResult> result = current_connection_->execute(*command.get());

    connection_mutex_.unlock();

    if (result->containsData())
    {
        shared_ptr<Buffer> buffer = result->buffer();

        if (buffer->size())
        {
            size_t num_associations = buffer->size();
            assert(buffer->has<int>("rec_num"));
            assert(buffer->has<int>("utn"));
            assert(buffer->has<int>("src_rec_num"));

            NullableVector<int>& rec_nums = buffer->get<int>("rec_num");
            NullableVector<int>& utns = buffer->get<int>("utn");
            NullableVector<int>& src_rec_nums = buffer->get<int>("src_rec_num");

            for (size_t cnt = 0; cnt < num_associations; ++cnt)
            {
                assert(!rec_nums.isNull(cnt));
                assert(!utns.isNull(cnt));
                assert(!src_rec_nums.isNull(cnt));

                associations.add(rec_nums.get(cnt),
                                 DBOAssociationEntry(utns.get(cnt), src_rec_nums.get(cnt)));
            }
        }
    }

    return associations;
}

// void DBInterface::deleteAllRowsWithVariableValue (DBOVariable *variable, string value,
// string filter)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    scoped_lock l(mutex_);
//    connection_->executeSQL(sql_generator_->getDeleteStatement(variable->getCurrentDBColumn(),
//    value, filter));
//}

// void DBInterface::updateAllRowsWithVariableValue (DBOVariable *variable, string value,
// string new_value,
// string filter)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    connection_->executeSQL(sql_generator_->getUpdateStatement(variable->getCurrentDBColumn(),
//    value, new_value,
// filter));
//}

// void DBInterface::testReading ()
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
//    prepareRead (object, read_list); //, string custom_filter_clause="", DBOVariable
//    *order=0);

//    start_time = boost::posix_time::microsec_clock::local_time();

//    loginf << "DBInterface: testReading: starting reading";
//    vector<shared_ptr <Buffer>> buffer_vector;

//    unsigned int num_rows=0;

//    while (!getReadingDone(object))
//    {
//        shared_ptr <Buffer> buffer = readDataChunk (object, false);
//        buffer_vector.push_back(buffer);

//        stop_time = boost::posix_time::microsec_clock::local_time();
//        boost::posix_time::time_duration diff = stop_time - start_time;

//        num_rows += buffer->size();
//        if (diff.total_seconds() > 0)
//            loginf << "DBInterface: testReading: got buffer size " << buffer->size() << " all " <<
//            num_rows
//<< " elapsed " << diff << " #el/sec " << num_rows/diff.total_seconds();
//    }

//    boost::posix_time::time_duration diff = stop_time - start_time;
//    loginf << "DBInterface: testReading: reading done: all " << num_rows << " elapsed " << diff <<
//    " #el/sec "
//<< num_rows/diff.total_seconds();
//    finalizeReadStatement (object);

//    loginf << "DBInterface: testReading: clearing buffers";
//    buffer_vector.clear();

//    loginf << "DBInterface: testReading: done";
//}
