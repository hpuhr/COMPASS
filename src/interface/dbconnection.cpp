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

#include "dbconnection.h"
#include "duckdbexecresult.h"
#include "dbprepare.h"
#include "dbreader.h"

#include "sqlgenerator.h"

#include "buffer.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "dbinterface.h"
#include "dbresult.h"
#include "dbtableinfo.h"
#include "logger.h"
#include "property.h"
#include "dbcontent.h"
#include "property_templates.h"
#include "compass.h"
#include "dbcontentmanager.h"

#include "util/timeconv.h"

#include <cstring> 

#include <QElapsedTimer>

/**
 */
DBConnection::DBConnection(DBInterface* interface)
:   interface_(*interface)
,   db_opened_(false     )
{
    loginf << "DBConnection: constructor";
}

/**
 */
DBConnection::~DBConnection()
{
    loginf << "DBConnection: destructor";

    assert(!dbOpened());
}

/**
 * Returns the current connection status.
 */
std::string DBConnection::status() const
{
    if (db_opened_)
        return "Ready";
    else
        return "Not connected";
}

/**
 * Connects to the given database file. Will disconnect from the current db if connected.
 */
Result DBConnection::connect(const std::string& file_name)
{
    loginf << "DBConnection: connect: '" << file_name << "'";

    //close any opened connection
    if (dbOpened())
        disconnect();

    auto connect_result = connect_impl(file_name);
    if (!connect_result.ok())
        return connect_result;

    db_opened_   = true;
    db_filename_ = file_name;

    loginf << "DBConnection: connect: connected!";

    return Result::succeeded();
}

/**
 * Disconnects from the currently connected database.
 */
void DBConnection::disconnect()
{
    loginf << "DBConnection: disconnecting, db open? " << db_opened_;

    if (!db_opened_)
        return;

    loginf << "DBConnection: disconnecting...";

    //close any active reader
    stopRead();

    disconnect_impl();

    db_opened_   = false;
    db_filename_ = "";
    created_tables_.clear();
}

/**
 * Exports the database to the given file.
 */
Result DBConnection::exportFile(const std::string& file_name)
{
    assert(dbOpened());

    auto r = exportFile_impl(file_name);
    
    //if (!r.ok())
    //    logerr << "DBConnection: exportFile: export failed";

    return r;
}

/**
 * Executes the given sql query without returning any data or specific error messages.
 */
Result DBConnection::execute(const std::string& sql)
{
    logdbg << "DBConnection: execute: sql statement execute: '" << sql << "'";

    assert(dbOpened());

    return executeSQL_impl(sql, nullptr, false);
}

/**
 * Executes the given sql query and optionally fetches a result buffer (if implemented by derived connection).
 */
std::shared_ptr<DBResult> DBConnection::execute(const std::string& sql, bool fetch_result_buffer)
{
    logdbg << "DBConnection: execute: sql statement execute: '" << sql << "'";

    assert(dbOpened());

    std::shared_ptr<DBResult> result(new DBResult());

    auto res = executeSQL_impl(sql, result.get(), fetch_result_buffer);

    if (res.ok() && fetch_result_buffer && !result->buffer())
    {
        //logerr << "DBConnection: execute: Query '" << sql << "': buffer creation failed";
        result->setError("buffer creation failed");
    }

    return result;
}

/**
 * Executes the given DBCommand.
 */
std::shared_ptr<DBResult> DBConnection::execute(const DBCommand& command)
{
    logdbg << "DBConnection: execute: executing single command";

    assert(dbOpened());

    bool fetch_buffer = command.expectsResult();

    std::shared_ptr<DBResult> result(new DBResult());
    bool ok = executeCmd_impl(command.get(), fetch_buffer ? &command.resultList() : nullptr, result.get());

    if (ok && fetch_buffer && !result->buffer())
    {
        //logerr << "DBConnection: execute: DBCommand '" << command.get() << "': buffer creation failed";
        result->setError("buffer creation failed");
    }
    
    return result;
}

/**
 * Executes the given DBCommandList.
 */
