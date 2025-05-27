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

#include "dbinstance.h"
#include "dbcommand.h"
#include "dbresult.h"
#include "dbtableinfo.h"
#include "dbconnection.h"
#include "sqlgenerator.h"

#include "duckdbinstance.h"

#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/target/target.h"

#include "sector.h"
#include "sectorlayer.h"
#include "source/dbdatasource.h"
#include "fft/dbfft.h"

#include "task/taskmanager.h"
#include "task/result/taskresult.h"
#include "task/result/report/section.h"
#include "task/result/report/sectioncontent.h"
#include "task/result/report/sectioncontentfigure.h"
#include "task/result/report/sectioncontenttable.h"
#include "task/result/report/sectioncontenttext.h"

#include "eval/results/evaluationtaskresult.h"

#include "viewpoint.h"

#include "compass.h"
#include "buffer.h"
#include "config.h"
#include "files.h"
#include "timeconv.h"
#include "number.h"
#include "asynctask.h"

#include "tbbhack.h"

#include <QApplication>
#include <QMessageBox>
#include <QMutexLocker>
#include <QThread>
#include <QElapsedTimer>

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

#define PROTECT_INSTANCE

/**
 */
DBInterface::DBInterface(string class_id, 
                         string instance_id, 
                         COMPASS* compass)
:   Configurable(class_id, instance_id, compass)
,   insert_mt_(true)
{
    registerParameter("read_chunk_size", &read_chunk_size_, 50000u);

    createSubConfigurables();
}

/**
 */
DBInterface::~DBInterface()
{
    logdbg << "DBInterface: desctructor: start";

    db_instance_ = nullptr;

    logdbg << "DBInterface: desctructor: end";
}

/**
 * Returns a connection-specific SQL generator.
 */
SQLGenerator DBInterface::sqlGenerator() const
{
    assert(db_instance_);
    return SQLGenerator(db_instance_->sqlConfiguration());
}

/**
 * Internal reset on fail.
 */
void DBInterface::reset()
{
    properties_loaded_ = false;
    properties_.clear();

    dbcolumn_content_flags_.clear();

    if (db_instance_)
    {
        db_instance_->close();
        db_instance_.reset();
    }
}

/**
 */
std::string DBInterface::dbFilename() const
{
    if (!db_instance_)
        return "";

    return db_instance_->dbFilename();
}

/**
 */
bool DBInterface::dbInMemory() const
{
    if (!db_instance_)
        return false;

    return db_instance_->dbInMem();
}

/**
 */
bool DBInterface::canCreateDBFileFromMemory() const
{
    return (ready() && dbInMemory());
}

/**
 */
bool DBInterface::ready() const
{
    if (!db_instance_)
        return false;
    
    return db_instance_->dbReady(); //ready means open and connected
}

/**
 * Opens a database file.
 */
void DBInterface::openDBFile(const std::string& filename, bool overwrite)
{
    assert(!filename.empty());

    bool new_file = !Files::fileExists(filename) || overwrite;

    openDBFileInternal(filename, overwrite);

    loginf << "DBInterface: openDBFile: new_file " << new_file;

    if (new_file)
        COMPASS::instance().logInfo("DBInterface") << "Database '" << filename << "' created";
}

/**
 * Opens an in-memory database.
 */
void DBInterface::openDBInMemory()
{
    openDBFileInternal("", false);
}

/**
 */
void DBInterface::openDBFileInternal(const std::string& filename, bool overwrite)
{
    try
    {
        bool in_mem = filename.empty();
        std::string filename_internal = in_mem ? DBInstance::InMemFilename : filename;

        //delete old file?
        if (!in_mem && overwrite && Files::fileExists(filename))
        {
            loginf << "DBInterface: openDBFile: deleting pre-existing file '" << filename << "'";
            Files::deleteFile(filename);
        }

        bool created_new_db = in_mem || !Files::fileExists(filename);

        //create connection
        db_instance_.reset(new DuckDBInstance(this));
        
        loginf << "DBInterface: openDBFile: opening file '" << filename_internal << "'";

        //connect to db
        assert (db_instance_);

        auto open_result = in_mem ? db_instance_->openInMemory() : db_instance_->open(filename);

        if (!open_result.ok())
        {
            logerr << "DBInterface: openDBFile: Database could not be opened: " << open_result.second;
            throw std::runtime_error ("Database could not be opened: " + open_result.second);
        }

        if (created_new_db)
        {
            //create some default tables
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

            //check db version against app version
            if (!hasProperty("APP_VERSION")
                    || getProperty("APP_VERSION") != COMPASS::instance().config().getString("version"))
            {
                string reason = hasProperty("APP_VERSION") ?
                            "DB from Version " + getProperty("APP_VERSION") + ", current "
                            + COMPASS::instance().config().getString("version") : "DB from Version older than 0.7.0";

                properties_loaded_ = false;
                properties_.clear();
                db_instance_->close();

                logerr << "DBInterface: openDBFile: Incorrect application version for database:\n " << reason;
                throw std::runtime_error ("Incorrect application version for database:\n "+reason);
            }

            assert (existsDataSourcesTable());
            assert (existsSectorsTable());
        }

        if (!existsTargetsTable())
            createTargetsTable();

        //determine maximum report content id
        auto max_id = getMaxReportContentID();
        ResultReport::Section::setCurrentContentID(max_id.has_value() ? max_id.value() + 1 : 0);

        //emit databaseOpenedSignal();

        loginf << "DBInterface: openDBFile: done";
    }
    catch(const std::exception& ex)
    {
        reset();

        logerr << "DBInterface: openDBFile: Error: " << ex.what();
        throw std::runtime_error(ex.what());
    }
}

/**
 * Transfers an opened memory database to the given file and switches to this version.
 */
void DBInterface::openDBFileFromMemory(const std::string& filename)
{
    assert(canCreateDBFileFromMemory());
    assert(!filename.empty());

    try
    {
        if (Files::fileExists(filename))
            Files::deleteFile(filename);

        //export memory database to given file
        auto export_result = db_instance_->exportToFile(filename);

        if (!export_result.ok())
        {
            logerr << "DBInterface: openDBFileFromMemory: Database could not be exported: " << export_result.second;
            throw std::runtime_error ("Database could not be exported: " + export_result.second);
        }

        //close current (in-mem) db
        closeDB();

        //open new physical db
        openDBFile(filename, false);
    }
    catch(const std::exception& ex)
    {
        reset();

        logerr << "DBInterface: openDBFileFromMemory: Error: " << ex.what();
        throw std::runtime_error(ex.what());
    }
}

/**
 * Closes the currently open database.
 */
void DBInterface::closeDB()
{
    loginf << "DBInterface: closeDB";

    if (properties_loaded_)  // false if database not opened
        saveProperties();

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        assert (db_instance_);
        db_instance_->close();

        properties_loaded_ = false;
        properties_.clear();

        dbcolumn_content_flags_.clear();
    }

    // signal emitted in COMPASS
}

/**
 * Exports the currently opened database to the given file.
 */
