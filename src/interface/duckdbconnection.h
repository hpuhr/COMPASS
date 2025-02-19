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
class DBScopedPrepare;
class DBScopedReader;

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
    std::shared_ptr<DBScopedReader> createReader(const std::shared_ptr<DBCommand>& select_cmd, 
                                                 size_t offset, 
                                                 size_t chunk_size) override final;

    DuckDBConnectionSettings& settings() { return settings_; }

protected:
    Result connect_impl(const std::string& file_name) override final;
    void disconnect_impl() override final;

    Result exportFile_impl(const std::string& file_name) override final;

    Result executeSQL_impl(const std::string& sql, DBResult* result, bool fetch_result_buffer) override final;
    bool executeCmd_impl(const std::string& command, const PropertyList* properties, DBResult* result) override final;

    Result insertBuffer_impl(const std::string& table_name, 
                             const std::shared_ptr<Buffer>& buffer,
                             const boost::optional<size_t>& idx_from, 
                             const boost::optional<size_t>& idx_to,
                             PropertyList* table_properties) override final;
    Result updateBuffer_impl(const std::string& table_name, 
                             const std::shared_ptr<Buffer>& buffer,
                             const std::string& key_column,
                             const boost::optional<size_t>& idx_from, 
                             const boost::optional<size_t>& idx_to) override final;

    ResultT<std::vector<std::string>> getTableList_impl() override final;

    Result cleanupDB_impl(const std::string& db_fn) override final;

    /**
     */
    db::SQLConfig sqlConfiguration_impl() const override final
    {
        db::SQLConfig config;

        //duckdb needs precise data types in its tables
        config.precise_types = true;

        //no indexing, queries should be quite fast in duckdb 
        //and indexing blows up the db file consiferably
        config.indexing = false;

        //duckdb can only comprehend ? as a placeholder
        config.placeholder = db::SQLPlaceholder::QuestionMark;

        //duckdb does not allow 'replace into' directly, this is handled via 'insert ... on conflict'
        config.use_conflict_resolution = true;

        return config;
    }
    
private:
    duckdb_database   db_         = nullptr;
    duckdb_connection connection_ = nullptr;

    DuckDBConnectionSettings settings_;
};
