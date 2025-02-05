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
std::pair<bool, std::string> DBConnection::connect(const std::string& file_name)
{
    loginf << "DBConnection: connect: '" << file_name << "'";

    //close any opened connection
    if (dbOpened())
        disconnect();

    auto connect_result = connect_impl(file_name);
    if (!connect_result.first)
        return connect_result;

    db_opened_   = true;
    db_filename_ = file_name;

    return std::make_pair(true, "");
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

    disconnect_impl();

    db_opened_   = false;
    db_filename_ = "";
    created_tables_.clear();
}

/**
 * Exports the database to the given file.
 */
bool DBConnection::exportFile(const std::string& file_name)
{
    if (!dbOpened())
    {
        logerr << "DBConnection: exportFile: not connected to database";
        return false;
    }
    
    if (!exportFile_impl(file_name))
    {
        logerr << "DBConnection: exportFile: export failed";
        return false;
    }

    return true;
}

/**
 * Executes the given sql query without returning any data or specific error messages.
 */
bool DBConnection::execute(const std::string& sql)
{
    logdbg << "DBConnection: execute: sql statement execute: '" << sql << "'";

    return executeSQL_impl(sql, nullptr, false);
}

/**
 * Executes the given sql query and optionally fetches a result buffer (if implemented by derived connection).
 */
