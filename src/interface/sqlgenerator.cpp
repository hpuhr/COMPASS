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

#include "sqlgenerator.h"
#include "compass.h"
#include "buffer.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "sqliteconnection.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "filtermanager.h"
#include "logger.h"
#include "propertylist.h"
#include "stringconv.h"
#include "dbcontentdbdatasource.h"

#include <algorithm>
#include <iomanip>
#include <string>



using namespace Utils;
using namespace std;

SQLGenerator::SQLGenerator(DBInterface& db_interface) : db_interface_(db_interface)
{
    stringstream ss;

    //    ss << "CREATE TABLE " << TABLE_NAME_MINMAX
    //       << " (variable_name VARCHAR(255), object_name VARCHAR(255), min VARCHAR(255), max "
    //          "VARCHAR(255),"
    //          "PRIMARY KEY (variable_name, object_name));";
    //    table_minmax_create_statement_ = ss.str();
    //    ss.str(string());

    ss << "CREATE TABLE " << TABLE_NAME_PROPERTIES
       << "(id VARCHAR(255), value TEXT, PRIMARY KEY (id));";
    table_properties_create_statement_ = ss.str();
    ss.str(string());

    ss << "CREATE TABLE " << TABLE_NAME_SECTORS
       << "(id INT, name VARCHAR(255), layer_name VARCHAR(255), json TEXT, PRIMARY KEY (id));";
    table_sectors_create_statement_ = ss.str();
    ss.str(string());

    ss << "CREATE TABLE " << TABLE_NAME_VIEWPOINTS
       << "(id INT, json TEXT, PRIMARY KEY (id));";
    table_view_points_create_statement_ = ss.str();
    ss.str(string());
}

SQLGenerator::~SQLGenerator() {}

string SQLGenerator::getCreateTableStatement(const DBObject& object)
{
    stringstream ss;

    //    CREATE TABLE contacts (
    //     contact_id integer PRIMARY KEY,
    //     first_name text NOT NULL,
    //     last_name text NOT NULL,
    //     email text NOT NULL UNIQUE,
    //     phone text NOT NULL UNIQUE
    //    );

    // sqlite
    //    if (data_type == "BOOLEAN")
    //        data_type = "BOOL";
    //    else if (data_type == "TEXT")
    //        data_type = "STRING";
    //    else if (data_type == "REAL")
    //        data_type = "DOUBLE";
    //    else if (data_type == "INTEGER")
    //        data_type = "INT";
    // mysql same

    ss << "CREATE TABLE " << object.dbTableName() << "(";

    string data_type;

    unsigned int cnt = 0;
    for (auto& var_it : object.variables())
    {
        ss << var_it->dbColumnName();

        data_type = var_it->dbDataTypeString();

        if (var_it->isKey())
        {
            if (data_type == "int")  // mysql defaults autoincrement
                ss << " INTEGER PRIMARY KEY AUTOINCREMENT";
            else
                ss << " " << data_type << " PRIMARY KEY";

            ss << " NOT NULL";
        }
        else
            ss << " " << data_type;

        if (cnt != object.numVariables() - 1)
            ss << ",";

        cnt++;
    }

    ss << ");";

    loginf << "SQLGenerator: getCreateTableStatement: sql '" << ss.str() << "'";
    return ss.str();
}

shared_ptr<DBCommand> SQLGenerator::getDataSourcesSelectCommand()
{
    using namespace DBContent;

    PropertyList list;
    list.addProperty(DBDataSource::id_column_);
    list.addProperty(DBDataSource::ds_type_column_);
    list.addProperty(DBDataSource::sac_column_);
    list.addProperty(DBDataSource::sic_column_);
    list.addProperty(DBDataSource::name_column_);
    list.addProperty(DBDataSource::short_name_);
    list.addProperty(DBDataSource::info_column_);
    list.addProperty(DBDataSource::counts_column_);

    shared_ptr<DBCommand> command = make_shared<DBCommand>(DBCommand());

    stringstream ss;

    ss << "SELECT ";

    bool first = true;

    for (const auto& prop_it : list.properties())
    {
        if (!first)
            ss << ",";

        ss << " " << prop_it.name();

        first = false;
    }

    ss << " FROM ";

    ss << DBDataSource::table_name_;

    ss << ";";

    command->set(ss.str());
    command->list(list);

    return command;
}

