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

#include "sqliteprepare.h"
#include "dbexecresult.h"

#include "logger.h"

/**
 */
SQLitePrepare(sqlite3* connection) 
:   connection_(connection) 
{
    assert(connection_);
}

/**
 */
SQLitePrepare::~SQLitePrepare() = default;

/**
 */
bool SQLitePrepare::init_impl(const std::string& sql_statement)
{
    int result;
    const char* remaining_sql;

    // Prepare the select statement
    remaining_sql = NULL;
    result = sqlite3_prepare_v2(connection_, sql_statement.c_str(), sql_statement.size(), &statement_, &remaining_sql);
    if (result != SQLITE_OK)
    {
        logerr << "SQLiteConnection: execute: error " << result << " " << sqlite3_errmsg(connection_);
        return false;
    }

    if (remaining_sql && *remaining_sql != '\0')
    {
        logerr << "SQLiteConnection: execute: there was unparsed sql text: " << remaining_sql;
        return false;
    }
    
    return true;
}

/**
 */
void SQLitePrepare::cleanup_impl()
{
    sqlite3_finalize(statement_);
}

/**
 */
void SQLitePrepare::cleanupBinds_impl()
{
    sqlite3_clear_bindings(statement_);
    sqlite3_reset(statement_);
}

/**
 */
bool SQLitePrepare::beginTransaction_impl()
{
    char* sErrMsg = 0;
    auto res = sqlite3_exec(connection_, "BEGIN TRANSACTION", NULL, NULL, &sErrMsg);
    return res == SQLITE_OK;
}

/**
 */
bool SQLitePrepare::endTransaction_impl()
{
    char* sErrMsg = 0;
    auto res = sqlite3_exec(connection_, "END TRANSACTION", NULL, NULL, &sErrMsg);
    return res == SQLITE_OK;
}

/**
 */
bool SQLitePrepare::executeBinds_impl()
{
    //@TODO
    return false;
}

/**
 */
bool SQLitePrepare::execute_impl(const ExecOptions* options, DBResult* result)
{
    //@TODO
    return false;
}
