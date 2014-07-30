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
#include "DBConnection.h"

class Buffer;

/**
 * @brief Interface for a MySQL database connection
 *
 * @details Uses the Mysql++ library.
 */
class MySQLppConnection : public DBConnection
{
public:
    MySQLppConnection(DBConnectionInfo *info);
    virtual ~MySQLppConnection();

    void init ();

    void executeSQL(std::string sql);

    void prepareBindStatement (std::string statement);
    void beginBindTransaction ();
    void stepAndClearBindings ();
    void endBindTransaction ();
    void finalizeBindStatement ();

    void bindVariable (unsigned int index, int value);
    void bindVariable (unsigned int index, double value);
    void bindVariable (unsigned int index, const char *value);
    void bindVariableNull (unsigned int index);

    DBResult *execute (DBCommand *command);
    DBResult *execute (DBCommandList *command_list);

    void prepareCommand (DBCommand *command);
    DBResult *stepPreparedCommand (unsigned int max_results=0);
    void finalizeCommand ();
    bool getPreparedCommandDone () { return prepared_command_done_; };

    /// @brief Added for performance test. Do not use.
    DBResult *readBulkCommand (DBCommand *command, std::string main_statement, std::string order_statement, unsigned int max_results=0);

    Buffer *getTableList();
    Buffer *getColumnList(std::string table);

private:
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
    DBCommand *prepared_command_;
    /// Prepared command finished flag.
    bool prepared_command_done_;

    /// Key which was last read, needed by test function readBulkCommand
    unsigned int last_key_;

    void prepareStatement (const char *sql);
    void finalizeStatement ();

    void execute (std::string command, Buffer *buffer);

    /// @brief Used for performance tests.
    void performanceTest ();
};

#endif /* MySQLppConnection_H_ */