std::shared_ptr<DBResult> DBConnection::execute(const DBCommandList& command_list)
{
    logdbg << "DBConnection: execute: executing " << command_list.getNumCommands() << " command(s)";

    assert(dbOpened());

    std::shared_ptr<DBResult> dbresult(new DBResult());

    unsigned int num_commands = command_list.getNumCommands();
    bool fetch_buffers = command_list.getExpectDataResult();

    std::shared_ptr<Buffer> buffer;
    if (fetch_buffers)
        buffer.reset(new Buffer(command_list.getResultList()));

    for (unsigned int cnt = 0; cnt < num_commands; cnt++)
    {
        const auto& cmd = command_list.getCommands().at(cnt);

        std::shared_ptr<DBResult> result(new DBResult());
        auto ok = executeCmd_impl(cmd, fetch_buffers ? &command_list.getResultList() : nullptr, result.get());
        if (!ok)
        {
            //logerr << "DBConnection: execute: DBCommand '" << cmd << "' failed: " << result->error();
            dbresult->setError(result->error());
            break;
        }
        else if (fetch_buffers)
        {
            if (!result->buffer())
            {
                //logerr << "DBConnection: execute: DBCommand '" << cmd << "': buffer creation failed";
                dbresult->setError("buffer creation failed");
                break;
            }

            std::shared_ptr<Buffer> b = result->buffer();
            buffer->seizeBuffer(*b);
        }
    }

    if (fetch_buffers && !dbresult->hasError())
        dbresult->buffer(buffer);

    logdbg << "DBConnection: execute: end";

    return dbresult;
}

/**
 * Creates a table of the given name and column properties.
 */
Result DBConnection::createTable(const std::string& table_name, 
                                 const std::vector<DBTableColumnInfo>& column_infos,
                                 const std::vector<db::Index>& indices)
{
    assert(dbOpened());

    auto it = created_tables_.find(table_name);
    if (it != created_tables_.end())
    {
        //@TODO: check properties of existing table against requested one
        return Result::succeeded();
    }

    //get sql statement
    std::string sql = SQLGenerator(sqlConfiguration()).getCreateTableStatement(table_name, column_infos, indices);

    auto result = execute(sql, false);
    assert(result);

    if (result->hasError())
    {
        //logerr << "DBConnection: createTable: creating table '" << table_name << "' failed: " << result->error();
        return Result::failed(result->error());
    }

    //update table info after inserting a new table
    updateTableInfo();

    return Result::succeeded();
}

/**
 */
Result DBConnection::deleteTable(const std::string& table_name)
{
    auto res = execute("DROP TABLE " + table_name + ";");
    if (!res.ok())
        return res;

    updateTableInfo();

    return Result::succeeded();
}

/**
 */
Result DBConnection::deleteTableContents(const std::string& table_name)
{
    return execute("DELETE FROM " + table_name + ";");
}

/**
 * Inserts the given buffer into the given table.
 */
Result DBConnection::insertBuffer(const std::string& table_name, 
                                  const std::shared_ptr<Buffer>& buffer,
                                  const boost::optional<size_t>& idx_from, 
                                  const boost::optional<size_t>& idx_to,
                                  PropertyList* table_properties)
{
    assert(dbOpened());

    QElapsedTimer t;
    if (perf_metrics_.has_value())
    {
        t.start();
    }

    auto res = insertBuffer_impl(table_name, buffer, idx_from, idx_to, table_properties);

    if (perf_metrics_.has_value())
    {
        perf_metrics_->insert_time       += t.elapsed();
        perf_metrics_->insert_num_chunks += 1;
        perf_metrics_->insert_num_rows   += buffer->size();
    }

    //if (!res.first)
    //    logerr << "DBConnection: : insertBuffer: inserting into table '" << table_name << "' failed: " << res.second;

    return res;
}

/**
 * Updates the given table using values of the given buffer.
 */
Result DBConnection::updateBuffer(const std::string& table_name, 
                                  const std::shared_ptr<Buffer>& buffer,
                                  const std::string& key_column,
                                  const boost::optional<size_t>& idx_from, 
                                  const boost::optional<size_t>& idx_to)
{
    assert(dbOpened());

    QElapsedTimer t;
    if (perf_metrics_.has_value())
    {
        t.start();
    }

    size_t idx0 = idx_from.has_value() ? idx_from.value()   : 0;
    size_t idx1 = idx_to.has_value()   ? idx_to.value() + 1 : buffer->size();
    assert(idx1 >= idx0);

    auto res = updateBuffer_impl(table_name, buffer, key_column, idx_from, idx_to);

    if (perf_metrics_.has_value())
    {
        perf_metrics_->update_time       += t.elapsed();
        perf_metrics_->update_num_chunks += 1;
        perf_metrics_->update_num_rows   += idx1 - idx0;
    }

    //if (!res.first)
    //    logerr << "DBConnection: : updateBuffer: updating table '" << table_name 
    //           << "' at key column '" << key_column << "' failed: " << res.second;

    return res;
}