void DBInterface::exportDBFile(const std::string& filename)
{
    assert (ready());

    Result export_result;

    //delete any existing file
    if (Utils::Files::fileExists(filename))
        Utils::Files::deleteFile(filename);

    try
    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        export_result = db_instance_->exportToFile(filename);
    }
    catch(const std::exception& ex)
    {
        logerr << "DBInterface: exportDBFile: Database could not be exported: " << ex.what();
        throw std::runtime_error ("Database could not be exported: " + std::string(ex.what()));
    }

    if (!export_result.ok())
    {
        logerr << "DBInterface: exportDBFile: Database could not be exported: " << export_result.second;
        throw std::runtime_error ("Database could not be exported: " + export_result.second);
    }
}

/**
 * Cleans the current db.
 * Note that the current database will be closed and reopened for this purpose.
 */
bool DBInterface::cleanupDB(bool show_dialog)
{
    loginf << "DBInterface: cleanupDB";

    assert (ready());

    //in-mem => nothing to do
    if (dbInMemory())
        return true;

    bool ok;

    if (show_dialog)
    {
        auto cb = [ this ] (const AsyncTaskState&, AsyncTaskProgressWrapper&) 
        { 
            return this->cleanupDBInternal();
        };

        AsyncFuncTask task(cb, "Optimizing Database", "Optimizing database", false);
        ok = task.runAsyncDialog();
    }
    else
    {
        ok = cleanupDBInternal().ok();
    }

    return ok;
}

/**
 */
Result DBInterface::cleanupDBInternal()
{
    loginf << "DBInterface: cleanupDBInternal: cleaning db...";

    assert(ready());
    assert(!cleanup_in_progress_);
    assert(!dbInMemory());

    cleanup_in_progress_ = true;

    Result res_cleanup, res_critical;

    try
    {
        auto res_reconnect = db_instance_->reconnect(true, &res_cleanup);

        //reconnection shall never fail
        if (!res_reconnect.ok())
            throw std::runtime_error("Reconnecting to database failed: " + res_reconnect.error());

        if (!res_cleanup.ok())
        {
            //cleanup didn't work => log and return false
            logerr << "DBInterface: cleanupDBInternal: Cleanup failed: " << res_cleanup.error();
        }

        res_critical = Result::succeeded();
    }
    catch(const std::exception& ex)
    {
        res_critical = Result::failed(ex.what());
    }

    cleanup_in_progress_ = false;

    if (!res_critical.ok())
    {
        //@TODO: correct way to resolve this worst case?

        reset();

        logerr << "DBInterface: cleanupDBInternal: Error: " << res_critical.error();
        throw std::runtime_error(res_critical.error());
    }

    return res_cleanup;
}

/**
 */
void DBInterface::generateSubConfigurable(const string& class_id,
                                          const string& instance_id)
{
    throw std::runtime_error("DBInterface: generateSubConfigurable: unknown class_id " + class_id);
}



/**
 * !Protect by mutex when calling!
 */
Result DBInterface::execute(const std::string& sql)
{
    assert(ready());

    auto res = db_instance_->defaultConnection().execute(sql);

    if (!res.ok())
    {
        logerr << "DBInterface: execute: Error executing statement '" << sql << "': " << res.error();
        throw std::runtime_error("Error executing statement '" + sql + "': " + res.error());
    }

    return res;
}

/**
 * !Protect by mutex when calling!
 */
std::shared_ptr<DBResult> DBInterface::execute(const DBCommand& cmd)
{
    assert(ready());

    auto res = db_instance_->defaultConnection().execute(cmd);

    if (res->hasError())
    {
        logerr << "DBInterface: execute: Error executing command '" << cmd.get() << "': " << res->error();
        throw std::runtime_error("Error executing command '" + cmd.get() + "': " + res->error());
    }

    return res;
}

/**
 * !Protect by mutex when calling!
 */
void DBInterface::updateTableInfo()
{
    assert(ready());

    auto res = db_instance_->updateTableInfo();

    if (!res.ok())
    {
        logerr << "DBInterface: updateTableInfo: Error updating table info: " << res.error();
        throw std::runtime_error("Error updating table info: " + res.error());
    }
}

/**
 */
const std::map<std::string, DBTableInfo>& DBInterface::tableInfo() const
{ 
    assert(ready());

    //@TODO: needs protection?
    return db_instance_->tableInfo();
}

/**
 */
bool DBInterface::existsTable(const string& table_name) const
{
    return tableInfo().count(table_name) == 1;
}

/**
 */
void DBInterface::createTable(const DBContent& object)
{
    assert(ready());

    loginf << "DBInterface: createTable: obj " << object.name();
    if (existsTable(object.dbTableName()))
    {
        logerr << "DBInterface: createTable: table " << object.dbTableName() << " already exists";
        return;
    }

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        string statement = sqlGenerator().getCreateTableStatement(object);

        execute(statement);
        updateTableInfo();
    }

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
    assert (ready());
    assert(object.existsInDB());

    assert (COMPASS::instance().dbContentManager().existsMetaVariable(DBContent::meta_var_rec_num_.name()));
    assert (COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_rec_num_.name()).existsIn(object.name()));

    Variable& rec_num_var = COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_rec_num_.name()).getFor(object.name());

    assert (object.hasVariable(rec_num_var.name()));

    unsigned long max_rn = 0;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        shared_ptr<DBCommand> command = sqlGenerator().getMaxULongIntValueCommand(object.dbTableName(),
                                                                                  rec_num_var.dbColumnName());
        shared_ptr<DBResult> result = execute(*command);
        assert(result->containsData());

        shared_ptr<Buffer> buffer = result->buffer();

        if (!buffer->size())
        {
            logwrn << "DBInterface: getMaxRecordNumber: no max record number found";
            return 0;
        }

        assert (!buffer->get<unsigned long>(rec_num_var.dbColumnName()).isNull(0));

        max_rn = buffer->get<unsigned long>(rec_num_var.dbColumnName()).get(0);
    }

    return max_rn;
}

/**
 */
unsigned int DBInterface::getMaxRefTrackTrackNum()
{
    assert(ready());

    DBContent& reftraj_content = COMPASS::instance().dbContentManager().dbContent("RefTraj");

    if(!reftraj_content.existsInDB())
        return 0;

    assert (COMPASS::instance().dbContentManager().existsMetaVariable(DBContent::meta_var_track_num_.name()));
    assert (COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_track_num_.name()).existsIn("RefTraj"));

    Variable& track_num_var = COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_track_num_.name()).getFor("RefTraj");

    assert (reftraj_content.hasVariable(track_num_var.name()));

    unsigned int max_tn = 0;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        shared_ptr<DBCommand> command = sqlGenerator().getMaxUIntValueCommand(reftraj_content.dbTableName(),
                                                                            track_num_var.dbColumnName());
        shared_ptr<DBResult> result = execute(*command);
        assert(result->containsData());

        shared_ptr<Buffer> buffer = result->buffer();

        if (!buffer->size())
        {
            logwrn << "DBInterface: getMaxRefTrackTrackNum: no max track number found";
            return 0;
        }

        assert (!buffer->get<unsigned int>(track_num_var.dbColumnName()).isNull(0));

        max_tn = buffer->get<unsigned int>(track_num_var.dbColumnName()).get(0);
    }

    return max_tn;
}

