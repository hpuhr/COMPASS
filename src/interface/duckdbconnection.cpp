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

#include "duckdbconnection.h"
#include "duckdbscopedappender.h"
#include "duckdbscopedprepare.h"
#include "duckdbcommon.h"

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

#include "util/timeconv.h"

#include <cstring>

namespace
{
    class DuckDBScopedConfig
    {
    public:
        DuckDBScopedConfig()
        {
            auto state = duckdb_create_config(&config_);
            ok_ = state == DuckDBSuccess;
        }

        virtual ~DuckDBScopedConfig()
        {
            if (ok_)
                duckdb_destroy_config(&config_);
        }

        void configure(const DuckDBConnectionSettings& settings)
        {
            settings.configure(&config_);
        }

        bool valid() const { return ok_; }
        duckdb_config* configuration() { return &config_; }

    private:
        duckdb_config config_;
        bool ok_ = false;
    };
}

/**
 */
DuckDBConnection::DuckDBConnection(DBInterface* interface)
:   interface_(*interface)
,   db_opened_(false     )
{
    loginf << "DuckDBConnection: constructor";
}

/**
 */
DuckDBConnection::~DuckDBConnection()
{
    disconnect();
}

/**
 */
std::string DuckDBConnection::status() const
{
    if (db_opened_)
        return "Ready";
    else
        return "Not connected";
}

/**
 */
std::pair<bool, std::string> DuckDBConnection::connect(const std::string& file_name)
{
    loginf << "DuckDBConnection: connect: '" << file_name << "'";

    //close any opened connection
    if (dbOpened())
        disconnect();

    // create the configuration object
    DuckDBScopedConfig config;
    if (!config.valid()) 
    {
        logerr << "DuckDBConnection: connect: could not create db configuration";
        return std::make_pair(false, "could not create db configuration");
    }

    //configure
    config.configure(settings_);

    //open db
    char* error = nullptr;
    auto state = duckdb_open_ext(file_name.c_str(), &db_, *config.configuration(), &error);
    if (state != DuckDBSuccess)
    {
        std::string err_str = error ? std::string(error) : std::string("unknown error");
        logerr << "DuckDBConnection: connect: could not open file: " << err_str;
        return std::make_pair(false, err_str);
    }

    //connect to db
    state = duckdb_connect(db_, &connection_);
    if (state != DuckDBSuccess)
    {
        logerr << "DuckDBConnection: connect: could not connect to database";
        return std::make_pair(false, "could not connect to database");
    }

    // char* sErrMsg = 0;
    // sqlite3_exec(db_handle_, "PRAGMA SYNCHRONOUS = OFF", NULL, NULL, &sErrMsg);
    // sqlite3_exec(db_handle_, "PRAGMA TEMP_STORE = 2", NULL, NULL, &sErrMsg);
    // sqlite3_exec(db_handle_, "PRAGMA JOURNAL_MODE = OFF", NULL, NULL, &sErrMsg);
    // sqlite3_exec(db_handle_, "PRAGMA LOCKING_MODE = EXCLUSIVE", NULL, NULL, &sErrMsg);
    // sqlite3_exec(db_handle_, "PRAGMA CACHE_SIZE = 500", NULL, NULL, &sErrMsg);

    //// sqlite3_exec(db_handle_, "PRAGMA locking_mode = EXCLUSIVE", NULL, NULL, &sErrMsg);

    db_opened_ = true;

    return std::make_pair(true, "");
}

/**
 */
void DuckDBConnection::disconnect()
{
    logdbg << "DuckDBConnection: disconnect";

    if (!db_opened_)
        return;

    duckdb_disconnect(&connection_);
    duckdb_close(&db_);

    db_opened_ = false;
}

/**
 */
void DuckDBConnection::exportFile(const std::string& file_name)
{
    // loginf << "DuckDBConnection: exportFile: file '" << file_name << "'";
    // assert (db_opened_);

    // string tmp_sql = "VACUUM INTO '"+file_name+"';";

    // loginf << "DuckDBConnection: exportFile: sql '" << tmp_sql << "'";

    // char* sErrMsg = 0;
    // sqlite3_exec(db_handle_, tmp_sql.c_str(), NULL, NULL, &sErrMsg);

}

