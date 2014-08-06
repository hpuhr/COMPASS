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
 * MySQLConConnection.cpp
 *
 *  Created on: Mar 7, 2013
 *      Author: sk
 */

#include "cppconn/exception.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/resultset.h"

#include "DBConnectionInfo.h"
#include "DBResult.h"
#include "DBCommand.h"
#include "DBCommandList.h"
#include "MySQLConConnection.h"
#include "Buffer.h"
#include "Logger.h"

#include "Data.h"
#include "String.h"

using namespace Utils::Data;
using namespace Utils::String;


MySQLConConnection::MySQLConConnection(DBConnectionInfo *info)
: DBConnection (info), driver_(get_driver_instance()), connection_ (0), prepared_command_(0),
  prepared_statement_(0), statement_ (0), resultset_(0), prepared_command_done_ (false)
{

}

MySQLConConnection::~MySQLConConnection()
{
    if (connection_)
    {
        delete connection_;
        connection_=0;
    }
}

void MySQLConConnection::connect()
{
    assert (info_->getType() == DB_TYPE_MYSQLCon);
    assert (!connection_);

    MySQLConnectionInfo *info = dynamic_cast<MySQLConnectionInfo*> (info_);
    assert (info);

    //tcp://127.0.0.1:3306
    std::string def = "tcp://"+info->getServer()+":"+intToString(info->getPort());

    loginf << "MySQLConConnection: init: def '" << def << "' user '" << info->getUser() << "' password '" <<
            info->getPassword() << "'";

    try
    {
        connection_ = driver_->connect(def, info->getUser(), info->getPassword());
    }
    catch (sql::SQLException &e)
    {
        logerr  << "MySQLConConnection: init: DB connection failed: " << e.what();
        throw std::runtime_error ("MySQLConConnection: init: DB connection failed");
    }

    assert (connection_);

    loginf  << "MySQLConConnection: init: successfully connected to server '" << connection_->getClientInfo() << "'";
}

void MySQLConConnection::openDatabase (std::string database_name)
{
    assert (connection_);

    //    if (info_ ->isNew())
    //    {
    //        std::string drop_db = "DROP DATABASE IF EXISTS "+info->getDB()+";";
    //        executeSQL (drop_db); // drop if exists
    //        std::string create_db = "CREATE DATABASE "+info->getDB()+";";
    //        executeSQL (create_db);
    //    }

    std::string use_db = "USE "+database_name+";";
    executeSQL (use_db);

    DBConnection::openDatabase(database_name);

    loginf  << "MySQLConConnection: openDatabase: successfully opened database '" << database_name << "'";
}

void MySQLConConnection::executeSQL(std::string sql)
{
    logdbg  << "MySQLConConnection: executeSQL: sql statement execute: '" <<sql << "'";

    assert (connection_);
    sql::Statement *stmt= connection_->createStatement();
    stmt->execute(sql);
    delete stmt;
}

void MySQLConConnection::prepareBindStatement (std::string statement)
{
    loginf  << "MySQLConConnection: prepareBindStatement: statement prepare '" <<statement << "'";

    assert (connection_);
    assert (!prepared_statement_);
    prepared_statement_ = connection_->prepareStatement(statement);
}

void MySQLConConnection::beginBindTransaction ()
{
    connection_->setAutoCommit(false);
}

void MySQLConConnection::stepAndClearBindings ()
{
    assert (prepared_statement_);
    prepared_statement_->executeUpdate();
}

void MySQLConConnection::endBindTransaction ()
{
}

void MySQLConConnection::finalizeBindStatement ()
{
    delete prepared_statement_;
    prepared_statement_=0;
}

void MySQLConConnection::bindVariable (unsigned int index, int value)
{
    assert (prepared_statement_);
    //loginf << "uga2 vak " << value;
    prepared_statement_->setInt (index, value);
}
void MySQLConConnection::bindVariable (unsigned int index, double value)
{
    assert (prepared_statement_);
    //loginf << "uga val " << value;
    prepared_statement_->setDouble (index, value);
}
void MySQLConConnection::bindVariable (unsigned int index, const char *value)
{
    assert (prepared_statement_);
    prepared_statement_->setString (index, value);
}

void MySQLConConnection::bindVariableNull (unsigned int index)
{
    assert (prepared_statement_);
    prepared_statement_->setNull(index, 0);
}


DBResult *MySQLConConnection::execute (DBCommand *command)
{
    logdbg  << "MySQLConConnection: execute";
    assert (command);
    DBResult *dbresult = new DBResult ();

    std::string sql = command->getCommandString();

    Buffer *buffer=0;
    if (command->getResultList()->getNumProperties() > 0) // data should be returned
    {
        buffer = new Buffer (*(command->getResultList()));
        dbresult->setBuffer(buffer);
    }

    try
    {
        logdbg  << "MySQLConConnection: execute: executing";
        execute (sql, buffer);
    }
    catch (sql::SQLException &e)
    {
        logerr  << "MySQLConConnection: execute: failed with command '" << sql << "' and error '" << e.what() << "'";
        //throw std::runtime_error ("MySQLConConnection: execute: failed with command '"+sql+"' and error '"+e.what()+"'");
    }
    logdbg  << "MySQLConConnection: execute: end";

    return dbresult;
}

