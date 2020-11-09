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

#ifndef MySQLppConnection_H_
#define MySQLppConnection_H_

#include <mysql++/mysql++.h>

#include <string>

#include "configurable.h"
#include "dbconnection.h"
#include "global.h"

class Buffer;
class DBInterface;
class DBTableInfo;
class MySQLppConnectionWidget;
class MySQLppConnectionInfoWidget;
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
    MySQLppConnection(const std::string& class_id, const std::string& instance_id,
                      DBInterface* interface);
    virtual ~MySQLppConnection() override;

    void addServer(const std::string& name);
    void deleteUsedServer();
    void setServer(const std::string& server);
    void connectServer();

    void createDatabase(const std::string& database_name);
    void deleteDatabase(const std::string& database_name);
    void openDatabase(const std::string& database_name);

    virtual void disconnect() override;

    void executeSQL(const std::string& sql) override;

    void prepareBindStatement(const std::string& statement) override;
    void beginBindTransaction() override;
    void stepAndClearBindings() override;
    void endBindTransaction() override;
    void finalizeBindStatement() override;

    void bindVariable(unsigned int index, int value) override;
    void bindVariable(unsigned int index, double value) override;
    void bindVariable(unsigned int index, const std::string& value) override;
    void bindVariableNull(unsigned int index) override;

    std::shared_ptr<DBResult> execute(const DBCommand& command) override;
    std::shared_ptr<DBResult> execute(const DBCommandList& command_list) override;

    void prepareCommand(const std::shared_ptr<DBCommand> command) override;
    std::shared_ptr<DBResult> stepPreparedCommand(unsigned int max_results = 0) override;
    void finalizeCommand() override;
    bool getPreparedCommandDone() override { return prepared_command_done_; }

    /// @brief Added for performance test. Do not use.
    // DBResult *readBulkCommand (DBCommand *command, std::string main_statement, std::string
    // order_statement,
    // unsigned int max_results=0);

    std::map<std::string, DBTableInfo> getTableInfo() override;
    std::vector<std::string> getDatabases() override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    QWidget* widget() override;
    // void deleteWidget () override;
    QWidget* infoWidget() override;
    std::string status() const override;
    std::string identifier() const override;
    std::string shortIdentifier() const override;
    std::string type() const override { return MYSQL_IDENTIFIER; }

    const std::map<std::string, MySQLServer*>& servers() const { return servers_; }

    const std::string& usedServerString() { return used_server_; }
    MySQLServer& usedServer()
    {
        assert(servers_.count(used_server_) == 1);
        return *servers_.at(used_server_);
    }

    MySQLServer& connectedServer()
    {
        assert(connected_server_);
        return *connected_server_;
    }

    //    void importSQLFile (const std::string& filename);
    //    void importSQLArchiveFile (const std::string& filename);

    std::string usedDatabase() const;
    void usedDatabase(const std::string& used_database);

protected:
    DBInterface& interface_;
    std::string used_server_;
    std::string used_database_;

    MySQLServer* connected_server_{nullptr};

    /// Used for all database queries
    mysqlpp::Connection connection_;

    /// Prepared query
    mysqlpp::Query prepared_query_;
    /// Parameters which are bound to the a query
    mysqlpp::SQLQueryParms prepared_parameters_;
    /// Result from query for incremental reading.
    mysqlpp::UseQueryResult result_step_;
    /// Query is in use flag.
    bool query_used_{false};

    // Transaction which can group queries (for fast insertion)
    mysqlpp::Transaction* transaction_{nullptr};

    /// Last prepared command
    std::shared_ptr<DBCommand> prepared_command_;
    /// Prepared command finished flag.
    bool prepared_command_done_{true};

    std::unique_ptr<MySQLppConnectionWidget> widget_;
    std::unique_ptr<MySQLppConnectionInfoWidget> info_widget_;

    std::map<std::string, MySQLServer*> servers_;

    void prepareStatement(const std::string& sql) override;
    void finalizeStatement() override;

    /// @brief Executes an SQL command which returns data (internal)
    void execute(const std::string& command, std::shared_ptr<Buffer> buffer);

    /// @brief Executes an SQL command which returns no data (internal)
    void execute(const std::string& command);

    void readRowIntoBuffer(mysqlpp::Row& row, const PropertyList& list, unsigned int num_properties,
                           std::shared_ptr<Buffer> buffer, unsigned int index);

    std::vector<std::string> getTableList();
    DBTableInfo getColumnList(const std::string& table);

    /// @brief Used for performance tests.
    void performanceTest();
};

#endif /* MySQLppConnection_H_ */
