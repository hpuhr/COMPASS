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

#include "buffer.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "dbinterface.h"
#include "dbresult.h"
#include "logger.h"
#include "propertylist.h"
#include "mysqlppconnectionwidget.h"
#include "mysqlppconnection.h"
#include "dbtableinfo.h"
#include "data.h"
#include "stringconv.h"
#include "mysqlserver.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include <QMessageBox>

using namespace Utils;
using namespace Utils::Data;


MySQLppConnection::MySQLppConnection(const std::string &instance_id, DBInterface *interface)
    : DBConnection ("MySQLppConnection", instance_id, interface), interface_(*interface), connected_server_(nullptr), connection_(mysqlpp::Connection (false)),
      prepared_query_(connection_.query()),
      prepared_parameters_(mysqlpp::SQLQueryParms(&prepared_query_)), query_used_(false), transaction_(nullptr), prepared_command_(nullptr),
      prepared_command_done_(false), widget_(nullptr)
{
    registerParameter("used_server", &used_server_, "");

    createSubConfigurables ();
}

MySQLppConnection::~MySQLppConnection()
{
}

void MySQLppConnection::setServer (const std::string &server)
{
    logdbg << "MySQLppConnection: setServer: '" << server << "'";
    used_server_ = server;
    assert (servers_.count(used_server_) == 1);
}

void MySQLppConnection::connectServer ()
{
    logdbg << "MySQLppConnection: connectServer";

    assert (servers_.count(used_server_) == 1);
    connected_server_ = servers_.at(used_server_);
    connection_.connect("", connected_server_->host().c_str(), connected_server_->user().c_str(), connected_server_->password().c_str(), connected_server_->port());
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

    connection_ready_ = true;

    emit connectedSignal();
    interface_.databaseOpened();

    //loginf  << "MySQLppConnection: init: performance test";
    //performanceTest ();
}

void MySQLppConnection::disconnect()
{
    connection_.disconnect();

    for (auto it : servers_)
        delete it.second;
    servers_.clear();

    connected_server_ = nullptr;

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    connected_server_ = nullptr;
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

    std::shared_ptr <DBResult> dbresult (new DBResult ());
    std::string sql = command.get();

    if (command.resultList().size() > 0) // data should be returned
    {
        std::shared_ptr <Buffer> buffer (new Buffer (command.resultList()));
        dbresult->buffer(buffer);
        logdbg  << "MySQLppConnection: execute: executing";
        execute (sql, buffer);
    }
    else
    {
        logdbg  << "MySQLppConnection: execute: executing";
        execute (sql);
    }

    logdbg  << "MySQLppConnection: execute: end";

    return dbresult;
}

std::shared_ptr <DBResult> MySQLppConnection::execute (const DBCommandList &command_list)
{
    std::shared_ptr <DBResult> dbresult (new DBResult ());

    unsigned int num_commands = command_list.getNumCommands();

    if (command_list.getResultList().size() > 0) // data should be returned
    {
        std::shared_ptr <Buffer> buffer (new Buffer (command_list.getResultList()));
        dbresult->buffer(buffer);

        for (unsigned int cnt=0; cnt < num_commands; cnt++)
            execute (command_list.getCommandString(cnt), buffer);
    }
    else
    {
        for (unsigned int cnt=0; cnt < num_commands; cnt++)
            execute (command_list.getCommandString(cnt));

    }

    logdbg  << "MySQLppConnection: execute: end";

    return dbresult;
}

