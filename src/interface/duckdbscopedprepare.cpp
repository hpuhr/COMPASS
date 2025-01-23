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

#include "duckdbscopedprepare.h"
#include "duckdbcommon.h"

/**
 */
DuckDBScopedPrepare::DuckDBScopedPrepare(duckdb_connection connection, 
                                         const std::string& sql_statement) 
{
    auto result = duckdb_prepare(connection, sql_statement.c_str(), &statement_);
    ok_ = result == DuckDBSuccess;
}

/**
 */
DuckDBScopedPrepare::~DuckDBScopedPrepare()
{
    if (ok_)
        duckdb_destroy_prepare(&statement_);
}

/**
 */
bool DuckDBScopedPrepare::valid() const
{
    return ok_;
}

/**
 */
std::shared_ptr<DuckDBResult> DuckDBScopedPrepare::execute()
{
    std::shared_ptr<DuckDBResult> result(new DuckDBResult);
    if (!ok_)
    {
        result->error_     = true;
        result->error_msg_ = "prepared command is invalid";
        return result;
    }

    auto state = duckdb_execute_prepared(statement_, &result->result_);
    result->error_ = state != DuckDBSuccess;

    if (state == DuckDBError)
    {
        result->error_msg_ = std::string(duckdb_result_error(&result->result_));
    }
    else if (state != DuckDBSuccess)
    {
        result->error_msg_ = "unknown error";
    }

    result->has_result_ = true;

    return result;
}
