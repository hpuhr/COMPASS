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
 * SQLiteConnection.h
 *
 *  Created on: Jan 11, 2012
 *      Author: sk
 */

#ifndef SQLITECONNECTION_H_
#define SQLITECONNECTION_H_

#include <sqlite3.h>
#include <string>
#include "DBConnection.h"

class Buffer;

/**
 * @brief Interface for a SQLite3 database connection
 *
 */
class SQLiteConnection : public DBConnection
{
public:
    SQLiteConnection(DBConnectionInfo *info);
    virtual ~SQLiteConnection();

    virtual void connect ();
    virtual void openDatabase (std::string database_name);

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

    Buffer *getTableList();
    Buffer *getColumnList(std::string table);

protected:
    /// Database handle to execute queries
    sqlite3* db_handle_;
    /// Statement for binding variables to.
    sqlite3_stmt *statement_;

    DBCommand *prepared_command_;
    bool prepared_command_done_;

    void execute (std::string command, Buffer *buffer);

    void prepareStatement (const char *sql);
    void finalizeStatement ();

};

#endif /* DBINTERFACE_H_ */