void MySQLppConnection::execute (const std::string &command, std::shared_ptr <Buffer> buffer)
{
    logdbg  << "MySQLppConnection: execute: command '" << command << "'";

    assert (buffer);
    unsigned int num_properties=0;

    const PropertyList &list = buffer->properties();
    num_properties = list.size();

    logdbg  << "MySQLppConnection: execute: creating query";
    mysqlpp::Query query = connection_.query(command);
    logdbg  << "MySQLppConnection: execute: creating storequeryresult";
    mysqlpp::StoreQueryResult res = query.store();

    logdbg  << "MySQLppConnection: execute: iterating result";
    // Display results
    unsigned int cnt=buffer->size();
    mysqlpp::StoreQueryResult::const_iterator it;
    for (it = res.begin(); it != res.end(); ++it)
    {
        mysqlpp::Row row = *it;
        readRowIntoBuffer (row, list, num_properties, buffer, cnt);
        cnt++;
    }

    logdbg  << "MySQLppConnection: execute done with size " << buffer->size();
}

void MySQLppConnection::readRowIntoBuffer (mysqlpp::Row &row, const PropertyList &list, unsigned int num_properties, std::shared_ptr <Buffer> buffer, unsigned int index)
{
    //logdbg << "MySQLppConnection::readRowIntoBuffer: start buffer size " << buffer->size() << " index " << index;
    for (unsigned int cnt=0; cnt < num_properties; cnt++)
    {
        if (row[cnt] != mysqlpp::null)
        {
            const Property &prop=list.at(cnt);

            switch (prop.dataType())
            {
            case PropertyDataType::BOOL:
                buffer->getBool(prop.name()).set(index, static_cast<bool> (row[cnt]));
                //loginf  << "sqlex: bool " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::UCHAR:
                buffer->getUChar(prop.name()).set(index, static_cast<unsigned char> (row[cnt]));
                //loginf  << "sqlex: uchar " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::CHAR:
                buffer->getChar(prop.name()).set(index, static_cast<signed char> (row[cnt]));
                //loginf  << "sqlex: char " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::INT:
                buffer->getInt(prop.name()).set(index, static_cast<int> (row[cnt]));
                //loginf  << "sqlex: int " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::UINT:
                buffer->getUInt(prop.name()).set(index, static_cast<unsigned int> (row[cnt]));
                //loginf  << "sqlex: uint " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::STRING:
                buffer->getString(prop.name()).set(index, static_cast<const char *> (row[cnt]));
                //loginf  << "sqlex: string " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::FLOAT:
                buffer->getFloat(prop.name()).set(index, static_cast<float> (row[cnt]));
                //loginf  << "sqlex: float " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::DOUBLE:
                buffer->getDouble(prop.name()).set(index, static_cast<double> (row[cnt]));
                //loginf  << "sqlex: double " << prop->id_ << " val " << *ptr;
                break;
            default:
                logerr  <<  "MySQLppConnection: readRowIntoBuffer: unknown property type";
                throw std::runtime_error ("MySQLppConnection: readRowIntoBuffer: unknown property type");
                break;
            }
        }
    }
    //logdbg << "MySQLppConnection::readRowIntoBuffer: end buffer size " << buffer->size() << " index " << index;
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
    logdbg  << "MySQLppConnection: prepareStatement: done.";
}
void MySQLppConnection::finalizeStatement ()
{
    logdbg  << "MySQLppConnection: finalizeStatement";
    prepared_query_.reset();
    query_used_=false;
    logdbg  << "MySQLppConnection: finalizeStatement: done";
}

void MySQLppConnection::prepareCommand (std::shared_ptr<DBCommand> command)
{
    logdbg  << "MySQLppConnection: prepareCommand";
    assert (prepared_command_==0);

    prepared_command_=command;
    prepared_command_done_=false;

    prepareStatement (command->get().c_str());
    logdbg  << "MySQLppConnection: prepareCommand: done";
}

