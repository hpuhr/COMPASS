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

#include "duckdbprepare.h"
#include "duckdbexecresult.h"
#include "dbresult.h"

/**
 */
DuckDBPrepare::DuckDBPrepare(duckdb_connection connection) 
:   connection_(connection) 
{
}

/**
 */
DuckDBPrepare::~DuckDBPrepare() = default;

/**
 */
bool DuckDBPrepare::init_impl(const std::string& sql_statement)
{
    auto result = duckdb_prepare(connection_, sql_statement.c_str(), &statement_);
    return result == DuckDBSuccess;
}

/**
 */
void DuckDBPrepare::cleanup_impl()
{
    duckdb_destroy_prepare(&statement_);
}

/**
 */
bool DuckDBPrepare::beginTransaction_impl()
{
    auto res = duckdb_query(connection_, "BEGIN TRANSACTION", nullptr);
    return res == DuckDBSuccess;
}

/**
 */
bool DuckDBPrepare::endTransaction_impl()
{
    auto res = duckdb_query(connection_, "END TRANSACTION", nullptr);
    return res == DuckDBSuccess;
}

/**
 */
bool DuckDBPrepare::executeBinds_impl()
{
    auto result = duckdb_execute_prepared(statement_, NULL);
    return result != DuckDBSuccess;
}

/**
 */
bool DuckDBPrepare::execute_impl(const ExecOptions* options, DBResult* result)
{
    assert(result);

    bool fetch_buffer = result && result->buffer() != nullptr;

    DuckDBExecResult exec_result;

    auto state = duckdb_execute_prepared(statement_, exec_result.result());
    exec_result.setResultValid();

    if (state == DuckDBError)
    {
        if (result) result->setError(exec_result.errorString());
        return false;
    }
    else if (state != DuckDBSuccess)
    {
        if (result) result->setError("unknown error");
        return false;
    }

    if (fetch_buffer && !exec_result.toBuffer(*result->buffer()))
    {
        if (result) result->setError("reading query result failed");
        return false;
    }

    return true;
}
