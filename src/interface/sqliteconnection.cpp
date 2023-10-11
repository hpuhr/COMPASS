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

#include "sqliteconnection.h"
#include "buffer.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "dbinterface.h"
#include "dbresult.h"
#include "dbtableinfo.h"
#include "logger.h"
#include "property.h"
//#include "savedfile.h"
//#include "stringconv.h"
//#include "files.h"
#include "util/timeconv.h"

#include <QApplication>

#include <cstring>

using namespace std;
using namespace Utils;

SQLiteConnection::SQLiteConnection(const std::string& class_id, const std::string& instance_id,
                                   DBInterface* interface)
    : Configurable(class_id, instance_id, interface), interface_(*interface), db_opened_(false)
{
    createSubConfigurables();

    loginf << "SQLiteConnection: constructor: SQLITE_VERSION " << SQLITE_VERSION;
}

SQLiteConnection::~SQLiteConnection()
{
    assert(!db_handle_);
}

void SQLiteConnection::openFile(const std::string& file_name)
{
    loginf << "SQLiteConnection: openFile: '" << file_name << "'";

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    int result = sqlite3_open_v2(file_name.c_str(), &db_handle_,
                                 SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);

//    int result = sqlite3_open(":memory:", &db_handle_);

    if (result != SQLITE_OK)
    {
        // Even in case of an error we get a valid db_handle (for the
        // purpose of calling sqlite3_errmsg on it ...)
        logerr << "SQLiteConnection: openFile: error " << result << " "
               << sqlite3_errmsg(db_handle_);
        sqlite3_close(db_handle_);
        throw std::runtime_error("SQLiteConnection: openFile: error");
    }

    char* sErrMsg = 0;
    sqlite3_exec(db_handle_, "PRAGMA SYNCHRONOUS = OFF", NULL, NULL, &sErrMsg);
    sqlite3_exec(db_handle_, "PRAGMA TEMP_STORE = 2", NULL, NULL, &sErrMsg);
    sqlite3_exec(db_handle_, "PRAGMA JOURNAL_MODE = OFF", NULL, NULL, &sErrMsg);
    sqlite3_exec(db_handle_, "PRAGMA LOCKING_MODE = EXCLUSIVE", NULL, NULL, &sErrMsg);
    sqlite3_exec(db_handle_, "PRAGMA CACHE_SIZE = 500", NULL, NULL, &sErrMsg);

    //sqlite3_exec(db_handle_, "PRAGMA locking_mode = EXCLUSIVE", NULL, NULL, &sErrMsg);

    db_opened_ = true;

    QApplication::restoreOverrideCursor();
}

void SQLiteConnection::exportFile(const std::string& file_name)
{
    loginf << "SQLiteConnection: exportFile: file '" << file_name << "'";
    assert (db_opened_);

    string tmp_sql = "VACUUM INTO '"+file_name+"';";

    loginf << "SQLiteConnection: exportFile: sql '" << tmp_sql << "'";

    char* sErrMsg = 0;
    sqlite3_exec(db_handle_, tmp_sql.c_str(), NULL, NULL, &sErrMsg);

}

void SQLiteConnection::disconnect()
{
    logdbg << "SQLiteConnection: disconnect";

    db_opened_ = false;

    if (db_handle_)
    {
        sqlite3_close(db_handle_);
        db_handle_ = nullptr;
    }
}

void SQLiteConnection::executeSQL(const std::string& sql)
{
    logdbg << "DBInterface: executeSQL: sql statement execute: '" << sql << "'";

    char* exec_err_msg = NULL;
    int result = sqlite3_exec(db_handle_, sql.c_str(), NULL, NULL, &exec_err_msg);
    if (result != SQLITE_OK)
    {
        logerr << "DBInterface: executeSQL: sqlite3_exec cmd '" << sql << "' failed: " << exec_err_msg;
        sqlite3_free(exec_err_msg);
        std::string error;
        error += "DBInterface: executeSQL: sqlite3_exec failed: ";
        error += exec_err_msg;
        throw std::runtime_error(error);
    }
}

