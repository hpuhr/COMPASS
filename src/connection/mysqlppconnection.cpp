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

#include "buffer.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "dbinterface.h"
#include "dbresult.h"
#include "logger.h"
#include "propertylist.h"
#include "mysqlppconnectioninfowidget.h"
#include "mysqlppconnectionwidget.h"
#include "mysqlppconnection.h"
#include "dbtableinfo.h"
#include "stringconv.h"
#include "mysqlserver.h"
#include "files.h"
#include "stringconv.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include <QMessageBox>
#include <QProgressDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QApplication>

using namespace Utils;

MySQLppConnection::MySQLppConnection(const std::string &class_id, const std::string &instance_id,
                                     DBInterface *interface)
    : DBConnection (class_id, instance_id, interface), interface_(*interface), connection_(mysqlpp::Connection ()),
      prepared_query_(connection_.query()), prepared_parameters_(mysqlpp::SQLQueryParms(&prepared_query_))
{
    registerParameter("used_server", &used_server_, "");

    connection_.set_option(new mysqlpp::LocalInfileOption(true));

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
    assert (servers_.count(used_server_) == 1);
    connected_server_ = servers_.at(used_server_);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    loginf << "MySQLppConnection: connectServer: host " << connected_server_->host() << " port "
           << connected_server_->port() << " user " << connected_server_->user() << " pw "
           << connected_server_->password();

    //bool connect(const char* db = 0, const char* server = 0, const char* user = 0, const char* password = 0,
    // unsigned int port = 0);

    bool ret = connection_.connect("", connected_server_->host().c_str(), connected_server_->user().c_str(),
                                   connected_server_->password().c_str(), connected_server_->port());

    QApplication::restoreOverrideCursor();

    if (!ret)
        throw std::runtime_error("MySQL server connect failed with error "
                                 + std::to_string(connection_.errnum()) + ": " + connection_.error());
}

void MySQLppConnection::createDatabase (const std::string &database_name)
{
    std::vector<std::string> databases = getDatabases();
    assert (std::find(databases.begin(), databases.end(), database_name) == databases.end());

    connection_.create_db(database_name.c_str());
}

void MySQLppConnection::deleteDatabase (const std::string &database_name)
{
    std::vector<std::string> databases = getDatabases();
    assert (std::find(databases.begin(), databases.end(), database_name) != databases.end());

    std::string drop_db = "DROP DATABASE IF EXISTS "+database_name+";";
    executeSQL (drop_db); // drop if exists
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
    used_database_ = database_name;

    emit connectedSignal();
    interface_.databaseContentChanged();

    if (info_widget_)
        info_widget_->updateSlot();

    //loginf  << "MySQLppConnection: init: performance test";
    //performanceTest ();
}

void MySQLppConnection::disconnect()
{
    loginf << "MySQLppConnection: disconnect";

    connection_.disconnect();
    connection_ready_ = false;

    for (auto it : servers_)
        delete it.second;
    servers_.clear();

    connected_server_ = nullptr;

    if (widget_)
        widget_ = nullptr;

    if (info_widget_)
        info_widget_ = nullptr;
}

void MySQLppConnection::executeSQL(const std::string &sql)
{
    logdbg  << "MySQLppConnection: executeSQL: sql statement execute: '" <<sql << "'";

    assert (!query_used_);
    assert (!prepared_command_);
    assert (prepared_command_done_);

    query_used_=true;

    try
    {
        mysqlpp::Query query = connection_.query(sql);

        if(!query.exec()) // execute it!
        {
            logerr  << "MySQLppConnection: executeSQL: error when executing '" << sql<<"' message '"
                    << query.error() << "'";
            logwrn << "MySQLppConnection: executeSQL: sql statement '" << sql << "'";
        }
    }
    catch (std::exception& e)
    {
        logwrn << "MySQLppConnection: executeSQL: sql error '" << e.what() << "'";
        logwrn << "MySQLppConnection: executeSQL: sql statement '" << sql << "'";
        query_used_=false;

        throw;
    }

    query_used_=false;
}