/**
 */
std::shared_ptr<DuckDBScopedAppender> DuckDBConnection::createAppender(const std::string& table)
{
    return std::shared_ptr<DuckDBScopedAppender>(new DuckDBScopedAppender(connection_, table));
}

/**
 */
std::shared_ptr<DuckDBScopedPrepare> DuckDBConnection::prepareStatement(const std::string& statement)
{
    return std::shared_ptr<DuckDBScopedPrepare>(new DuckDBScopedPrepare(connection_, statement));
}

/**
 */
std::shared_ptr<DBResult> DuckDBConnection::execute(const std::string& sql, bool fetch_result_buffer)
{
    std::shared_ptr<DBResult> dbresult(new DBResult());

    logdbg << "DuckDBConnection: execute: sql statement execute: '" << sql << "'";

    DuckDBResult result;

    auto state = duckdb_query(connection_, sql.c_str(), &result.result_);
    if (state == DuckDBError) 
    {
        std::string err_str(duckdb_result_error(&result.result_));
        logerr << "DuckDBConnection: execute: duckdb_query cmd '" << sql << "' failed: " << err_str;

        dbresult->setError(err_str);
    }

    if (fetch_result_buffer && !dbresult->hasError())
    {
        auto b = result.toBuffer();
        if (!b)
        {
            logerr << "DuckDBConnection: execute: duckdb_query cmd '" << sql << "': buffer creation failed";
            dbresult->setError("buffer creation failed");
        }
        else
        {
            dbresult->buffer(b);
        }
    }

    return dbresult;
}

/**
 */
std::shared_ptr<DBResult> DuckDBConnection::execute(const DBCommand& command)
{
    std::shared_ptr<DBResult> dbresult(new DBResult());
    std::string sql = command.get();

    logdbg << "DuckDBConnection: execute: executing single command";

    auto result = executeCmd(sql);
    assert(result);

    if (!result->usable())
    {
        logerr << "DuckDBConnection: execute: DBCommand '" << sql << "' failed: " << result->errorMsg();
        dbresult->setError(result->errorMsg());
    }
    else if (command.resultList().size() > 0) // data should be returned
    {
        auto b = result->toBuffer(command.resultList());
        if (!b)
        {
            logerr << "DuckDBConnection: execute: DBCommand '" << sql << "': buffer creation failed";
            dbresult->setError("buffer creation failed");
        }
        else
        {
            dbresult->buffer(b);
        }
    }

    logdbg << "DuckDBConnection: execute: end";

    return dbresult;
}

/**
 */
std::shared_ptr<DBResult> DuckDBConnection::execute(const DBCommandList& command_list)
{
    std::shared_ptr<DBResult> dbresult(new DBResult());

    unsigned int num_commands = command_list.getNumCommands();
    bool fetch_buffers = command_list.getResultList().size() > 0;

    logdbg << "DuckDBConnection: execute: executing " << num_commands << " command(s)";

    std::shared_ptr<Buffer> buffer;
    if (fetch_buffers)
        buffer.reset(new Buffer(command_list.getResultList()));

    for (unsigned int cnt = 0; cnt < num_commands; cnt++)
    {
        const auto& sql = command_list.getCommandString(cnt);

        auto result = executeCmd(sql);
        assert(result);

        if (!result->usable())
        {
            logerr << "DuckDBConnection: execute: DBCommand '" << sql << "' failed: " << result->errorMsg();
            dbresult->setError(result->errorMsg());
            break;
        }
        else if (fetch_buffers)
        {
            auto b = result->toBuffer(command_list.getResultList());
            if (!b)
            {
                logerr << "DuckDBConnection: execute: DBCommand '" << sql << "': buffer creation failed";
                dbresult->setError("buffer creation failed");
                break;
            }
            else
            {
                buffer->seizeBuffer(*b);
            }
        }
    }

    if (fetch_buffers && !dbresult->hasError())
        dbresult->buffer(buffer);

    logdbg << "DuckDBConnection: execute: end";

    return dbresult;
}