/**
 */
boost::optional<unsigned long> DBInterface::getMaxReportContentID()
{
    assert(ready());

    if (!existsReportContentsTable())
        return boost::optional<unsigned long>();

    unsigned int max_tn = 0;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        shared_ptr<DBCommand> command = sqlGenerator().getMaxUIntValueCommand(ResultReport::SectionContent::DBTableName,
                                                                              ResultReport::SectionContent::DBColumnContentID.name());
        shared_ptr<DBResult> result = execute(*command);
        assert(result->containsData());

        shared_ptr<Buffer> buffer = result->buffer();
        assert(buffer->size() <= 1);

        if (buffer->size() == 0)
            return boost::optional<unsigned long>();

        assert(!buffer->get<unsigned int>(ResultReport::SectionContent::DBColumnContentID.name()).isNull(0));

        max_tn = buffer->get<unsigned int>(ResultReport::SectionContent::DBColumnContentID.name()).get(0);
    }

    return max_tn;
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

//    shared_ptr<DBResult> result = execute(*command);

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
    assert(ready());
    assert(!existsDataSourcesTable());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        execute(sqlGenerator().getTableDataSourcesCreateStatement());
        updateTableInfo();
    }
}

/**
 */
std::vector<std::unique_ptr<dbContent::DBDataSource>> DBInterface::getDataSources()
{
    logdbg << "DBInterface: getDataSources: start";

    assert(ready());

    using namespace dbContent;

    std::vector<std::unique_ptr<DBDataSource>> sources;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        shared_ptr<DBCommand> command = sqlGenerator().getDataSourcesSelectCommand();

        loginf << "DBInterface: getDataSources: sql '" << command->get() << "'";

        shared_ptr<DBResult> result = execute(*command);
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
    }

    return sources;
}

/**
 */
void DBInterface::saveDataSources(const std::vector<std::unique_ptr<dbContent::DBDataSource>>& data_sources)
{
    loginf << "DBInterface: saveDataSources: num " << data_sources.size();

    using namespace dbContent;

    assert (ready());

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
    assert(ready());
    assert(!existsFFTsTable());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        execute(sqlGenerator().getTableFFTsCreateStatement());
        updateTableInfo();
    }
}

/**
 */
std::vector<std::unique_ptr<DBFFT>> DBInterface::getFFTs()
{
    logdbg << "DBInterface: getFFTs: start";

    assert(ready());

    using namespace dbContent;

    std::vector<std::unique_ptr<DBFFT>> sources;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        shared_ptr<DBCommand> command = sqlGenerator().getFFTSelectCommand();

        logdbg << "DBInterface: getFFTs: sql '" << command->get() << "'";

        shared_ptr<DBResult> result = execute(*command);
        assert(result->containsData());
        shared_ptr<Buffer> buffer = result->buffer();

        logdbg << "DBInterface: getFFTs: json '" << buffer->asJSON().dump(4) << "'";

        assert(buffer->properties().hasProperty(DBFFT::name_column_));
        assert(buffer->properties().hasProperty(DBFFT::info_column_));

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
    }

    return sources;
}

/**
 */
void DBInterface::saveFFTs(const std::vector<std::unique_ptr<DBFFT>>& ffts)
{
    logdbg << "DBInterface: saveFFTs: num " << ffts.size();

    using namespace dbContent;

    assert (ready());

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

    assert(ready());
    assert(existsTable(table));

    int tmp;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        string sql = sqlGenerator().getCountStatement(table);

        logdbg << "DBInterface: count: sql '" << sql << "'";

        DBCommand command;
        command.set(sql);

        PropertyList list;
        list.addProperty("count", PropertyDataType::INT);
        command.list(list);

        shared_ptr<DBResult> result = execute(command);
        assert(result->containsData());

        tmp = result->buffer()->get<int>("count").get(0);
    }

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

    assert(ready());
    assert (!properties_loaded_);

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        DBCommand command;
        command.set(sqlGenerator().getSelectAllPropertiesStatement());

        PropertyList list;
        list.addProperty("id", PropertyDataType::STRING);
        list.addProperty("value", PropertyDataType::STRING);
        command.list(list);

        shared_ptr<DBResult> result = execute(command);
        assert(result->containsData());

        shared_ptr<Buffer> buffer = result->buffer();

        assert(buffer);
        assert(buffer->has<string>("id"));
        assert(buffer->has<string>("value"));

        NullableVector<string>& id_vec = buffer->get<string>("id");
        NullableVector<string>& value_vec = buffer->get<string>("value");

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

    if (!db_instance_)
    {
        logwrn << "DBInterface: saveProperties: failed since no database instance exists";
        return;
    }

    // boost::mutex::scoped_lock locker(connection_mutex_); // done in closeConnection
    assert (properties_loaded_);

    nlohmann::json dbcolumn_content_json = dbcolumn_content_flags_;
    properties_[dbcolumn_content_property_name_] = dbcolumn_content_json.dump();

    string str;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        for (auto& prop_it : properties_)
        {
            auto sql = sqlGenerator().getInsertPropertyStatement(prop_it.first, prop_it.second);
            execute(sql);
        }
    }

    loginf << "DBInterface: saveProperties: done";
}

/**
 */
std::vector<std::shared_ptr<SectorLayer>> DBInterface::loadSectors()
{
    loginf << "DBInterface: loadSectors";

    assert(ready());

    std::vector<std::shared_ptr<SectorLayer>> sector_layers;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        DBCommand command;
        command.set(sqlGenerator().getSelectAllSectorsStatement());

        PropertyList list;
        list.addProperty("id", PropertyDataType::INT);
        list.addProperty("name", PropertyDataType::STRING);
        list.addProperty("layer_name", PropertyDataType::STRING);
        list.addProperty("json", PropertyDataType::STRING);
        command.list(list);

        shared_ptr<DBResult> result = execute(command);
        assert(result->containsData());

        shared_ptr<Buffer> buffer = result->buffer();

        assert(buffer);
        assert(buffer->has<int>("id"));
        assert(buffer->has<string>("name"));
        assert(buffer->has<string>("layer_name"));
        assert(buffer->has<string>("json"));

        NullableVector<int>& id_vec = buffer->get<int>("id");
        NullableVector<string>& name_vec = buffer->get<string>("name");
        NullableVector<string>& layer_name_vec = buffer->get<string>("layer_name");
        NullableVector<string>& json_vec = buffer->get<string>("json");

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
    }

    return sector_layers;
}

/**
 */