//shared_ptr<DBCommand> SQLGenerator::getDistinctDataSourcesSelectCommand(DBObject& object)
//{
//    // "SELECT DISTINCT sensor_number__value FROM " << table_names_.at(DBO_PLOTS) << " WHERE
//    // mapped_position__present = '1' AND sensor_number__present = '1' ORDER BY
//    // sensor_number__value;";
//    // return distinct_radar_numbers_statement_;

//    assert (false); // TODO

//    //    string local_key_dbovar = object.currentDataSourceDefinition().localKey();
//    //    assert(object.hasVariable(local_key_dbovar));
//    //    const DBTableColumn& local_key_col = object.variable(local_key_dbovar).currentDBColumn();

//    //    vector<const DBTableColumn*> columns;
//    //    columns.push_back(&local_key_col);

//    //    PropertyList list;
//    //    list.addProperty(local_key_col.name(), PropertyDataType::INT);

//    //    return getSelectCommand(object.currentMetaTable(), columns, true);
//}

shared_ptr<DBCommand> SQLGenerator::getADSBInfoCommand(DBObject& adsb_obj)
{

    PropertyList list;
    list.addProperty("TARGET_ADDR", PropertyDataType::INT);
    list.addProperty("MOPS_VERSION", PropertyDataType::INT);
    list.addProperty("MIN_NUCP_NIC", PropertyDataType::CHAR);
    list.addProperty("MAX_NUCP_NIC", PropertyDataType::CHAR);
    list.addProperty("MIN_NACP", PropertyDataType::CHAR);
    list.addProperty("MAX_NACP", PropertyDataType::CHAR);

    shared_ptr<DBCommand> command = make_shared<DBCommand>(DBCommand());

    stringstream ss;

    ss << "SELECT TARGET_ADDR, MOPS_VERSION, MIN(NUCP_NIC), MAX(NUCP_NIC), MIN(NAC_P), MAX(NAC_P)";

    ss << " FROM ";

    ss << adsb_obj.dbTableName();

    ss << " group by TARGET_ADDR;";

    command->set(ss.str());
    command->list(list);

    return command;
}

string SQLGenerator::getCreateAssociationTableStatement(const string& table_name)
{
    stringstream ss;

    ss << "CREATE TABLE " << table_name
       << " (assoc_id INTEGER PRIMARY KEY AUTOINCREMENT, rec_num INTEGER, utn INTEGER, src_rec_num "
          "INTEGER, ds_id INTEGER);";

    return ss.str();
}

shared_ptr<DBCommand> SQLGenerator::getSelectAssociationsCommand(const string& table_name)
{
    shared_ptr<DBCommand> command = make_shared<DBCommand>(DBCommand());

    stringstream ss;

    ss << "SELECT assoc_id, rec_num, utn, src_rec_num FROM " << table_name;

    PropertyList property_list;
    property_list.addProperty("assoc_id", PropertyDataType::INT);
    property_list.addProperty("rec_num", PropertyDataType::INT);
    property_list.addProperty("utn", PropertyDataType::INT);
    property_list.addProperty("src_rec_num", PropertyDataType::INT);

    command->set(ss.str());
    command->list(property_list);

    return command;
}

string SQLGenerator::getCountStatement(const string& table)
{
    return "SELECT COUNT(*) FROM " + table + ";";
}

shared_ptr<DBCommand> SQLGenerator::getTableSelectMinMaxNormalStatement(const DBObject& object)
{
    logdbg << "SQLGenerator: getTableSelectMinMaxNormalStatement: start for table " << object.dbTableName();

    stringstream ss;

    shared_ptr<DBCommand> command(new DBCommand());
    assert(command);

    PropertyList command_list;

    ss << "SELECT ";

    bool first = true;

    for (auto& var_it : object.variables())
    {
        logdbg << "SQLGenerator: getTableSelectMinMaxNormalStatement: current name "
               << var_it->name();

        if (!first)
            ss << ",";

        ss << "MIN(" << var_it->dbColumnName() << "),MAX(" << var_it->dbColumnName() << ")";

        command_list.addProperty(var_it->dbColumnName() + "MIN", PropertyDataType::STRING);
        command_list.addProperty(var_it->dbColumnName() + "MAX", PropertyDataType::STRING);

        first = false;
    }

    ss << " FROM " << object.dbTableName() << ";";

    command->set(ss.str());
    command->list(command_list);

    logdbg << "SQLGenerator: getTableSelectMinMaxNormalStatement: sql '" << ss.str() << "'";

    return command;
}