void MySQLppConnection::prepareBindStatement (const std::string &statement)
{
    logdbg  << "MySQLppConnection: prepareBindStatement: statement prepare '" <<statement << "'";

    assert (!query_used_);
    prepared_query_.reset();
    prepared_query_ << statement;
    prepared_query_.parse();
    query_used_=true;

    if (info_widget_)
        info_widget_->updateSlot();
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
    assert (query_used_);

    prepared_query_.reset();
    query_used_=false;

    if (info_widget_)
        info_widget_->updateSlot();
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
void MySQLppConnection::bindVariable (unsigned int index, const std::string &value)
{
    logdbg  << "MySQLppConnection: bindVariable: index " << index << " value '" << value << "'";
    prepared_parameters_[index] = value.c_str();
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
    logdbg  << "MySQLppConnection: execute: command '" << command << "' with buffer";

    assert (buffer);
    assert (!query_used_);
    assert (!prepared_command_);
    assert (prepared_command_done_);

    query_used_=true;

    unsigned int num_properties=0;

    const PropertyList &list = buffer->properties();
    num_properties = list.size();

    logdbg  << "MySQLppConnection: execute: creating query";
    mysqlpp::Query query = connection_.query(command);
    logdbg  << "MySQLppConnection: execute: creating storequeryresult";
    mysqlpp::StoreQueryResult res = query.store();

    logdbg  << "MySQLppConnection: execute: iterating result";
    // Display results
    size_t cnt = buffer->size();

    mysqlpp::StoreQueryResult::const_iterator it;
    for (it = res.begin(); it != res.end(); ++it)
    {
        mysqlpp::Row row = *it;
        readRowIntoBuffer (row, list, num_properties, buffer, cnt);
        cnt++;
    }

    query_used_=false;

    logdbg  << "MySQLppConnection: execute done with size " << buffer->size();
}

void MySQLppConnection::readRowIntoBuffer (mysqlpp::Row &row, const PropertyList &list, unsigned int num_properties,
                                           std::shared_ptr <Buffer> buffer, unsigned int index)
{
    //logdbg << "MySQLppConnection::readRowIntoBuffer: start buffer size " << buffer->size() << " index " << index;
    for (unsigned int cnt=0; cnt < num_properties; cnt++)
    {
            const Property &prop=list.at(cnt);

            switch (prop.dataType())
            {
            case PropertyDataType::BOOL:
                if (row[cnt] != mysqlpp::null)
                    buffer->get<bool>(prop.name()).set(index, static_cast<bool> (row[cnt]));
//                else
//                    buffer->get<bool>(prop.name()).setNone(index);
                //loginf  << "sqlex: bool " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::UCHAR:
                if (row[cnt] != mysqlpp::null)
                    buffer->get<unsigned char>(prop.name()).set(index, static_cast<unsigned char> (row[cnt]));
//                else
//                    buffer->get<unsigned char>(prop.name()).setNone(index);
                //loginf  << "sqlex: uchar " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::CHAR:
                if (row[cnt] != mysqlpp::null)
                    buffer->get<char>(prop.name()).set(index, static_cast<signed char> (row[cnt]));
//                else
//                    buffer->get<char>(prop.name()).setNone(index);
                //loginf  << "sqlex: char " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::INT:
                if (row[cnt] != mysqlpp::null)
                    buffer->get<int>(prop.name()).set(index, static_cast<int> (row[cnt]));
//                else
//                    buffer->get<int>(prop.name()).setNone(index);
                //loginf  << "sqlex: int " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::UINT:
                if (row[cnt] != mysqlpp::null)
                    buffer->get<unsigned int>(prop.name()).set(index, static_cast<unsigned int> (row[cnt]));
//                else
//                    buffer->get<unsigned int>(prop.name()).setNone(index);
                //loginf  << "sqlex: uint " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::STRING:
                if (row[cnt] != mysqlpp::null)
                    buffer->get<std::string>(prop.name()).set(index, static_cast<const char *> (row[cnt]));
//                else
//                    buffer->get<std::string>(prop.name()).setNone(index);
                //loginf  << "sqlex: string " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::FLOAT:
                if (row[cnt] != mysqlpp::null)
                    buffer->get<float>(prop.name()).set(index, static_cast<float> (row[cnt]));
//                else
//                    buffer->get<float>(prop.name()).setNone(index);
                //loginf  << "sqlex: float " << prop->id_ << " val " << *ptr;
                break;
            case PropertyDataType::DOUBLE:
                if (row[cnt] != mysqlpp::null)
                    buffer->get<double>(prop.name()).set(index, static_cast<double> (row[cnt]));
//                else
//                    buffer->get<double>(prop.name()).setNone(index);
                //loginf  << "sqlex: double " << prop->id_ << " val " << *ptr;
                break;
            default:
                logerr  <<  "MySQLppConnection: readRowIntoBuffer: unknown property type";
                throw std::runtime_error ("MySQLppConnection: readRowIntoBuffer: unknown property type");
                break;
            }
    }
    //logdbg << "MySQLppConnection::readRowIntoBuffer: end buffer size " << buffer->size() << " index " << index;
}

void MySQLppConnection::execute (const std::string &command)
{
    logdbg  << "MySQLppConnection: execute: command '" << command << "'";

    assert (!query_used_);
    assert (!prepared_command_);
    assert (prepared_command_done_);

    query_used_=true;

    logdbg  << "MySQLppConnection: execute: creating query";
    mysqlpp::Query query = connection_.query(command);
    logdbg  << "MySQLppConnection: execute: creating storequeryresult";
    mysqlpp::StoreQueryResult res = query.store();

    assert (res.begin() == res.end());

    query_used_=false;
    logdbg  << "MySQLppConnection: execute done";
}


void MySQLppConnection::prepareStatement (const std::string &sql)
{
    logdbg  << "MySQLppConnection: prepareStatement: sql '" << sql << "'";

    assert (!query_used_);
    query_used_=true;

    result_step_ = mysqlpp::UseQueryResult ();

    prepared_query_.clear();
    prepared_query_ << sql;

    if (!(result_step_ = prepared_query_.use()))
    {
        throw std::runtime_error ("MySQLppConnection: prepareStatement: query error '"
                                  +std::string(prepared_query_.error())+"'");
    }

    if (info_widget_)
        info_widget_->updateSlot();

    logdbg  << "MySQLppConnection: prepareStatement: done.";
}
void MySQLppConnection::finalizeStatement ()
{
    logdbg  << "MySQLppConnection: finalizeStatement";

    assert (query_used_);

    prepared_query_.clear();
    prepared_query_.reset();
    query_used_=false;

    if (info_widget_)
        info_widget_->updateSlot();

    logdbg  << "MySQLppConnection: finalizeStatement: done";
}

void MySQLppConnection::prepareCommand (std::shared_ptr<DBCommand> command)
{
    logdbg  << "MySQLppConnection: prepareCommand";
    assert (!prepared_command_);
    assert (prepared_command_done_);

    prepared_command_=command;
    prepared_command_done_=false;

    if (info_widget_)
        info_widget_->updateSlot();

    prepareStatement (command->get().c_str());
    logdbg  << "MySQLppConnection: prepareCommand: done";
}

std::shared_ptr <DBResult> MySQLppConnection::stepPreparedCommand (unsigned int max_results)
{
    logdbg  << "MySQLppConnection: stepPreparedCommand";

    assert (prepared_command_);
    assert (!prepared_command_done_);

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
    logdbg  << "MySQLppConnection: stepPreparedCommand: buffer size " << buffer->size() << " max results " << max_results;

    assert (buffer->size() <= max_results+1); // because of max_results--

    if (buffer->size() == 0 || done)
    {
        logdbg  << "MySQLppConnection: stepPreparedCommand: reading done";
        prepared_command_done_=true;
        if (done)
            buffer->lastOne(true);
        else
            buffer=nullptr;
    }

    logdbg  << "MySQLppConnection: stepPreparedCommand: done";
    return dbresult;
}
void MySQLppConnection::finalizeCommand ()
{
    logdbg  << "MySQLppConnection: finalizeCommand";
    assert (prepared_command_);
    //assert (prepared_command_done_); true if ok, false if quit job

    bool first = true;
    while (mysqlpp::Row row = result_step_.fetch_row())
        if (first)
        {
            loginf << "MySQLppConnection: finalizeCommand: stepping through result set to finalize";
            first = false;
        }

    prepared_command_=nullptr; // should be deleted by caller
    prepared_command_done_=true;
    finalizeStatement();

    if (info_widget_)
        info_widget_->updateSlot();

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

    size_t size = buffer->size();

    for (unsigned int cnt=0; cnt < size; cnt++)
        tables.push_back(buffer->get<std::string>("name").get(cnt));

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
        table_info.addColumn (buffer->get<std::string>("COLUMN_NAME").get(cnt),
                              buffer->get<std::string>("DATA_TYPE").get(cnt),
                              buffer->get<std::string>("COLUMN_KEY").get(cnt) == "PRI",
                              buffer->get<bool>("IS_NULLABLE").get(cnt),
                              buffer->get<std::string>("COLUMN_COMMENT").get(cnt));
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
            std::string tmp = buffer->get<std::string>("name").get(cnt);

            if (tmp == "mysql" || tmp == "information_schema"|| tmp == "performance_schema" || tmp == "sys")
                // omit system databases
                continue;

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

QWidget* MySQLppConnection::widget ()
{
    if (!widget_)
        widget_.reset(new MySQLppConnectionWidget(*this));

    assert (widget_);
    return widget_.get();
}

//void MySQLppConnection::deleteWidget ()
//{
//    for (auto& server_it : servers_)
//        server_it.second->deleteWidget();

//    widget_ = nullptr;
//}

QWidget *MySQLppConnection::infoWidget ()
{
    if (!info_widget_)
        info_widget_.reset(new MySQLppConnectionInfoWidget(*this));

    assert (info_widget_);
    return info_widget_.get();
}

std::string MySQLppConnection::status () const
{
    if (connection_ready_)
        return "Ready";
    else
        return "Not connected";
}

std::string MySQLppConnection::identifier () const
{
    assert (connection_ready_);

    return used_server_+": "+used_database_;
}

void MySQLppConnection::addServer (const std::string& name)
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
        assert (servers_.count (server->instanceId()) == 0);
        servers_.insert (std::pair <std::string, MySQLServer*> (server->instanceId(), server));
    }
    else
        throw std::runtime_error ("MySQLppConnection: generateSubConfigurable: unknown class_id "+class_id );
}