bool DBInterface::areColumnsNull(const std::string& table_name, 
                                 const std::vector<std::string> columns)
{
    assert(ready());

    bool cols_null = false;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        string str = sqlGenerator().getSelectNullCount(table_name, columns);

        loginf << "DBInterface: areColumnsNull: sql '" << str << "'";

        DBCommand command;
        command.set(str);

        PropertyList list;
        list.addProperty("count", PropertyDataType::INT);
        command.list(list);

        shared_ptr<DBResult> result = execute(command);
        assert(result->containsData());

        shared_ptr<Buffer> buffer = result->buffer();

        assert(buffer);
        assert(buffer->has<int>("count"));

        NullableVector<int>& count_vec = buffer->get<int>("count");
        assert (count_vec.contentSize() == 1);

        cols_null = count_vec.get(0) != 0;

        loginf << "DBInterface: areColumnsNull: null count " << count_vec.get(0);
    }

    return cols_null;
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
    assert(ready());
    assert(!existsViewPointsTable());

    setProperty("view_points_version", ViewPoint::VP_COLLECTION_CONTENT_VERSION);

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        execute(sqlGenerator().getTableViewPointsCreateStatement());
        updateTableInfo();
    }
}

/**
 */
void DBInterface::setViewPoint(const unsigned int id, const string& value)
{
    if (!db_instance_)
    {
        logwrn << "DBInterface: setViewPoint: failed since no database instance exists";
        return;
    }

    if (!existsViewPointsTable())
        createViewPointsTable();

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        string str = sqlGenerator().getInsertViewPointStatement(id, value);
        logdbg << "DBInterface: setViewPoint: cmd '" << str << "'";

        execute(str);
    }
}

/**
 */
map<unsigned int, string> DBInterface::viewPoints()
{
    loginf << "DBInterface: viewPoints";

    assert(ready());
    assert(existsViewPointsTable());

    map<unsigned int, string> view_points;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        DBCommand command;
        command.set(sqlGenerator().getSelectAllViewPointsStatement());

        PropertyList list;
        list.addProperty("id", PropertyDataType::UINT);
        list.addProperty("json", PropertyDataType::STRING);
        command.list(list);

        shared_ptr<DBResult> result = execute(command);
        assert(result->containsData());

        shared_ptr<Buffer> buffer = result->buffer();

        assert(buffer);
        assert(buffer->has<unsigned int>("id"));
        assert(buffer->has<string>("json"));
      
        NullableVector<unsigned int>& id_vec = buffer->get<unsigned int>("id");
        NullableVector<string>& json_vec = buffer->get<string>("json");

        for (size_t cnt = 0; cnt < buffer->size(); ++cnt)
        {
            assert(!id_vec.isNull(cnt));
            assert(!view_points.count(id_vec.get(cnt)));
            if (!json_vec.isNull(cnt))
                view_points[id_vec.get(cnt)] = json_vec.get(cnt);
        }
    }

    loginf << "DBInterface: loadViewPoints: loaded " << view_points.size() << " view points";

    return view_points;
}

/**
 */
void DBInterface::deleteViewPoint(const unsigned int id)
{
    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        auto sql = sqlGenerator().getDeleteStatement(TABLE_NAME_VIEWPOINTS, "id="+to_string(id));
        execute(sql);
    }
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

    assert(ready());
    assert(!existsSectorsTable());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        auto sql = sqlGenerator().getTableSectorsCreateStatement();

        loginf << "DBInterface: createSectorsTable: sql '" << sql << "'";

        execute(sql);
        updateTableInfo();
    }
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

    if (!db_instance_)
    {
        logwrn << "DBInterface: saveSector: failed since no database instance exists";
        return;
    }

    assert(ready());

    if (!existsSectorsTable())
        createSectorsTable();

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        // insert and replace
        string str = sqlGenerator().getReplaceSectorStatement(sector->id(), sector->name(), sector->layerName(), sector->jsonDataStr());
        logdbg << "DBInterface: saveSector: cmd '" << str << "'";

        execute(str);
    }
}

/**
 */
void DBInterface::deleteSector(shared_ptr<Sector> sector)
{
    assert(ready());

    unsigned int sector_id = sector->id();

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        string cmd = sqlGenerator().getDeleteStatement(TABLE_NAME_SECTORS,"id="+to_string(sector_id));
        execute(cmd);
    }
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

    assert(ready());
    assert(!existsTargetsTable());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        auto sql = sqlGenerator().getTableTargetsCreateStatement();

        loginf << "DBInterface: createTargetsTable: sql '" << sql << "'";

        execute(sql);
        updateTableInfo();
    }
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

    assert(ready());

    std::vector<std::unique_ptr<dbContent::Target>> targets;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        DBCommand command;
        command.set(sqlGenerator().getSelectAllTargetsStatement());

        PropertyList list;
        list.addProperty("utn", PropertyDataType::UINT);
        list.addProperty("json", PropertyDataType::STRING);
        command.list(list);

        shared_ptr<DBResult> result = execute(command);
        assert(!result->hasError());
        assert(result->containsData());

        shared_ptr<Buffer> buffer = result->buffer();

        assert(buffer);
        assert(buffer->has<unsigned int>("utn"));
        assert(buffer->has<string>("json"));

        NullableVector<unsigned int>& utn_vec = buffer->get<unsigned int>("utn");
        NullableVector<string>& json_vec = buffer->get<string>("json");

        unsigned int utn;
        string json_str;

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
    }

    return targets;
}

/**
 */
void DBInterface::saveTargets(const std::vector<std::unique_ptr<dbContent::Target>>& targets)
{
    loginf << "DBInterface: saveTargets";

    assert(ready());

    clearTargetsTable();

    {
        //storing all targets at once via a buffer is faster 
        std::shared_ptr<Buffer> buffer(new Buffer(dbContent::Target::DBPropertyList));

        auto& id_vec   = buffer->get<unsigned int>(dbContent::Target::DBColumnID.name());
        auto& info_vec = buffer->get<nlohmann::json>(dbContent::Target::DBColumnInfo.name());

        size_t idx = 0;
        for (auto& tgt_it : targets)
        {
            id_vec.set(idx, tgt_it->utn_);
            info_vec.set(idx, tgt_it->info());
            ++idx;
        }

        insertBuffer(TABLE_NAME_TARGETS, buffer);
    }

    loginf << "DBInterface: saveTargets: done";
}

/**
 * Inserts or updates individual targets.
 * Note: Slow when applied to many targets, because each target is inserted/updated individually. 
 * Use updateTargets() to update many targets.
 */
void DBInterface::saveTargets(const std::vector<std::unique_ptr<dbContent::Target>>& targets,
                              const std::set<unsigned int>& utns)
{
    loginf << "DBInterface: saveTargets: saving " << utns.size() << " utn(s)";

    for (const auto& t : targets)
        if (utns.count(t->utn()))
            saveTarget(t);
}

/**
 */
void DBInterface::saveTarget(const std::unique_ptr<dbContent::Target>& target)
{
    loginf << "DBInterface: saveTarget: utn " << target->utn();

    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        string str = sqlGenerator().getInsertTargetStatement(target->utn_, target->info().dump());

        // uses replace with utn as unique key
        execute(str);
    }
}

/**
 * Updates existing targets.
 * Note: This is faster than saveTargets() when many targets are updated.
 */
