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
class DBScopedReader;

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
    std::shared_ptr<DBScopedReader> createReader(const std::shared_ptr<DBCommand>& select_cmd, 
                                                 size_t offset, 
                                                 size_t chunk_size) override final;
protected:
    Result connect_impl(const std::string& file_name) override final;
    void disconnect_impl() override final;

    Result exportFile_impl(const std::string& file_name) override final;

    Result executeSQL_impl(const std::string& sql, DBResult* result, bool fetch_result_buffer) override final;
    bool executeCmd_impl(const std::string& command, const PropertyList* properties, DBResult* result) override final;

    ResultT<std::vector<std::string>> getTableList_impl() override final;

    /**
     */
    db::SQLConfig sqlConfiguration_impl() const override final
    {
        db::SQLConfig config;

        //no precise types needed, e.g. suitable integer types will be chosen in the background
        config.precise_types = false;

        //needed for query speed
        config.indexing = true;

        config.placeholder = db::SQLPlaceholder::AtVar;
        config.use_conflict_resolution = false;

        return config;
    }

private:
    /// Database handle to execute queries
    sqlite3* db_handle_{nullptr};
};
