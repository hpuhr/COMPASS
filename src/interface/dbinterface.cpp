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
#include "sqliteconnection.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbresult.h"
#include "dbtableinfo.h"
#include "sqliteconnection.h"
#include "files.h"
#include "util/timeconv.h"
#include "util/number.h"
#include "sector.h"
#include "sectorlayer.h"
#include "source/dbdatasource.h"
#include "fft/dbfft.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/target/target.h"
#include "viewpoint.h"
#include "duckdbconnection.h"
#include "dbconnection.h"
#include "sqlgenerator.h"

#include <QApplication>
#include <QMessageBox>
#include <QMutexLocker>
#include <QThread>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem/path.hpp>

#include <fstream>

using namespace Utils;
using namespace std;
using namespace dbContent;

const string PROP_TIMESTAMP_MIN_NAME {"timestamp_min"};
const string PROP_TIMESTAMP_MAX_NAME {"timestamp_max"};
const string PROP_LATITUDE_MIN_NAME  {"latitude_min"};
const string PROP_LATITUDE_MAX_NAME  {"latitude_max"};
const string PROP_LONGITUDE_MIN_NAME {"longitude_min"};
const string PROP_LONGITUDE_MAX_NAME {"longitude_max"};

/**
 */
DBInterface::DBInterface(string class_id, 
                         string instance_id, 
                         COMPASS* compass)
:   Configurable(class_id, instance_id, compass)
{
    boost::mutex::scoped_lock locker(connection_mutex_);

    registerParameter("read_chunk_size", &read_chunk_size_, 50000u);

    createSubConfigurables();
}

/**
 */
DBInterface::~DBInterface()
{
    logdbg << "DBInterface: desctructor: start";

    boost::mutex::scoped_lock locker(connection_mutex_);

    db_connection_ = nullptr;

    logdbg << "DBInterface: desctructor: end";
}

/**
 */
bool DBInterface::connectionNeedsPreciseDBTypes() const
{
    return db_connection_ ? db_connection_->needsPreciseDBTypes() : true;
}

/**
 */
SQLGenerator DBInterface::sqlGenerator() const
{
    return SQLGenerator(connectionNeedsPreciseDBTypes());
}

/**
 */
void DBInterface::openDBFile(const std::string& filename, bool overwrite)
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    db_filename_ = "";

    if (overwrite && Files::fileExists(filename))
    {
        loginf << "DBInterface: openDBFile: deleting pre-existing file '" << filename << "'";
        Files::deleteFile(filename);
    }

    bool created_new_db = !Files::fileExists(filename);

    auto ext = boost::filesystem::path(filename).extension().string();
    if (ext == ".duckdb")
    {
        db_connection_.reset(new DuckDBConnection(this));
    }
    else
    {
        db_connection_.reset(new SQLiteConnection(this));
    }

    loginf << "DBInterface: openDBFile: opening file '" << filename << "'";

    assert (db_connection_);
    auto connect_result = db_connection_->connect(filename);
    if (!connect_result.first)
    {
        //@TODO: error handling
    }

    db_filename_ = filename;

    db_connection_->updateTableInfo();

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

/**
 */
void DBInterface::exportDBFile(const std::string& filename)
{
    assert (dbOpen());

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    boost::mutex::scoped_lock locker(connection_mutex_);

    db_connection_->exportFile(filename);

    QApplication::restoreOverrideCursor();
}

/**
 */
void DBInterface::closeDBFile()
{
    loginf << "DBInterface: closeDBFile";

    {
        boost::mutex::scoped_lock locker(connection_mutex_);

        if (properties_loaded_)  // false if database not opened
            saveProperties();

        assert (db_connection_);
        db_connection_->disconnect();

        db_filename_       = "";
        properties_loaded_ = false;

        properties_.clear();

        dbcolumn_content_flags_.clear();
    }

    // signal emitted in COMPASS
}

/**
 */
bool DBInterface::dbOpen()
{
    assert (db_connection_);
    return db_connection_->dbOpened();
}

/**
 */
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

/**
 */
const DBConnection& DBInterface::connection() const
{
    assert(db_connection_);
    return *db_connection_;
}

/**
 */
