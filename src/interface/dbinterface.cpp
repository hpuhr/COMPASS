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

#include "dbinterface.h"
#include "compass.h"
#include "buffer.h"
#include "config.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "sqliteconnection.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbresult.h"
#include "dbtableinfo.h"
#include "dimension.h"
#include "jobmanager.h"
#include "sqliteconnection.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"
#include "files.h"
#include "util/timeconv.h"
#include "sector.h"
#include "sectorlayer.h"
#include "evaluationmanager.h"
#include "source/dbdatasource.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/target/target.h"

#include <QApplication>
#include <QMessageBox>
#include <QMutexLocker>
#include <QThread>

#include "boost/date_time/posix_time/posix_time.hpp"

#include <fstream>

using namespace Utils;
using namespace std;
using namespace dbContent;

const string PROP_TIMESTAMP_MIN_NAME {"timestamp_min"};
const string PROP_TIMESTAMP_MAX_NAME {"timestamp_max"};
const string PROP_LATITUDE_MIN_NAME {"latitude_min"};
const string PROP_LATITUDE_MAX_NAME {"latitude_max"};
const string PROP_LONGITUDE_MIN_NAME {"longitude_min"};
const string PROP_LONGITUDE_MAX_NAME {"longitude_max"};

DBInterface::DBInterface(string class_id, string instance_id, COMPASS* compass)
    : Configurable(class_id, instance_id, compass), sql_generator_(*this)
{
    boost::mutex::scoped_lock locker(connection_mutex_);

    registerParameter("read_chunk_size", &read_chunk_size_, 50000);

    createSubConfigurables();
}

DBInterface::~DBInterface()
{
    logdbg << "DBInterface: desctructor: start";

    boost::mutex::scoped_lock locker(connection_mutex_);

    db_connection_ = nullptr;

    logdbg << "DBInterface: desctructor: end";
}

void DBInterface::updateTableInfo()
{
    boost::mutex::scoped_lock locker2(table_info_mutex_);

    loginf << "DBInterface: updateTableInfo";
    table_info_.clear();

    boost::mutex::scoped_lock locker(connection_mutex_);

    assert(db_connection_);
    table_info_ = db_connection_->getTableInfo();

    loginf << "DBInterface: updateTableInfo: found " << table_info_.size() << " tables";
}

void DBInterface::openDBFile(const std::string& filename, bool overwrite)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    if (overwrite && Files::fileExists(filename))
    {
        loginf << "DBInterface: openDBFile: deleting pre-existing file '" << filename << "'";
        Files::deleteFile(filename);
    }

    bool created_new_db = !Files::fileExists(filename);

    loginf << "DBInterface: openDBFile: opening file '" << filename << "'";
    assert (db_connection_);
    db_connection_->openFile(filename);

    updateTableInfo();

    if (created_new_db)
    {
        assert (!existsPropertiesTable());
        createPropertiesTable();
        properties_loaded_ = true;

        setProperty("APP_VERSION", COMPASS::instance().config().getString("version"));
        saveProperties();

        assert (!existsDataSourcesTable());
        createDataSourcesTable();

        assert (!existsSectorsTable());
        createSectorsTable();
    }
    else
    {
        assert (existsPropertiesTable());
        loadProperties();

        if (!hasProperty("APP_VERSION")
                 || getProperty("APP_VERSION") != COMPASS::instance().config().getString("version"))
        {
            string reason = hasProperty("APP_VERSION") ?
                        "DB from Version " + getProperty("APP_VERSION") + ", current "
                           + COMPASS::instance().config().getString("version") : "DB from Version older than 0.7.0";

            table_info_.clear(); // not need to lock
            properties_loaded_ = false;
            properties_.clear();
            db_connection_->disconnect();

            QApplication::restoreOverrideCursor();

            throw std::runtime_error ("Incorrect application version for database:\n "+reason);
        }

        assert (existsDataSourcesTable());
        assert (existsSectorsTable());
    }

    if (!existsTargetsTable())
        createTargetsTable();

    //emit databaseOpenedSignal();

    QApplication::restoreOverrideCursor();

    loginf << "DBInterface: openDBFile: done";
}

void DBInterface::exportDBFile(const std::string& filename)
{
    assert (dbOpen());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    boost::mutex::scoped_lock locker(connection_mutex_);

    db_connection_->exportFile(filename);

    QApplication::restoreOverrideCursor();
}

void DBInterface::closeDBFile()
{
    loginf << "DBInterface: closeDBFile";

    {
        boost::mutex::scoped_lock locker(connection_mutex_);

        if (properties_loaded_)  // false if database not opened
            saveProperties();

        assert (db_connection_);
        db_connection_->disconnect();

        properties_loaded_ = false;

        properties_.clear();
        table_info_.clear(); // no need to lock
    }

    // signal emitted in COMPASS
}