DBResult *MySQLConConnection::execute (DBCommandList *command_list)
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
    logdbg  << "MySQLConConnection: execute: end";

    return dbresult;
}

void MySQLConConnection::execute (std::string command, Buffer *buffer)
{
    logdbg  << "MySQLConConnection: execute: command '" << command << "'";

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
    }

    sql::Statement *stmt= connection_->createStatement();
    sql::ResultSet *res = stmt->executeQuery(command);

    unsigned int cnt=0;
    while (res->next())
    {
        if (buffer && cnt != 0)
        {
            buffer->incrementIndex();
        }
        if (buffer && cnt == 0)
            buffer->unsetFirstWrite();

        if (buffer)
            adresses = buffer->getAdresses();

        for (unsigned int cnt2=0; cnt2 < num_properties; cnt2++)
        {
            assert (buffer);
            prop=properties->at(cnt2);

            if(res->isNull(1+cnt2))
                setNan(prop->data_type_int_, adresses->at(cnt2));
            else
            {
                switch (prop->data_type_int_)
                {
                case P_TYPE_BOOL:
                {
                    *(bool*) adresses->at(cnt2) = res->getBoolean(1+cnt2);
                    //loginf  << "sqlex: bool " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_UCHAR:
                {
                    *(unsigned char*) adresses->at(cnt2) = (unsigned char) res->getInt(1+cnt2);
                    //loginf  << "sqlex: uchar " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_CHAR:
                {
                    *(char*) adresses->at(cnt2) = (int) res->getInt(1+cnt2);
                    //loginf  << "sqlex: char " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_INT:
                {
                    *(int*) adresses->at(cnt2) =  res->getInt(1+cnt2);
                    //loginf  << "sqlex: int " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_UINT:
                {
                    *(unsigned int*) adresses->at(cnt2) = (unsigned int) res->getInt(1+cnt2);
                    //loginf  << "sqlex: uint " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_STRING:
                {
                    *(std::string*) adresses->at(cnt2) = res->getString(1+cnt2);
                    //loginf  << "sqlex: string " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_FLOAT:
                {
                    *(float*) adresses->at(cnt2) = (float) res->getDouble(1+cnt2);
                    //loginf  << "sqlex: float " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_DOUBLE:
                {
                    *(double*) adresses->at(cnt2) = res->getDouble(1+cnt2);
                    //loginf  << "sqlex: double " << prop->id_ << " val " << *ptr;
                }
                break;
                default:
                    logerr  <<  "MySQLConConnection: execute: unknown property type";
                    throw std::runtime_error ("MySQLConConnection: execute: unknown property type");
                    break;
                }
            }
        }
        cnt++;
    }
    delete res;
    delete stmt;
    if (buffer)
        logdbg  << "MySQLConConnection: execute done with size " << buffer->getSize();
    else
        logdbg  << "MySQLConConnection: execute done";
}

void MySQLConConnection::prepareStatement (const char *sql)
{
    logdbg  << "MySQLConConnection: prepareStatement: sql '" << sql << "'";
    assert (!statement_);
    assert (!resultset_);

    statement_ = connection_->createStatement();
    resultset_ = statement_->executeQuery(sql);

    logdbg  << "MySQLConConnection: prepareStatement: sql '" << sql << "' done.";
}
void MySQLConConnection::finalizeStatement ()
{
    logdbg  << "MySQLConConnection: finalizeStatement";

    assert (resultset_);
    delete resultset_;
    resultset_=0;

    assert (statement_);
    delete statement_;
    statement_=0;

    logdbg  << "MySQLConConnection: finalizeStatement: done";
}

void MySQLConConnection::prepareCommand (DBCommand *command)
{
    logdbg  << "MySQLConConnection: prepareCommand";
    assert (prepared_command_==0);
    assert (command);

    prepared_command_=command;
    prepared_command_done_=false;

    prepareStatement (command->getCommandString().c_str());
    logdbg  << "MySQLConConnection: prepareCommand: done";
}
DBResult *MySQLConConnection::stepPreparedCommand (unsigned int max_results)
{
    logdbg  << "MySQLConConnection: stepPreparedCommand";

    assert (resultset_);

    DBResult *dbresult = new DBResult ();
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

    unsigned int row_counter=0;
    bool done=true;

    max_results--;

    unsigned int cnt=0;
    while (resultset_->next())
    {
        // ...get each field we want and output it to the screen
        // Note: The first field/column in our result-set is field 1 (one) and -NOT- field 0 (zero)
        // Also, if we know the name of the field then we can also get it directly by name by using:
        // res->getString("TheNameOfTheField");
        //cout << res->getString(1) << endl;

        if (buffer && cnt != 0)
        {
            buffer->incrementIndex();
        }
        if (buffer && cnt == 0)
            buffer->unsetFirstWrite();

        if (buffer)
            adresses = buffer->getAdresses();

        for (unsigned int cnt2=0; cnt2 < num_properties; cnt2++)
        {
            assert (buffer);
            prop=properties->at(cnt2);

            if(resultset_->isNull(1+cnt2))
                setNan(prop->data_type_int_, adresses->at(cnt2));
            else
            {
                switch (prop->data_type_int_)
                {
                case P_TYPE_BOOL:
                {
                    *(bool*) adresses->at(cnt2) = resultset_->getBoolean(1+cnt2);
                    //loginf  << "sqlex: bool " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_UCHAR:
                {
                    *(unsigned char*) adresses->at(cnt2) = (unsigned char) resultset_->getInt(1+cnt2);
                    //loginf  << "sqlex: uchar " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_CHAR:
                {
                    *(char*) adresses->at(cnt2) = (int) resultset_->getInt(1+cnt2);
                    //loginf  << "sqlex: char " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_INT:
                {
                    *(int*) adresses->at(cnt2) =  resultset_->getInt(1+cnt2);
                    //loginf  << "sqlex: int " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_UINT:
                {
                    *(unsigned int*) adresses->at(cnt2) = (unsigned int) resultset_->getInt(1+cnt2);
                    //loginf  << "sqlex: uint " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_STRING:
                {
                    *(std::string*) adresses->at(cnt2) = resultset_->getString(1+cnt2);
                    //loginf  << "sqlex: string " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_FLOAT:
                {
                    *(float*) adresses->at(cnt2) = (float) resultset_->getDouble(1+cnt2);
                    //loginf  << "sqlex: float " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_DOUBLE:
                {
                    *(double*) adresses->at(cnt2) = resultset_->getDouble(1+cnt2);
                    //loginf  << "sqlex: double " << prop->id_ << " val " << *ptr;
                }
                break;
                default:
                    logerr  <<  "MySQLConConnection: stepPreparedCommand: unknown property type";
                    throw std::runtime_error ("MySQLConConnection: stepPreparedCommand: unknown property type");
                    break;
                }
            }
        }
        cnt++;

        if (max_results != 0 && row_counter >= max_results)
        {
            done=false;
            break;
        }

        ++result_cnt;
        ++row_counter;
    }


    if (done)
    {
        logdbg  << "MySQLConConnection: stepPreparedCommand: reading done";
        prepared_command_done_=true;
    }

    logdbg  << "MySQLConConnection: stepPreparedCommand: done";
    return dbresult;
}
void MySQLConConnection::finalizeCommand ()
{
    logdbg  << "MySQLConConnection: finalizeCommand";
    assert (prepared_command_ != 0);

    prepared_command_=0; // should be deleted by caller
    prepared_command_done_=true;
    finalizeStatement();
    logdbg  << "MySQLConConnection: finalizeCommand: done";
}

Buffer *MySQLConConnection::getTableList()  // buffer of table name strings
{
    logdbg << "MySQLConConnection: getTableList";

    DBCommand command;
    //command.setCommandString ("SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA = '"+db_name+"' ORDER BY TABLE_NAME DESC;");
    command.setCommandString ("SHOW TABLES;");
    PropertyList list;
    list.addProperty ("name", P_TYPE_STRING);
    command.setPropertyList (list);

    DBResult *result = execute(&command);
    assert (result->containsData());
    Buffer *buffer = result->getBuffer();
    delete result;

    return buffer;
}

Buffer *MySQLConConnection::getColumnList(std::string table) // buffer of column name string, data type
{
    logdbg << "MySQLConConnection: getColumnList: table " << table;

    DBCommand command;
    command.setCommandString ("SHOW COLUMNS FROM "+table);
    //command.setCommandString ("SELECT COLUMN_NAME, DATA_TYPE, COLUMN_KEY FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '"+database_name+"' AND TABLE_NAME = '"+table+"' ORDER BY COLUMN_NAME DESC;");

    PropertyList list;
    list.addProperty ("name", P_TYPE_STRING);
    list.addProperty ("type", P_TYPE_STRING);
    list.addProperty ("null", P_TYPE_BOOL);
    list.addProperty ("key_string", P_TYPE_STRING);
    list.addProperty ("default", P_TYPE_STRING);
    list.addProperty ("extra", P_TYPE_STRING);
//    list.addProperty ("name", P_TYPE_STRING);
//    list.addProperty ("type", P_TYPE_STRING);
//    list.addProperty ("key_string", P_TYPE_STRING);
    command.setPropertyList (list);

    DBResult *result = execute(&command);
    assert (result->containsData());
    Buffer *buffer = result->getBuffer();
    delete result;

    buffer->addProperty ("key", P_TYPE_BOOL);
    buffer->setIndex(0);

    for (unsigned int cnt=0; cnt < buffer->getSize(); cnt++)
    {
        if (cnt != 0)
            buffer->incrementIndex();

        std::string key_string = *(std::string*) buffer->get(2);

        *(bool*)buffer->get(3) = (key_string.compare("PRI") == 0);
    }

    return buffer;
}
