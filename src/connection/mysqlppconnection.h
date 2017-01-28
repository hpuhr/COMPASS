/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * MySQLppConnection.h
 *
 *  Created on: Jul 19, 2012
 *      Author: sk
 */

#ifndef MySQLppConnection_H_
#define MySQLppConnection_H_

#include <mysql++/mysql++.h>
#include <string>

#include "configurable.h"
#include "dbconnection.h"

class Buffer;
class DBTableInfo;
class MySQLppConnectionWidget;
class MySQLServer;
class PropertyList;

/**
 * @brief Interface for a MySQL database connection
 *
 * @details Uses the Mysql++ library.
 */
class MySQLppConnection : public DBConnection
{
public:
    MySQLppConnection(const std::string &instance_id, Configurable *parent);
    virtual ~MySQLppConnection();

    void addServer (std::string name);
    void deleteUsedServer ();
    void setServer (const std::string &server);
    void connectServer ();
    virtual void openDatabase (const std::string &database_name);

    virtual void disconnect ();

    void executeSQL(const std::string &sql);

    void prepareBindStatement (const std::string &statement);
    void beginBindTransaction ();
    void stepAndClearBindings ();
    void endBindTransaction ();
    void finalizeBindStatement ();

    void bindVariable (unsigned int index, int value);
    void bindVariable (unsigned int index, double value);
    void bindVariable (unsigned int index, const char *value);
    void bindVariableNull (unsigned int index);

    std::shared_ptr <DBResult> execute (const DBCommand &command);
    std::shared_ptr <DBResult> execute (const DBCommandList &command_list);

    void prepareCommand (const DBCommand &command);
    std::shared_ptr <DBResult> stepPreparedCommand (unsigned int max_results=0);
    void finalizeCommand ();
    bool getPreparedCommandDone () { return prepared_command_done_; }

    /// @brief Added for performance test. Do not use.
    //DBResult *readBulkCommand (DBCommand *command, std::string main_statement, std::string order_statement, unsigned int max_results=0);
    std::map <std::string, DBTableInfo> getTableInfo ();
    std::vector <std::string> getDatabases();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    QWidget *widget ();

    const std::map <std::string, MySQLServer*> &servers () const { return servers_; }

    const std::string &usedServerString () { return used_server_; }
    MySQLServer& usedServer () { assert (servers_.count (used_server_) == 1); return *servers_.at(used_server_); }

    MySQLServer& connectedServer () { assert (connected_server_); return *connected_server_; }

private:
    std::string used_server_;
    MySQLServer *connected_server_;

    /// Used for all database queries
    mysqlpp::Connection connection_;

    /// Prepared query
    mysqlpp::Query prepared_query_;
    /// Parameters which are bound to the a query
    mysqlpp::SQLQueryParms prepared_parameters_;
    /// Result from query for incremental reading.
    mysqlpp::UseQueryResult result_step_;
    /// Query is in use flag.
    bool query_used_;

    // Transaction which can group queries (for fast insertion)
    mysqlpp::Transaction *transaction_;

    /// Last prepared command
    const DBCommand *prepared_command_;
    /// Prepared command finished flag.
    bool prepared_command_done_;

    MySQLppConnectionWidget *widget_;

    std::map <std::string, MySQLServer*> servers_;

    void prepareStatement (const char *sql);
    void finalizeStatement ();

    /// @brief Executes an SQL command which returns data (internal)
    void execute (const std::string &command, std::shared_ptr <Buffer> buffer);

    /// @brief Executes an SQL command which returns no data (internal)
    void execute (const std::string &command);

    void readRowIntoBuffer (mysqlpp::Row &row, const PropertyList &list, unsigned int num_properties, std::shared_ptr <Buffer> buffer, unsigned int index);

    std::vector<std::string> getTableList();
    DBTableInfo getColumnList(const std::string &table);

    /// @brief Used for performance tests.
    void performanceTest ();
};

#endif /* MySQLppConnection_H_ */