void SQLiteConnection::prepareBindStatement(const std::string& statement)
{
    const char* tail = 0;
    int ret =
            sqlite3_prepare_v2(db_handle_, statement.c_str(), statement.size(), &statement_, &tail);

    if (ret != SQLITE_OK)
    {
        logerr << "DBInterface: prepareBindStatement: error preparing bind";
        return;
    }
}
void SQLiteConnection::beginBindTransaction()
{
    char* sErrMsg = 0;
    sqlite3_exec(db_handle_, "BEGIN TRANSACTION", NULL, NULL, &sErrMsg);
}
void SQLiteConnection::stepAndClearBindings()
{
    logdbg << "DBInterface: stepAndClearBindings: stepping statement";
    int ret2;
    while ((ret2 = sqlite3_step(statement_)))
    {
        logdbg << "DBInterface: stepAndClearBindings: stepping returned " << ret2;
        if (ret2 == SQLITE_DONE)
        {
            logdbg << "DBInterface: stepAndClearBindings: bind done";
            break;
        }
        else if (ret2 > SQLITE_OK && ret2 < SQLITE_ROW)
        {
            logerr << "DBInterface: stepAndClearBindings: error while bind: " << ret2 << ": "
                   << sqlite3_errmsg(db_handle_);
            throw std::runtime_error("DBInterface: stepAndClearBindings: error while bind");
        }
    }

    logdbg << "DBInterface: stepAndClearBindings: clearing statement";

    sqlite3_clear_bindings(statement_);
    sqlite3_reset(statement_);
}

void SQLiteConnection::endBindTransaction()
{
    char* sErrMsg = 0;
    sqlite3_exec(db_handle_, "END TRANSACTION", NULL, NULL, &sErrMsg);
}
void SQLiteConnection::finalizeBindStatement() { sqlite3_finalize(statement_); }

int SQLiteConnection::changes()
{
    assert (db_handle_);
    return sqlite3_changes(db_handle_);
}

void SQLiteConnection::bindVariable(unsigned int index, int value)
{
    logdbg << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_int(statement_, index, value);
}
void SQLiteConnection::bindVariable(unsigned int index, double value)
{
    logdbg << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_double(statement_, index, value);
}
void SQLiteConnection::bindVariable(unsigned int index, const std::string& value)
{
    logdbg << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_text(statement_, index, value.c_str(), -1, SQLITE_TRANSIENT);
}

void SQLiteConnection::bindVariable(unsigned int index, long value)
{
    logdbg << "SQLiteConnection: bindVariable: index " << index << " value '" << value << "'";
    sqlite3_bind_int64(statement_, index, value);
}

void SQLiteConnection::bindVariableNull(unsigned int index)
{
    sqlite3_bind_null(statement_, index);
}

std::shared_ptr<DBResult> SQLiteConnection::execute(const DBCommand& command)
{
    std::shared_ptr<DBResult> dbresult(new DBResult());
    std::string sql = command.get();

    if (command.resultList().size() > 0)  // data should be returned
    {
        std::shared_ptr<Buffer> buffer(new Buffer(command.resultList()));
        dbresult->buffer(buffer);
        logdbg << "MySQLppConnection: execute: executing";
        execute(sql, buffer);
    }
    else
    {
        logdbg << "MySQLppConnection: execute: executing";
        execute(sql);
    }

    logdbg << "SQLiteConnection: execute: end";

    return dbresult;
}

std::shared_ptr<DBResult> SQLiteConnection::execute(const DBCommandList& command_list)
{
    std::shared_ptr<DBResult> dbresult(new DBResult());

    unsigned int num_commands = command_list.getNumCommands();

    if (command_list.getResultList().size() > 0)  // data should be returned
    {
        std::shared_ptr<Buffer> buffer(new Buffer(command_list.getResultList()));
        dbresult->buffer(buffer);

        for (unsigned int cnt = 0; cnt < num_commands; cnt++)
            execute(command_list.getCommandString(cnt), buffer);
    }
    else
    {
        for (unsigned int cnt = 0; cnt < num_commands; cnt++)
            execute(command_list.getCommandString(cnt));
    }
    logdbg << "SQLiteConnection: execute: end";

    return dbresult;
}

void SQLiteConnection::execute(const std::string& command)
{
    logdbg << "SQLiteConnection: execute";

    int result;

    prepareStatement(command.c_str());
    result = sqlite3_step(statement_);

    if (result != SQLITE_DONE)
    {
        logerr << "SQLiteConnection: execute: problem while stepping the result: " << result << " "
               << sqlite3_errmsg(db_handle_);
        throw std::runtime_error("SQLiteConnection: execute: problem while stepping the result");
    }

    finalizeStatement();
}