std::shared_ptr <DBResult> MySQLppConnection::stepPreparedCommand (unsigned int max_results)
{
    logdbg  << "MySQLppConnection: stepPreparedCommand";

    assert (prepared_command_);

    std::string sql = prepared_command_->get();
    assert (prepared_command_->resultList().size() > 0); // data should be returned

    std::shared_ptr <Buffer> buffer (new Buffer (prepared_command_->resultList()));
    assert (buffer->size() == 0);
    std::shared_ptr <DBResult> dbresult (new DBResult(buffer));

    unsigned int num_properties = buffer->properties().size();
    const PropertyList &list = buffer->properties();
    unsigned int cnt = 0;

    bool done=true;

    max_results--;

    while (mysqlpp::Row row = result_step_.fetch_row())
    {
        readRowIntoBuffer (row, list, num_properties, buffer, cnt);
        assert (buffer->size() == cnt+1);

        if (max_results != 0 && cnt >= max_results)
        {
            done=false;
            break;
        }

        ++cnt;
    }

    //loginf  << "MySQLppConnection: stepPreparedCommand: buffer size " << buffer->size() << " max results " << max_results;

    assert (buffer->size() != 0 && buffer->size() <= max_results+1); // because of max_results--

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

std::map <std::string, DBTableInfo> MySQLppConnection::getTableInfo ()
{
    std::map <std::string, DBTableInfo> info;

    for (auto it : getTableList())
        info.insert (std::pair<std::string, DBTableInfo> (it, getColumnList(it)));

    return info;
}

std::vector <std::string> MySQLppConnection::getTableList()  // buffer of table name strings
{
    std::vector <std::string> tables;

    DBCommand command;
    //command.setCommandString ("SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA = '"+db_name+"' ORDER BY TABLE_NAME DESC;");
    command.set ("SHOW TABLES;");
    PropertyList list;
    list.addProperty ("name", PropertyDataType::STRING);
    command.list (list);

    std::shared_ptr <DBResult> result = execute(command);
    assert (result->containsData());
    std::shared_ptr <Buffer> buffer = result->buffer();

    unsigned int size = buffer->size();
    for (unsigned int cnt=0; cnt < size; cnt++)
        tables.push_back(buffer->getString("name").get(cnt));

    return tables;
}

DBTableInfo MySQLppConnection::getColumnList(const std::string &table) // buffer of column name string, data type
{
    DBTableInfo table_info (table);

    DBCommand command;
    //    command.setCommandString ("SELECT COLUMN_NAME, DATA_TYPE, COLUMN_KEY FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '"
    //            +database_name+"' AND TABLE_NAME = '"+table+"' ORDER BY COLUMN_NAME DESC;");
    //command.set ("SHOW COLUMNS FROM "+table);

    assert (connected_server_);
    std::string database = connected_server_->database();

    //SELECT COLUMN_NAME, DATA_TYPE, COLUMN_KEY, IS_NULLABLE, COLUMN_COMMENT FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = 'job_awam_0019' AND TABLE_NAME = 'sd_track' ORDER BY COLUMN_NAME DESC;
    command.set ("SELECT COLUMN_NAME, DATA_TYPE, COLUMN_KEY, IS_NULLABLE, COLUMN_COMMENT FROM INFORMATION_SCHEMA.COLUMNS WHERE TABLE_SCHEMA = '"+database+"' AND TABLE_NAME = '"+table+"';");

    PropertyList list;
    list.addProperty ("COLUMN_NAME", PropertyDataType::STRING);
    list.addProperty ("DATA_TYPE", PropertyDataType::STRING);
    list.addProperty ("COLUMN_KEY", PropertyDataType::STRING);
    list.addProperty ("IS_NULLABLE", PropertyDataType::BOOL);
    list.addProperty ("COLUMN_COMMENT", PropertyDataType::STRING);
    //list.addProperty ("comment", PropertyDataType::STRING);
    command.list (list);

    std::shared_ptr <DBResult> result = execute(command);
    assert (result->containsData());
    std::shared_ptr <Buffer> buffer = result->buffer();

    for (unsigned int cnt=0; cnt < buffer->size(); cnt++)
    {
        table_info.addColumn (buffer->getString("COLUMN_NAME").get(cnt), buffer->getString("DATA_TYPE").get(cnt),
                              buffer->getString("COLUMN_KEY").get(cnt) == "PRI", buffer->getBool("IS_NULLABLE").get(cnt),
                              buffer->getString("COLUMN_COMMENT").get(cnt));
    }

    return table_info;
}

std::vector<std::string> MySQLppConnection::getDatabases ()
{
    std::vector <std::string> names;

    DBCommand command;
    command.set("SHOW DATABASES;");

    PropertyList list;
    list.addProperty("name", PropertyDataType::STRING);
    command.list(list);

    std::shared_ptr <DBResult> result = execute(command);
    assert (result->containsData());

    std::shared_ptr <Buffer> buffer = result->buffer();
    if (!buffer->firstWrite())
    {
        for (unsigned int cnt=0; cnt < buffer->size(); cnt++)
        {
            std::string tmp = buffer->getString("name").get(cnt);
            names.push_back(tmp);
        }
    }

    return names;
}

void MySQLppConnection::performanceTest ()
{
    //SELECT sd_radar.POS_SYS_X_NM, sd_radar.POS_SYS_Y_NM, sd_radar.MODEC_CODE_FT, sd_radar.TOD, sd_radar.DETECTION_TYPE, sd_radar.DS_ID, sd_radar.REC_NUM FROM sd_radar ORDER BY REC_NUM;'
    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    unsigned int chunk_size=100000;

    loginf  << "MySQLppConnection: performanceTest: start";

    start_time = boost::posix_time::microsec_clock::local_time();

    std::shared_ptr<DBCommand> command = std::make_shared<DBCommand> (DBCommand());
    command->set("SELECT sd_radar.REC_NUM, sd_radar.POS_SYS_X_NM, sd_radar.POS_SYS_Y_NM, sd_radar.MODEC_CODE_FT, sd_radar.TOD, sd_radar.DETECTION_TYPE, sd_radar.DS_ID FROM sd_radar ORDER BY REC_NUM;");
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
    command->list (list);

    prepareCommand (command);

    size_t rows=0;
    bool quit = false;
    while (!quit)
    {
        std::shared_ptr <DBResult> result = stepPreparedCommand(chunk_size);
        assert (result->containsData());
        std::shared_ptr <Buffer> buffer = result->buffer();
        assert (buffer->size() != 0);

        if (buffer->lastOne())
            quit=true;
        else
            assert (buffer->size() == chunk_size);

        rows += buffer->size();

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

QWidget *MySQLppConnection::widget ()
{
    if (!widget_)
    {
        widget_ = new MySQLppConnectionWidget(*this);
    }

    assert (widget_);
    return widget_;
}

void MySQLppConnection::addServer (std::string name)
{
    logdbg << "MySQLppConnection: addServer: name '" << name << "'";

    if (servers_.count (name) != 0)
        throw std::invalid_argument ("MySQLppConnection: addServer: name '"+name+"' already in use");

    addNewSubConfiguration ("MySQLServer", name);
    generateSubConfigurable ("MySQLServer", name);
}

void MySQLppConnection::deleteUsedServer ()
{
    logdbg << "MySQLppConnection: deleteUsedServer: name '" << used_server_ << "'";
    if (servers_.count (used_server_) != 0)
    {
        MySQLServer *server = servers_.at(used_server_);
        servers_.erase(servers_.find(used_server_));
        delete server;

        used_server_ = "";
    }
    else
        throw std::invalid_argument ("MySQLppConnection: deleteServer: unknown server '"+used_server_+"'");
}

void MySQLppConnection::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
  logdbg  << "MySQLppConnection: generateSubConfigurable: generating " << instance_id;
  if (class_id == "MySQLServer")
  {
    MySQLServer *server = new MySQLServer (instance_id, *this);
    assert (servers_.count (server->getInstanceId()) == 0);
    servers_.insert (std::pair <std::string, MySQLServer*> (server->getInstanceId(), server));
  }
  else
    throw std::runtime_error ("MySQLppConnection: generateSubConfigurable: unknown class_id "+class_id );
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