//void MySQLppConnection::importSQLFile (const std::string& filename)
//{
//    loginf  << "MySQLppConnection: importSQLFile: importing " << filename;
//    assert (Files::fileExists(filename));

//    QProgressDialog* progress_dialog = new QProgressDialog (tr("Importing SQL File"), tr(""), 0, 100);
//    progress_dialog->setCancelButton(0);
//    progress_dialog->setModal(true);
//    progress_dialog->setWindowModality(Qt::ApplicationModal);
//    progress_dialog->show();

//    for (unsigned int cnt=0; cnt < 10; cnt++)
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

//    std::ifstream is;
//    is.open (filename.c_str(), std::ios::binary );
//    is.seekg (0, std::ios::end);
//    size_t file_byte_size = is.tellg();
//    is.close();
//    assert (file_byte_size);
//    loginf  << "MySQLppConnection: importSQLFile: file_byte_size: " << file_byte_size;

//    for (unsigned int cnt=0; cnt < 10; cnt++)
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

//    std::ifstream sql_file (filename);
//    assert (sql_file.is_open());

//    std::string line;
//    std::stringstream ss;
//    size_t line_cnt = 0;
//    size_t byte_cnt = 0;
//    size_t error_cnt = 0;

//    while (getline (sql_file,line))
//    {
//        try
//        {
//            byte_cnt += line.size();

