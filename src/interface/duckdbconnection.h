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

#include "duckdbconnectionsettings.h"
#include "dbconnection.h"

#include <memory>
#include <string>
#include <vector>

#include <boost/optional.hpp>

class DuckDBScopedAppender;
class DuckDBScopedPrepare;

class Buffer;
class DBInterface;
class DBResult;
class PropertyList;

/**
 */
class DuckDBConnection : public DBConnection
{
public:
    DuckDBConnection(DBInterface* interface);
    virtual ~DuckDBConnection();

    std::shared_ptr<DuckDBScopedAppender> createAppender(const std::string& table);
    std::shared_ptr<DBScopedPrepare> prepareStatement(const std::string& statement, 
                                                      bool begin_transaction) override final;

    //duckdb needs precise data types in its tables
    bool needsPreciseDBTypes() const override final { return true; }

    //no indexing, queries should be quite fast in duckdb 
    //and indexing blows up the db file consiferably
    bool useIndexing() const override final { return false; }

    DuckDBConnectionSettings& settings() { return settings_; }

protected:
    std::pair<bool, std::string> connect_impl(const std::string& file_name) override final;
    void disconnect_impl() override final;

    bool exportFile_impl(const std::string& file_name) override final;

    bool executeSQL_impl(const std::string& sql, DBResult* result, bool fetch_result_buffer) override final;
    bool executeCmd_impl(const std::string& command, const PropertyList* properties, DBResult* result) override final;

    std::pair<bool, std::string> insertBuffer_impl(const std::string& table_name, 
                                                   const std::shared_ptr<Buffer>& buffer) override final;

    boost::optional<std::vector<std::string>> getTableList_impl() override final;
    
private:
    duckdb_database   db_;
    duckdb_connection connection_;

    DuckDBConnectionSettings settings_;
};
