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
 *  Created on: Jul 19, 2012
 *      Author: sk
 */

#include "boost/date_time/posix_time/posix_time.hpp"
#include "Buffer.h"
#include "DBCommand.h"
#include "DBCommandList.h"
#include "DBResult.h"
#include "DBConnectionInfo.h"
#include "Logger.h"
#include "MySQLppConnection.h"

#include <QMessageBox>

#include "Data.h"
#include "String.h"

using namespace Utils::String;
using namespace Utils::Data;


MySQLppConnection::MySQLppConnection(DBConnectionInfo *info)
: DBConnection (info), connection_(mysqlpp::Connection (false)), prepared_query_(connection_.query()),
  prepared_parameters_(mysqlpp::SQLQueryParms(&prepared_query_)), query_used_(false), transaction_(0)
{
    assert (info_);
    assert (info_ ->getType() == DB_TYPE_MYSQLpp);


    prepared_command_=0;
    prepared_command_done_=false;
}

MySQLppConnection::~MySQLppConnection()
{
    connection_.disconnect();
}

void MySQLppConnection::init()
{
    assert (info_->getType() == DB_TYPE_MYSQLpp);

    MySQLppConnectionInfo *info = (MySQLppConnectionInfo*) info_;

    if (!connection_.connect("", info->getServer().c_str(), info->getUser().c_str(), info->getPassword().c_str(), info->getPort()))
    {
        logerr  << "MySQLppConnection: init: DB connection failed: " << connection_.error();
        throw std::runtime_error ("MySQLppConnection: init: DB connection failed");
    }

//    if (info_ ->isNew())
//    {
//        std::string drop_db = "DROP DATABASE IF EXISTS "+info->getDB()+";";
//        executeSQL (drop_db); // drop if exists
//        connection_.create_db(info->getDB().c_str()); // so, no database? create it first then.
//    }
    connection_.select_db(info->getDB().c_str());

    assert (connection_.connected ());
    loginf  << "MySQLppConnection: init: sucessfully connected to server '" << connection_.server_version ();

    //  loginf  << "MySQLppConnection: init: performance test";
    //  performanceTest ();
}

void MySQLppConnection::executeSQL(std::string sql)
{
    logdbg  << "MySQLppConnection: executeSQL: sql statement execute: '" <<sql << "'";

    mysqlpp::Query query = connection_.query(sql);
    if(!query.exec()) // execute it!
    {
        logerr  << "MySQLppConnection: executeSQL: error when executing '" << sql<<"' message '" << query.error() << "'";
        throw std::runtime_error("MySQLppConnection: executeSQL: error when executing");
    }
}

void MySQLppConnection::prepareBindStatement (std::string statement)
{
    logdbg  << "MySQLppConnection: prepareBindStatement: statement prepare '" <<statement << "'";

    assert (!query_used_);
    prepared_query_.reset();
    prepared_query_ << statement;
    prepared_query_.parse();
    query_used_=true;
}

void MySQLppConnection::beginBindTransaction ()
{
    assert (transaction_==0);
    transaction_ = new mysqlpp::Transaction (connection_);
}

void MySQLppConnection::stepAndClearBindings ()
{
    logdbg  << "DBInterface: stepAndClearBindings: stepping statement '" << prepared_query_.str(prepared_parameters_)<< "'";

    if (!prepared_query_.execute(prepared_parameters_))
    {
        logerr  << "MySQLppConnection: stepAndClearBindings: error when executing '" << prepared_query_.error() << "'";
        throw std::runtime_error("MySQLppConnection: stepAndClearBindings: error when executing");
    }
    prepared_parameters_.clear();
}

void MySQLppConnection::endBindTransaction ()
{
    transaction_->commit();
    delete transaction_;
    transaction_=0;
}

void MySQLppConnection::finalizeBindStatement ()
{
    query_used_=false;
}

void MySQLppConnection::bindVariable (unsigned int index, int value)
{
    logdbg  << "MySQLppConnection: bindVariable: index " << index << " value '" << value << "'";
    prepared_parameters_[index] = value;
}
void MySQLppConnection::bindVariable (unsigned int index, double value)
{
    logdbg  << "MySQLppConnection: bindVariable: index " << index << " value '" << value << "'";
    prepared_parameters_[index] = value;
}
void MySQLppConnection::bindVariable (unsigned int index, const char *value)
{
    logdbg  << "MySQLppConnection: bindVariable: index " << index << " value '" << value << "'";
    prepared_parameters_[index] = value;
}