void DBInterface::updateTargets(const std::vector<std::unique_ptr<dbContent::Target>>& targets,
                                const std::set<unsigned int>& utns)
{
    loginf << "DBInterface: updateTargets: updating " << utns.size() << " utn(s)";

    assert(ready());

    {
        //updating many targets at once via a buffer is faster 
        std::shared_ptr<Buffer> buffer(new Buffer(dbContent::Target::DBPropertyList));

        auto& id_vec   = buffer->get<unsigned int>(dbContent::Target::DBColumnID.name());
        auto& info_vec = buffer->get<nlohmann::json>(dbContent::Target::DBColumnInfo.name());

        size_t idx = 0;
        for (auto& tgt_it : targets)
        {
            if (utns.count(tgt_it->utn()) == 0)
                continue;

            id_vec.set(idx, tgt_it->utn_);
            info_vec.set(idx, tgt_it->info());
            ++idx;
        }

        updateBuffer(TABLE_NAME_TARGETS, Target::DBColumnID.name(), buffer);
    }
}

/**
 */
void DBInterface::clearAssociations(const DBContent& dbcontent)
{
    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        string str = sqlGenerator().getSetNullStatement(dbcontent.dbTableName(),
                                                        dbcontent.variable("UTN").dbColumnName());      
        // uses replace with utn as unique key
        execute(str);
    }
}

/**
 */
bool DBInterface::existsTaskResultsTable() const
{
    return existsTable(TaskResult::DBTableName);
}

/**
 */
bool DBInterface::existsReportContentsTable() const
{
    return existsTable(ResultReport::SectionContent::DBTableName);
}

/**
 */
void DBInterface::createTaskResultsTable()
{
    assert(ready());
    assert(!existsTaskResultsTable());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        execute(sqlGenerator().getTableTaskResultsCreateStatement());
        updateTableInfo();
    }
}

/**
 */
void DBInterface::createReportContentsTable()
{
    assert(ready());
    assert(!existsReportContentsTable());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        execute(sqlGenerator().getTableReportContentsCreateStatement());
        updateTableInfo();
    }
}

/**
 */
Result DBInterface::saveResult(const TaskResult& result, bool cleanup_db_if_needed)
{
    assert(ready());

    try
    {
        //create needed tables
        if (!existsTaskResultsTable())
            createTaskResultsTable();

        if (!existsReportContentsTable())
            createReportContentsTable();
        
        //remove any old result with the same id/name
        bool result_deleted = false;
        auto del_result = deleteResult(result, false, &result_deleted);
        if (!del_result.ok())
            throw std::runtime_error(del_result.error());

        auto result_id   = result.id();
        auto result_name = result.name();

        //write result
        {
            std::shared_ptr<Buffer> buffer(new Buffer(TaskResult::DBPropertyList));

            auto& id_vec      = buffer->get<unsigned int>(TaskResult::DBColumnID.name());
            auto& name_vec    = buffer->get<std::string>(TaskResult::DBColumnName.name());
            auto& content_vec = buffer->get<nlohmann::json>(TaskResult::DBColumnJSONContent.name());
            auto& type_vec    = buffer->get<int>(TaskResult::DBColumnResultType.name());

            id_vec.set(0, result_id);
            name_vec.set(0, result.name());
            content_vec.set(0, result.toJSON());
            type_vec.set(0, (int)result.type());

            insertBuffer(TaskResult::DBTableName, buffer);
        }

        //write contents
        auto report_contents = result.report()->reportContents(true);
        {
            size_t chunk_size_bytes = 1e09;
            size_t current_bytes    = 0;
            size_t current_row      = 0;

            std::shared_ptr<Buffer> buffer;

            NullableVector<unsigned int>*   content_id_vec = nullptr;
            NullableVector<unsigned int>*   result_id_vec  = nullptr;
            NullableVector<int>*            type_vec       = nullptr;
            NullableVector<nlohmann::json>* content_vec    = nullptr;

            for (const auto& c : report_contents)
            {
                if (!buffer || current_bytes > chunk_size_bytes)
                {
                    //insert old buffer if available
                    if (buffer)
                    {
                        //loginf << "DBInterface: saveResult: writing" << buffer->size() << " row(s)";
                        insertBuffer(ResultReport::SectionContent::DBTableName, buffer);
                    }

                    //create new buffer
                    buffer.reset(new Buffer(ResultReport::SectionContent::DBPropertyList));

                    content_id_vec = &buffer->get<unsigned int>(ResultReport::SectionContent::DBColumnContentID.name());
                    result_id_vec  = &buffer->get<unsigned int>(ResultReport::SectionContent::DBColumnResultID.name());
                    type_vec       = &buffer->get<int>(ResultReport::SectionContent::DBColumnType.name());
                    content_vec    = &buffer->get<nlohmann::json>(ResultReport::SectionContent::DBColumnJSONContent.name());

                    current_bytes = 0;
                    current_row   = 0;
                }

                assert(buffer);

                //!this might trigger recomputations from temporarily generated data,
                //which is immediately thrown away afterwards!
                auto   c_json  = c->toJSON();
                size_t c_bytes = c_json.dump().size();

                content_id_vec->set(current_row, c->contentID());
                result_id_vec->set(current_row, result_id);
                type_vec->set(current_row, (int)c->contentType());
                content_vec->set(current_row, c_json);

                current_bytes += c_bytes;
                current_row   += 1;
            }

            //insert remaining buffer content
            if (current_row > 0)
            {
                //loginf << "DBInterface: saveResult: writing" << buffer->size() << " row(s)";
                insertBuffer(ResultReport::SectionContent::DBTableName, buffer);
            }
        }

        //cleanup db?
        if (result_deleted && cleanup_db_if_needed)
            cleanupDB(false);
    }
    catch(const std::exception& ex)
    {
        logerr << "DBInterface: saveResult: Could not store result: " << ex.what();
        return Result::failed(ex.what());
    }
    catch(...)
    {
        logerr << "DBInterface: saveResult: Could not store result: Unknown error";
        return Result::failed("Unknown error");
    }

    return Result::succeeded();
}

/**
 */
