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

#pragma once

#include <sqlite3.h>

#include "dbconnection.h"

#include <memory>
#include <string>

class Buffer;
class DBInterface;
class PropertyList;
class DBResult;
class DBScopedPrepare;

/**
 * @brief Interface for a SQLite3 database connection
 *
 */
class SQLiteConnection : public DBConnection
{
public:
    SQLiteConnection(DBInterface* interface);
    virtual ~SQLiteConnection();

    std::shared_ptr<DBScopedPrepare> prepareStatement(const std::string& statement, 
                                                      bool begin_transaction) override final;

    //no precise types needed, e.g. suitable integer types will be chosen in the background
    bool needsPreciseDBTypes() const override final { return false; }

    //needed for query speed
    bool useIndexing() const override final { return true; }

protected:
    std::pair<bool, std::string> connect_impl(const std::string& file_name) override final;
    void disconnect_impl() override final;

    bool exportFile_impl(const std::string& file_name) override final;

    bool executeSQL_impl(const std::string& sql, DBResult* result, bool fetch_result_buffer) override final;
    bool executeCmd_impl(const std::string& command, const PropertyList* properties, DBResult* result) override final;

    boost::optional<std::vector<std::string>> getTableList_impl() override final;

private:
    /// Database handle to execute queries
    sqlite3* db_handle_{nullptr};
};
