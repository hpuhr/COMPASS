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
 * MySQLConConnection.h
 *
 *  Created on: Mar 7, 2013
 *      Author: sk
 */

#ifndef MYSQLCONCONNECTION_H_
#define MYSQLCONCONNECTION_H_

#include <string>

#include "DBConnection.h"
#include "cppconn/driver.h"
#include "cppconn/resultset.h"

class Buffer;

/**
 * @brief Interface for a MySQL database connection
 *
 * @details Uses the Mysql connector library.
 */
class MySQLConConnection : public DBConnection
{
public:
    MySQLConConnection(DBConnectionInfo *info);
    virtual ~MySQLConConnection();

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

    Buffer *getTableList(std::string database_name);
    Buffer *getColumnList(std::string database_name, std::string table);

protected:
    sql::Driver *driver_;
    sql::Connection *connection_;

    /// Last prepared command
    DBCommand *prepared_command_;
    sql::PreparedStatement *prepared_statement_;

    sql::Statement *statement_;
    sql::ResultSet *resultset_;
    /// Prepared command finished flag.
    bool prepared_command_done_;

    void prepareStatement (const char *sql);
    void finalizeStatement ();

    void execute (std::string command, Buffer *buffer);
};

#endif /* MYSQLCONCONNECTION_H_ */