string SQLGenerator::getInsertPropertyStatement(const string& id,
                                                const string& value)
{
    stringstream ss;
    assert(id.size() < 255);
    if (value.size() > 1000)
        logwrn << "SQLGenerator: getInsertPropertyStatement: value size very large ("
               << value.size() << ")";

    // REPLACE into table (id, name, age) values(1, "A", 19)
    ss << "REPLACE INTO " << TABLE_NAME_PROPERTIES << " VALUES ('" << id << "', '" << value
       << "');";
    return ss.str();
}
string SQLGenerator::getSelectPropertyStatement(const string& id)
{
    stringstream ss;
    ss << "SELECT value FROM " << TABLE_NAME_PROPERTIES << " WHERE id = '" << id << "';";
    return ss.str();
}

string SQLGenerator::getSelectAllPropertiesStatement()
{
    stringstream ss;
    ss << "SELECT id, value FROM " << TABLE_NAME_PROPERTIES << ";";
    return ss.str();
}

string SQLGenerator::getInsertViewPointStatement(const unsigned int id, const string& json)
{
    stringstream ss;

    // REPLACE into table (id, name, age) values(1, "A", 19)
    ss << "REPLACE INTO " << TABLE_NAME_VIEWPOINTS << " VALUES ('" << id << "', '" << json
       << "');";
    return ss.str();
}

string SQLGenerator::getSelectAllViewPointsStatement()
{
    stringstream ss;
    ss << "SELECT id, json FROM " << TABLE_NAME_VIEWPOINTS << ";";
    return ss.str();
}

string SQLGenerator::getReplaceSectorStatement(const unsigned int id, const string& name,
                                               const string& layer_name, const string& json)
{
    stringstream ss;

    // REPLACE into table (id, name, age) values(1, "A", 19)
    ss << "REPLACE INTO " << TABLE_NAME_SECTORS << " VALUES ('" << id << "', '" << name << "', '"
       << layer_name << "', '" << json << "');";
    return ss.str();
}

string SQLGenerator::getSelectAllSectorsStatement()
{
    stringstream ss;
    ss << "SELECT id, name, layer_name, json FROM " << TABLE_NAME_SECTORS << ";";
    return ss.str();
}

//string SQLGenerator::getInsertMinMaxStatement(const string& variable_name,
//                                              const string& object_name,
//                                              const string& min, const string& max)
//{
//    stringstream ss;
//    ss << "REPLACE INTO " << TABLE_NAME_MINMAX << " VALUES ('" << variable_name << "', '"
//       << object_name << "', '" << min << "', '" << max << "');";
//    return ss.str();
//}
//string SQLGenerator::getSelectMinMaxStatement(const string& variable_name,
//                                              const string& object_name)
//{
//    stringstream ss;
//    ss << "SELECT min,max FROM " << TABLE_NAME_MINMAX << " WHERE variable_name = '" << variable_name
//       << "' AND object_name = '" << object_name << "';";
//    return ss.str();
//}

//string SQLGenerator::getSelectMinMaxStatement()
//{
//    stringstream ss;
//    ss << "SELECT variable_name,object_name,min,max FROM " << TABLE_NAME_MINMAX << ";";
//    return ss.str();
//}

string SQLGenerator::getSelectNullCount (const string& table_name, const vector<string> columns)
{
    stringstream ss;
    ss << "SELECT count(*) FROM " << table_name << " WHERE";

    bool first = true;
    for (auto& col_it : columns)
    {
        if (!first)
            ss << " OR";

        ss << " " << col_it << " IS NULL";

        first = false;
    }
    ss << ";";

    return ss.str();
}

//string SQLGenerator::getTableMinMaxCreateStatement() { return table_minmax_create_statement_; }

string SQLGenerator::getTablePropertiesCreateStatement()
{
    return table_properties_create_statement_;
}

string SQLGenerator::getTableSectorsCreateStatement()
{
    return table_sectors_create_statement_;
}

string SQLGenerator::getTableViewPointsCreateStatement()
{
    return table_view_points_create_statement_;
}