void MySQLppConnection::bindVariableNull (unsigned int index)
{
    logdbg  << "MySQLppConnection: bindVariableNull: index " << index ;
    prepared_parameters_[index] = mysqlpp::null;
}


DBResult *MySQLppConnection::execute (DBCommand *command)
{
    logdbg  << "MySQLppConnection: execute";
    assert (command);
    DBResult *dbresult = new DBResult ();

    std::string sql = command->getCommandString();

    Buffer *buffer=0;
    if (command->getResultList()->getNumProperties() > 0) // data should be returned
    {
        buffer = new Buffer (*(command->getResultList()));
        dbresult->setBuffer(buffer);
    }

    logdbg  << "MySQLppConnection: execute: executing";
    execute (sql, buffer);

    logdbg  << "MySQLppConnection: execute: end";

    return dbresult;
}

DBResult *MySQLppConnection::execute (DBCommandList *command_list)
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
    logdbg  << "MySQLppConnection: execute: end";

    return dbresult;
}

void MySQLppConnection::execute (std::string command, Buffer *buffer)
{
    logdbg  << "MySQLppConnection: execute: command '" << command << "'";

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

    logdbg  << "MySQLppConnection: execute: creating query";
    mysqlpp::Query query = connection_.query(command);
    logdbg  << "MySQLppConnection: execute: creating storequeryresult";
    mysqlpp::StoreQueryResult res = query.store();

    logdbg  << "MySQLppConnection: execute: iterating result";
    // Display results
    unsigned int cnt=0;
    mysqlpp::StoreQueryResult::const_iterator it;
    for (it = res.begin(); it != res.end(); ++it)
    {
        mysqlpp::Row row = *it;

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

            if(row[cnt2] == mysqlpp::null)
                setNan(prop->data_type_int_, adresses->at(cnt2));
            else
            {
                switch (prop->data_type_int_)
                {
                case P_TYPE_BOOL:
                {
                    *(bool*) adresses->at(cnt2) = (bool) row[cnt2];
                    //loginf  << "sqlex: bool " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_UCHAR:
                {
                    *(unsigned char*) adresses->at(cnt2) = (unsigned int) row[cnt2];
                    //loginf  << "sqlex: uchar " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_CHAR:
                {
                    *(char*) adresses->at(cnt2) = (int) row[cnt2];
                    //loginf  << "sqlex: char " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_INT:
                {
                    *(int*) adresses->at(cnt2) = (int) row[cnt2];
                    //loginf  << "sqlex: int " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_UINT:
                {
                    *(unsigned int*) adresses->at(cnt2) = (unsigned int) row[cnt2];
                    //loginf  << "sqlex: uint " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_STRING:
                {
                    *(std::string*) adresses->at(cnt2) = (const char *) row[cnt2];
                    //loginf  << "sqlex: string " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_FLOAT:
                {
                    *(float*) adresses->at(cnt2) = (float) row[cnt2];
                    //loginf  << "sqlex: float " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_DOUBLE:
                {
                    *(double*) adresses->at(cnt2) = (double) row[cnt2];
                    //loginf  << "sqlex: double " << prop->id_ << " val " << *ptr;
                }
                break;
                default:
                    logerr  <<  "MySQLppConnection: execute: unknown property type";
                    throw std::runtime_error ("MySQLppConnection: execute: unknown property type");
                    break;
                }
            }
        }
        cnt++;
    }

    if (buffer)
        logdbg  << "MySQLppConnection: execute done with size " << buffer->getSize();
    else
        logdbg  << "MySQLppConnection: execute done";
}

void MySQLppConnection::prepareStatement (const char *sql)
{
    logdbg  << "MySQLppConnection: prepareStatement: sql '" << sql << "'";

    assert (!query_used_);

    result_step_ = mysqlpp::UseQueryResult ();

    prepared_query_.clear();
    prepared_query_ << sql;

    if (!(result_step_ = prepared_query_.use()))
    {
        throw std::runtime_error ("MySQLppConnection: prepareStatement: query error '"+std::string(prepared_query_.error())+"'");
    }

    query_used_=true;
    logdbg  << "MySQLppConnection: prepareStatement: sql '" << sql << "' done.";
}
void MySQLppConnection::finalizeStatement ()
{
    logdbg  << "MySQLppConnection: finalizeStatement";
    prepared_query_.reset();
    query_used_=false;
    logdbg  << "MySQLppConnection: finalizeStatement: done";
}

void MySQLppConnection::prepareCommand (DBCommand *command)
{
    logdbg  << "MySQLppConnection: prepareCommand";
    assert (prepared_command_==0);
    assert (command);

    prepared_command_=command;
    prepared_command_done_=false;

    prepareStatement (command->getCommandString().c_str());
    logdbg  << "MySQLppConnection: prepareCommand: done";
}
DBResult *MySQLppConnection::stepPreparedCommand (unsigned int max_results)
{
    logdbg  << "MySQLppConnection: stepPreparedCommand";

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
    bool done=true;

    max_results--;

    //  try
    //  {
    row_counter=0;
    while (mysqlpp::Row row = result_step_.fetch_row())
    {
        if (buffer && row_counter != 0)
        {
            buffer->incrementIndex();
        }

        for (unsigned int cnt2=0; cnt2 < num_properties; cnt2++)
        {
            assert (buffer);
            prop=properties->at(cnt2);

            if (buffer)
                adresses = buffer->getAdresses();

            if(row[cnt2] == mysqlpp::null)
                setNan(prop->data_type_int_, adresses->at(cnt2));
            else
            {
                switch (prop->data_type_int_)
                {
                case P_TYPE_BOOL:
                {
                    *(bool*) adresses->at(cnt2) = (bool) row[cnt2];
                    //loginf  << "sqlex: bool " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_UCHAR:
                {
                    *(unsigned char*) adresses->at(cnt2) = (unsigned int) row[cnt2];
                    //loginf  << "sqlex: uchar " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_CHAR:
                {
                    *(char*) adresses->at(cnt2) = (int) row[cnt2];
                    //loginf  << "sqlex: char " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_INT:
                {
                    *(int*) adresses->at(cnt2) = (int) row[cnt2];
                    //loginf  << "sqlex: int " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_UINT:
                {
                    *(unsigned int*) adresses->at(cnt2) = (unsigned int) row[cnt2];
                    //loginf  << "sqlex: uint " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_STRING:
                {
                    *(std::string*) adresses->at(cnt2) = (const char *) row[cnt2];
                    //loginf  << "sqlex: string " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_FLOAT:
                {
                    *(float*) adresses->at(cnt2) = (float) row[cnt2];
                    //loginf  << "sqlex: float " << prop->id_ << " val " << *ptr;
                }
                break;
                case P_TYPE_DOUBLE:
                {
                    *(double*) adresses->at(cnt2) = (double) row[cnt2];
                    //loginf  << "sqlex: double " << prop->id_ << " val " << *ptr;
                }
                break;
                default:
                    logerr  <<  "MySQLppConnection: execute: unknown property type";
                    throw std::runtime_error ("MySQLppConnection: execute: unknown property type");
                    break;
                }
            }
        }

        if (max_results != 0 && row_counter >= max_results)
        {
            done=false;
            break;
        }

        ++result_cnt;
        ++row_counter;
    }
    //  }
    //  catch (...)
    //  {
    //    logerr  << "MySQLppConnection: stepPreparedCommand: exception caught";
    //    done=true;
    //    return 0;
    //  }
    //
    if (done)
    {
        logdbg  << "MySQLppConnection: stepPreparedCommand: reading done";
        prepared_command_done_=true;
    }

    logdbg  << "MySQLppConnection: stepPreparedCommand: done";
    return dbresult;
}
void MySQLppConnection::finalizeCommand ()
{
    logdbg  << "MySQLppConnection: finalizeCommand";
    assert (prepared_command_ != 0);

    prepared_command_=0; // should be deleted by caller
    prepared_command_done_=true;
    finalizeStatement();
    logdbg  << "MySQLppConnection: finalizeCommand: done";
}

Buffer *MySQLppConnection::getTableList()  // buffer of table name strings
{
    MySQLppConnectionInfo *info = (MySQLppConnectionInfo*) info_;
    std::string db_name = info->getDB();

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

Buffer *MySQLppConnection::getColumnList(std::string table) // buffer of column name string, data type
{
    MySQLppConnectionInfo *info = (MySQLppConnectionInfo*) info_;
    std::string db_name = info->getDB();

    DBCommand command;
    command.setCommandString ("SELECT COLUMN_NAME, DATA_TYPE, COLUMN_KEY FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '"+db_name+"' AND TABLE_NAME = '"+table+"' ORDER BY COLUMN_NAME DESC;");
    PropertyList list;
    list.addProperty ("name", P_TYPE_STRING);
    list.addProperty ("type", P_TYPE_STRING);
    list.addProperty ("key_string", P_TYPE_STRING);
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

void MySQLppConnection::performanceTest ()
{
    //SELECT sd_radar.POS_SYS_X_NM, sd_radar.POS_SYS_Y_NM, sd_radar.MODEC_CODE_FT, sd_radar.TOD, sd_radar.DETECTION_TYPE, sd_radar.DS_ID, sd_radar.REC_NUM FROM sd_radar ORDER BY REC_NUM;'
    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    unsigned int num_reads=20;
    unsigned int chunk_size=100000;

    last_key_=0;

    loginf  << "MySQLppConnection: performanceTest: start";

    start_time = boost::posix_time::microsec_clock::local_time();

    DBCommand command;
    command.setCommandString("SELECT sd_radar.REC_NUM, sd_radar.POS_SYS_X_NM, sd_radar.POS_SYS_Y_NM, sd_radar.MODEC_CODE_FT, sd_radar.TOD, sd_radar.DETECTION_TYPE, sd_radar.DS_ID FROM sd_radar;");
    //ORDER BY REC_NUM

    PropertyList list;
    list.addProperty ("id", P_TYPE_INT);
    list.addProperty ("posx", P_TYPE_DOUBLE);
    list.addProperty ("posy", P_TYPE_DOUBLE);
    list.addProperty ("modec", P_TYPE_INT);
    list.addProperty ("time", P_TYPE_DOUBLE);
    list.addProperty ("det", P_TYPE_INT);
    list.addProperty ("ds", P_TYPE_INT);
    command.setPropertyList (list);

    //  prepareCommand (&command);

    //  for (unsigned int cnt=0; cnt < num_reads; cnt++)
    //  {
    //      DBResult *result = stepPreparedCommand(chunk_size);
    //      Buffer *buffer = result->getBuffer();
    //      delete result;
    //      delete buffer;
    //  }
    //
    //  finalizeCommand ();

    for (unsigned int cnt=0; cnt < num_reads; cnt++)
    {
        DBResult *result = readBulkCommand(&command,
                "SELECT sd_radar.REC_NUM, sd_radar.POS_SYS_X_NM, sd_radar.POS_SYS_Y_NM, sd_radar.MODEC_CODE_FT, sd_radar.TOD, sd_radar.DETECTION_TYPE, sd_radar.DS_ID FROM sd_radar",
                "",  chunk_size);
        Buffer *buffer = result->getBuffer();
        delete result;
        delete buffer;
    }

    stop_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time - start_time;
    double load_time = diff.total_milliseconds()/1000.0;

    loginf  << "MySQLppConnection: performanceTest: end after load time " << load_time << "s";
}

DBResult *MySQLppConnection::readBulkCommand (DBCommand *command, std::string main_statement, std::string order_statement, unsigned int max_results)
{
    assert (command);
    DBResult *dbresult = new DBResult ();

    std::string sql = main_statement+" WHERE REC_NUM > "+intToString(last_key_)+" "+ order_statement+" LIMIT 0,"+intToString(max_results)+";";

    loginf  << "MySQLppConnection: readBulkCommand: sql '" << sql << "'";

    Buffer *buffer=0;
    if (command->getResultList()->getNumProperties() > 0) // data should be returned
    {
        buffer = new Buffer (*(command->getResultList()));
        dbresult->setBuffer(buffer);
    }

    logdbg  << "MySQLppConnection: execute: executing";
    execute (sql, buffer);

    unsigned int size = buffer->getSize();
    last_key_ = *((unsigned int*)buffer->get(size-1, 0));

    logdbg  << "MySQLppConnection: execute: end last_key " << last_key_;

    return dbresult;
}