Result DBInterface::deleteResult(const TaskResult& result, 
                                 bool cleanup_db_if_needed,
                                 bool* deleted)
{
    if (deleted)
        *deleted = false;

    auto id = result.id();

    if (!existsTaskResultsTable() || 
        !existsReportContentsTable())
    {
        logerr << "DBInterface: deleteResult: Result tables do not exist";
        return Result::failed("Result tables do not exist");
    }

    bool results_deleted = false;

    try
    {
        //get old result(s) which resemble(s) the new one (either in id or name)
        auto sel_filter = db::SQLFilter(TaskResult::DBColumnID.name(), std::to_string(id), db::SQLFilter::ComparisonOp::Is)
            .OR(TaskResult::DBColumnName.name(), "'" + result.name() + "'", db::SQLFilter::ComparisonOp::Is).statement();

        auto sel_result = select(TaskResult::DBTableName, TaskResult::DBPropertyList, sel_filter);
        if (!sel_result.ok())
            throw std::runtime_error("Locating old result failed");

        //collect id(s) of old result(s)
        std::vector<unsigned int> ids_to_remove;

        size_t n = sel_result.result()->size();
        const auto& id_vector = sel_result.result()->get<unsigned int>(TaskResult::DBColumnID.name());
        for (size_t i = 0; i < n; ++i)
            ids_to_remove.push_back(id_vector.get(i));

        loginf << "DBInterface: deleteResult: Deleting " << ids_to_remove.size() << " old result(s)";

        //delete old result(s)
        for (auto old_id : ids_to_remove)
        {
            auto del_content_cmd = sqlGenerator().getDeleteCommand(ResultReport::SectionContent::DBTableName,
                                                                   db::SQLFilter(ResultReport::SectionContent::DBColumnResultID.name(), 
                                                                                 std::to_string(old_id), 
                                                                                 db::SQLFilter::ComparisonOp::Is).statement());
            auto del_result_cmd = sqlGenerator().getDeleteCommand(TaskResult::DBTableName,
                                                                  db::SQLFilter(TaskResult::DBColumnID.name(), 
                                                                                std::to_string(old_id), 
                                                                                db::SQLFilter::ComparisonOp::Is).statement());
            auto result_del_content = execute(*del_content_cmd);
            auto result_del_result  = execute(*del_result_cmd );

            if (result_del_content->hasError())
                throw std::runtime_error(result_del_content->error());
            if (result_del_result->hasError())
                throw std::runtime_error(result_del_result->error());

            results_deleted = true;
        }
    }
    catch(const std::exception& ex)
    {
        logerr << "DBInterface: deleteResult: Could not delete result: " << ex.what();
        return Result::failed(ex.what());
    }
    catch(...)
    {
        logerr << "DBInterface: deleteResult: Could not delete result: Unknown error";
        return Result::failed("Unknown error");
    }

    if (deleted)
        *deleted = results_deleted;

    //cleanup db?
    if (results_deleted && cleanup_db_if_needed)
        cleanupDB(false);

    return Result::succeeded();
}

/**
 */
ResultT<std::vector<std::shared_ptr<TaskResult>>> DBInterface::loadResults()
{
    assert(ready());

    std::vector<std::shared_ptr<TaskResult>> results;

    //no results stored yet?
    if (!existsTaskResultsTable())
        return ResultT<std::vector<std::shared_ptr<TaskResult>>>::succeeded(results);

    try
    {
        auto cmd = sqlGenerator().getSelectCommand(TaskResult::DBTableName, TaskResult::DBPropertyList, "");
        auto result = execute(*cmd);
        if (result->hasError() || !result->containsData() || !result->buffer())
            throw std::runtime_error("Could not obtain results table");

        auto b = result->buffer();

        size_t nr = b->size();

        auto& id_vec      = b->get<unsigned int>(TaskResult::DBColumnID.name());
        auto& name_vec    = b->get<std::string>(TaskResult::DBColumnName.name());
        auto& content_vec = b->get<nlohmann::json>(TaskResult::DBColumnJSONContent.name());
        auto& type_vec    = b->get<int>(TaskResult::DBColumnResultType.name());

        results.resize(nr);

        auto& task_man = COMPASS::instance().taskManager();

        for (size_t i = 0; i < nr; ++i)
        {
            const auto& result_name = name_vec.get(i);
            const auto& result_type = type_vec.get(i);
            const auto& result_id   = id_vec.get(i);

            const auto& json_content = content_vec.get(i);
            if (!json_content.contains(TaskResult::FieldType))
                throw std::runtime_error("Missing type field in result JSON");

            int rtype = json_content.at(TaskResult::FieldType);

            results[ i ] = task_man.createResult(result_id, (task::TaskResultType)rtype);
            if (!results[ i ])
                throw std::runtime_error("Could not create result from type");

            bool ok = results[ i ]->fromJSON(json_content);
            if (!ok)
                throw std::runtime_error("Could not read result from JSON");

            if (results[ i ]->name() != result_name ||
                results[ i ]->type() != result_type ||
                results[ i ]->id()   != result_id)
                throw std::runtime_error("Result contents invalid");
        }
    }
    catch(const std::exception& ex)
    {
        logerr << "DBInterface: loadResults: Could not load results: " << ex.what();
        return ResultT<std::vector<std::shared_ptr<TaskResult>>>::failed(ex.what());
    }
    catch(...)
    {
        logerr << "DBInterface: loadResults: Could not load results: Unknown error";
        return ResultT<std::vector<std::shared_ptr<TaskResult>>>::failed("Unknown error");
    }

    return ResultT<std::vector<std::shared_ptr<TaskResult>>>::succeeded(results);
}

/**
 */
ResultT<std::shared_ptr<ResultReport::SectionContent>> DBInterface::loadContent(ResultReport::Section* section, 
                                                                                unsigned int content_id)
{
    assert(ready());

    std::shared_ptr<ResultReport::SectionContent> content;

    loginf << "DBInterface: loadContent: Loading content id " << content_id << "...";

    try
    {
        PropertyList properties;
        properties.addProperty(ResultReport::SectionContent::DBColumnContentID);
        properties.addProperty(ResultReport::SectionContent::DBColumnResultID);
        properties.addProperty(ResultReport::SectionContent::DBColumnType);
        properties.addProperty(ResultReport::SectionContent::DBColumnJSONContent);

        auto filter_col = ResultReport::SectionContent::DBColumnContentID.name();

        auto cmd = sqlGenerator().getSelectCommand(ResultReport::SectionContent::DBTableName, 
                                                   ResultReport::SectionContent::DBPropertyList,
                                                   db::SQLFilter(filter_col, std::to_string(content_id), db::SQLFilter::ComparisonOp::Is).statement());
        auto result = execute(*cmd);

        if (result->hasError() || !result->containsData() || !result->buffer())
            throw std::runtime_error("Could not obtain content from table");

        auto b = result->buffer();

        size_t nr = b->size();
        if (nr == 0)
            throw std::runtime_error("Content id not found in table");
        else if (nr > 1)
            throw std::runtime_error("Multiple content ids found in table");

        auto& content_id_vec = b->get<unsigned int>(ResultReport::SectionContent::DBColumnContentID.name());
        auto& type_vec       = b->get<int>(ResultReport::SectionContent::DBColumnType.name());
        auto& content_vec    = b->get<nlohmann::json>(ResultReport::SectionContent::DBColumnJSONContent.name());

        ResultReport::SectionContent::ContentType type = (ResultReport::SectionContent::ContentType)type_vec.get(0);

        //create empty content depending on type
        //@TODO: small factory?
        if (type == ResultReport::SectionContent::ContentType::Figure)
        {
            content.reset(new ResultReport::SectionContentFigure(section));
        }
        else if (type == ResultReport::SectionContent::ContentType::Table)
        {
            content.reset(new ResultReport::SectionContentTable(section));
        }
        else if (type == ResultReport::SectionContent::ContentType::Text)
        {
            content.reset(new ResultReport::SectionContentText(section));
        }
        else
        {
            throw std::runtime_error("Invalid content type");
        }

        //read content
        bool ok = content->fromJSON(content_vec.get(0));
        if (!ok)
            throw std::runtime_error("Could not read content from JSON");

        //check content
        if (content->contentID() != content_id_vec.get(0) ||
            content->contentType() != type)
            throw std::runtime_error("contents invalid");
    }
    catch(const std::exception& ex)
    {
        logerr << "DBInterface: loadContent: Could not load content: " << ex.what();
        return ResultT<std::shared_ptr<ResultReport::SectionContent>>::failed(ex.what());
    }
    catch(...)
    {
        logerr << "DBInterface: loadContent: Could not load content: Unknown error";
        return ResultT<std::shared_ptr<ResultReport::SectionContent>>::failed("Unknown error");
    }

    loginf << "DBInterface: loadContent: Loaded.";

    return ResultT<std::shared_ptr<ResultReport::SectionContent>>::succeeded(content);
}