/**
 */
std::shared_ptr<DuckDBResult> DuckDBConnection::executeCmd(const std::string& command)
{
    logdbg << "DuckDBConnection: executeCmd";

    DuckDBScopedPrepare prepare(connection_, command);
    auto result = prepare.execute();
    assert(result);

    return result;
}

/**
 */
bool DuckDBConnection::createTable(const DBContent& dbcontent)
{
    if (created_tables_.find(dbcontent.dbTableName()) != created_tables_.end())
        return true;

    //SQLGenerator gen(interface_);
    //auto sql = gen.getCreateTableStatement(dbcontent);

    std::map<PropertyDataType, std::string> dbtype_map = 
        {{PropertyDataType::BOOL, "BOOLEAN"},
         {PropertyDataType::CHAR, "TINYINT"},
         {PropertyDataType::UCHAR, "UTINYINT"},
         {PropertyDataType::INT, "INTEGER"},
         {PropertyDataType::UINT, "UINTEGER"},
         {PropertyDataType::LONGINT, "BIGINT"},
         {PropertyDataType::ULONGINT, "UBIGINT"},
         {PropertyDataType::FLOAT, "FLOAT"},
         {PropertyDataType::DOUBLE, "DOUBLE"},
         {PropertyDataType::STRING, "VARCHAR"},
         {PropertyDataType::JSON, "VARCHAR"},
         {PropertyDataType::TIMESTAMP, "BIGINT"}};

    std::stringstream ss;
    ss << "CREATE TABLE " << dbcontent.dbTableName() << "(";

    std::string data_type;

    PropertyList properties;

    unsigned int cnt = 0;
    for (auto& var_it : dbcontent.variables())
    {
        ss << var_it.second->dbColumnName();

        std::string data_type = dbtype_map.at(var_it.second->dataType());
        ss << " " << data_type;

        if (cnt != dbcontent.numVariables() - 1)
            ss << ",";

        cnt++;

        properties.addProperty(var_it.second->dbColumnName(), var_it.second->dataType());
    }

    ss << ");";

    std::string sql = ss.str();

    auto result = execute(sql);
    assert(result);

    if (result->hasError())
    {
        logerr << "DuckDBConnection: createTable: creating table '" << dbcontent.dbTableName() << "' failed: " << result->error();
        return false;
    }

    created_tables_.insert(std::make_pair(dbcontent.dbTableName(), properties));

    return true;
}

/**
 */
bool DuckDBConnection::insertBuffer(const std::string& table_name, 
                                    const std::shared_ptr<Buffer>& buffer)
{
    if (!buffer || buffer->properties().size() == 0 || buffer->size() == 0)
        return false;

    auto appender = createAppender(table_name);
    if (!appender->valid())
    {
        logerr << "DuckDBConnection: insertBuffer: inserting buffer into table '" << table_name << "' failed";
        return false;
    }

    size_t appender_column_count = appender->columnCount();

    loginf << "column count: " << appender_column_count;

    const auto& buffer_properties = buffer->properties();

    auto it = created_tables_.find(table_name);
    bool use_buffer_props = it == created_tables_.end();
    const PropertyList* properties = use_buffer_props ? &buffer_properties : &it->second;

    #define UpdateFunc(PDType, DType)                             \
        appender->append<DType>(buffer->get<DType>(pname).get(r));

    #define NotFoundFunc                                                                                 \
        logerr << "DuckDBConnection: insertBuffer: unknown property type " << Property::asString(dtype); \
        assert(false);

    auto n = buffer->size();

    loginf << "buffer properties: " << buffer_properties.size() << ", used properties: " << properties->size();

    unsigned int np = properties->size();
    assert(np == appender_column_count);

    std::vector<char> has_property(np, 0);
    for (unsigned int c = 0; c < properties->size(); ++c)
        has_property[ c ] = buffer->hasAnyPropertyNamed(properties->at(c).name()) ? 1 : 0;

    for (unsigned int r = 0; r < n; ++r)
    {
        //loginf << "appending row " << std::to_string(r + 1) << "/" << std::to_string(n);

        for (unsigned int c = 0; c < np; ++c)
        {
            if (!has_property[ c ])
            {
                appender->appendNull();
                continue;
            }

            const auto& p = properties->at(c);
            auto dtype = p.dataType();
            const auto& pname = p.name();

            if (buffer->isNull(p, r))
            {
                appender->appendNull();
                continue;
            }

            SwitchPropertyDataType(dtype, UpdateFunc, NotFoundFunc);
        }
        assert(appender->currentColumnCount() == np);
        appender->endRow();

        //if (r % 2000 == 0)
        //    appender->flush();
    }
    appender->flush();

    return true;
}