string SQLGenerator::insertDBUpdateStringBind(shared_ptr<Buffer> buffer,
                                              string tablename)
{
    assert(buffer);
    assert(tablename.size() > 0);

    const vector<Property>& properties = buffer->properties().properties();

    // INSERT INTO table_name (column1, column2, column3, ...) VALUES (value1, value2, value3, ...);

    unsigned int size = properties.size();
    logdbg << "SQLGenerator: insertDBUpdateStringBind: creating db string";
    stringstream ss;  // create a stringstream

    ss << "INSERT INTO " << tablename << " (";

    stringstream values_ss;
    values_ss << "VALUES (";

    for (unsigned int cnt = 0; cnt < size; cnt++)
    {
        ss << properties.at(cnt).name();

        values_ss << "@VAR" + to_string(cnt + 1);

        if (cnt != size - 1)
        {
            ss << ", ";
            values_ss << ", ";
        }
    }

    ss << ") " << values_ss.str() << ");";

    logdbg << "SQLGenerator: insertDBUpdateStringBind: var insert string '" << ss.str() << "'";

    return ss.str();
}

string SQLGenerator::createDBUpdateStringBind(shared_ptr<Buffer> buffer,
                                              const string& key_col_name,
                                              string table_name)
{
    assert(buffer);
    assert(table_name.size() > 0);

    const vector<Property>& properties = buffer->properties().properties();

    // UPDATE table_name SET col1=val1,col2=value2 WHERE somecol=someval;

    unsigned int size = properties.size();
    logdbg << "SQLGenerator: createDBUpdateStringBind: creating db string";
    stringstream ss;  // create a stringstream

    logdbg << "SQLGenerator: createDBUpdateStringBind: idvar name " << key_col_name;

    ss << "UPDATE " << table_name << " SET ";

    if (key_col_name != properties.at(size - 1).name())
        throw runtime_error(
                "SQLGenerator: createDBUpdateStringBind: id var not at last position");

    for (unsigned int cnt = 0; cnt < size; cnt++)
    {
        if (key_col_name == properties.at(cnt).name())
        {
            if (cnt == size - 1)
                continue;
            else
                throw runtime_error(
                        "SQLGenerator: createDBUpdateStringBind: id var at other than last position " +
                        to_string(cnt));
        }
        ss << properties.at(cnt).name() << "=";

        ss << "@VAR" + to_string(cnt + 1);

        if (cnt != size - 2)
        {
            ss << ", ";
        }
    }

    ss << " WHERE " << key_col_name << "=";

    ss << "@VAR" << to_string(size);

    ss << ";";

    logdbg << "SQLGenerator: createDBUpdateStringBind: var update string '" << ss.str() << "'";

    return ss.str();
}


shared_ptr<DBCommand> SQLGenerator::getSelectCommand(
        const DBObject& object, DBOVariableSet read_list, const string& filter, bool use_order,
        DBOVariable* order_variable, bool use_order_ascending, const string& limit)
{
    logdbg << "SQLGenerator: getSelectCommand: dbo " << object.name()
           << " read list size " << read_list.getSize();
    assert(read_list.getSize() != 0);

    shared_ptr<DBCommand> command = make_shared<DBCommand>(DBCommand());

    stringstream ss;

    ss << "SELECT ";

    logdbg << "SQLGenerator: getSelectCommand: collecting required variables";

    PropertyList property_list;

    string column_name;
    string table_db_name = object.dbTableName();

    bool first = true;
    for (auto var_it : read_list.getSet())
        // look what tables are needed for loaded variables and add variables to sql query
    {
        DBOVariable* variable = var_it;

        if (!first)
            ss << ", ";

        column_name = variable->dbColumnName();

        ss << table_db_name << "." << column_name;

        property_list.addProperty(column_name, variable->dataType());

        first = false;
    }

    ss << " FROM " << table_db_name;  // << table->getAllTableNames();




    logdbg << "SQLGenerator: getSelectCommand: filterting statement";
    // add filter statement
    if (filter.size() > 0)
        ss << " WHERE "  << filter;

    if (use_order)
    {
        assert(order_variable);

        ss << " ORDER BY " << order_variable->dbColumnName();

        if (use_order_ascending)
            ss << " ASC";
        else
            ss << " DESC";
    }

    if (limit.size() > 0)
    {
        ss << " LIMIT " << limit;
        logdbg << "SQLGenerator: getSelectCommand: limiting";
    }

    ss << ";";

    command->set(ss.str());
    command->list(property_list);

    logdbg << "SQLGenerator: getSelectCommand: command sql '" << ss.str() << "'";

    return command;
}