/**
 */
void DBInterface::initDBContentBuffer(DBContent& dbcontent, 
                                      std::shared_ptr<Buffer> buffer)
{
    // create record numbers & and store new max rec num
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

/**
 */
void DBInterface::insertDBContent(DBContent& dbcontent, std::shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: insertDBContent: dbo " << dbcontent.name() << " buffer size " << buffer->size();

    assert(ready());
    assert(buffer);

    // create table if required
    if (!existsTable(dbcontent.dbTableName()))
        createTable(dbcontent);

    initDBContentBuffer(dbcontent, buffer);
    insertBuffer(dbcontent.dbTableName(), buffer);
}

/**
 */
void DBInterface::insertBuffer(const string& table_name, 
                               shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: insertBuffer: table name " << table_name << " buffer size " << buffer->size();

    assert(ready());
    assert(buffer);

    if (!existsTable(table_name))
    {
        logerr << "DBInterface: insertBuffer: table with name '" << table_name << "' does not exist";
        throw runtime_error("DBInterface: insertBuffer: table with name '" + table_name + "' does not exist");
    }

    Result res;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        res = db_instance_->defaultConnection().insertBuffer(table_name, buffer);
    }

    if (!res.ok())
    {
        logerr << "DBInterface: insertBuffer: inserting into table '" << table_name << "' failed: " << res.error();
        throw runtime_error("DBInterface: insertBuffer: inserting into table '" + table_name + "' failed: " + res.error());
    }
}

/**
 * Inserts multiple dbcontent buffers at once, possibly utilizing parallelization.
 */
void DBInterface::insertDBContent(const std::map<std::string, std::shared_ptr<Buffer>>& buffers)
{
    assert(ready());

    bool db_supports_mt = db_instance_->sqlConfiguration().supports_mt;
    bool exec_mt        = insert_mt_ && db_supports_mt && buffers.size() > 1;

    size_t max_size = 0;
    size_t min_size = std::numeric_limits<size_t>::max();
    for (const auto& it : buffers)
    {
        if (it.second->size() < min_size)
            min_size = it.second->size();
        if (it.second->size() > max_size)
            max_size = it.second->size();
    }

    logdbg << "DBInterface: insertDBContent: inserting " << buffers.size() << " object(s) " 
           << (exec_mt ? "multi-threaded" : "single-threaded") 
           << " min_size " << (buffers.size() ? min_size : 0) << " max_size " << (buffers.size() ? max_size : 0);

    auto& dbc_manager = COMPASS::instance().dbContentManager();

    struct Job
    {
        Job() = default;
        Job(size_t idx,
            const std::string& tname,
            const std::shared_ptr<Buffer>& b,
            const boost::optional<size_t>& from,
            const boost::optional<size_t>& to)
        :   index   (idx  )
        ,   table   (tname)
        ,   buffer  (b    )
        ,   idx_from(from )
        ,   idx_to  (to   ) {}

        size_t                  index;
        std::string             table;
        std::shared_ptr<Buffer> buffer;
        boost::optional<size_t> idx_from;
        boost::optional<size_t> idx_to;
    };

    std::vector<Job> insert_jobs;

    int chunk_size = -1;

    //init single-threaded
    size_t idx = 0;
    for (auto& it : buffers)
    {
        assert(it.second);

        auto& dbcontent = dbc_manager.dbContent(it.first);

        //create table if needed
        if (!existsTable(dbcontent.dbTableName()))
            createTable(dbcontent);

        //init buffer
        initDBContentBuffer(dbcontent, it.second);

        if (chunk_size < 1 || !exec_mt)
        {
            insert_jobs.push_back(Job(idx++, dbcontent.dbTableName(), it.second, {}, {}));
        }
        else
        {
            auto n = it.second->size();
            for (size_t idx = 0; idx < n; idx += (size_t)chunk_size)
            {
                size_t idx_start = idx;
                size_t idx_end   = std::min(idx_start + chunk_size, n);

                assert(idx_start < idx_end);

                insert_jobs.push_back(Job(idx++, dbcontent.dbTableName(), it.second, idx_start, idx_end - 1));
            }
        }
    }

    unsigned int n = insert_jobs.size();

    logdbg << "DBInterface: insertDBContent: created " << n << " job(s)";

    //create connections for multithreading (if supported)
    std::vector<DBConnectionWrapper> mt_connections;
    if (db_supports_mt)
    {
        mt_connections.resize(n);

        for (unsigned int i = 0; i < n; ++i)
        {
            mt_connections[ i ] = db_instance_->concurrentConnection(i);
            assert(!mt_connections[ i ].isEmpty());

            if (mt_connections[ i ].hasError())
            {
                logerr << "DBInterface: insertDBContent: creating mt connection failed: " << mt_connections[ i ].error();
                throw runtime_error("DBInterface: insertDBContent: creating mt connection failed: " + mt_connections[ i ].error());
            }
        }
    }

    //insert buffers
    std::vector<Result> results(n, 0);
    if (exec_mt)
    {
        //insert multithreaded (if supported)
        tbb::parallel_for(uint(0), n, [ & ](unsigned int i) 
        {
            auto& job = insert_jobs.at(i);
            results.at(i) = mt_connections.at(i).connection().insertBuffer(job.table, job.buffer, job.idx_from, job.idx_to);
        });
    }
    else
    {
        //single threaded insert
        for (unsigned int i = 0; i < n; ++i)
        {
            auto& job = insert_jobs.at(i);
            results.at(i) = db_instance_->defaultConnection().insertBuffer(job.table, job.buffer, job.idx_from, job.idx_to);
        }
    }

    //check results
    for (unsigned int i = 0; i < n; ++i)
    {
        const auto& table_name = insert_jobs.at(i).table;
        const auto& result     = results.at(i);
        if (!result.ok())
        {
            logerr << "DBInterface: insertBuffer: inserting into table '" << table_name << "' failed: " << result.error();
            throw runtime_error("DBInterface: insertBuffer: inserting into table '" + table_name + "' failed: " + result.error());
        }
    }
}

/**
 */
