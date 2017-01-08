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

using namespace Utils;
using namespace Utils::Data;


MySQLppConnection::MySQLppConnection(const DBConnectionInfo &info)
    : DBConnection (info), connection_(mysqlpp::Connection (false)), prepared_query_(connection_.query()),
      prepared_parameters_(mysqlpp::SQLQueryParms(&prepared_query_)), query_used_(false), transaction_(0)
{
    assert (info_.getType() == DB_TYPE_MYSQLpp);


    prepared_command_=0;
    prepared_command_done_=false;
}

MySQLppConnection::~MySQLppConnection()
{
    connection_.disconnect();
}

void MySQLppConnection::connect()
{
    assert (info.getType() == DB_TYPE_MYSQLpp);

    const MySQLConnectionInfo &info = dynamic_cast<const MySQLConnectionInfo &> (info_);
    assert (info);

    if (!connection_.connect("", info.server().c_str(), info.user().c_str(), info.password().c_str(), info.port()))
    {
        logerr  << "MySQLppConnection: init: DB connection failed: " << connection_.error();
        throw std::runtime_error ("MySQLppConnection: init: DB connection failed");
    }

    assert (connection_.connected ());

    loginf  << "MySQLppConnection: init: sucessfully connected to server '" << connection_.server_version ();

}

void MySQLppConnection::openDatabase (const std::string &database_name)
{
    //    if (info_ ->isNew())
    //    {
    //        std::string drop_db = "DROP DATABASE IF EXISTS "+info->getDB()+";";
    //        executeSQL (drop_db); // drop if exists
    //        connection_.create_db(info->getDB().c_str()); // so, no database? create it first then.
    //    }
    connection_.select_db(database_name);
    loginf  << "MySQLppConnection: openDatabase: successfully opened database '" << database_name << "'";

    //loginf  << "MySQLppConnection: init: performance test";
    //performanceTest ();
}

void MySQLppConnection::executeSQL(const std::string &sql)
{
    logdbg  << "MySQLppConnection: executeSQL: sql statement execute: '" <<sql << "'";

    mysqlpp::Query query = connection_.query(sql);
    if(!query.exec()) // execute it!
    {
        logerr  << "MySQLppConnection: executeSQL: error when executing '" << sql<<"' message '" << query.error() << "'";
        throw std::runtime_error("MySQLppConnection: executeSQL: error when executing");
    }
}

void MySQLppConnection::prepareBindStatement (const std::string &statement)
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


std::shared_ptr <DBResult> MySQLppConnection::execute (const DBCommand &command)
{
    logdbg  << "MySQLppConnection: execute";

    std::shared_ptr <DBResult> dbresult = std::make_shared <DBResult> ();

    std::string sql = command.get();

    Buffer *buffer=0;
    if (command.resultList().size() > 0) // data should be returned
    {
        buffer = new Buffer (command.resultList());
        dbresult->setBuffer(buffer);
    }

    logdbg  << "MySQLppConnection: execute: executing";
    execute (sql, *buffer);

    logdbg  << "MySQLppConnection: execute: end";

    return dbresult;
}

std::shared_ptr <DBResult>MySQLppConnection::execute (const DBCommandList &command_list)
{
    std::shared_ptr <DBResult> dbresult = std::make_shared <DBResult> ();

    unsigned int num_commands = command_list.getNumCommands();

    Buffer *buffer=0;

    if (command_list.getResultList().size() > 0) // data should be returned
    {
        buffer = new Buffer (command_list.getResultList());
        dbresult->setBuffer(buffer);
    }

    //    assert (false);
    //    // TODO FIXME

    for (unsigned int cnt=0; cnt < num_commands; cnt++)
    {
        //        if (cnt != 0)
        //            buffer->incrementIndex();

        std::string sql = command_list.getCommandString(cnt);

        if (buffer)
            execute (sql, *buffer);
        else
            execute (sql);
    }
    logdbg  << "MySQLppConnection: execute: end";

    return dbresult;
}

void MySQLppConnection::execute (const std::string &command, Buffer &buffer)
{
    logdbg  << "MySQLppConnection: execute: command '" << command << "'";

    assert (buffer);
    unsigned int num_properties=0;

    const PropertyList &list = buffer.properties();
    num_properties = list.size();

    logdbg  << "MySQLppConnection: execute: creating query";
    mysqlpp::Query query = connection_.query(command);
    logdbg  << "MySQLppConnection: execute: creating storequeryresult";
    mysqlpp::StoreQueryResult res = query.store();

    logdbg  << "MySQLppConnection: execute: iterating result";
    // Display results
    unsigned int cnt=buffer.size();
    mysqlpp::StoreQueryResult::const_iterator it;
    for (it = res.begin(); it != res.end(); ++it)
    {
        mysqlpp::Row row = *it;
        readRowIntoBuffer (row, list, num_properties, buffer, cnt);
        cnt++;
    }

    logdbg  << "MySQLppConnection: execute done with size " << buffer.size();
}