void SQLiteConnection::execute(const std::string& command, std::shared_ptr<Buffer> buffer)
{
    logdbg << "SQLiteConnection: execute";

    assert(buffer);
    unsigned int num_properties = 0;

    const PropertyList& list = buffer->properties();
    num_properties = list.size();

    unsigned int cnt = buffer->size();

    int result;

    prepareStatement(command.c_str());

    // Now step throught the result lines
    for (result = sqlite3_step(statement_); result == SQLITE_ROW; result = sqlite3_step(statement_))
    {
        readRowIntoBuffer(list, num_properties, buffer, cnt);
        cnt++;
    }

    if (result != SQLITE_DONE)
    {
        logerr << "SQLiteConnection: execute: problem while stepping the result: " << result << " "
               << sqlite3_errmsg(db_handle_);
        throw std::runtime_error("SQLiteConnection: execute: problem while stepping the result");
    }

    finalizeStatement();
}

void SQLiteConnection::readRowIntoBuffer(const PropertyList& list, unsigned int num_properties,
                                         std::shared_ptr<Buffer> buffer, unsigned int index)
{
    logdbg << "SQLiteConnection: readRowIntoBuffer: num_properties " << num_properties;

    for (unsigned int cnt = 0; cnt < num_properties; cnt++)
    {
        const Property& prop = list.at(cnt);

        switch (prop.dataType())
        {
            case PropertyDataType::BOOL:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: bool index " << index
                           << " property '" << prop.name()
                           << "' value " << static_cast<bool>(sqlite3_column_int(statement_, cnt));

                    buffer->get<bool>(prop.name())
                            .set(index, static_cast<bool>(sqlite3_column_int(statement_, cnt)));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: bool index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::UCHAR:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: unsigned char index " << index
                           << " property '" << prop.name()
                           << "' value " << (int) static_cast<unsigned char>(sqlite3_column_int(statement_, cnt));

                    buffer->get<unsigned char>(prop.name())
                            .set(index,
                                 static_cast<unsigned char>(sqlite3_column_int(statement_, cnt)));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: unsigned char index " << index
                           << " property '" << prop.name() << " is null";

                break;
            case PropertyDataType::CHAR:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: char index " << index
                           << " property '" << prop.name()
                           << "' value " << (int)static_cast<char>(sqlite3_column_int(statement_, cnt));

                    buffer->get<char>(prop.name())
                            .set(index, static_cast<char>(sqlite3_column_int(statement_, cnt)));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: char index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::INT:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: int index " << index
                           << " property '" << prop.name()
                           << "' value " << static_cast<int>(sqlite3_column_int(statement_, cnt));

                    buffer->get<int>(prop.name())
                            .set(index, static_cast<int>(sqlite3_column_int(statement_, cnt)));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: int index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::UINT:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: unsigned int index " << index
                           << " property '" << prop.name()
                           << "' value " << static_cast<unsigned int>(sqlite3_column_int(statement_, cnt));

                    buffer->get<unsigned int>(prop.name())
                            .set(index, static_cast<unsigned int>(sqlite3_column_int(statement_, cnt)));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: unsigned int index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::LONGINT:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: int index " << index
                           << " property '" << prop.name()
                           << "' value " << sqlite3_column_int64(statement_, cnt);

                    buffer->get<long>(prop.name())
                            .set(index, sqlite3_column_int64(statement_, cnt));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: int index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::ULONGINT:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: unsigned int index " << index
                           << " property '" << prop.name()
                           << "' value " << static_cast<unsigned long>(sqlite3_column_int64(statement_, cnt));

                    buffer->get<unsigned long>(prop.name())
                            .set(index, static_cast<unsigned long>(sqlite3_column_int64(statement_, cnt)));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: unsigned int index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::FLOAT:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: float index " << index
                           << " property '" << prop.name()
                           << "' value " << static_cast<float>(sqlite3_column_double(statement_, cnt));

                    buffer->get<float>(prop.name())
                            .set(index, static_cast<float>(sqlite3_column_double(statement_, cnt)));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: float index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::DOUBLE:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: double index " << index
                           << " property '" << prop.name()
                           << "' value " << static_cast<double>(sqlite3_column_double(statement_, cnt));

                    buffer->get<double>(prop.name())
                            .set(index, static_cast<double>(sqlite3_column_double(statement_, cnt)));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: double index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::STRING:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: string index " << index
                           << " property '" << prop.name()
                           << "' value "
                           << std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement_, cnt)));

                    buffer->get<std::string>(prop.name())
                            .set(index, std::string(reinterpret_cast<const char*>(
                                                        sqlite3_column_text(statement_, cnt))));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: string index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::JSON:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: json index " << index
                           << " property '" << prop.name()
                           << "' value "
                           << std::string(reinterpret_cast<const char*>(sqlite3_column_text(statement_, cnt)));

                    buffer->get<nlohmann::json>(prop.name())
                            .set(index, nlohmann::json::parse(std::string(reinterpret_cast<const char*>(
                                                        sqlite3_column_text(statement_, cnt)))));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: string index " << index
                           << " property '" << prop.name() << " is null";
                break;
            case PropertyDataType::TIMESTAMP:
                if (sqlite3_column_type(statement_, cnt) != SQLITE_NULL)
                {
                    logdbg << "SQLiteConnection: readRowIntoBuffer: unsigned int index " << index
                           << " property '" << prop.name()
                           << "' value " << Time::toStringLong(
                                  static_cast<unsigned long>(sqlite3_column_int64(statement_, cnt)));

                    buffer->get<boost::posix_time::ptime>(prop.name())
                            .set(index, Time::fromLong(static_cast<unsigned long>(sqlite3_column_int64(statement_, cnt))));
                }
                else
                    logdbg << "SQLiteConnection: readRowIntoBuffer: unsigned int index " << index
                           << " property '" << prop.name() << " is null";
                break;
            default:
                logerr << "MySQLppConnection: readRowIntoBuffer: unknown property type";
                throw std::runtime_error(
                            "MySQLppConnection: readRowIntoBuffer: unknown property type");
                break;
        }
    }
}