bool DBInterface::dbOpen()
{
    assert (db_connection_);
    return db_connection_->dbOpened();
}

bool DBInterface::ready()
{
    if (!db_connection_)
    {
        logdbg << "DBInterface: ready: no connection";
        return false;
    }

    logdbg << "DBInterface: ready: connection ready " << db_connection_->dbOpened();
    return db_connection_->dbOpened();
}

SQLiteConnection& DBInterface::connection()
{
    assert(db_connection_);
    return *db_connection_;
}

void DBInterface::generateSubConfigurable(const string& class_id,
                                          const string& instance_id)
{
    logdbg << "DBInterface: generateSubConfigurable: generating variable " << instance_id;

    if (class_id == "SQLiteConnection")
    {
        assert (!db_connection_);

        db_connection_.reset( new SQLiteConnection(class_id, instance_id, this));
        assert (db_connection_);

    }
    else
        throw runtime_error("DBInterface: generateSubConfigurable: unknown class_id " +
                            class_id);
}

void DBInterface::checkSubConfigurables()
{
    if (!db_connection_)
    {
        addNewSubConfiguration("SQLiteConnection", "SQLite Connection");
        generateSubConfigurable("SQLiteConnection", "SQLite Connection");
    }
}

bool DBInterface::existsTable(const string& table_name)
{
    boost::mutex::scoped_lock locker(table_info_mutex_);
    return table_info_.count(table_name) == 1;
}

void DBInterface::createTable(const DBContent& object)
{
    loginf << "DBInterface: createTable: obj " << object.name();
    if (existsTable(object.dbTableName()))
    {
        logerr << "DBInterface: createTable: table " << object.dbTableName() << " already exists";
        return;
    }

    string statement = sql_generator_.getCreateTableStatement(object);

    boost::mutex::scoped_lock locker(connection_mutex_);

    db_connection_->executeSQL(statement);

    locker.unlock();

    updateTableInfo();

    loginf << "DBInterface: createTable: checking " << object.dbTableName();
    assert(existsTable(object.dbTableName()));
}

bool DBInterface::existsPropertiesTable() { return existsTable(TABLE_NAME_PROPERTIES); }

unsigned int DBInterface::getMaxRecordNumber(DBContent& object)
{
    assert (dbOpen());
    assert(object.existsInDB());

    assert (COMPASS::instance().dbContentManager().existsMetaVariable(DBContent::meta_var_rec_num_.name()));
    assert (COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_rec_num_.name()).existsIn(object.name()));

    Variable& rec_num_var = COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_rec_num_.name()).getFor(object.name());

    assert (object.hasVariable(rec_num_var.name()));

    boost::mutex::scoped_lock locker(connection_mutex_);

    shared_ptr<DBCommand> command = sql_generator_.getMaxUIntValueCommand(object.dbTableName(),
                                                                             rec_num_var.dbColumnName());

    shared_ptr<DBResult> result = db_connection_->execute(*command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    if (!buffer->size())
    {
        logwrn << "DBInterface: getMaxRecordNumber: no max record number found";
        return 0;
    }

    assert (!buffer->get<unsigned int>(rec_num_var.dbColumnName()).isNull(0));
    return buffer->get<unsigned int>(rec_num_var.dbColumnName()).get(0);
}

unsigned int DBInterface::getMaxRefTrackTrackNum()
{
    assert (dbOpen());

    DBContent& reftraj_content = COMPASS::instance().dbContentManager().dbContent("RefTraj");

    if(!reftraj_content.existsInDB())
        return 0;

    assert (COMPASS::instance().dbContentManager().existsMetaVariable(DBContent::meta_var_track_num_.name()));
    assert (COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_track_num_.name()).existsIn("RefTraj"));

    Variable& track_num_var = COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_track_num_.name()).getFor("RefTraj");

    assert (reftraj_content.hasVariable(track_num_var.name()));

    boost::mutex::scoped_lock locker(connection_mutex_);

    shared_ptr<DBCommand> command = sql_generator_.getMaxUIntValueCommand(reftraj_content.dbTableName(),
                                                                          track_num_var.dbColumnName());

    shared_ptr<DBResult> result = db_connection_->execute(*command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    if (!buffer->size())
    {
        logwrn << "DBInterface: getMaxRefTrackTrackNum: no max track number found";
        return 0;
    }

    assert (!buffer->get<unsigned int>(track_num_var.dbColumnName()).isNull(0));
    return buffer->get<unsigned int>(track_num_var.dbColumnName()).get(0);
}


