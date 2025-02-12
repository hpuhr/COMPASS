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

#include "dbdefs.h"
#include "result.h"

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <boost/optional.hpp>

class Buffer;
class DBInterface;
class DBCommand;
class DBCommandList;
class DBResult;
class DBTableInfo;
class DBTableColumnInfo;
class PropertyList;
class DBScopedPrepare;
class DBScopedReader;
class SQLGenerator;

/**
 */
class DBConnection
{
public:
    DBConnection(DBInterface* interface); 
    virtual ~DBConnection();

    Result connect(const std::string& file_name);
    void disconnect();

    std::string status() const;
    bool dbOpened() { return db_opened_; }

    Result exportFile(const std::string& file_name);

    Result execute(const std::string& sql);
    std::shared_ptr<DBResult> execute(const std::string& sql, bool fetch_buffer);
    std::shared_ptr<DBResult> execute(const DBCommand& command);
    std::shared_ptr<DBResult> execute(const DBCommandList& command_list);

    Result createTable(const std::string& table_name, 
                                             const std::vector<DBTableColumnInfo>& column_infos,
                                             const std::string& dbcontent_name = "");
    Result updateTableInfo();
    void printTableInfo();
    const std::map<std::string, DBTableInfo>& tableInfo() const { return created_tables_; }
    Result insertBuffer(const std::string& table_name, 
                        const std::shared_ptr<Buffer>& buffer,
                        PropertyList* table_properties = nullptr);
    Result updateBuffer(const std::string& table_name, 
                        const std::shared_ptr<Buffer>& buffer,
                        const std::string& key_column,
                        const boost::optional<size_t>& idx_from = boost::optional<size_t>(), 
                        const boost::optional<size_t>& idx_to = boost::optional<size_t>());
    Result startRead(const std::shared_ptr<DBCommand>& select_cmd, 
                     size_t offset, 
                     size_t chunk_size);
    std::shared_ptr<DBResult> readChunk();
    void stopRead();

    SQLGenerator sqlGenerator() const;

    virtual std::shared_ptr<DBScopedPrepare> prepareStatement(const std::string& statement, 
                                                              bool begin_transaction) = 0;
    virtual std::shared_ptr<DBScopedReader> createReader(const std::shared_ptr<DBCommand>& select_cmd, 
                                                         size_t offset, 
                                                         size_t chunk_size) = 0;
    
    //db backend specific sql settings
    virtual db::SQLConfig sqlConfiguration() const = 0;
    
protected:
    virtual Result connect_impl(const std::string& file_name) = 0;
    virtual void disconnect_impl() = 0;

    virtual Result exportFile_impl(const std::string& file_name) = 0;

    virtual Result executeSQL_impl(const std::string& sql, DBResult* result, bool fetch_result_buffer) = 0; 
    virtual bool executeCmd_impl(const std::string& command, const PropertyList* properties, DBResult* result) = 0;

    virtual Result insertBuffer_impl(const std::string& table_name, 
                                     const std::shared_ptr<Buffer>& buffer,
                                     PropertyList* table_properties);
    virtual Result updateBuffer_impl(const std::string& table_name, 
                                     const std::shared_ptr<Buffer>& buffer,
                                     const std::string& key_column,
                                     const boost::optional<size_t>& idx_from, 
                                     const boost::optional<size_t>& idx_to);

    virtual ResultT<DBTableInfo> getColumnList_impl(const std::string& table);
    virtual ResultT<std::vector<std::string>> getTableList_impl() = 0;

    DBInterface& interface() { return interface_; }
    const DBInterface& interface() const { return interface_; }

private:
    ResultT<DBTableInfo> getColumnList(const std::string& table);
    ResultT<std::vector<std::string>> getTableList();

    DBInterface&                       interface_;
    std::map<std::string, DBTableInfo> created_tables_;
    bool                               db_opened_ = false;
    std::string                        db_filename_;

    std::shared_ptr<DBScopedReader>    active_reader_;
};