//shared_ptr<DBCommand> SQLGenerator::getSelectCommand(const DBObject& object,
//                                                     const vector<string>& columns,
//                                                     bool distinct)
//{
//        logdbg << "SQLGenerator: getSelectCommand: meta table " << meta_table.name()
//               << " db columns size " << columns.size();
//        assert(columns.size() != 0);

//        shared_ptr<DBCommand> command = make_shared<DBCommand>(DBCommand());

//        stringstream ss;

//        ss << "SELECT ";

//        if (distinct)
//            ss << "DISTINCT ";

//        vector<string> used_tables;

//        logdbg << "SQLGenerator: getSelectCommand: collecting required variables";

//        PropertyList property_list;

//        bool first = true;
//        for (auto col_it : columns)
//            // look what tables are needed for loaded variables and add variables to sql query
//        {
//            if (property_list.hasProperty(col_it->name())) // for already added ones
//                continue;

//            if (!first)
//                ss << ", ";

//            string table_db_name = col_it->table().name();

//            if (find(used_tables.begin(), used_tables.end(), table_db_name) == used_tables.end())
//                used_tables.push_back(table_db_name);

//            ss << table_db_name << "." << col_it->name();

//            property_list.addProperty(col_it->name(), col_it->propertyType());

//            first = false;
//        }

//        ss << " FROM ";

//        string main_table_name = meta_table.mainTableName();

//        logdbg << "SQLGenerator: getSelectCommand: left join query";
//        //    SELECT news.id, users.username, news.title, news.date, news.body, COUNT(comments.id)
//        //    FROM news
//        //    LEFT JOIN users
//        //    ON news.user_id = users.id
//        //    LEFT JOIN comments
//        //    ON comments.news_id = news.id
//        //    GROUP BY news.id
//        ss << main_table_name;

//        assert(used_tables.size() > 0);
//        vector<string>::iterator it;
//        for (it = used_tables.begin(); it != used_tables.end(); it++)
//        {
//            if (it->compare(main_table_name) != 0)  // not main table
//            {
//                ss << " LEFT JOIN " << *it;
//                ss << " ON " << subTableKeyClause(meta_table, *it);
//            }
//        }

//        ss << ";";

//        command->set(ss.str());
//        command->list(property_list);

//        logdbg << "SQLGenerator: getSelectCommand: command sql '" << ss.str() << "'";

//        return command;
//}

//string SQLGenerator::subTablesWhereClause(const DBTable& table,
//                                               const vector<string>& used_tables)
//{
//    assert (false); // TODO
//    stringstream ss;

//    bool first = true;

//    for (auto it : meta_table.subTableDefinitions())
//    {
//        if (find(used_tables.begin(), used_tables.end(), it.second->subTableName()) ==
//                used_tables.end())
//            continue;

//        if (!first)
//            ss << " AND ";

//        ss << meta_table.mainTableName() << "." << it.second->mainTableKey() << "="
//           << it.second->subTableName() << "." << it.second->subTableKey();
//        first = false;
//    }

//    return ss.str();
//}

//string SQLGenerator::subTableKeyClause(const DBTable& table,
//                                            const string& sub_table_name)
//{
//    assert (false); // TODO
//    if (meta_table.subTableDefinitions().count(sub_table_name) == 0)
//        throw range_error("SQLGenerator: getSubTableKeyClause: sub_table_name " +
//                               sub_table_name + " not found");

//    auto subtable = meta_table.subTableDefinitions().at(sub_table_name);

//    // found subtable
//    stringstream ss;

//    ss << meta_table.mainTableName() << "." << subtable->mainTableKey() << "="
//       << subtable->subTableName() << "." << subtable->subTableKey();

//    return ss.str();
//}

string SQLGenerator::getDeleteStatement (const string& table, const string& filter)
{
    // DELETE FROM table_name [WHERE Clause]
    return "DELETE FROM "+table+" WHERE "+filter+";";
}