void SQLiteConnection::prepareStatement(const std::string& sql)
{
    logdbg << "SQLiteConnection: prepareStatement: sql '" << sql << "'";
    int result;
    const char* remaining_sql;

    // Prepare the select statement
    remaining_sql = NULL;
    result = sqlite3_prepare_v2(db_handle_, sql.c_str(), sql.size(), &statement_, &remaining_sql);
    if (result != SQLITE_OK)
    {
        logerr << "SQLiteConnection: execute: error " << result << " "
               << sqlite3_errmsg(db_handle_);
        sqlite3_close(db_handle_);
        throw std::runtime_error("SQLiteConnection: execute: error");
    }

    if (remaining_sql && *remaining_sql != '\0')
    {
        logerr << "SQLiteConnection: execute: there was unparsed sql text: " << remaining_sql;
        throw std::runtime_error("SQLiteConnection: execute: there was unparsed sql text");
    }
}
void SQLiteConnection::finalizeStatement() { sqlite3_finalize(statement_); }

void SQLiteConnection::prepareCommand(const std::shared_ptr<DBCommand> command)
{
    assert(prepared_command_ == 0);
    assert(command);

    prepared_command_ = command;
    prepared_command_done_ = false;

    prepareStatement(command->get().c_str());
}
std::pair<std::shared_ptr<DBResult>, bool> SQLiteConnection::stepPreparedCommand(unsigned int max_results)
{
    assert(prepared_command_);
    assert(!prepared_command_done_);

    std::string sql = prepared_command_->get();
    assert(prepared_command_->resultList().size() > 0);  // data should be returned

    std::shared_ptr<Buffer> buffer(new Buffer(prepared_command_->resultList()));
    assert(buffer->size() == 0);
    std::shared_ptr<DBResult> dbresult(new DBResult(buffer));

    unsigned int num_properties = buffer->properties().size();
    const PropertyList& list = buffer->properties();

    unsigned int cnt = 0;
    int result;
    bool done = true;
    bool last_one = false;

    max_results--;

    // Now step throught the result lines
    for (result = sqlite3_step(statement_); result == SQLITE_ROW; result = sqlite3_step(statement_))
    {
        readRowIntoBuffer(list, num_properties, buffer, cnt);

        if (buffer->size())  // 0 == 1 otherwise
            assert(buffer->size() == cnt + 1);

        if (max_results != 0 && cnt >= max_results)
        {
            done = false;
            break;
        }

        ++cnt;
    }

    if (result != SQLITE_ROW && result != SQLITE_DONE)
    {
        logerr << "SQLiteConnection: stepPreparedCommand: problem while stepping the result: "
               << result << " " << sqlite3_errmsg(db_handle_);
        throw std::runtime_error(
                    "SQLiteConnection: stepPreparedCommand: problem while stepping the result");
    }

    assert(buffer->size() <= max_results + 1);  // because of max_results--

    if (result == SQLITE_DONE || buffer->size() == 0 || done)
    {
        assert(done);
        logdbg << "SQLiteConnection: stepPreparedCommand: reading done";
        prepared_command_done_ = true;

        last_one = true;
    }

    return {dbresult, last_one};
}
void SQLiteConnection::finalizeCommand()
{
    assert(prepared_command_ != nullptr);
    sqlite3_finalize(statement_);
    prepared_command_ = nullptr;  // should be deleted by caller
    prepared_command_done_ = true;
}

