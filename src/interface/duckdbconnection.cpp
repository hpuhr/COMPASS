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
#include "duckdbappender.h"
#include "duckdbprepare.h"
#include "duckdbexecresult.h"
#include "duckdbreader.h"

#include "dbtemporarytable.h"
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
:   DBConnection(interface)
{
    loginf << "DuckDBConnection: constructor";
}

/**
 */
DuckDBConnection::~DuckDBConnection()
{
    loginf << "DuckDBConnection: destructor";

    if (dbOpened())
        disconnect();
}

/**
 */
Result DuckDBConnection::connect_impl(const std::string& file_name)
{
    //close any opened connection
    if (dbOpened())
        disconnect();

    // create the configuration object
    DuckDBScopedConfig config;
    if (!config.valid()) 
        return Result::failed("could not create db configuration");

    //configure
    config.configure(settings_);

    //open db
    char* error = nullptr;
    auto state = duckdb_open_ext(file_name.c_str(), &db_, *config.configuration(), &error);
    if (state != DuckDBSuccess)
    {
        std::string err_str = error ? std::string(error) : std::string("unknown error");
        return Result::failed(err_str);
    }

    //connect to db
    state = duckdb_connect(db_, &connection_);
    if (state != DuckDBSuccess)
        return Result::failed("could not connect to database");

    return Result::succeeded();
}

/**
 */
void DuckDBConnection::disconnect_impl()
{
    loginf << "DuckDBConnection: disconnecting...";

    duckdb_disconnect(&connection_);
    duckdb_close(&db_);

    connection_ = nullptr;
    db_         = nullptr;
}

/**
 */
Result DuckDBConnection::exportFile_impl(const std::string& file_name)
{
    //@TODO

    // loginf << "DuckDBConnection: exportFile: file '" << file_name << "'";
    // assert (db_opened_);

    // string tmp_sql = "VACUUM INTO '"+file_name+"';";

    // loginf << "DuckDBConnection: exportFile: sql '" << tmp_sql << "'";

    // char* sErrMsg = 0;
    // sqlite3_exec(db_handle_, tmp_sql.c_str(), NULL, NULL, &sErrMsg);

    return Result::failed("not yet implemented");
}

/**
 */
std::shared_ptr<DuckDBScopedAppender> DuckDBConnection::createAppender(const std::string& table)
{
    return std::shared_ptr<DuckDBScopedAppender>(new DuckDBScopedAppender(connection_, table));
}

/**
 */
std::shared_ptr<DBScopedPrepare> DuckDBConnection::prepareStatement(const std::string& statement,
                                                                    bool begin_transaction)
{
    return std::shared_ptr<DBScopedPrepare>(new DuckDBScopedPrepare(connection_, statement, begin_transaction));
}

/**
 */
std::shared_ptr<DBScopedReader> DuckDBConnection::createReader(const std::shared_ptr<DBCommand>& select_cmd, 
                                                               size_t offset, 
                                                               size_t chunk_size)
{
    return std::shared_ptr<DBScopedReader>(new DuckDBScopedReader(connection_, select_cmd, offset, chunk_size));
}

/**
 */
Result DuckDBConnection::executeSQL_impl(const std::string& sql, DBResult* result, bool fetch_result_buffer)
{
    DuckDBExecResult dbresult;

    auto state = duckdb_query(connection_, sql.c_str(), dbresult.result());
    dbresult.result_valid_ = true;
    dbresult.result_error_ = state == DuckDBError;

    if (dbresult.hasError()) 
    {
        auto err = dbresult.errorString();
        if (result)
            result->setError(err);
        return Result::failed(err);
    }

    if (fetch_result_buffer && result)
    {
        //@TODO: fetch buffer
    }

    return Result::succeeded();
}

/**
 */
bool DuckDBConnection::executeCmd_impl(const std::string& command, 
                                       const PropertyList* properties, 
                                       DBResult* result)
{
    DuckDBScopedPrepare prepare(connection_, command);

    if (!prepare.valid())
    {
        loginf << "preparing command '" << command << "' failed";
        if (result)
            result->setError("could not prepare command '" + command + "': " + prepare.lastError());
        return false;
    }

    DBPrepare::ExecOptions options;
    if (result && properties)
        options.bufferProperties(*properties);

    auto dbresult = prepare.execute(options);
    assert(dbresult);

    if (dbresult->hasError())
        loginf << "executing command '" << command << "' failed";

    if (result)
        *result = *dbresult;

    return !dbresult->hasError();
}

/**
 */
