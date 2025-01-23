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

#include <duckdb.h>

#include "duckdbconnectionsettings.h"
#include "property.h"
#include "propertylist.h"

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <set>

#include <boost/optional.hpp>

class DuckDBScopedAppender;
class DuckDBScopedPrepare;
class DuckDBResult;

class Buffer;
class DBInterface;
class DBCommand;
class DBCommandList;
class DBResult;
class DBConnectionInfo;
class DBTableInfo;
class DBContent;

/**
 */
class DuckDBConnection
{
public:
    typedef std::pair<std::string, PropertyDataType> TableColumn;
    typedef std::vector<TableColumn> TableDescription;

    DuckDBConnection(DBInterface* interface);
    virtual ~DuckDBConnection();

    std::pair<bool, std::string> connect(const std::string& file_name);
    void disconnect();

    void exportFile(const std::string& file_name);
    
    std::shared_ptr<DuckDBScopedAppender> createAppender(const std::string& table);
    std::shared_ptr<DuckDBScopedPrepare> prepareStatement(const std::string& statement);

    std::shared_ptr<DBResult> execute(const std::string& sql, bool fetch_result_buffer = false);
    std::shared_ptr<DBResult> execute(const DBCommand& command);
    std::shared_ptr<DBResult> execute(const DBCommandList& command_list);

    bool createTable(const DBContent& dbcontent);
    std::map<std::string, DBTableInfo> getTableInfo();
    bool insertBuffer(const std::string& table_name, const std::shared_ptr<Buffer>& buffer);
    bool insertBuffer(const DBContent& dbcontent, const std::shared_ptr<Buffer>& buffer);

    std::string status() const;

    bool dbOpened() { return db_opened_; }

    DuckDBConnectionSettings& settings() { return settings_; }

protected:
    std::shared_ptr<DuckDBResult> executeCmd(const std::string& command);

    std::vector<std::string> getTableList();
    DBTableInfo getColumnList(const std::string& table);

    DBInterface& interface_;

    /// Database handle to execute queries
    duckdb_database   db_;
    duckdb_connection connection_;
    bool              db_opened_ = false;

    std::map<std::string, PropertyList> created_tables_;

    DuckDBConnectionSettings settings_;
};