std::shared_ptr<DBResult> DBConnection::execute(const std::string& sql, bool fetch_result_buffer)
{
    std::shared_ptr<DBResult> result(new DBResult());

    logdbg << "DBConnection: execute: sql statement execute: '" << sql << "'";

    bool ok = executeSQL_impl(sql, result.get(), fetch_result_buffer);
    if (!ok)
    {
        logerr << "DBConnection: execute: Query '" << sql << "' failed: " << result->error();
    }
    else if (fetch_result_buffer && !result->buffer())
    {
        logerr << "DBConnection: execute: Query '" << sql << "': buffer creation failed";
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

    bool fetch_buffer = command.expectsResult();

    std::shared_ptr<DBResult> result(new DBResult());
    bool ok = executeCmd_impl(command.get(), fetch_buffer ? &command.resultList() : nullptr, result.get());

    if (ok && fetch_buffer && !result->buffer())
    {
        logerr << "DBConnection: execute: DBCommand '" << command.get() << "': buffer creation failed";
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
            logerr << "DBConnection: execute: DBCommand '" << cmd << "' failed: " << result->error();
            dbresult->setError(result->error());
            break;
        }
        else if (fetch_buffers)
        {
            if (!result->buffer())
            {
                logerr << "DBConnection: execute: DBCommand '" << cmd << "': buffer creation failed";
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
bool DBConnection::createTable(const std::string& table_name, 
                               const std::vector<DBTableColumnInfo>& column_infos,
                               const std::string& dbcontent_name)
{
    if (created_tables_.find(table_name) != created_tables_.end())
        return true;

    //get sql statement
    bool needs_precise_types = needsPreciseDBTypes();
    auto sql_placeholder     = sqlPlaceholder();
    bool use_indexing        = useIndexing();

    std::string sql = SQLGenerator(needs_precise_types, sql_placeholder).getCreateTableStatement(table_name, column_infos, use_indexing, dbcontent_name);

    auto result = execute(sql, false);
    assert(result);

    if (result->hasError())
    {
        logerr << "DBConnection: createTable: creating table '" << table_name << "' failed: " << result->error();
        return false;
    }

    //update table info after inserting a new table
    updateTableInfo();

    return true;
}

/**
 * Inserts the given buffer into the given table.
 */
bool DBConnection::insertBuffer(const std::string& table_name, 
                                const std::shared_ptr<Buffer>& buffer,
                                PropertyList* table_properties)
{
    auto res = insertBuffer_impl(table_name, buffer, table_properties);

    if (!res.first)
        logerr << "DBConnection: : insertBuffer: inserting into table '" << table_name << "' failed: " << res.second;

    return res.first;
}

/**
 * Updates the given table using values of the given buffer.
 */
bool DBConnection::updateBuffer(const std::string& table_name, 
                                const std::shared_ptr<Buffer>& buffer,
                                const std::string& key_column,
                                const boost::optional<size_t>& idx_from, 
                                const boost::optional<size_t>& idx_to)
{
    auto res = updateBuffer_impl(table_name, buffer, key_column, idx_from, idx_to);

    if (!res.first)
        logerr << "DBConnection: : updateBuffer: updating table '" << table_name 
               << "' at key column '" << key_column << "' failed: " << res.second;

    return res.first;
}

/**
 */
std::pair<bool, std::string> DBConnection::insertBuffer_impl(const std::string& table_name, 
                                                             const std::shared_ptr<Buffer>& buffer,
                                                             PropertyList* table_properties)
{
    auto sql = SQLGenerator(needsPreciseDBTypes(),
                            sqlPlaceholder()).getInsertDBUpdateStringBind(buffer, table_name);

    auto stmnt = prepareStatement(sql, true);
    assert(stmnt);
    if (!stmnt->valid())
        return std::make_pair(false, "could not prepare update statement");

    if (!stmnt->executeBuffer(buffer))
        return std::make_pair(false, "could not execute statement on buffer");

    //cleanup prepared statement
    stmnt.reset();

    return std::make_pair(true, "");
}

/**
 */
std::pair<bool, std::string> DBConnection::updateBuffer_impl(const std::string& table_name, 
                                                             const std::shared_ptr<Buffer>& buffer,
                                                             const std::string& key_column,
                                                             const boost::optional<size_t>& idx_from, 
                                                             const boost::optional<size_t>& idx_to)
{
    auto sql = SQLGenerator(needsPreciseDBTypes(),
                            sqlPlaceholder()).getCreateDBUpdateStringBind(buffer, key_column, table_name);

    //loginf << "executing statement:\n" << sql;

    auto stmnt = prepareStatement(sql, true);
    assert(stmnt);
    if (!stmnt->valid())
        return std::make_pair(false, "could not prepare update statement");

    if (!stmnt->executeBuffer(buffer, idx_from, idx_to))
        return std::make_pair(false, "could not execute statement on buffer");

    //cleanup prepared statement
    stmnt.reset();

    return std::make_pair(true, "");
}

/**
 * Gets a list of tables in the database.
 */
boost::optional<std::vector<std::string>> DBConnection::getTableList()
{
    auto tlist = getTableList_impl();

    if (!tlist.has_value())
    {
        logerr << "DBConnection: : getTableList: retrieving table list failed"; 
        return boost::optional<std::vector<std::string>>();
    }

    return tlist;
}

/**
 * Gets info about a specific table's columns.
 */
boost::optional<DBTableInfo> DBConnection::getColumnList(const std::string& table)
{
    auto clist = getColumnList_impl(table);

    if (!clist.has_value())
    {
        logerr << "DBConnection: : getColumnList: retrieving column list failed"; 
        return boost::optional<DBTableInfo>();
    }

    return clist;
}

/**
 * Basic implementation which should work for sql like interfaces.
 */
boost::optional<DBTableInfo> DBConnection::getColumnList_impl(const std::string& table)
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

    if (!result->containsData())
        return boost::optional<DBTableInfo>();

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

    return table_info;
}

/**
 * Updates info about tables in the currently opened database.
 */
bool DBConnection::updateTableInfo()
{
    created_tables_.clear();

    auto tlist = getTableList();

    //assert for now, should never happen
    assert(tlist.has_value());

    for (const auto& tname : tlist.value())
    {
        auto tinfo = getColumnList(tname);

        //assert for now, should never happen
        assert(tinfo.has_value());

        created_tables_[ tname ] = tinfo.value();
    }

    return true;
}

/**
 */
void DBConnection::printTableInfo()
{
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