//            if (line.find ("delimiter") != std::string::npos || line.find ("DELIMITER") != std::string::npos
//                    || line.find ("VIEW") != std::string::npos)
//            {
//                loginf << "MySQLppConnection: importSQLFile: breaking at delimiter, bytes " << byte_cnt;
//                break;
//            }

//            ss << line << '\n';

//            if (line.back() == ';')
//            {
//                //                loginf << "MySQLppConnection: importSQLFile: line cnt " << line_cnt << " of " << line_count
//                //                       << " strlen " << ss.str().size() << "'";

//                if (ss.str().size())
//                    executeSQL (ss.str());

//                ss.str("");
//            }

//            if (line_cnt % 10 == 0)
//            {
//                progress_dialog->setValue(100*byte_cnt/file_byte_size);
//                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//            }

//            line_cnt ++;
//        }
//        catch (std::exception& e)
//        {
//            logwrn << "MySQLppConnection: importSQLFile: sql error '" << e.what() << "'";
//            ss.str("");
//            error_cnt++;

//            if (error_cnt > 3)
//            {
//                logwrn << "MySQLppConnection: importSQLFile: quit after too many errors";

//                QMessageBox m_warning (QMessageBox::Warning, "MySQL Text Import Failed",
//                                       "Quit after too many SQL errors. Please make sure that"
//                                       " the SQL file is correct.",
//                                       QMessageBox::Ok);
//                m_warning.exec();