std::map<std::string, DBTableInfo> SQLiteConnection::getTableInfo()
{
    loginf << "SQLiteConnection: getTableInfo";

    std::map<std::string, DBTableInfo> info;

    for (auto it : getTableList())
    {
        loginf << "SQLiteConnection: getTableInfo: table " << it;
        info.insert(std::pair<std::string, DBTableInfo>(it, getColumnList(it)));
    }

    return info;
}

std::vector<std::string> SQLiteConnection::getTableList()  // buffer of table name strings
{
    std::vector<std::string> tables;

    DBCommand command;
    command.set("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name DESC;");
    // command.set ("SELECT name FROM sqlite_master WHERE name != 'sqlite_sequence' ORDER BY name  DESC;");

    PropertyList list;
    list.addProperty("name", PropertyDataType::STRING);
    command.list(list);

    std::shared_ptr<DBResult> result = execute(command);
    assert(result->containsData());
    std::shared_ptr<Buffer> buffer = result->buffer();

    unsigned int size = buffer->size();
    std::string table_name;

    for (unsigned int cnt = 0; cnt < size; cnt++)
    {
        table_name = buffer->get<std::string>("name").get(cnt);

        if (table_name.find("sqlite") != std::string::npos)  // ignore sqlite system tables
            continue;

        tables.push_back(buffer->get<std::string>("name").get(cnt));
    }

    return tables;
}

DBTableInfo SQLiteConnection::getColumnList(
        const std::string& table)  // buffer of column name string, data type
{
    logdbg << "SQLiteConnection: getColumnList: table " << table;

    DBTableInfo table_info(table);

    DBCommand command;
    command.set("PRAGMA table_info(" + table + ")");

    // int cid, string name, string type,int notnull, string dflt_value, int pk

    PropertyList list;
    list.addProperty("cid", PropertyDataType::INT);
    list.addProperty("name", PropertyDataType::STRING);
    list.addProperty("type", PropertyDataType::STRING);
    list.addProperty("notnull", PropertyDataType::INT);
    list.addProperty("dfltvalue", PropertyDataType::STRING);
    list.addProperty("pk", PropertyDataType::INT);

    command.list(list);

    std::shared_ptr<DBResult> result = execute(command);
    assert(result->containsData());
    std::shared_ptr<Buffer> buffer = result->buffer();

    for (unsigned int cnt = 0; cnt < buffer->size(); cnt++)
    {
        // loginf << "UGA " << table << ": "  << buffer->getString("name").get(cnt);

        assert(buffer->has<std::string>("type"));

        std::string data_type = buffer->get<std::string>("type").get(cnt);

        if (data_type == "BOOLEAN")
            data_type = "BOOL";
        else if (data_type == "TEXT")
            data_type = "STRING";
        else if (data_type == "REAL")
            data_type = "DOUBLE";
        else if (data_type == "INTEGER")
            data_type = "INT";

        assert(buffer->has<std::string>("name"));
        assert(buffer->has<int>("pk"));
        assert(buffer->has<int>("notnull"));

        table_info.addColumn(buffer->get<std::string>("name").get(cnt), data_type,
                             buffer->get<int>("pk").get(cnt) > 0,
                             !buffer->get<int>("notnull").get(cnt), "");
    }

    return table_info;
}

void SQLiteConnection::generateSubConfigurable(const std::string& class_id,
                                               const std::string& instance_id)
{
    throw std::runtime_error("SQLiteConnection: generateSubConfigurable: unknown class_id " + class_id);
}

std::string SQLiteConnection::status() const
{
    if (db_opened_)
        return "Ready";
    else
        return "Not connected";
}