//std::map<unsigned int, std::tuple<std::set<unsigned int>, std::tuple<bool, unsigned int, unsigned int>,
//std::tuple<bool, unsigned int, unsigned int>>> DBInterface::queryADSBInfo()
//{
//    DBContent& object = COMPASS::instance().dbContentManager().dbContent("ADSB");

//    assert(object.existsInDB());

//    boost::mutex::scoped_lock locker(connection_mutex_);

//    std::map<unsigned int, std::tuple<std::set<unsigned int>, std::tuple<bool, unsigned int, unsigned int>,
//            std::tuple<bool, unsigned int, unsigned int>>> data;

//    shared_ptr<DBCommand> command = sql_generator_.getADSBInfoCommand(object);

//    shared_ptr<DBResult> result = db_connection_->execute(*command);

//    assert(result->containsData());

//    shared_ptr<Buffer> buffer = result->buffer();
//    assert (buffer->has<int>("TARGET_ADDR"));
//    assert (buffer->has<int>("MOPS_VERSION"));
//    assert (buffer->has<char>("MIN_NUCP_NIC"));
//    assert (buffer->has<char>("MAX_NUCP_NIC"));
//    assert (buffer->has<char>("MIN_NACP"));
//    assert (buffer->has<char>("MAX_NACP"));

//    NullableVector<int>& tas = buffer->get<int>("TARGET_ADDR");
//    NullableVector<int>& mops = buffer->get<int>("MOPS_VERSION");
//    NullableVector<char>& min_nus = buffer->get<char>("MIN_NUCP_NIC");
//    NullableVector<char>& max_nus = buffer->get<char>("MAX_NUCP_NIC");
//    NullableVector<char>& min_nas = buffer->get<char>("MIN_NACP");
//    NullableVector<char>& max_nas = buffer->get<char>("MAX_NACP");

//    for (unsigned int cnt = 0; cnt < buffer->size(); cnt++)
//    {
//        if (tas.isNull(cnt))
//            continue;

//        if (!mops.isNull(cnt))
//            get<0>(data[tas.get(cnt)]).insert(mops.get(cnt));

//        if (!min_nus.isNull(cnt) && !max_nus.isNull(cnt))
//            get<1>(data[tas.get(cnt)]) =
//                    std::tuple<bool, unsigned int, unsigned int>(true, min_nus.get(cnt), max_nus.get(cnt));

//        if (!min_nas.isNull(cnt) && !max_nas.isNull(cnt))
//            get<2>(data[tas.get(cnt)]) =
//                    std::tuple<bool, unsigned int, unsigned int>(true, min_nas.get(cnt), max_nas.get(cnt));
//    }

//    return data;
//}

bool DBInterface::existsDataSourcesTable()
{
    return existsTable(dbContent::DBDataSource::table_name_);
}