//                break;
//            }
//        }

//    }
//    delete progress_dialog;
//    progress_dialog = nullptr;

//    QMessageBox msgBox;
//    std::string msg;
//    if (error_cnt)
//        msg = "The SQL file was imported with "+std::to_string(error_cnt)+" SQL errors.";
//    else
//        msg = "The SQL file was imported without SQL errors.";

//    msgBox.setText(msg.c_str());
//    msgBox.exec();

//    sql_file.close();
//    interface_.databaseContentChanged();
//}

//void MySQLppConnection::importSQLArchiveFile(const std::string& filename)
//{
//    loginf  << "MySQLppConnection: importSQLArchiveFile: importing " << filename;
//    assert (Files::fileExists(filename));

//    // if gz but not tar.gz or tgz
//    bool raw = String::hasEnding (filename, ".gz") && !String::hasEnding (filename, ".tar.gz");

//    loginf  << "MySQLppConnection: importSQLArchiveFile: importing " << filename << " raw " << raw;

//    struct archive *a;
//    struct archive_entry *entry;
//    int r;

//    a = archive_read_new();

//    if (raw)
//    {
//        archive_read_support_filter_gzip(a);
//        archive_read_support_filter_bzip2(a);
//        archive_read_support_format_raw(a);
//    }
//    else
//    {
//        archive_read_support_filter_all(a);
//        archive_read_support_format_all(a);

//    }
//    r = archive_read_open_filename(a, filename.c_str(), 10240); // Note 1

//    if (r != ARCHIVE_OK)
//        throw std::runtime_error("MySQLppConnection: importSQLArchiveFile: archive error: "
//                                 +std::string(archive_error_string(a)));

//    const void *buff;
//    size_t size;
//    int64_t offset;

//    size_t line_cnt = 0;
//    size_t error_cnt = 0;
//    size_t byte_cnt = 0;

//    QMessageBox msg_box;
//    std::string msg = "Importing archive '"+filename+"'.";
//    msg_box.setText(msg.c_str());
//    msg_box.setStandardButtons(QMessageBox::NoButton);
//    msg_box.show();

//    for (unsigned int cnt=0; cnt < 10; cnt++)
//        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

//    while (archive_read_next_header(a, &entry) == ARCHIVE_OK)
//    {
//        loginf << "Archive file found: " << archive_entry_pathname(entry) << " size " << archive_entry_size(entry);

//        msg_box.setInformativeText(archive_entry_pathname(entry));

//        bool done=false;

//        std::stringstream ss;

//        for (;;)
//        {
//            r = archive_read_data_block(a, &buff, &size, &offset);