void MySQLppConnection::readRowIntoBuffer (mysqlpp::Row &row, const PropertyList &list, unsigned int num_properties, Buffer &buffer, unsigned int index)
{
    for (unsigned int cnt=0; cnt < num_properties; cnt++)
    {
        if (row[cnt] != mysqlpp::null)
        {
            const Property &prop=list.at(cnt);

            switch (prop.getDataType())
            {
            case PropertyDataType::BOOL:
                buffer.getBool(prop.getId()).set(index, static_cast<bool> (row[cnt]));
                //loginf  << "sqlex: bool " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::UCHAR:
                buffer.getUChar(prop.getId()).set(index, static_cast<unsigned char> (row[cnt]));
                //loginf  << "sqlex: uchar " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::CHAR:
                buffer.getChar(prop.getId()).set(index, static_cast<signed char> (row[cnt]));
                //loginf  << "sqlex: char " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::INT:
                buffer.getInt(prop.getId()).set(index, static_cast<int> (row[cnt]));
                //loginf  << "sqlex: int " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::UINT:
                buffer.getUInt(prop.getId()).set(index, static_cast<unsigned int> (row[cnt]));
                //loginf  << "sqlex: uint " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::STRING:
                buffer.getString(prop.getId()).set(index, static_cast<const char *> (row[cnt]));
                //loginf  << "sqlex: string " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::FLOAT:
                buffer.getFloat(prop.getId()).set(index, static_cast<float> (row[cnt]));
                //loginf  << "sqlex: float " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::DOUBLE:
                buffer.getDouble(prop.getId()).set(index, static_cast<double> (row[cnt]));
                //loginf  << "sqlex: double " << prop->id_ << " val " << *ptr;
                break;
            default:
                logerr  <<  "MySQLppConnection: readRowIntoBuffer: unknown property type";
                throw std::runtime_error ("MySQLppConnection: readRowIntoBuffer: unknown property type");
                break;
            }
        }
    }
}

