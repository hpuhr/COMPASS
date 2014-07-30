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
 * DBInterface.cpp
 *
 *  Created on: Feb 1, 2012
 *      Author: sk
 */

#include <cstring>

#include "Buffer.h"
#include "DBCommand.h"
#include "DBCommandList.h"
#include "DBResult.h"
#include "DBConnectionInfo.h"
#include "Logger.h"
#include "SQLiteConnection.h"

#include "Data.h"

using namespace Utils::Data;


SQLiteConnection::SQLiteConnection(DBConnectionInfo *info)
: DBConnection (info)
{
    assert (info_);
    assert (info_ ->getType() == DB_TYPE_SQLITE);

    db_handle_ = 0;
    prepared_command_=0;
    prepared_command_done_=false;
}

SQLiteConnection::~SQLiteConnection()
{
    if (db_handle_)
    {
        sqlite3_close(db_handle_);
        db_handle_=0;
    }
}

void SQLiteConnection::init()
{
    assert (info_->getType() == DB_TYPE_SQLITE);

    SQLite3ConnectionInfo *info = (SQLite3ConnectionInfo*) info_;

    int result = sqlite3_open_v2(info->getFilename().c_str(), &db_handle_, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

    if (result != SQLITE_OK)
    {
        // Even in case of an error we get a valid db_handle (for the
        // purpose of calling sqlite3_errmsg on it ...)
        logerr  <<  "SQLiteConnection: createRDBFile: error " <<  result << " " <<  sqlite3_errmsg(db_handle_);
        sqlite3_close(db_handle_);
        throw std::runtime_error ("SQLiteConnection: createRDBFile: error");
    }

    char * sErrMsg = 0;
    sqlite3_exec(db_handle_, "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);
}

void SQLiteConnection::executeSQL(std::string sql)
{
    logdbg  << "DBInterface: executeSQL: sql statement execute: '" <<sql << "'";

    char* exec_err_msg = NULL;
    int result = sqlite3_exec(db_handle_, sql.c_str(), NULL, NULL, &exec_err_msg);
    if (result != SQLITE_OK)
    {
        logerr  << "DBInterface: executeSQL: sqlite3_exec failed: " << exec_err_msg;
        sqlite3_free(exec_err_msg);
        std::string error;
        error += "DBInterface: executeSQL: sqlite3_exec failed: ";
        error += exec_err_msg;
        throw std::runtime_error (error);
    }
}

void SQLiteConnection::prepareBindStatement (std::string statement)
{
    const char * tail = 0;
    int ret=sqlite3_prepare_v2(db_handle_, statement.c_str(), statement.size(), &statement_, &tail);

    if (ret != SQLITE_OK)
    {
        logerr  << "DBInterface: prepareBindStatement: error preparing bind";
        return;
    }
}
void SQLiteConnection::beginBindTransaction ()
{
    char * sErrMsg = 0;
    sqlite3_exec(db_handle_, "BEGIN TRANSACTION", NULL, NULL, &sErrMsg);
}
void SQLiteConnection::stepAndClearBindings ()
{
    logdbg  << "DBInterface: stepAndClearBindings: stepping statement";
    int ret2;
    while ((ret2=sqlite3_step(statement_)))
    {
        logdbg  << "DBInterface: stepAndClearBindings: stepping returned "<< ret2;
        if (ret2 == SQLITE_DONE)
        {
            logdbg  << "DBInterface: stepAndClearBindings: bind done";
            break;
        }
        else if (ret2 > SQLITE_OK  && ret2 < SQLITE_ROW)
        {
            logerr  << "DBInterface: stepAndClearBindings: error while bind: " << ret2;
            throw std::runtime_error ("DBInterface: stepAndClearBindings: error while bind");
        }
    }

    logdbg  << "DBInterface: stepAndClearBindings: clearing statement";

    sqlite3_clear_bindings(statement_);
    sqlite3_reset(statement_);

}

void SQLiteConnection::endBindTransaction ()
{
    char * sErrMsg = 0;
    sqlite3_exec(db_handle_, "END TRANSACTION", NULL, NULL, &sErrMsg);
}
void SQLiteConnection::finalizeBindStatement ()
{
    sqlite3_finalize(statement_);
}

void SQLiteConnection::bindVariable (unsigned int index, int value)
{
    logdbg  << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_int(statement_, index, value);
}
void SQLiteConnection::bindVariable (unsigned int index, double value)
{
    logdbg  << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_double(statement_, index, value);
}
void SQLiteConnection::bindVariable (unsigned int index, const char *value)
{
    logdbg  << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_text(statement_, index, value, -1, SQLITE_TRANSIENT);
}

void SQLiteConnection::bindVariableNull (unsigned int index)
{
    sqlite3_bind_null(statement_, index);
}

// TODO: beware of se deleted propertylist, new buffer should use deep copied list
DBResult *SQLiteConnection::execute (DBCommand *command)
{
    assert (command);
    DBResult *dbresult = new DBResult ();

    std::string sql = command->getCommandString();

    Buffer *buffer=0;
    if (command->getResultList()->getNumProperties() > 0) // data should be returned
    {
        buffer = new Buffer (*(command->getResultList()));
        dbresult->setBuffer(buffer);
    }

    execute (sql, buffer);

    logdbg  << "SQLiteConnection: execute: end";

    return dbresult;
}

DBResult *SQLiteConnection::execute (DBCommandList *command_list)
{
    assert (command_list);
    DBResult *dbresult = new DBResult ();

    unsigned int num_commands = command_list->getNumCommands();

    Buffer *buffer=0;

    if (command_list->getResultList()->getNumProperties() > 0) // data should be returned
    {
        buffer = new Buffer (*(command_list->getResultList()));
        dbresult->setBuffer(buffer);
    }

    for (unsigned int cnt=0; cnt < num_commands; cnt++)
    {
        if (cnt != 0)
            buffer->incrementIndex();

        std::string sql = command_list->getCommandString(cnt);

        execute (sql, buffer);
    }
    logdbg  << "SQLiteConnection: execute: end";

    return dbresult;
}

void SQLiteConnection::execute (std::string command, Buffer *buffer)
{
    logdbg  << "SQLiteConnection: execute";

    unsigned int num_properties=0;
    PropertyList *list=0;
    Property *prop=0;
    std::vector <Property*> *properties = 0;
    std::vector<void*>* adresses;

    if (buffer)
    {
        list = buffer->getPropertyList();
        num_properties = list->getNumProperties();
        properties = list->getProperties();
        //buffer->setIndex(0); // HARDCORE
    }

    int row_counter;
    int result;

    prepareStatement(command.c_str());

    // Now step throught the result lines
    for (result = sqlite3_step(statement_), row_counter = 0; result == SQLITE_ROW; result = sqlite3_step(statement_), ++row_counter)
    {
        if (buffer && row_counter != 0)
        {
            buffer->incrementIndex();
        }
        if (buffer && row_counter == 0)
                buffer->unsetFirstWrite();

        for (unsigned int cnt=0; cnt < num_properties; cnt++)
        {
            assert (buffer);
            prop=properties->at(cnt);

            if (buffer)
            {
                adresses = buffer->getAdresses();

                if (sqlite3_column_type(statement_, cnt) == SQLITE_NULL)
                    setNan(prop->data_type_int_, adresses->at(cnt));
                else
                {
                    switch (prop->data_type_int_)
                    {
                    case P_TYPE_BOOL:
                    {
                        *(bool*) adresses->at(cnt) = sqlite3_column_int(statement_, cnt) > 0;
                        //loginf  << "sqlex: bool " << prop->id_ << " val " << *ptr;
                    }
                    break;
                    case P_TYPE_UCHAR:
                    {
                        *(unsigned char*) adresses->at(cnt) = (unsigned char) sqlite3_column_int(statement_, cnt);
                        //loginf  << "sqlex: uchar " << prop->id_ << " val " << *ptr;
                    }
                    break;
                    case P_TYPE_CHAR:
                    {
                        *(char*) adresses->at(cnt) = (char) sqlite3_column_int(statement_, cnt);
                        //loginf  << "sqlex: char " << prop->id_ << " val " << *ptr;
                    }
                    break;
                    case P_TYPE_INT:
                    {
                        *(int*) adresses->at(cnt) = sqlite3_column_int(statement_, cnt);
                        //loginf  << "sqlex: int " << prop->id_ << " val " << *ptr;
                    }
                    break;
                    case P_TYPE_UINT:
                    {
                        *(unsigned int*) adresses->at(cnt) = (unsigned int) sqlite3_column_int(statement_, cnt);
                        //loginf  << "sqlex: uint " << prop->id_ << " val " << *ptr;
                    }
                    break;
                    case P_TYPE_STRING:
                    {
                        *(std::string*) adresses->at(cnt) = (const char *)sqlite3_column_text(statement_, cnt);
                        //loginf  << "sqlex: string " << prop->id_ << " val " << *ptr;
                    }
                    break;
                    case P_TYPE_FLOAT:
                    {
                        *(float*) adresses->at(cnt) = (float) sqlite3_column_double (statement_, cnt);
                        //loginf  << "sqlex: float " << prop->id_ << " val " << *ptr;
                    }
                    break;
                    case P_TYPE_DOUBLE:
                    {
                        *(double*) adresses->at(cnt) = sqlite3_column_double (statement_, cnt);
                        //loginf  << "sqlex: double " << prop->id_ << " val " << *ptr;
                    }
                    break;
                    default:
                        logerr  <<  "SQLiteConnection: execute: unknown property type";
                        throw std::runtime_error ("SQLiteConnection: execute: unknown property type");
                        break;
                    }
                }
            }
        }
    }

    if (result != SQLITE_DONE)
    {
        logerr <<  "SQLiteConnection: execute: problem while stepping the result: " <<  result << " " <<  sqlite3_errmsg(db_handle_);
        throw std::runtime_error ("SQLiteConnection: execute: problem while stepping the result");
    }

    finalizeStatement();
}

void SQLiteConnection::prepareStatement (const char *sql)
{
    logdbg  << "SQLiteConnection: prepareStatement: sql '" << sql << "'";
    int result;
    const char* remaining_sql;

    // Prepare the select statement
    remaining_sql = NULL;
    result = sqlite3_prepare_v2(db_handle_, sql, strlen(sql), &statement_, &remaining_sql);
    if (result != SQLITE_OK)
    {
        logerr <<  "SQLiteConnection: execute: error " <<  result << " " <<  sqlite3_errmsg(db_handle_);
        sqlite3_close(db_handle_);
        throw std::runtime_error ("SQLiteConnection: execute: error");
    }

    if (remaining_sql && *remaining_sql != '\0')
    {
        logerr  <<  "SQLiteConnection: execute: there was unparsed sql text: " << remaining_sql;
        throw std::runtime_error ("SQLiteConnection: execute: there was unparsed sql text");
    }
}
void SQLiteConnection::finalizeStatement ()
{
    sqlite3_finalize(statement_);
}

void SQLiteConnection::prepareCommand (DBCommand *command)
{
    assert (prepared_command_==0);
    assert (command);

    prepared_command_=command;
    prepared_command_done_=false;

    prepareStatement (command->getCommandString().c_str());
}
DBResult *SQLiteConnection::stepPreparedCommand (unsigned int max_results)
{
    assert (prepared_command_);

    DBResult *dbresult = new DBResult ();

    std::string sql = prepared_command_->getCommandString();

    Buffer *buffer=0;
    if (prepared_command_->getResultList()->getNumProperties() > 0) // data should be returned
    {
        buffer = new Buffer (*(prepared_command_->getResultList()));
        dbresult->setBuffer(buffer);
    }

    unsigned int num_properties=0;
    PropertyList *list=0;
    Property *prop=0;
    std::vector <Property*> *properties = 0;
    unsigned int result_cnt=0;
    std::vector<void*>* adresses;

    if (buffer)
    {
        list = buffer->getPropertyList();
        num_properties = list->getNumProperties();
        properties = list->getProperties();
        buffer->setIndex(0);
    }

    unsigned int row_counter;
    int result;

    max_results--;

    // Now step throught the result lines
    for (result = sqlite3_step(statement_), row_counter = 0; result == SQLITE_ROW; result = sqlite3_step(statement_), ++row_counter)
    {
        if (buffer && row_counter != 0)
        {
            buffer->incrementIndex();
        }

        for (unsigned int cnt=0; cnt < num_properties; cnt++)
        {
            assert (buffer);
            prop=properties->at(cnt);

            if (buffer)
                adresses = buffer->getAdresses();

            if (sqlite3_column_type(statement_, cnt) == SQLITE_NULL)
                setNan(prop->data_type_int_, adresses->at(cnt));
            else
            {
                switch (prop->data_type_int_)
                {
                case P_TYPE_BOOL:
                {
                    *(bool*) adresses->at(cnt) = sqlite3_column_int(statement_, cnt) > 0;
                }
                break;
                case P_TYPE_UCHAR:
                {
                    *(unsigned char*) adresses->at(cnt) = (unsigned char) sqlite3_column_int(statement_, cnt);
                }
                break;
                case P_TYPE_CHAR:
                {
                    *(char*) adresses->at(cnt) = (char) sqlite3_column_int(statement_, cnt);
                }
                break;
                case P_TYPE_INT:
                {
                    *(int*) adresses->at(cnt) = sqlite3_column_int(statement_, cnt);
                }
                break;
                case P_TYPE_UINT:
                {
                    *(unsigned int*) adresses->at(cnt) = (unsigned int) sqlite3_column_int(statement_, cnt);
                }
                break;
                case P_TYPE_STRING:
                {
                    *(std::string*) adresses->at(cnt)  = (const char *)sqlite3_column_text(statement_, cnt);
                }
                break;
                case P_TYPE_FLOAT:
                {
                    *(float*) adresses->at(cnt) = (float) sqlite3_column_double (statement_, cnt);
                }
                break;
                case P_TYPE_DOUBLE:
                {
                    *(double*) adresses->at(cnt) = sqlite3_column_double (statement_, cnt);
                }
                break;
                default:
                    logerr  <<  "SQLiteConnection: execute: unknown property type";
                    throw std::runtime_error ("SQLiteConnection: execute: unknown property type");
                    break;
                }
            }
        }
        if (max_results != 0 && row_counter >= max_results)
            break;
        //    if (buffer)
        //    {
        //      buffer->incrementIndex();
        //    }
        result_cnt++;
    }

    if (result != SQLITE_ROW && result != SQLITE_DONE)
    {
        logerr <<  "SQLiteConnection: stepPreparedCommand: problem while stepping the result: " <<  result << " " <<  sqlite3_errmsg(db_handle_);
        throw std::runtime_error ("SQLiteConnection: stepPreparedCommand: problem while stepping the result");
    }

    if (result == SQLITE_DONE)
    {
        logdbg  << "SQLiteConnection: stepPreparedCommand: reading done";
        prepared_command_done_=true;
    }

    return dbresult;
}
void SQLiteConnection::finalizeCommand ()
{
    assert (prepared_command_ != 0);
    sqlite3_finalize(statement_);
    prepared_command_=0; // should be deleted by caller
    prepared_command_done_=true;
}

Buffer *SQLiteConnection::getTableList()  // buffer of table name strings
{
//    MySQLConnectionInfo *info = (MySQLConnectionInfo*) info_;
//    std::string db_name = info->getDB();

    DBCommand command;
    command.setCommandString ("SELECT name FROM sqlite_master WHERE name != 'sqlite_sequence' ORDER BY name DESC;");
    PropertyList list;
    list.addProperty ("name", P_TYPE_STRING);
    command.setPropertyList (list);

    DBResult *result = execute(&command);
    assert (result->containsData());
    Buffer *buffer = result->getBuffer();
    delete result;

    return buffer;
}
Buffer *SQLiteConnection::getColumnList(std::string table) // buffer of column name string, data type
{
    DBCommand command;
    command.setCommandString ("PRAGMA table_info("+table+")");
    PropertyList list;
    list.addProperty ("cid", P_TYPE_INT);
    list.addProperty ("name", P_TYPE_STRING);
    list.addProperty ("type", P_TYPE_STRING);
    list.addProperty ("notnull", P_TYPE_BOOL);
    list.addProperty ("dfltvalue", P_TYPE_INT);
    list.addProperty ("key", P_TYPE_BOOL);
    command.setPropertyList (list);

    DBResult *result = execute(&command);
    assert (result->containsData());
    Buffer *buffer = result->getBuffer();
    delete result;

    return buffer;
}