//            if (r == ARCHIVE_EOF)
//                break;
//            if (r != ARCHIVE_OK)
//                throw std::runtime_error("MySQLppConnection: importSQLArchiveFile: archive error: "
//                                         +std::string(archive_error_string(a)));

//            std::string str (reinterpret_cast<char const*>(buff), size);

//            //loginf << "UGA read offset " << offset << " size " << size;

//            std::vector<std::string> lines = String::split(str, '\n');
//            std::string line;

//            //loginf << "UGA read str has " << lines.size() << " lines";

//            for (std::vector<std::string>::iterator line_it = lines.begin(); line_it != lines.end(); line_it++)
//            {
//                if (line_it + 1 == lines.end() )
//                {
//                    //loginf << "last one";
//                    ss << *line_it;
//                    break;
//                }

//                try
//                {
//                    ss << *line_it << '\n';

//                    line = ss.str();

//                    byte_cnt += line.size();

//                    if (line.find ("delimiter") != std::string::npos || line.find ("DELIMITER") != std::string::npos
//                            || line.find ("VIEW") != std::string::npos)
//                    {
//                        loginf << "MySQLppConnection: importSQLArchiveFile: breaking at delimiter, bytes " << byte_cnt;
//                        done = true;
//                        break;
//                    }

//                    if (line_it->back() == ';')
//                    {
//                        //                        loginf << "MySQLppConnection: importSQLArchiveFile: line cnt " << line_cnt
//                        //                               <<  " strlen " << ss.str().size() << "'";

//                        if (line.size())
//                            executeSQL (line+"\n");

//                        ss.str("");
//                    }

//                    if (line_cnt % 10 == 0)
//                    {
//                        logdbg << "MySQLppConnection: importSQLArchiveFile: line cnt " << line_cnt;
//                        msg = "Read " + std::to_string(line_cnt) + " lines from "
//                                + std::string(archive_entry_pathname(entry)) + " archive entry.";

//                        msg_box.setInformativeText(msg.c_str());
//                        msg_box.show();
//                        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//                        //                        progress_dialog->setValue(100*byte_cnt/file_byte_size);
//                        //                        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//                    }

//                    line_cnt ++;
//                }
//                catch (std::exception& e)
//                {
//                    logwrn << "MySQLppConnection: importSQLArchiveFile: sql error '" << e.what() << "'";
//                    ss.str("");
//                    error_cnt++;

//                    if (error_cnt > 3)
//                    {
//                        logwrn << "MySQLppConnection: importSQLArchiveFile: quit after too many errors";

//                        QMessageBox m_warning (QMessageBox::Warning, "MySQL Archive Import Failed",
//                                               "Quit after too many SQL errors. Please make sure that"
//                                               " the archive is correct as specified in the user manual.",
//                                               QMessageBox::Ok);
//                        m_warning.exec();
//                        done=true;
//                        break;
//                    }
//                }
//            }

//            if (done)
//                break;
//        }

//        if (done)
//            break;

//        loginf << "MySQLppConnection: importSQLArchiveFile: archive file " << archive_entry_pathname(entry) << " imported";
//    }

//    msg_box.close();

//    QMessageBox msgBox;
//    if (error_cnt)
//        msg = "The SQL archive file was imported with "+std::to_string(error_cnt)+" SQL errors.";
//    else
//        msg = "The SQL archive file was imported without SQL errors.";

//    msgBox.setText(msg.c_str());
//    msgBox.exec();

//    r = archive_read_close(a);
//    if (r != ARCHIVE_OK)
//        throw std::runtime_error("MySQLppConnection: importSQLArchiveFile: archive read close error: "
//                                 +std::string(archive_error_string(a)));

//    r = archive_read_free(a);

//    if (r != ARCHIVE_OK)
//        throw std::runtime_error("MySQLppConnection: importSQLArchiveFile: archive read free error: "
//                                 +std::string(archive_error_string(a)));

//    interface_.databaseContentChanged();
//}

//DBResult *MySQLppConnection::readBulkCommand (DBCommand *command, std::string main_statement,
//std::string order_statement, unsigned int max_results)
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