void MySQLppConnection::execute (const std::string &command)
{
    logdbg  << "MySQLppConnection: execute: command '" << command << "'";

    logdbg  << "MySQLppConnection: execute: creating query";
    mysqlpp::Query query = connection_.query(command);
    logdbg  << "MySQLppConnection: execute: creating storequeryresult";
    mysqlpp::StoreQueryResult res = query.store();

    assert (res.begin() == res.end());
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

void MySQLppConnection::prepareCommand (const DBCommand &command)
{
    logdbg  << "MySQLppConnection: prepareCommand";
    assert (prepared_command_==0);

    prepared_command_=&command;
    prepared_command_done_=false;

    prepareStatement (command.get().c_str());
    logdbg  << "MySQLppConnection: prepareCommand: done";
}

std::shared_ptr <DBResult> MySQLppConnection::stepPreparedCommand (unsigned int max_results)
{
    logdbg  << "MySQLppConnection: stepPreparedCommand";

    assert (prepared_command_);

    std::shared_ptr <DBResult> dbresult = std::make_shared <DBResult> ();

    std::string sql = prepared_command_->get();

    assert (prepared_command_->resultList()->size() > 0); // data should be returned
    Buffer *buffer = new Buffer (prepared_command_->resultList());
    assert (buffer->properties().size() > 0);
    dbresult->setBuffer(buffer);


    unsigned int num_properties=buffer->properties().size();
    const PropertyList &list = buffer->properties();
    unsigned int cnt = 0;

    bool done=true;

    max_results--;

    while (mysqlpp::Row row = result_step_.fetch_row())
    {
        readRowIntoBuffer (row, list, num_properties, *buffer, cnt);

        if (max_results != 0 && cnt >= max_results)
        {
            done=false;
            break;
        }

        ++cnt;
    }

    if (done)
    {
        logdbg  << "MySQLppConnection: stepPreparedCommand: reading done";
        prepared_command_done_=true;
        buffer->lastOne(true);
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

std::shared_ptr <Buffer> MySQLppConnection::getTableList()  // buffer of table name strings
{
    DBCommand command;
    //command.setCommandString ("SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA = '"+db_name+"' ORDER BY TABLE_NAME DESC;");
    command.set ("SHOW TABLES;");
    PropertyList list;
    list.addProperty ("name", PropertyDataType::STRING);
    command.list (list);

    std::shared_ptr <DBResult> result = execute(command);
    assert (result->containsData());
    std::shared_ptr <Buffer> buffer = std::make_shared<Buffer> (*result->getBuffer());

    return buffer;
}

std::shared_ptr <Buffer>MySQLppConnection::getColumnList(const std::string &table) // buffer of column name string, data type
{
    DBCommand command;
    //    command.setCommandString ("SELECT COLUMN_NAME, DATA_TYPE, COLUMN_KEY FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '"
    //            +database_name+"' AND TABLE_NAME = '"+table+"' ORDER BY COLUMN_NAME DESC;");
    command.set ("SHOW COLUMNS FROM "+table);

    //SELECT COLUMN_NAME, DATA_TYPE, COLUMN_KEY FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = 'job_awam_0019' AND TABLE_NAME = 'sd_track' ORDER BY COLUMN_NAME DESC;

    PropertyList list;
    list.addProperty ("name", PropertyDataType::STRING);
    list.addProperty ("type", PropertyDataType::STRING);
    list.addProperty ("null", PropertyDataType::BOOL);
    list.addProperty ("key_string", PropertyDataType::STRING);
    list.addProperty ("default", PropertyDataType::STRING);
    list.addProperty ("extra", PropertyDataType::STRING);
    command.list (list);

    std::shared_ptr <DBResult> result = execute(command);
    assert (result->containsData());
    std::shared_ptr <Buffer> buffer = std::make_shared<Buffer> (*result->getBuffer());

    buffer->addProperty ("key", PropertyDataType::BOOL);
    //buffer->setIndex(0);

    for (unsigned int cnt=0; cnt < buffer->size(); cnt++)
    {
        std::string key_string = buffer->getString("key_string").get(cnt);
        buffer->getBool("key").set(cnt, key_string.compare("PRI") == 0);
    }

    return buffer;
}

void MySQLppConnection::performanceTest ()
{
    //SELECT sd_radar.POS_SYS_X_NM, sd_radar.POS_SYS_Y_NM, sd_radar.MODEC_CODE_FT, sd_radar.TOD, sd_radar.DETECTION_TYPE, sd_radar.DS_ID, sd_radar.REC_NUM FROM sd_radar ORDER BY REC_NUM;'
    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    unsigned int chunk_size=100000;

    loginf  << "MySQLppConnection: performanceTest: start";

    start_time = boost::posix_time::microsec_clock::local_time();

    DBCommand command;
    command.set("SELECT sd_radar.REC_NUM, sd_radar.POS_SYS_X_NM, sd_radar.POS_SYS_Y_NM, sd_radar.MODEC_CODE_FT, sd_radar.TOD, sd_radar.DETECTION_TYPE, sd_radar.DS_ID FROM sd_radar ORDER BY REC_NUM;");
    //ORDER BY REC_NUM

    PropertyList list;
    list.addProperty ("REC_NUM", PropertyDataType::INT);
    list.addProperty ("POS_LAT_DEG", PropertyDataType::DOUBLE);
    list.addProperty ("POS_LONG_DEG", PropertyDataType::DOUBLE);
    list.addProperty ("MODEC_CODE_FT", PropertyDataType::DOUBLE);
    list.addProperty ("TOD", PropertyDataType::DOUBLE);
    list.addProperty ("POS_SYS_X_NM", PropertyDataType::DOUBLE);
    list.addProperty ("POS_SYS_Y_NM", PropertyDataType::DOUBLE);
    //list.addProperty ("DETECTION_TYPE", PropertyDataType::INT);
    //list.addProperty ("DS_ID", PropertyDataType::INT);
    command.list (list);

    prepareCommand (command);

    unsigned int rows=0;
    bool quit = false;
    while (!quit)
    {
        std::shared_ptr <DBResult> result = stepPreparedCommand(chunk_size);
        assert (result->containsData());
        Buffer *buffer = result->getBuffer();
        assert (buffer->size() != 0);
        rows += buffer->size();
        if (buffer->lastOne())
            quit=true;

        delete buffer;
        loginf << "MySQLppConnection: performanceTest: got " << rows << " rows";
    }

    finalizeCommand ();

//    for (unsigned int cnt=0; cnt < num_reads; cnt++)
//    {
//        DBResult *result = readBulkCommand(&command,
//                                           "SELECT sd_radar.REC_NUM, sd_radar.POS_SYS_X_NM, sd_radar.POS_SYS_Y_NM, sd_radar.MODEC_CODE_FT, sd_radar.TOD, sd_radar.DETECTION_TYPE, sd_radar.DS_ID FROM sd_radar",
//                                           "",  chunk_size);
//        Buffer *buffer = result->getBuffer();
//        delete result;
//        delete buffer;
//    }

    stop_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time - start_time;
    double load_time = diff.total_milliseconds()/1000.0;

    loginf  << "MySQLppConnection: performanceTest: end after load time " << load_time << "s rows " << rows << " " << rows/load_time << " r/s";
}

//DBResult *MySQLppConnection::readBulkCommand (DBCommand *command, std::string main_statement, std::string order_statement, unsigned int max_results)
//{
//    assert (command);
//    DBResult *dbresult = new DBResult ();

//    std::string sql = main_statement+" WHERE REC_NUM > "+String::intToString(last_key_)+" "+ order_statement+" LIMIT 0,"+intToString(max_results)+";";

//    loginf  << "MySQLppConnection: readBulkCommand: sql '" << sql << "'";

//    Buffer *buffer=0;
//    if (command->getResultList()->getNumProperties() > 0) // data should be returned
//    {
//        buffer = new Buffer (*(command->getResultList()));
//        dbresult->setBuffer(buffer);
//    }

//    logdbg  << "MySQLppConnection: execute: executing";
//    execute (sql, buffer);

//    unsigned int size = buffer->getSize();
//    last_key_ = *((unsigned int*)buffer->get(size-1, 0));

//    logdbg  << "MySQLppConnection: execute: end last_key " << last_key_;

//    return dbresult;
//}