void DBInterface::createDataSourcesTable()
{
    assert(!existsDataSourcesTable());
    connection_mutex_.lock();
    db_connection_->executeSQL(sql_generator_.getTableDataSourcesCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

std::vector<std::unique_ptr<dbContent::DBDataSource>> DBInterface::getDataSources()
{
    logdbg << "DBInterface: getDataSources: start";

    using namespace dbContent;

    boost::mutex::scoped_lock locker(connection_mutex_);

    shared_ptr<DBCommand> command = sql_generator_.getDataSourcesSelectCommand();

    loginf << "DBInterface: getDataSources: sql '" << command->get() << "'";

    shared_ptr<DBResult> result = db_connection_->execute(*command);
    assert(result->containsData());
    shared_ptr<Buffer> buffer = result->buffer();

    logdbg << "DBInterface: getDataSources: json '" << buffer->asJSON().dump(4) << "'";

    assert(buffer->properties().hasProperty(DBDataSource::id_column_));
    assert(buffer->properties().hasProperty(DBDataSource::ds_type_column_));
    assert(buffer->properties().hasProperty(DBDataSource::sac_column_));
    assert(buffer->properties().hasProperty(DBDataSource::sic_column_));
    assert(buffer->properties().hasProperty(DBDataSource::name_column_));
    assert(buffer->properties().hasProperty(DBDataSource::short_name_));
    assert(buffer->properties().hasProperty(DBDataSource::info_column_));
    assert(buffer->properties().hasProperty(DBDataSource::counts_column_));

    std::vector<std::unique_ptr<DBDataSource>> sources;

    for (unsigned cnt = 0; cnt < buffer->size(); cnt++)
    {
        if (buffer->get<unsigned int>(DBDataSource::id_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL id, will be omitted";
            continue;
        }

        if (buffer->get<string>(DBDataSource::ds_type_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL content type, will be omitted";
            continue;
        }

        if (buffer->get<unsigned int>(DBDataSource::sac_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL sac, will be omitted";
            continue;
        }

        if (buffer->get<unsigned int>(DBDataSource::sic_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL sic, will be omitted";
            continue;
        }

        if (buffer->get<string>(DBDataSource::name_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL name, will be omitted";
            continue;
        }

        std::unique_ptr<DBDataSource> src {new DBDataSource()};

        src->id(buffer->get<unsigned int>(DBDataSource::id_column_.name()).get(cnt));
        src->dsType(buffer->get<string>(DBDataSource::ds_type_column_.name()).get(cnt));
        src->sac(buffer->get<unsigned int>(DBDataSource::sac_column_.name()).get(cnt));
        src->sic(buffer->get<unsigned int>(DBDataSource::sic_column_.name()).get(cnt));
        src->name(buffer->get<string>(DBDataSource::name_column_.name()).get(cnt));

        if (!buffer->get<string>(DBDataSource::short_name_.name()).isNull(cnt))
            src->shortName(buffer->get<string>(DBDataSource::short_name_.name()).get(cnt));

        if (!buffer->get<string>(DBDataSource::info_column_.name()).isNull(cnt))
            src->info(buffer->get<string>(DBDataSource::info_column_.name()).get(cnt));

        if (!buffer->get<string>(DBDataSource::counts_column_.name()).isNull(cnt))
            src->counts(buffer->get<string>(DBDataSource::counts_column_.name()).get(cnt));

        sources.emplace_back(move(src));
    }

    return sources;
}

void DBInterface::saveDataSources(const std::vector<std::unique_ptr<dbContent::DBDataSource>>& data_sources)
{
    loginf << "DBInterface: saveDataSources: num " << data_sources.size();

    using namespace dbContent;

    assert (dbOpen());

    clearTableContent(DBDataSource::table_name_);

    PropertyList list;
    list.addProperty(DBDataSource::id_column_);
    list.addProperty(DBDataSource::ds_type_column_);
    list.addProperty(DBDataSource::sac_column_);
    list.addProperty(DBDataSource::sic_column_);
    list.addProperty(DBDataSource::name_column_);
    list.addProperty(DBDataSource::short_name_);
    list.addProperty(DBDataSource::info_column_);
    list.addProperty(DBDataSource::counts_column_);

    shared_ptr<Buffer> buffer = make_shared<Buffer>(list);

    unsigned int cnt = 0;
    for (auto& ds_it : data_sources)
    {
        buffer->get<unsigned int>(DBDataSource::id_column_.name()).set(cnt, ds_it->id());
        buffer->get<string>(DBDataSource::ds_type_column_.name()).set(cnt, ds_it->dsType());
        buffer->get<unsigned int>(DBDataSource::sac_column_.name()).set(cnt, ds_it->sac());
        buffer->get<unsigned int>(DBDataSource::sic_column_.name()).set(cnt, ds_it->sic());
        buffer->get<string>(DBDataSource::name_column_.name()).set(cnt, ds_it->name());

        if (ds_it->hasShortName())
            buffer->get<string>(DBDataSource::short_name_.name()).set(cnt, ds_it->shortName());

        buffer->get<string>(DBDataSource::info_column_.name()).set(cnt, ds_it->infoStr());
        buffer->get<string>(DBDataSource::counts_column_.name()).set(cnt, ds_it->countsStr());

        ++cnt;
    }

    loginf << "DBInterface: saveDataSources: buffer size " << buffer->size();

    insertBuffer(DBDataSource::table_name_, buffer);

    loginf << "DBInterface: saveDataSources: done";
}



size_t DBInterface::count(const string& table)
{
    logdbg << "DBInterface: count: table " << table;
    assert(existsTable(table));

    boost::mutex::scoped_lock locker(connection_mutex_);
    assert(db_connection_);

    string sql = sql_generator_.getCountStatement(table);

    logdbg << "DBInterface: count: sql '" << sql << "'";

    DBCommand command;
    command.set(sql);

    PropertyList list;
    list.addProperty("count", PropertyDataType::INT);
    command.list(list);

    shared_ptr<DBResult> result = db_connection_->execute(command);

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

    assert (!properties_loaded_);

    boost::mutex::scoped_lock locker(connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllPropertiesStatement());

    PropertyList list;
    list.addProperty("id", PropertyDataType::STRING);
    list.addProperty("value", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = db_connection_->execute(command);

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

        if (properties_.count(id_vec.get(cnt)))
            logerr << "DBInterface: loadProperties: property '" << id_vec.get(cnt) << "' already exists";

        assert(!properties_.count(id_vec.get(cnt)));
        if (!value_vec.isNull(cnt))
            properties_[id_vec.get(cnt)] = value_vec.get(cnt);
    }

    for (auto& prop_it : properties_)
        loginf << "DBInterface: loadProperties: id '" << prop_it.first << "' value '"
               << prop_it.second << "'";

    properties_loaded_ = true;
}

void DBInterface::saveProperties()
{
    loginf << "DBInterface: saveProperties";

    if (!db_connection_)
    {
        logwrn << "DBInterface: saveProperties: failed since no database connection exists";
        return;
    }

    // boost::mutex::scoped_lock locker(connection_mutex_); // done in closeConnection
    assert(db_connection_);
    assert (properties_loaded_);

    string str;

    for (auto& prop_it : properties_)
    {
        string str = sql_generator_.getInsertPropertyStatement(prop_it.first, prop_it.second);
        db_connection_->executeSQL(str);
    }

    loginf << "DBInterface: saveProperties: done";
}

std::vector<std::shared_ptr<SectorLayer>> DBInterface::loadSectors()
{
    loginf << "DBInterface: loadSectors";

    boost::mutex::scoped_lock locker(connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllSectorsStatement());

    PropertyList list;
    list.addProperty("id", PropertyDataType::INT);
    list.addProperty("name", PropertyDataType::STRING);
    list.addProperty("layer_name", PropertyDataType::STRING);
    list.addProperty("json", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = db_connection_->execute(command);

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

    std::vector<std::shared_ptr<SectorLayer>> sector_layers;

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

        auto lay_it = std::find_if(sector_layers.begin(), sector_layers.end(),
                                   [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

        if (lay_it == sector_layers.end())
        {
            sector_layers.push_back(make_shared<SectorLayer>(layer_name));

            lay_it = std::find_if(
                        sector_layers.begin(), sector_layers.end(),
                        [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});
        }

        shared_ptr<Sector> new_sector = make_shared<Sector>(id, name, layer_name, json_str);
        string layer_name = new_sector->layerName();

        (*lay_it)->addSector(new_sector);
        assert ((*lay_it)->hasSector(name));

        loginf << "DBInterface: loadSectors: loaded sector '" << name << "' in layer '"
               << layer_name << "' num points " << (*lay_it)->sector(name)->size();
    }

    return sector_layers;
}

bool DBInterface::areColumnsNull (const std::string& table_name, const std::vector<std::string> columns)
{
    boost::mutex::scoped_lock locker(connection_mutex_);

    string str = sql_generator_.getSelectNullCount(table_name, columns);

    loginf << "DBInterface: areColumnsNull: sql '" << str << "'";

    DBCommand command;
    command.set(str);

    PropertyList list;
    list.addProperty("count", PropertyDataType::INT);
    command.list(list);

    shared_ptr<DBResult> result = db_connection_->execute(command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    assert(buffer->has<int>("count"));

    NullableVector<int> count_vec = buffer->get<int>("count");
    assert (count_vec.size() == 1);

    loginf << "DBInterface: areColumnsNull: null count " << count_vec.get(0);

    return count_vec.get(0) != 0;
}

bool DBInterface::existsViewPointsTable()
{
    return existsTable(TABLE_NAME_VIEWPOINTS);
}

void DBInterface::createViewPointsTable()
{
    assert(!existsViewPointsTable());

    setProperty("view_points_version", "0.2");

    connection_mutex_.lock();
    db_connection_->executeSQL(sql_generator_.getTableViewPointsCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

void DBInterface::setViewPoint(const unsigned int id, const string& value)
{
    if (!db_connection_)
    {
        logwrn << "DBInterface: setViewPoint: failed since no database connection exists";
        return;
    }

    // boost::mutex::scoped_lock locker(connection_mutex_); // done in closeConnection
    assert(db_connection_);

    if (!existsViewPointsTable())
        createViewPointsTable();

    string str = sql_generator_.getInsertViewPointStatement(id, value);

    logdbg << "DBInterface: setViewPoint: cmd '" << str << "'";
    db_connection_->executeSQL(str);
}

map<unsigned int, string> DBInterface::viewPoints()
{
    loginf << "DBInterface: viewPoints";

    assert (existsViewPointsTable());

    boost::mutex::scoped_lock locker(connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllViewPointsStatement());

    PropertyList list;
    list.addProperty("id", PropertyDataType::UINT);
    list.addProperty("json", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = db_connection_->execute(command);

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
    boost::mutex::scoped_lock locker(connection_mutex_);
    db_connection_->executeSQL(sql_generator_.getDeleteStatement(TABLE_NAME_VIEWPOINTS,
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
    db_connection_->executeSQL(sql_generator_.getTableSectorsCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}


void DBInterface::clearSectorsTable()
{
    clearTableContent(TABLE_NAME_SECTORS);
}


void DBInterface::saveSector(shared_ptr<Sector> sector)
{
    loginf << "DBInterface: saveSector: sector " << sector->name() << " layer " << sector->layerName()
           << " id " << sector->id();

    if (!db_connection_)
    {
        logwrn << "DBInterface: saveSector: failed since no database connection exists";
        return;
    }

    assert(db_connection_);

    if (!existsSectorsTable())
        createSectorsTable();

    // insert and replace
    string str = sql_generator_.getReplaceSectorStatement(sector->id(), sector->name(), sector->layerName(),
                                                          sector->jsonDataStr());

    logdbg << "DBInterface: saveSector: cmd '" << str << "'";
    {
        boost::mutex::scoped_lock locker(connection_mutex_);
        db_connection_->executeSQL(str);
    }


}

void DBInterface::deleteSector(shared_ptr<Sector> sector)
{
    unsigned int sector_id = sector->id();

    boost::mutex::scoped_lock locker(connection_mutex_);
    string cmd = sql_generator_.getDeleteStatement(TABLE_NAME_SECTORS,"id="+to_string(sector_id));

    //loginf << "UGA '" << cmd << "'";
    db_connection_->executeSQL(cmd);
}

void DBInterface::deleteAllSectors()
{
    clearTableContent(TABLE_NAME_SECTORS);
}

bool DBInterface::existsTargetsTable()
{
    return existsTable(TABLE_NAME_TARGETS);
}

void DBInterface::createTargetsTable()
{
    loginf << "DBInterface: createTargetsTable";

    assert(!existsTargetsTable());

    connection_mutex_.lock();

    loginf << "DBInterface: createTargetsTable: sql '" << sql_generator_.getTableTargetsCreateStatement() << "'";
    db_connection_->executeSQL(sql_generator_.getTableTargetsCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

void DBInterface::clearTargetsTable()
{
    clearTableContent(TABLE_NAME_TARGETS);
}

std::vector<std::unique_ptr<dbContent::Target>> DBInterface::loadTargets()
{
    loginf << "DBInterface: loadTargets";

    boost::mutex::scoped_lock locker(connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllTargetsStatement());

    PropertyList list;
    list.addProperty("utn", PropertyDataType::UINT);
    list.addProperty("json", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = db_connection_->execute(command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    assert(buffer->has<unsigned int>("utn"));
    assert(buffer->has<string>("json"));

    NullableVector<unsigned int> utn_vec = buffer->get<unsigned int>("utn");
    NullableVector<string> json_vec = buffer->get<string>("json");

    unsigned int utn;
    string json_str;

    std::vector<std::unique_ptr<dbContent::Target>> targets;
    std::set<unsigned int> existing_utns;

    for (size_t cnt = 0; cnt < buffer->size(); ++cnt)
    {
        assert(!utn_vec.isNull(cnt));
        assert(!json_vec.isNull(cnt));

        utn = utn_vec.get(cnt);
        json_str = json_vec.get(cnt);

        assert (!existing_utns.count(utn));
        existing_utns.insert(utn);

        //shared_ptr<dbContent::Target> target = make_shared<dbContent::Target>(utn, nlohmann::json::parse(json_str));

        targets.emplace_back(new dbContent::Target(utn, nlohmann::json::parse(json_str)));

//        loginf << "DBInterface: loadTargets: loaded target " << utn << " json '"
//               << json_str << "'";
    }

    return targets;
}

void DBInterface::saveTargets(const std::vector<std::unique_ptr<dbContent::Target>>& targets)
{
    loginf << "DBInterface: saveTargets";

    assert (db_connection_);

    clearTargetsTable();

    string str;

    for (auto& tgt_it : targets)
    {
        string str = sql_generator_.getInsertTargetStatement(tgt_it->utn_, tgt_it->info().dump());
        db_connection_->executeSQL(str);
    }

    loginf << "DBInterface: saveTargets: done";
}

void DBInterface::saveTarget(const std::unique_ptr<dbContent::Target>& target)
{
    loginf << "DBInterface: saveTarget: utn " << target->utn();

    string str = sql_generator_.getInsertTargetStatement(target->utn_, target->info().dump());
    // uses replace with utn as unique key
    db_connection_->executeSQL(str);
}


void DBInterface::insertBuffer(DBContent& dbcontent, std::shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: insertBuffer: dbo " << dbcontent.name() << " buffer size "
           << buffer->size();

    // create table if required
    if (!existsTable(dbcontent.dbTableName()))
        createTable(dbcontent);

    // create record numbers & and store new max rec num
    {
        assert (dbcontent.hasVariable(DBContent::meta_var_rec_num_.name()));

        Variable& rec_num_var = dbcontent.variable(DBContent::meta_var_rec_num_.name());
        assert (rec_num_var.dataType() == PropertyDataType::UINT);

        string rec_num_col_str = rec_num_var.dbColumnName();
        assert (!buffer->has<unsigned int>(rec_num_col_str));

        buffer->addProperty(rec_num_col_str, PropertyDataType::UINT);

        assert (COMPASS::instance().dbContentManager().hasMaxRecordNumber());
        unsigned int max_rec_num = COMPASS::instance().dbContentManager().maxRecordNumber();

        NullableVector<unsigned int>& rec_num_vec = buffer->get<unsigned int>(rec_num_col_str);

        unsigned int buffer_size = buffer->size();

        for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
        {
            ++max_rec_num;
            rec_num_vec.set(cnt, max_rec_num);
        }

        COMPASS::instance().dbContentManager().maxRecordNumber(max_rec_num);
    }

    insertBuffer(dbcontent.dbTableName(), buffer);
}

void DBInterface::insertBuffer(const string& table_name, shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: insertBuffer: table name " << table_name << " buffer size "
           << buffer->size();

    assert(db_connection_);
    assert(buffer);

    if (!existsTable(table_name))
        throw runtime_error("DBInterface: insertBuffer: table with name '" + table_name +
                            "' does not exist");

    const PropertyList& properties = buffer->properties();

    { // check table exists and has correct columns
        boost::mutex::scoped_lock locker(table_info_mutex_);
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
    }

    string bind_statement = sql_generator_.insertDBUpdateStringBind(buffer, table_name);

    boost::mutex::scoped_lock locker(connection_mutex_);

    logdbg << "DBInterface: insertBuffer: preparing bind statement";
    db_connection_->prepareBindStatement(bind_statement);
    db_connection_->beginBindTransaction();

    logdbg << "DBInterface: insertBuffer: starting inserts";
    size_t size = buffer->size();

    for (unsigned int cnt = 0; cnt < size; ++cnt)
    {
        insertBindStatementUpdateForCurrentIndex(buffer, cnt);
    }

    logdbg << "DBInterface: insertBuffer: ending bind transactions";
    db_connection_->endBindTransaction();
    logdbg << "DBInterface: insertBuffer: finalizing bind statement";
    db_connection_->finalizeBindStatement();
}

void DBInterface::updateBuffer(const std::string& table_name, const std::string& key_col,
                               shared_ptr<Buffer> buffer, int from_index, int to_index)
{
    logdbg << "DBInterface: updateBuffer: table " << table_name << " buffer size "
           << buffer->size() << " key " << key_col;

    assert(db_connection_);
    assert(buffer);

    // TODO check
    //    const PropertyList& properties = buffer->properties();

    //    for (unsigned int cnt = 0; cnt < properties.size(); cnt++)
    //    {
    //        if (!table.hasColumn(properties.at(cnt).name()))
    //            throw runtime_error("DBInterface: updateBuffer: column '" +
    //                                properties.at(cnt).name() + "' does not exist in table " +
    //                                table.name());
    //    }

    string bind_statement =
            sql_generator_.createDBUpdateStringBind(buffer, key_col, table_name);

    boost::mutex::scoped_lock locker(connection_mutex_);

    logdbg << "DBInterface: updateBuffer: preparing bind statement '" << bind_statement << "'";
    db_connection_->prepareBindStatement(bind_statement);
    db_connection_->beginBindTransaction();

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
    db_connection_->endBindTransaction();
    logdbg << "DBInterface: updateBuffer: finalizing bind statement";
    db_connection_->finalizeBindStatement();

    logdbg << "DBInterface: updateBuffer: changes " << db_connection_->changes() << " indexes " << to_index - from_index +1;
}

void DBInterface::prepareRead(
        const DBContent& dbobject, VariableSet read_list, string custom_filter_clause,
        bool use_order, Variable* order_variable)
{
    assert(db_connection_);

    assert(dbobject.existsInDB());

    connection_mutex_.lock();

    shared_ptr<DBCommand> read = sql_generator_.getSelectCommand(
                dbobject, read_list, custom_filter_clause, use_order, order_variable);

    logdbg << "DBInterface: prepareRead: dbo " << dbobject.name() << " sql '" << read->get() << "'";
    db_connection_->prepareCommand(read);
}

shared_ptr<Buffer> DBInterface::readDataChunk(const DBContent& dbobject)
{
    // locked by prepareRead
    assert(db_connection_);

    shared_ptr<DBResult> result = db_connection_->stepPreparedCommand(read_chunk_size_);

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

    buffer->dbContentName(dbobject.name());

    assert(buffer);

    bool last_one = db_connection_->getPreparedCommandDone();
    buffer->lastOne(last_one);

    return buffer;
}

void DBInterface::finalizeReadStatement(const DBContent& dbobject)
{
    connection_mutex_.unlock();
    assert(db_connection_);

    logdbg << "DBInterface: finishReadSystemTracks: start ";
    db_connection_->finalizeCommand();
}

void DBInterface::deleteBefore(const DBContent& dbcontent, boost::posix_time::ptime before_timestamp)
{
    connection_mutex_.lock();
    assert(db_connection_);

    std::shared_ptr<DBCommand> command = sql_generator_.getDeleteCommand(dbcontent, before_timestamp);

    db_connection_->execute(*command.get());

    connection_mutex_.unlock();
}

void DBInterface::createPropertiesTable()
{
    assert(!existsPropertiesTable());
    connection_mutex_.lock();
    db_connection_->executeSQL(sql_generator_.getTablePropertiesCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

void DBInterface::clearTableContent(const string& table_name)
{
    boost::mutex::scoped_lock locker(connection_mutex_);
    // DELETE FROM tablename;
    db_connection_->executeSQL("DELETE FROM " + table_name + ";");
}

void DBInterface::insertBindStatementUpdateForCurrentIndex(shared_ptr<Buffer> buffer,
                                                           unsigned int buffer_index)
{
    assert(buffer);
    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: start";
    const PropertyList& list = buffer->properties();
    unsigned int size = list.size();
    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: creating bind for " << size
           << " elements";

    unsigned int index_cnt = 0;

    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: starting for loop";
    for (unsigned int property_cnt = 0; property_cnt < size; property_cnt++)
    {
        const Property& property = list.at(property_cnt);
        PropertyDataType data_type = property.dataType();

//        loginf << "DBInterface: insertBindStatementUpdateForCurrentIndex: at cnt " << cnt << " id "
//               << property.name() << " index cnt " << index_cnt;

        index_cnt = property_cnt + 1;

        if (buffer->isNull(property, buffer_index))
        {
            db_connection_->bindVariableNull(index_cnt);
//            logwrn << "DBInterface: insertBindStatementUpdateForCurrentIndex: at " << property.name()
//                   << " buffer_index " << buffer_index << " is null";
            continue;
        }

        switch (data_type)
        {
        case PropertyDataType::BOOL:
            db_connection_->bindVariable(
                        index_cnt, static_cast<int>(buffer->get<bool>(property.name()).get(buffer_index)));
            break;
        case PropertyDataType::CHAR:
            db_connection_->bindVariable(
                        index_cnt, static_cast<int>(buffer->get<char>(property.name()).get(buffer_index)));
            break;
        case PropertyDataType::UCHAR:
            db_connection_->bindVariable(
                        index_cnt,
                        static_cast<int>(buffer->get<unsigned char>(property.name()).get(buffer_index)));
            break;
        case PropertyDataType::INT:
            db_connection_->bindVariable(
                        index_cnt, buffer->get<int>(property.name()).get(buffer_index));
            break;
        case PropertyDataType::UINT:
            db_connection_->bindVariable(
                        index_cnt, static_cast<int>(buffer->get<unsigned int>(property.name()).get(buffer_index)));
            break;
        case PropertyDataType::LONGINT:
            db_connection_->bindVariable(index_cnt, buffer->get<long>(property.name()).get(buffer_index));
            break;
        case PropertyDataType::ULONGINT:
            db_connection_->bindVariable(
                        index_cnt, static_cast<long>(buffer->get<unsigned long>(property.name()).get(buffer_index)));
            break;
        case PropertyDataType::FLOAT:
            db_connection_->bindVariable(
                        index_cnt, static_cast<double>(buffer->get<float>(property.name()).get(buffer_index)));
            break;
        case PropertyDataType::DOUBLE:
            db_connection_->bindVariable(index_cnt,
                                         buffer->get<double>(property.name()).get(buffer_index));
            break;
        case PropertyDataType::STRING:
            db_connection_->bindVariable(
                        index_cnt, buffer->get<string>(property.name()).get(buffer_index));
            break;
        case PropertyDataType::JSON:
            db_connection_->bindVariable(
                        index_cnt, buffer->get<nlohmann::json>(property.name()).get(buffer_index).dump());
            break;
        case PropertyDataType::TIMESTAMP:
            db_connection_->bindVariable(
                        index_cnt, Time::toLong(buffer->get<boost::posix_time::ptime>(property.name()).get(buffer_index)));
            break;
        default:
            logerr << "Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type "
                       << Property::asString(data_type);
            throw runtime_error(
                        "Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type " +
                        Property::asString(data_type));
        }
    }

    db_connection_->stepAndClearBindings();

    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: done";
}