void DBInterface::generateSubConfigurable(const string& class_id,
                                          const string& instance_id)
{
    throw std::runtime_error("DBInterface: generateSubConfigurable: unknown class_id " + class_id);
}

/**
 */
const std::map<std::string, DBTableInfo>& DBInterface::tableInfo() 
{ 
    assert(db_connection_);
    boost::mutex::scoped_lock locker(connection_mutex_);
    return db_connection_->tableInfo();
}

/**
 */
bool DBInterface::existsTable(const string& table_name)
{
    return tableInfo().count(table_name) == 1;
}

/**
 */
void DBInterface::createTable(const DBContent& object)
{
    assert(db_connection_);

    loginf << "DBInterface: createTable: obj " << object.name();
    if (existsTable(object.dbTableName()))
    {
        logerr << "DBInterface: createTable: table " << object.dbTableName() << " already exists";
        return;
    }

    string statement = sqlGenerator().getCreateTableStatement(object, db_connection_->useIndexing());

    boost::mutex::scoped_lock locker(connection_mutex_);

    db_connection_->execute(statement, false);
    db_connection_->updateTableInfo();

    locker.unlock();

    loginf << "DBInterface: createTable: checking " << object.dbTableName();
    assert(existsTable(object.dbTableName()));
}

/**
 */
bool DBInterface::existsPropertiesTable() 
{ 
    return existsTable(TABLE_NAME_PROPERTIES); 
}

/**
 */
unsigned long DBInterface::getMaxRecordNumber(DBContent& object)
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

    shared_ptr<DBCommand> command = sqlGenerator().getMaxULongIntValueCommand(object.dbTableName(),
                                                                              rec_num_var.dbColumnName());

    shared_ptr<DBResult> result = db_connection_->execute(*command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    if (!buffer->size())
    {
        logwrn << "DBInterface: getMaxRecordNumber: no max record number found";
        return 0;
    }

    assert (!buffer->get<unsigned long>(rec_num_var.dbColumnName()).isNull(0));
    return buffer->get<unsigned long>(rec_num_var.dbColumnName()).get(0);
}