Result DuckDBConnection::insertBuffer_impl(const std::string& table_name, 
                                           const std::shared_ptr<Buffer>& buffer,
                                           const boost::optional<size_t>& idx_from, 
                                           const boost::optional<size_t>& idx_to,
                                           PropertyList* table_properties)
{
    //loginf << "DuckDBConnection: insertBuffer_impl: inserting into table '" << table_name << "'";

    if (!buffer || buffer->properties().size() == 0 || buffer->size() == 0)
        return Result::failed("input buffer invalid");

    //loginf << "creating appender...";

    auto appender = createAppender(table_name);
    if (!appender->valid())
        return Result::failed("creating duckdb appender failed: " + appender->lastError());

    size_t appender_column_count = appender->columnCount();

    //loginf << "configuring table props...";

    const auto& buffer_properties = buffer->properties();
    const auto& created_tables    = tableInfo();

    PropertyList table_props;
    const PropertyList* properties = nullptr;

    if (table_properties)
    {
        //loginf << "using externally passed properties";

        //use externally passed properties
        properties = table_properties;
    }
    else if (created_tables.count(table_name))
    {
        //loginf << "using stored table properties";

        //use stored table properties
        //@TODO: might fail!
        auto it = created_tables.find(table_name);
        auto tp = it->second.tableProperties();

        assert(tp.has_value());
        table_props = tp.value();

        properties = &table_props;
    }
    else
    {
        //loginf << "using buffer properties";

        //use buffers properties
        //note: !in this case the buffer properties need to reflect the table properties 1:1!
        properties = &buffer_properties;
    }
    assert(properties);

    //loginf << "update func...";

    #define UpdateFunc(PDType, DType, Suffix)                                                                               \
        bool ok = appender->append<DType>(buffer->get<DType>(pname).get(r));                                                \
        if (!ok)                                                                                                            \
            logerr << "DuckDBConnection: insertBuffer_impl: appending '" << pname << "' failed: " << appender->lastError(); \
        assert(ok);

    #define NotFoundFunc                                                                                      \
        logerr << "DuckDBConnection: insertBuffer_impl: unknown property type " << Property::asString(dtype); \
        assert(false);

    auto n = buffer->size();

    //loginf << "buffer properties: " << buffer_properties.size() << ", used properties: " << properties->size();

    //loginf << "inserting " << n << "row(s)...";

    unsigned int np = properties->size();
    assert(np == appender_column_count);

    std::vector<char> has_property(np, 0);
    for (unsigned int c = 0; c < properties->size(); ++c)
        has_property[ c ] = buffer->hasAnyPropertyNamed(properties->at(c).name()) ? 1 : 0;

    unsigned int r0 = idx_from.has_value() ? idx_from.value()   : 0;
    unsigned int r1 = idx_to.has_value()   ? idx_to.value() + 1 : n;
    assert(r0 <= r1);

    for (unsigned int r = r0; r < r1; ++r)
    {
        //loginf << "appending row " << std::to_string(r + 1) << "/" << std::to_string(n);

        for (unsigned int c = 0; c < np; ++c)
        {
            const auto& p     = properties->at(c);
            const auto& pname = p.name();
            const auto* pb    = has_property[ c ] ? &buffer->properties().get(p.name()) : nullptr;

            if (!pb || buffer->isNull(*pb, r))
            {
                appender->appendNull();
                continue;
            }

            auto dtype = pb->dataType();
            
            SwitchPropertyDataType(dtype, UpdateFunc, NotFoundFunc);
        }
        assert(appender->currentColumnCount() == np);
        appender->endRow();

        //if (r % 2000 == 0)
        //    appender->flush();
    }

    appender->flush();

    //cleanup appender
    appender.reset();

    return Result::succeeded();
}

/**
 * In DuckDB we update via a temporary table which seems to be much faster.
 */
Result DuckDBConnection::updateBuffer_impl(const std::string& table_name, 
                                           const std::shared_ptr<Buffer>& buffer,
                                           const std::string& key_column,
                                           const boost::optional<size_t>& idx_from, 
                                           const boost::optional<size_t>& idx_to)
{
    if (!buffer || buffer->properties().size() == 0 || buffer->size() == 0)
        return Result::failed("input buffer invalid");

    //create temporary table for buffer data
    std::vector<DBTableColumnInfo> table_columns;
    for (const auto& p : buffer->properties().properties())
        table_columns.push_back(DBTableColumnInfo(p.name(), p.dataType(), false));

    DBScopedTemporaryTable temp_table(this, table_columns);
    if (!temp_table.valid())
        return Result::failed("could not create temporary table for transaction: " + temp_table.result().error());

    //insert buffer data into temp table
    auto res_insert = insertBuffer(temp_table.name(), buffer, idx_from, idx_to);
    if (!res_insert.ok())
        return res_insert;

    //assign temp table data to target table
    std::vector<std::string> update_columns;
    bool key_col_found = false;
    for (const auto& p : buffer->properties().properties())
    {
        if (p.name() == key_column)
            key_col_found = true;
        else
            update_columns.push_back(p.name());
    }

    if (!key_col_found)
        return Result::failed("key column '" + key_column + "' not found in buffer");
    
    auto sql = sqlGenerator().getUpdateTableFromTableStatement(temp_table.name(),
                                                               table_name,
                                                               update_columns,
                                                               key_column);
    auto res_sql = execute(sql);
    if (!res_sql.ok())
        return res_sql;

    return Result::succeeded();
}

/**
 */
ResultT<std::vector<std::string>> DuckDBConnection::getTableList_impl()
{
    std::vector<std::string> tables;

    DBCommand command;
    command.set("SELECT table_name FROM duckdb_tables()");

    PropertyList list;
    list.addProperty("table_name", PropertyDataType::STRING);
    command.list(list);

    std::shared_ptr<DBResult> result = execute(command);
    assert(result);

    if (result->hasError())
        return ResultT<std::vector<std::string>>::failed(result->error());
    if (!result->buffer() || !result->containsData())
        return ResultT<std::vector<std::string>>::failed("table list could not be retrieved");

    std::shared_ptr<Buffer> buffer = result->buffer();

    unsigned int size = buffer->size();
    std::string table_name;

    for (unsigned int cnt = 0; cnt < size; cnt++)
    {
        table_name = buffer->get<std::string>("table_name").get(cnt);

        //@TODO: any tables to skip?

        tables.push_back(table_name);
    }

    return ResultT<std::vector<std::string>>::succeeded(tables);
}
