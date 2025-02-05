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

#include "sqliteconnection.h"
#include "sqliteprepare.h"

#include "buffer.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "dbinterface.h"
#include "dbresult.h"
#include "dbtableinfo.h"
#include "logger.h"
#include "property.h"
#include "util/timeconv.h"

#include <QApplication>

#include <cstring>

using namespace std;
using namespace Utils;

/**
 */
SQLiteConnection::SQLiteConnection(DBInterface* interface)
:   DBConnection(interface)
{
    loginf << "SQLiteConnection: constructor: SQLITE_VERSION " << SQLITE_VERSION;
}

/**
 */
SQLiteConnection::~SQLiteConnection()
{
    loginf << "SQLiteConnection: destructor";
    
    if (dbOpened())
        disconnect();
}

/**
 */
std::shared_ptr<DBScopedPrepare> SQLiteConnection::prepareStatement(const std::string& statement, 
                                                                    bool begin_transaction)
{
    return std::shared_ptr<DBScopedPrepare>(new SQLiteScopedPrepare(db_handle_, statement, begin_transaction));
}

/**
 */
std::pair<bool, std::string> SQLiteConnection::connect_impl(const std::string& file_name)
{
    loginf << "SQLiteConnection: connect_impl: '" << file_name << "'";

    int result = sqlite3_open_v2(file_name.c_str(), &db_handle_,
                                 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

//    int result = sqlite3_open(":memory:", &db_handle_);

    if (result != SQLITE_OK)
    {
        // Even in case of an error we get a valid db_handle (for the
        // purpose of calling sqlite3_errmsg on it ...)
        std::string err(sqlite3_errmsg(db_handle_));
        sqlite3_close(db_handle_);
        return std::make_pair(false, err);
    }

    char* sErrMsg = 0;
    sqlite3_exec(db_handle_, "PRAGMA SYNCHRONOUS = OFF", NULL, NULL, &sErrMsg);
    sqlite3_exec(db_handle_, "PRAGMA TEMP_STORE = 2", NULL, NULL, &sErrMsg);
    sqlite3_exec(db_handle_, "PRAGMA JOURNAL_MODE = OFF", NULL, NULL, &sErrMsg);
    sqlite3_exec(db_handle_, "PRAGMA LOCKING_MODE = EXCLUSIVE", NULL, NULL, &sErrMsg);
    sqlite3_exec(db_handle_, "PRAGMA CACHE_SIZE = 500", NULL, NULL, &sErrMsg);

    //sqlite3_exec(db_handle_, "PRAGMA locking_mode = EXCLUSIVE", NULL, NULL, &sErrMsg);

    return std::make_pair(true, "");
}

/**
 */
void SQLiteConnection::disconnect_impl()
{
    if (db_handle_)
    {
        sqlite3_close(db_handle_);
        db_handle_ = nullptr;
    }
}

/**
 */
bool SQLiteConnection::exportFile_impl(const std::string& file_name)
{
    assert (dbOpened());

    string tmp_sql = "VACUUM INTO '"+file_name+"';";

    char* sErrMsg = 0;
    sqlite3_exec(db_handle_, tmp_sql.c_str(), NULL, NULL, &sErrMsg);

    return true;
}

/**
 */
bool SQLiteConnection::executeSQL_impl(const std::string& sql, 
                                       DBResult* result, 
                                       bool fetch_result_buffer)
{
    //@TODO
    return false;
}

/**
 */
bool SQLiteConnection::executeCmd_impl(const std::string& command, 
                                       const PropertyList* properties, 
                                       DBResult* result)
{
    //@TODO
    return false;
}

/**
 */
boost::optional<std::vector<std::string>> SQLiteConnection::getTableList_impl()
{
    std::vector<std::string> tables;

    DBCommand command;
    command.set("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name DESC;");
    // command.set ("SELECT name FROM sqlite_master WHERE name != 'sqlite_sequence' ORDER BY name  DESC;");

    PropertyList list;
    list.addProperty("name", PropertyDataType::STRING);
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