/**
 */
bool DuckDBConnection::insertBuffer(const DBContent& dbcontent, 
                                    const std::shared_ptr<Buffer>& buffer)
{
    if (!createTable(dbcontent))
        return false;

    return insertBuffer(dbcontent.dbTableName(), buffer);
}

/**
 */
std::map<std::string, DBTableInfo> DuckDBConnection::getTableInfo()
{
    loginf << "DuckDBConnection: getTableInfo";

    std::map<std::string, DBTableInfo> info;

    for (auto it : getTableList())
    {
        loginf << "DuckDBConnection: getTableInfo: table " << it;
        info.insert(std::pair<std::string, DBTableInfo>(it, getColumnList(it)));
    }

    return info;
}

/**
 */
std::vector<std::string> DuckDBConnection::getTableList() // buffer of table name strings
{
    std::vector<std::string> tables;

    DBCommand command;
    command.set("SELECT table_name FROM duckdb_tables()");

    PropertyList list;
    list.addProperty("table_name", PropertyDataType::STRING);
    command.list(list);

    std::shared_ptr<DBResult> result = execute(command);
    assert(result->containsData());
    std::shared_ptr<Buffer> buffer = result->buffer();

    unsigned int size = buffer->size();
    std::string table_name;

    for (unsigned int cnt = 0; cnt < size; cnt++)
    {
        table_name = buffer->get<std::string>("name").get(cnt);

        if (table_name.find("sqlite") != std::string::npos)  // ignore sqlite system tables
            continue;

        tables.push_back(buffer->get<std::string>("name").get(cnt));
    }

    return tables;
}

/**
 */
DBTableInfo DuckDBConnection::getColumnList(const std::string& table)  // buffer of column name string, data type
{
    logdbg << "DuckDBConnection: getColumnList: table " << table;

    DBTableInfo table_info(table);

    DBCommand command;
    command.set("PRAGMA table_info(" + table + ")");

    // int cid, string name, string type,int notnull, string dflt_value, int pk

    PropertyList list;
    list.addProperty("cid", PropertyDataType::INT);
    list.addProperty("name", PropertyDataType::STRING);
    list.addProperty("type", PropertyDataType::STRING);
    list.addProperty("notnull", PropertyDataType::INT);
    list.addProperty("dfltvalue", PropertyDataType::STRING);
    list.addProperty("pk", PropertyDataType::INT);

    command.list(list);

    std::shared_ptr<DBResult> result = execute(command);
    assert(result->containsData());
    std::shared_ptr<Buffer> buffer = result->buffer();

    for (unsigned int cnt = 0; cnt < buffer->size(); cnt++)
    {
        // loginf << "UGA " << table << ": "  << buffer->getString("name").get(cnt);

        assert(buffer->has<std::string>("type"));

        std::string data_type = buffer->get<std::string>("type").get(cnt);

        if (data_type == "BOOLEAN")
            data_type = "BOOL";
        else if (data_type == "TEXT")
            data_type = "STRING";
        else if (data_type == "REAL")
            data_type = "DOUBLE";
        else if (data_type == "INTEGER")
            data_type = "INT";

        assert(buffer->has<std::string>("name"));
        assert(buffer->has<int>("pk"));
        assert(buffer->has<int>("notnull"));

        table_info.addColumn(buffer->get<std::string>("name").get(cnt), data_type,
                             buffer->get<int>("pk").get(cnt) > 0,
                             !buffer->get<int>("notnull").get(cnt), "");
    }

    return table_info;
}