/**
 * Default implementation: use prepared INSERT statement.
 */
Result DBConnection::insertBuffer_impl(const std::string& table_name, 
                                       const std::shared_ptr<Buffer>& buffer,
                                       const boost::optional<size_t>& idx_from, 
                                       const boost::optional<size_t>& idx_to,
                                       PropertyList* table_properties)
{
    auto sql = SQLGenerator(sqlConfiguration()).getInsertDBUpdateStringBind(buffer, table_name);

    //loginf << "executing statement:\n" << sql;

    auto stmnt = prepareStatement(sql, true);
    assert(stmnt);

    if (!stmnt->valid())
        return Result::failed("could not prepare insert statement: " + stmnt->lastError());

    auto res = stmnt->executeBuffer(buffer, idx_from, idx_to);
    if (!res.ok())
        return Result::failed("could not execute insert statement on buffer: " + res.error());

    //cleanup prepared statement
    stmnt.reset();

    return Result::succeeded();
}

/**
 * Default implementation: use prepared UPDATE statement.
 */
Result DBConnection::updateBuffer_impl(const std::string& table_name, 
                                                             const std::shared_ptr<Buffer>& buffer,
                                                             const std::string& key_column,
                                                             const boost::optional<size_t>& idx_from, 
                                                             const boost::optional<size_t>& idx_to)
{
    auto sql = SQLGenerator(sqlConfiguration()).getCreateDBUpdateStringBind(buffer, key_column, table_name);

    //loginf << "executing statement:\n" << sql;

    auto stmnt = prepareStatement(sql, true);
    assert(stmnt);

    if (!stmnt->valid())
        return Result::failed("could not prepare update statement" + stmnt->lastError());

    auto res = stmnt->executeBuffer(buffer, idx_from, idx_to);
    if (!res.ok())
        return Result::failed("could not execute update statement on buffer: " + res.error());

    //cleanup prepared statement
    stmnt.reset();

    return Result::succeeded();
}

/**
 * Gets a list of tables in the database.
 */
ResultT<std::vector<std::string>> DBConnection::getTableList()
{
    assert(dbOpened());

    auto res = getTableList_impl();

    //if (!res.first)
    //    logerr << "DBConnection: : getTableList: retrieving table list failed"; 

    return res;
}

/**
 * Gets info about a specific table's columns.
 */
ResultT<DBTableInfo> DBConnection::getColumnList(const std::string& table)
{
    assert(dbOpened());

    auto res = getColumnList_impl(table);

    //if (!res.first)
    //    logerr << "DBConnection: : getColumnList: retrieving column list failed"; 

    return res;
}

/**
 * Basic implementation which should work for sql like interfaces.
 */
ResultT<DBTableInfo> DBConnection::getColumnList_impl(const std::string& table)
{
    DBTableInfo table_info(table);

    DBCommand command;
    command.set("PRAGMA table_info(" + table + ")");

    // int cid, string name, string type,int notnull, string dflt_value, int pk

    PropertyList list;
    list.addProperty("cid", PropertyDataType::INT);
    list.addProperty("name", PropertyDataType::STRING);
    list.addProperty("type", PropertyDataType::STRING);
    list.addProperty("notnull", PropertyDataType::BOOL);
    list.addProperty("dfltvalue", PropertyDataType::STRING);
    list.addProperty("pk", PropertyDataType::BOOL);

    command.list(list);

    std::shared_ptr<DBResult> result = execute(command);
    assert(result);

    if (result->hasError())
        return ResultT<DBTableInfo>::failed(result->error());

    if (!result->containsData())
        return ResultT<DBTableInfo>::failed("table information could not be retrieved");

    std::shared_ptr<Buffer> buffer = result->buffer();

    for (unsigned int cnt = 0; cnt < buffer->size(); cnt++)
    {
        assert(buffer->has<std::string>("type"));

        std::string data_type = buffer->get<std::string>("type").get(cnt);

        assert(buffer->has<std::string>("name"));
        assert(buffer->has<bool>("pk"));
        assert(buffer->has<bool>("notnull"));

        table_info.addColumn(buffer->get<std::string>("name").get(cnt), 
                             data_type,
                             buffer->get<bool>("pk").get(cnt),
                             !buffer->get<bool>("notnull").get(cnt), 
                             "");
    }

    return ResultT<DBTableInfo>::succeeded(table_info);
}