/**
 */
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

    shared_ptr<DBCommand> command = sqlGenerator().getMaxUIntValueCommand(reftraj_content.dbTableName(),
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

/**
 */
bool DBInterface::existsDataSourcesTable()
{
    return existsTable(dbContent::DBDataSource::table_name_);
}

/**
 */
void DBInterface::createDataSourcesTable()
{
    assert(!existsDataSourcesTable());
    connection_mutex_.lock();
    db_connection_->execute(sqlGenerator().getTableDataSourcesCreateStatement(), false);
    db_connection_->updateTableInfo();
    connection_mutex_.unlock();
}

/**
 */
std::vector<std::unique_ptr<dbContent::DBDataSource>> DBInterface::getDataSources()
{
    logdbg << "DBInterface: getDataSources: start";

    using namespace dbContent;

    boost::mutex::scoped_lock locker(connection_mutex_);

    shared_ptr<DBCommand> command = sqlGenerator().getDataSourcesSelectCommand();

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

/**
 */
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

/**
 */
bool DBInterface::existsFFTsTable()
{
    return existsTable(DBFFT::table_name_);
}

/**
 */
void DBInterface::createFFTsTable()
{
    assert(!existsFFTsTable());
    connection_mutex_.lock();
    db_connection_->execute(sqlGenerator().getTableFFTsCreateStatement(), false);
    db_connection_->updateTableInfo();
    connection_mutex_.unlock();
}

/**
 */
std::vector<std::unique_ptr<DBFFT>> DBInterface::getFFTs()
{
    logdbg << "DBInterface: getFFTs: start";

    using namespace dbContent;

    boost::mutex::scoped_lock locker(connection_mutex_);

    shared_ptr<DBCommand> command = sqlGenerator().getFFTSelectCommand();

    loginf << "DBInterface: getFFTs: sql '" << command->get() << "'";

    shared_ptr<DBResult> result = db_connection_->execute(*command);
    assert(result->containsData());
    shared_ptr<Buffer> buffer = result->buffer();

    logdbg << "DBInterface: getFFTs: json '" << buffer->asJSON().dump(4) << "'";

    assert(buffer->properties().hasProperty(DBFFT::name_column_));
    assert(buffer->properties().hasProperty(DBFFT::info_column_));

    std::vector<std::unique_ptr<DBFFT>> sources;

    for (unsigned cnt = 0; cnt < buffer->size(); cnt++)
    {
        if (buffer->get<string>(DBFFT::name_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getFFTs: data source cnt " << cnt
                   << " has NULL name, will be omitted";
            continue;
        }

        std::unique_ptr<DBFFT> src {new DBFFT()};

        src->name(buffer->get<string>(DBFFT::name_column_.name()).get(cnt));

        if (!buffer->get<string>(DBFFT::info_column_.name()).isNull(cnt))
            src->info(nlohmann::json::parse(buffer->get<string>(DBFFT::info_column_.name()).get(cnt)));

        sources.emplace_back(std::move(src));
    }

    return sources;
}

/**
 */
void DBInterface::saveFFTs(const std::vector<std::unique_ptr<DBFFT>>& ffts)
{
    logdbg << "DBInterface: saveFFTs: num " << ffts.size();

    using namespace dbContent;

    assert (dbOpen());

    if (existsFFTsTable())
        clearTableContent(DBFFT::table_name_);
    else
        createFFTsTable();

    PropertyList list;
    list.addProperty(DBFFT::name_column_);
    list.addProperty(DBFFT::info_column_);

    shared_ptr<Buffer> buffer = make_shared<Buffer>(list);

    unsigned int cnt = 0;
    for (auto& ds_it : ffts)
    {
        buffer->get<string>(DBFFT::name_column_.name()).set(cnt, ds_it->name());
        buffer->get<string>(DBFFT::info_column_.name()).set(cnt, ds_it->infoStr());

        ++cnt;
    }

    logdbg << "DBInterface: saveFFTs: buffer size " << buffer->size();

    insertBuffer(DBFFT::table_name_, buffer);

    logdbg << "DBInterface: saveFFTs: done";
}

/**
 */
size_t DBInterface::count(const string& table)
{
    logdbg << "DBInterface: count: table " << table;
    assert(existsTable(table));

    boost::mutex::scoped_lock locker(connection_mutex_);
    assert(db_connection_);

    string sql = sqlGenerator().getCountStatement(table);

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

/**
 */
void DBInterface::setProperty(const string& id, const string& value)
{
    properties_[id] = value;
}

/**
 */
string DBInterface::getProperty(const string& id)
{
    assert(hasProperty(id));
    return properties_.at(id);
}

/**
 */
void DBInterface::removeProperty(const std::string& id)
{
    assert(hasProperty(id));
    properties_.erase(id);
}

/**
 */
bool DBInterface::hasProperty(const string& id) 
{ 
    return properties_.count(id); 
}

/**
 */
void DBInterface::loadProperties()
{
    loginf << "DBInterface: loadProperties";

    assert (!properties_loaded_);

    boost::mutex::scoped_lock locker(connection_mutex_);

    DBCommand command;
    command.set(sqlGenerator().getSelectAllPropertiesStatement());

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

    // column with content
    if (properties_.count(dbcolumn_content_property_name_))
    {
        nlohmann::json dbcolumn_content_json = nlohmann::json::parse(properties_.at(dbcolumn_content_property_name_));
        dbcolumn_content_flags_ = dbcolumn_content_json.get<std::map<std::string, std::set<std::string>>>();
    }

    properties_loaded_ = true;
}

/**
 */
bool DBInterface::hasContentIn(const std::string& table_name, const std::string& column_name) const
{
    if (!properties_loaded_)
        return false;

    if (dbcolumn_content_flags_.count(table_name))
        return dbcolumn_content_flags_.at(table_name).count(column_name);

    return false;
}

/**
 */
void DBInterface::setContentIn(const std::string& table_name, const std::string& column_name)
{
    if (!properties_loaded_)
        return;

    if (!dbcolumn_content_flags_[table_name].count(column_name))
        dbcolumn_content_flags_[table_name].insert(column_name);
}

/**
 */
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

    nlohmann::json dbcolumn_content_json = dbcolumn_content_flags_;
    properties_[dbcolumn_content_property_name_] = dbcolumn_content_json.dump();

    string str;

    for (auto& prop_it : properties_)
    {
        string str = sqlGenerator().getInsertPropertyStatement(prop_it.first, prop_it.second);
        db_connection_->execute(str, false);
    }

    loginf << "DBInterface: saveProperties: done";
}

/**
 */
std::vector<std::shared_ptr<SectorLayer>> DBInterface::loadSectors()
{
    loginf << "DBInterface: loadSectors";

    boost::mutex::scoped_lock locker(connection_mutex_);

    DBCommand command;
    command.set(sqlGenerator().getSelectAllSectorsStatement());

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

        auto eval_sector = new Sector(id, name, layer_name, true);
        bool ok = eval_sector->readJSON(json_str);
        assert(ok);

        string layer_name = eval_sector->layerName();

        (*lay_it)->addSector(shared_ptr<Sector>(eval_sector));
        assert ((*lay_it)->hasSector(name));

        loginf << "DBInterface: loadSectors: loaded sector '" << name << "' in layer '"
               << layer_name << "' num points " << (*lay_it)->sector(name)->size();
    }

    return sector_layers;
}

/**
 */
bool DBInterface::areColumnsNull (const std::string& table_name, const std::vector<std::string> columns)
{
    boost::mutex::scoped_lock locker(connection_mutex_);

    string str = sqlGenerator().getSelectNullCount(table_name, columns);

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

/**
 */
bool DBInterface::existsViewPointsTable()
{
    return existsTable(TABLE_NAME_VIEWPOINTS);
}

/**
 */
void DBInterface::createViewPointsTable()
{
    assert(!existsViewPointsTable());

    setProperty("view_points_version", ViewPoint::VP_COLLECTION_CONTENT_VERSION);

    connection_mutex_.lock();
    db_connection_->execute(sqlGenerator().getTableViewPointsCreateStatement(), false);
    db_connection_->updateTableInfo();
    connection_mutex_.unlock();
}

/**
 */
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

    string str = sqlGenerator().getInsertViewPointStatement(id, value);

    logdbg << "DBInterface: setViewPoint: cmd '" << str << "'";
    db_connection_->execute(str, false);
}

/**
 */
map<unsigned int, string> DBInterface::viewPoints()
{
    loginf << "DBInterface: viewPoints";

    assert (existsViewPointsTable());

    boost::mutex::scoped_lock locker(connection_mutex_);

    DBCommand command;
    command.set(sqlGenerator().getSelectAllViewPointsStatement());

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

/**
 */
void DBInterface::deleteViewPoint(const unsigned int id)
{
    boost::mutex::scoped_lock locker(connection_mutex_);
    db_connection_->execute(sqlGenerator().getDeleteStatement(TABLE_NAME_VIEWPOINTS, "id="+to_string(id)), false);
}

/**
 */
void DBInterface::deleteAllViewPoints()
{
    clearTableContent(TABLE_NAME_VIEWPOINTS);
}

/**
 */
bool DBInterface::existsSectorsTable()
{
    return existsTable(TABLE_NAME_SECTORS);
}

/**
 */
void DBInterface::createSectorsTable()
{
    loginf << "DBInterface: createSectorsTable";

    assert(!existsSectorsTable());

    connection_mutex_.lock();

    loginf << "DBInterface: createSectorsTable: sql '" << sqlGenerator().getTableSectorsCreateStatement() << "'";
    db_connection_->execute(sqlGenerator().getTableSectorsCreateStatement(), false);
    db_connection_->updateTableInfo();
    connection_mutex_.unlock();
}

/**
 */
void DBInterface::clearSectorsTable()
{
    clearTableContent(TABLE_NAME_SECTORS);
}

/**
 */
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
    string str = sqlGenerator().getReplaceSectorStatement(sector->id(), sector->name(), sector->layerName(),
                                                          sector->jsonDataStr());

    logdbg << "DBInterface: saveSector: cmd '" << str << "'";
    {
        boost::mutex::scoped_lock locker(connection_mutex_);
        db_connection_->execute(str, false);
    }
}

/**
 */
void DBInterface::deleteSector(shared_ptr<Sector> sector)
{
    unsigned int sector_id = sector->id();

    boost::mutex::scoped_lock locker(connection_mutex_);
    string cmd = sqlGenerator().getDeleteStatement(TABLE_NAME_SECTORS,"id="+to_string(sector_id));

    //loginf << "UGA '" << cmd << "'";
    db_connection_->execute(cmd, false);
}

/**
 */
void DBInterface::deleteAllSectors()
{
    clearTableContent(TABLE_NAME_SECTORS);
}

/**
 */
bool DBInterface::existsTargetsTable()
{
    return existsTable(TABLE_NAME_TARGETS);
}

/**
 */
void DBInterface::createTargetsTable()
{
    loginf << "DBInterface: createTargetsTable";

    assert(!existsTargetsTable());

    connection_mutex_.lock();

    loginf << "DBInterface: createTargetsTable: sql '" << sqlGenerator().getTableTargetsCreateStatement() << "'";
    db_connection_->execute(sqlGenerator().getTableTargetsCreateStatement(), false);
    db_connection_->updateTableInfo();

    connection_mutex_.unlock();
}

/**
 */
void DBInterface::clearTargetsTable()
{
    clearTableContent(TABLE_NAME_TARGETS);
}

/**
 */
std::vector<std::unique_ptr<dbContent::Target>> DBInterface::loadTargets()
{
    loginf << "DBInterface: loadTargets";

    boost::mutex::scoped_lock locker(connection_mutex_);

    DBCommand command;
    command.set(sqlGenerator().getSelectAllTargetsStatement());

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

/**
 */
void DBInterface::saveTargets(const std::vector<std::unique_ptr<dbContent::Target>>& targets)
{
    loginf << "DBInterface: saveTargets";

    assert (db_connection_);

    clearTargetsTable();

    string str;

    for (auto& tgt_it : targets)
    {
        string str = sqlGenerator().getInsertTargetStatement(tgt_it->utn_, tgt_it->info().dump());
        db_connection_->execute(str, false);
    }

    loginf << "DBInterface: saveTargets: done";
}

/**
 */
void DBInterface::saveTarget(const std::unique_ptr<dbContent::Target>& target)
{
    loginf << "DBInterface: saveTarget: utn " << target->utn();

    string str = sqlGenerator().getInsertTargetStatement(target->utn_, target->info().dump());
    // uses replace with utn as unique key
    db_connection_->execute(str, false);
}

/**
 */
void DBInterface::clearAssociations(const DBContent& dbcontent)
{
    string str = sqlGenerator().getSetNullStatement(dbcontent.dbTableName(),
                                                    dbcontent.variable("UTN").dbColumnName());
    // uses replace with utn as unique key
    db_connection_->execute(str, false);
}

/**
 */
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
        assert (rec_num_var.dataType() == PropertyDataType::ULONGINT);

        string rec_num_col_str = rec_num_var.dbColumnName();
        assert (!buffer->has<unsigned long>(rec_num_col_str));

        buffer->addProperty(rec_num_col_str, PropertyDataType::ULONGINT);

        assert (COMPASS::instance().dbContentManager().hasMaxRecordNumberWODBContentID());
        unsigned long max_rec_num = COMPASS::instance().dbContentManager().maxRecordNumberWODBContentID();

        NullableVector<unsigned long>& rec_num_vec = buffer->get<unsigned long>(rec_num_col_str);

        unsigned int buffer_size = buffer->size();
        unsigned int dbcont_id = dbcontent.id();


        for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
        {
            ++max_rec_num;
            rec_num_vec.set(cnt, Number::recNumAddDBContId(max_rec_num, dbcont_id));
        }

        COMPASS::instance().dbContentManager().maxRecordNumberWODBContentID(max_rec_num);
    }

    insertBuffer(dbcontent.dbTableName(), buffer);
}

/**
 */
void DBInterface::insertBuffer(const string& table_name, 
                               shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: insertBuffer: table name " << table_name << " buffer size "
           << buffer->size();

    assert(db_connection_);
    assert(buffer);

    if (!existsTable(table_name))
        throw runtime_error("DBInterface: insertBuffer: table with name '" + table_name +
                            "' does not exist");

    if (!db_connection_->insertBuffer(table_name, buffer))
        throw runtime_error("DBInterface: insertBuffer: inserting into table '" + table_name + "' failed");
}

/**
 */
void DBInterface::updateBuffer(const std::string& table_name, const std::string& key_col,
                               shared_ptr<Buffer> buffer, int from_index, int to_index)
{
    logdbg << "DBInterface: updateBuffer: table " << table_name << " buffer size "
           << buffer->size() << " key " << key_col;

    assert(db_connection_);
    assert(buffer);

    if (!existsTable(table_name))
        throw runtime_error("DBInterface: updateBuffer: table with name '" + table_name +
                            "' does not exist");

    if (!db_connection_->updateBuffer(table_name, buffer, key_col, from_index, to_index))
        throw runtime_error("DBInterface: updateBuffer: updating table '" + table_name + "' failed");
}

/**
 */
void DBInterface::prepareRead(const DBContent& dbobject, 
                              VariableSet read_list, 
                              string custom_filter_clause,
                              bool use_order, 
                              Variable* order_variable)
{
    assert(db_connection_);

    assert(dbobject.existsInDB());

    connection_mutex_.lock();

    shared_ptr<DBCommand> read = sqlGenerator().getSelectCommand(
                dbobject, read_list, custom_filter_clause, use_order, order_variable);

    logdbg << "DBInterface: prepareRead: dbo " << dbobject.name() << " sql '" << read->get() << "'";
    db_connection_->prepareCommand(read);
}

/**
 */
std::pair<std::shared_ptr<Buffer>, bool> DBInterface::readDataChunk(const DBContent& dbobject)
{
    // locked by prepareRead
    assert(db_connection_);

    shared_ptr<DBResult> result;
    bool last_one = false;

    std::tie(result, last_one) = db_connection_->stepPreparedCommand(read_chunk_size_);

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

    return {buffer, last_one};
}

/**
 */
void DBInterface::finalizeReadStatement(const DBContent& dbobject)
{
    connection_mutex_.unlock();
    assert(db_connection_);

    logdbg << "DBInterface: finishReadSystemTracks: start ";
    db_connection_->finalizeCommand();
}

/**
 */
void DBInterface::deleteBefore(const DBContent& dbcontent, 
                               boost::posix_time::ptime before_timestamp)
{
    connection_mutex_.lock();
    assert(db_connection_);

    std::shared_ptr<DBCommand> command = sqlGenerator().getDeleteCommand(dbcontent, before_timestamp);

    db_connection_->execute(*command.get());

    connection_mutex_.unlock();
}

/**
 */
void DBInterface::deleteAll(const DBContent& dbcontent)
{
    connection_mutex_.lock();
    assert(db_connection_);

    std::shared_ptr<DBCommand> command = sqlGenerator().getDeleteCommand(dbcontent);

    db_connection_->execute(*command.get());

    connection_mutex_.unlock();
}

/**
 */
void DBInterface::deleteContent(const DBContent& dbcontent, 
                                unsigned int sac, 
                                unsigned int sic)
{
    loginf << "DBInterface: deleteContent: dbcontent " << dbcontent.name()
           << " sac/sic " << sac << "/" << sic;

    connection_mutex_.lock();
    assert(db_connection_);

    std::shared_ptr<DBCommand> command = sqlGenerator().getDeleteCommand(dbcontent, sac, sic);

    db_connection_->execute(*command.get());

    connection_mutex_.unlock();
}

/**
 */
void DBInterface::deleteContent(const DBContent& dbcontent, unsigned int sac, unsigned int sic, unsigned int line_id)
{
    loginf << "DBInterface: deleteContent: dbcontent " << dbcontent.name()
           << " sac/sic " << sac << "/" << sic << " line " << line_id;

    connection_mutex_.lock();
    assert(db_connection_);

    std::shared_ptr<DBCommand> command = sqlGenerator().getDeleteCommand(dbcontent, sac, sic, line_id);

    db_connection_->execute(*command.get());

    connection_mutex_.unlock();
}

/**
 */
void DBInterface::createPropertiesTable()
{
    assert(!existsPropertiesTable());
    connection_mutex_.lock();
    db_connection_->execute(sqlGenerator().getTablePropertiesCreateStatement(), false);
    db_connection_->updateTableInfo();
    connection_mutex_.unlock();
}

/**
 */
void DBInterface::clearTableContent(const string& table_name)
{
    boost::mutex::scoped_lock locker(connection_mutex_);
    // DELETE FROM tablename;
    db_connection_->execute("DELETE FROM " + table_name + ";", false);
}