void DBInterface::updateBuffer(const std::string& table_name, 
                               const std::string& key_col,
                               shared_ptr<Buffer> buffer, 
                               int from_index, 
                               int to_index)
{
    logdbg << "DBInterface: updateBuffer: table " << table_name << " buffer size " << buffer->size() << " key " << key_col;

    assert(ready());
    assert(buffer);

    if (!existsTable(table_name))
    {
        logerr << "DBInterface: updateBuffer: table with name '" << table_name << "' does not exist";
        throw runtime_error("DBInterface: updateBuffer: table with name '" + table_name + "' does not exist");
    }

    Result res;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        res = db_instance_->defaultConnection().updateBuffer(table_name, buffer, key_col, from_index, to_index);
    }

    if (!res.ok())
    {
        logerr << "DBInterface: updateBuffer: updating table '" << table_name << "' failed: " << res.error();
        throw runtime_error("DBInterface: updateBuffer: updating table '" + table_name + "' failed: " + res.error());
    }
}

/**
 */
void DBInterface::prepareRead(const DBContent& dbobject, 
                              VariableSet read_list, 
                              string custom_filter_clause,
                              bool use_order, 
                              Variable* order_variable)
{
    logdbg << "DBInterface: prepareRead: dbo " << dbobject.name();

    assert(ready());
    assert(dbobject.existsInDB());

    shared_ptr<DBCommand> read = sqlGenerator().getSelectCommand(
        dbobject, read_list, custom_filter_clause, use_order, order_variable);

    logdbg << "DBInterface: prepareRead: sql '" << read->get() << "'";

    Result res;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        res = db_instance_->defaultConnection().startRead(read, 0, read_chunk_size_);
    }
    
    if (!res.ok())
    {
        logerr << "DBInterface: prepareRead: preparing read for dbcontent '" << dbobject.name() << "' failed: " << res.error();
        throw runtime_error("DBInterface: prepareRead: preparing read for dbcontent '" + dbobject.name() + "' failed: " + res.error());
    }
}

/**
 */
std::pair<std::shared_ptr<Buffer>, bool> DBInterface::readDataChunk(const DBContent& dbobject)
{
    // locked by prepareRead
    assert(ready());

    shared_ptr<DBResult> result;
    bool last_one = false;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        result = db_instance_->defaultConnection().readChunk();
    }

    if (!result)
    {
        logerr << "DBInterface: readDataChunk: connection returned unknown error";
        throw runtime_error("DBInterface: readDataChunk: connection returned unknown error");
    }
    if (result->hasError())
    {
        logerr << "DBInterface: readDataChunk: connection returned error: " << result->error();
        throw runtime_error("DBInterface: readDataChunk: connection returned unknown error: " + result->error());
    }
    if (!result->containsData())
    {
        logerr << "DBInterface: readDataChunk: buffer does not contain data";
        throw runtime_error("DBInterface: readDataChunk: buffer does not contain data");
    }

    last_one = !result->hasMore();

    shared_ptr<Buffer> buffer = result->buffer();

    buffer->dbContentName(dbobject.name());

    assert(buffer);

    return {buffer, last_one};
}

/**
 */
void DBInterface::finalizeReadStatement(const DBContent& dbobject)
{
    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        db_instance_->defaultConnection().stopRead();
    }
}

/**
 */
void DBInterface::deleteBefore(const DBContent& dbcontent, 
                               boost::posix_time::ptime before_timestamp)
{
    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        std::shared_ptr<DBCommand> command = sqlGenerator().getDeleteCommand(dbcontent, before_timestamp);
        execute(*command.get());
    }
}

/**
 */
void DBInterface::deleteAll(const DBContent& dbcontent)
{
    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        std::shared_ptr<DBCommand> command = sqlGenerator().getDeleteCommand(dbcontent);
        execute(*command.get());
    }
}

/**
 */
void DBInterface::deleteContent(const DBContent& dbcontent, 
                                unsigned int sac, 
                                unsigned int sic)
{
    loginf << "DBInterface: deleteContent: dbcontent " << dbcontent.name() << " sac/sic " << sac << "/" << sic;

    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        std::shared_ptr<DBCommand> command = sqlGenerator().getDeleteCommand(dbcontent, sac, sic);

        execute(*command.get());
    }
}

/**
 */
void DBInterface::deleteContent(const DBContent& dbcontent, unsigned int sac, unsigned int sic, unsigned int line_id)
{
    loginf << "DBInterface: deleteContent: dbcontent " << dbcontent.name() << " sac/sic " << sac << "/" << sic << " line " << line_id;

    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        std::shared_ptr<DBCommand> command = sqlGenerator().getDeleteCommand(dbcontent, sac, sic, line_id);

        execute(*command.get());
    }
}

/**
 */
void DBInterface::clearTableContent(const string& table_name)
{
    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        // DELETE FROM tablename;
        execute("DELETE FROM " + table_name + ";");
    }
}

/**
 */
ResultT<std::shared_ptr<Buffer>> DBInterface::select(const std::string& table_name, 
                                                     const PropertyList& properties,
                                                     const std::string& filter)
{
    if (!existsTable(table_name))
        return ResultT<std::shared_ptr<Buffer>>::failed("Table does not exist");

    std::shared_ptr<Buffer> buffer;

    try
    {
        auto cmd = sqlGenerator().getSelectCommand(table_name, properties, filter);
        auto result = execute(*cmd);
        if (result->hasError() || !result->containsData() || !result->buffer())
            throw std::runtime_error("Could not obtain results table");

        buffer = result->buffer();
    }
    catch(const std::exception& ex)
    {
        logerr << "DBInterface: select: Could not select data: " << ex.what();
        return ResultT<std::vector<std::shared_ptr<TaskResult>>>::failed(ex.what());
    }
    catch(...)
    {
        logerr << "DBInterface: select: Could not select data: Unknown error";
        return ResultT<std::vector<std::shared_ptr<TaskResult>>>::failed("Unknown error");
    }

    return ResultT<std::shared_ptr<Buffer>>::succeeded(buffer);
}

/**
 */
void DBInterface::createPropertiesTable()
{
    assert(ready());
    assert(!existsPropertiesTable());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        execute(sqlGenerator().getTablePropertiesCreateStatement());
        updateTableInfo();
    }
}

/**
 */
void DBInterface::startPerformanceMetrics() const
{
    assert(ready());

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        db_instance_->defaultConnection().startPerformanceMetrics();
    }
}

/**
 */
db::PerformanceMetrics DBInterface::stopPerformanceMetrics() const
{
    assert(ready());

    db::PerformanceMetrics pm;

    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        pm = db_instance_->defaultConnection().stopPerformanceMetrics();
    }

    return pm;
}

/**
 */
bool DBInterface::hasActivePerformanceMetrics() const
{
    if (!ready())
        return false;

    bool ok;
    {
        #ifdef PROTECT_INSTANCE
        boost::mutex::scoped_lock locker(instance_mutex_);
        #endif

        ok = db_instance_->defaultConnection().hasActivePerformanceMetrics();
    }

    return ok;
}

/**
 */
std::string DBInterface::dbInfo()
{
    if (!ready())
        return "db not ready";

    return db_instance_->dbInfo();
}