/**
 * Updates info about tables in the currently opened database.
 */
Result DBConnection::updateTableInfo()
{
    assert(dbOpened());

    created_tables_.clear();

    auto list_res = getTableList();
    if (!list_res.ok())
        return list_res;
    
    assert(list_res.hasResult());

    for (const auto& tname : list_res.result())
    {
        auto table_res = getColumnList(tname);
        if (!table_res.ok())
            return table_res;

        assert(table_res.hasResult());

        created_tables_[ tname ] = table_res.result();
    }

    return Result::succeeded();
}

/**
 */
void DBConnection::printTableInfo()
{
    assert(dbOpened());

    for (const auto& table : tableInfo())
    {
        auto table_name = table.first;

        loginf << "[" << table_name << "]";
        loginf << "columns: " << table.second.columns().size();

        const auto& tinfo = table.second;
        for (const auto& ci : tinfo.columns())
        {
            std::stringstream ss;
            ss << "name: " << ci.name() << " ";
            ss << "dtype_prop: " << (ci.hasPropertyType() ? Property::asString(ci.propertyType()) : "-") << " ";
            ss << "dtype_db: " << (ci.hasDBType() ? ci.dbType() : "-") << " ";
            ss << "key: " << ci.key() << " ";
            ss << "null_allowed: " << ci.nullAllowed() << " ";
            ss << "comment: " << ci.comment();

            loginf << ss.str();
        }

        loginf << "";
    }
}

/**
 */
Result DBConnection::startRead(const std::shared_ptr<DBCommand>& select_cmd, 
                               size_t offset, 
                               size_t chunk_size)
{
    assert(dbOpened());
    assert(select_cmd && QString::fromStdString(select_cmd->get()).startsWith("SELECT ") && select_cmd->expectsResult());

    active_reader_ = createReader(select_cmd, offset, chunk_size);
    return Result(active_reader_->isReady(), active_reader_->lastError());
}

/**
 */
std::shared_ptr<DBResult> DBConnection::readChunk()
{
    assert(dbOpened());

    std::shared_ptr<DBResult> result;

    if (!active_reader_)
    {
        result.reset(new DBResult);
        result->setError("no reader active");
        return result;
    }
    if (!active_reader_->isReady())
    {
        result.reset(new DBResult);
        result->setError("active reader not ready");
        return result;
    }

    QElapsedTimer t;
    if (perf_metrics_.has_value())
    {
        t.start();
    }

    result = active_reader_->readChunk();
    assert(result);

    if (perf_metrics_.has_value())
    {
        perf_metrics_->read_time       += t.elapsed();
        perf_metrics_->read_num_chunks += (result && result->containsData() ? 1 : 0);
        perf_metrics_->read_num_rows   += (result && result->containsData() ? result->buffer()->size() : 0);
    }

    //error during read?
    if (result->hasError())
        return result;

    assert(result->buffer() && result->containsData());

    // if (!result->hasMore())
    // {
    //     //reader has finished => reset
    //     stopRead();
    // }

    return result;
}

/**
 */
void DBConnection::stopRead()
{
    active_reader_.reset();
}

/**
 */
SQLGenerator DBConnection::sqlGenerator() const
{
    return SQLGenerator(sqlConfiguration());
}

/**
 */
void DBConnection::startPerformanceMetrics() const
{
    perf_metrics_ = db::PerformanceMetrics();
    perf_metrics_->valid = true;
}

/**
 */
db::PerformanceMetrics DBConnection::stopPerformanceMetrics() const
{
    if(!perf_metrics_.has_value())
        return db::PerformanceMetrics();

    db::PerformanceMetrics pm = perf_metrics_.value();
    perf_metrics_.reset();

    return pm;
}
