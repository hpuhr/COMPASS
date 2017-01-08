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
 * SQLGenerator.cpp
 *
 *  Created on: Jan 11, 2012
 *      Author: sk
 */

#include <algorithm>
#include <string>
#include <iomanip>
#include "Buffer.h"
#include "DBCommandList.h"
#include "DBCommand.h"
#include "DBInterface.h"
#include "DBObject.h"
#include "ATSDB.h"
#include "DBOVariable.h"
#include "FilterManager.h"
#include "Logger.h"
#include "SQLGenerator.h"
#include "PropertyList.h"
//#include "StructureDescriptionManager.h"
#include "DBTableColumn.h"
#include "DBTable.h"
#include "MetaDBTable.h"
#include "DBSchema.h"
#include "String.h"

using namespace Utils;
using namespace std;

SQLGenerator::SQLGenerator(const DBInterface &db_interface)
    : db_interface_(db_interface)
{


    db_type_set_=false;

    table_name_properties_ = "aatc_properties";
    table_name_minxmax_ = "aatc_minmax";

    std::stringstream ss;

    ss << "CREATE TABLE " << table_name_minxmax_ << " (id VARCHAR(255), dbo_type INT, min VARCHAR(255), max VARCHAR(255),"
            "PRIMARY KEY (id, dbo_type));";
    table_minmax_create_statement_ = ss.str();
    ss.str(std::string());

    ss << "CREATE TABLE " << table_name_properties_ << "(id VARCHAR(255), value VARCHAR(1701), PRIMARY KEY (id));";
    table_properties_create_statement_ = ss.str();
    ss.str(std::string());
}

SQLGenerator::~SQLGenerator()
{
}

//DBCommand *SQLGenerator::getSelectCommand(const std::string &dbo_type, const DBOVariableSet &read_list, const std::string &custom_filter_clause,
//        DBOVariable *order)
//{
//    assert (ATSDB::getInstance().existsDBObject(dbo_type));
//    //DBOVariableSet &read_list = ATSDB::getInstance().getDBObject(type)->getReadList();
//    const MetaDBTable &table = ATSDB::getInstance().getDBObject(dbo_type).getCurrentMetaTable ();

//// AVIBIT HACK
////    if (order == 0)
////    {
////        order = ATSDB::getInstance().getDBOVariable (DBO_UNDEFINED, "id");
////        assert (order->existsIn (type));
//    //std::string idname = order->getFor (type)->getCurrentVariableName();
////    }


//    std::vector <std::string> filtered_variable_names; // what


//    if (custom_filter_clause.size() == 0)
//    {
//        loginf << "SQLGenerator: getSelectCommand: getting filter clause from FilterManager";
//        custom_filter_clause = FilterManager::getInstance().getSQLCondition(dbo_type, filtered_variable_names);
//    }

//    //return getSelectCommand (read_list.getPropertyList(type), table, filtered_variable_names, custom_filter_clause,idname);
//    return getSelectCommand (read_list.getPropertyList(), table, filtered_variable_names, custom_filter_clause, "",
//            "", false, true);
//    //(PropertyList variables, MetaDBTable *table,
//    //    std::vector <std::string> &filtered_variable_names, std::string filter, std::string order, std::string limit,
//    //    bool distinct, bool left_join)

//}

//DBCommand *SQLGenerator::getDataSourcesSelectCommand (const std::string &dbo_type)
//{
//    assert (ATSDB::getInstance().existsDBObject(dbo_type));
//    assert (ATSDB::getInstance().getDBObject(dbo_type)->hasCurrentDataSource ());



//    DBODataSourceDefinition *ds = ATSDB::getInstance().getDBObject(dbo_type)->getCurrentDataSource ();
//    assert (DBSchemaManager::getInstance().getCurrentSchema()->hasMetaTable(ds->getMetaTableName()));

//    MetaDBTable *meta =  DBSchemaManager::getInstance().getCurrentSchema()->getMetaTable(ds->getMetaTableName());
//    assert (meta->hasTableColumn(ds->getForeignKey()));
//    assert (meta->hasTableColumn(ds->getNameColumn()));

//    PropertyList list;
//    list.addProperty (ds->getForeignKey(), PropertyDataType::INT);
//    list.addProperty (ds->getNameColumn(), PropertyDataType::STRING); //DS_NAME SAC SIC
//    list.addProperty ("DS_NAME", PropertyDataType::STRING);
//    list.addProperty ("SAC", PropertyDataType::UCHAR);
//    list.addProperty ("SIC", PropertyDataType::UCHAR);
//    list.addProperty ("POS_LAT_DEG", PropertyDataType::DOUBLE);
//    list.addProperty ("POS_LONG_DEG", PropertyDataType::DOUBLE);

//    assert (false);
//    // TODO height hack to be resolved
////    if (type == DBO_PLOTS)
////        list.addProperty ("GROUND_ALTITUDE_AMSL_M", PropertyDataType::DOUBLE);
////    else if (type == DBO_SYSTEM_TRACKS || type == DBO_MLAT)
////        list.addProperty ("WGS_ELEV_CARTESIAN_M", PropertyDataType::DOUBLE);

//    std::vector <std::string> filtered_variable_names; // what
//    return getSelectCommand (list, meta, filtered_variable_names);
//}

//DBCommand *SQLGenerator::getDistinctDataSourcesSelectCommand (const std::string &dbo_type)
//{
//    // "SELECT DISTINCT sensor_number__value FROM " << table_names_.at(DBO_PLOTS) << " WHERE mapped_position__present = '1' AND sensor_number__present = '1' ORDER BY sensor_number__value;";
//    //return distinct_radar_numbers_statement_;

//    assert (ATSDB::getInstance().existsDBObject(dbo_type));
//    assert (ATSDB::getInstance().getDBObject(dbo_type)->hasCurrentDataSource());

//    DBODataSourceDefinition *ds = ATSDB::getInstance().getDBObject(dbo_type)->getCurrentDataSource ();
//    std::string sensor_id = ds->getLocalKey();

//    MetaDBTable *table = ATSDB::getInstance().getDBObject(dbo_type)->getCurrentMetaTable ();

//    PropertyList list;
//    list.addProperty (sensor_id, PropertyDataType::INT);

//    std::vector <std::string> filtered_variable_names; // what
//    return getSelectCommand (list, table, filtered_variable_names, "", sensor_id, "", true);
//}

//DBCommand *SQLGenerator::getCountStatement (const std::string &dbo_type, unsigned int sensor_number)
//{
//    assert (ATSDB::getInstance().existsDBObject(dbo_type));
//    DBObject *object = ATSDB::getInstance().getDBObject(dbo_type);
//    assert (object);
//    assert (object->hasCurrentDataSource());

//    std::string data_source_key = object->getCurrentDataSource()->getLocalKey();

//    std::string table_name = object->getCurrentMetaTable()->getTableDBName();

//    PropertyList list;
//    list.addProperty ("count", PropertyDataType::UINT);

//    DBCommand *command = new DBCommand ();
//    command->setPropertyList(list);
//    command->setCommandString("SELECT COUNT(*) FROM " + table_name + " WHERE "+data_source_key+"="+String::intToString(sensor_number));

//    return command;
//}

//DBCommand *SQLGenerator::getDistinctStatistics (const std::string &dbo_type, DBOVariable *variable, unsigned int sensor_number)
//{
//    assert (ATSDB::getInstance().existsDBObject(dbo_type));
//    DBObject *object = ATSDB::getInstance().getDBObject(dbo_type);
//    assert (object);
//    assert (object->hasCurrentDataSource());

//    assert (variable->existsIn(dbo_type));

//    DBOVariable *typevar = variable->getFor(dbo_type);

//    assert (ATSDB::getInstance().getDBObject(DBO_UNDEFINED)->hasVariable("frame_time"));
//    DBOVariable *metatimevar = ATSDB::getInstance().getDBObject(DBO_UNDEFINED)->getVariable("frame_time");
//    assert (metatimevar->existsIn(type));
//    DBOVariable *timevar = metatimevar->getFor(type);

//    std::string data_source_key = object->getCurrentDataSource()->getLocalKey();

//    std::string table_name = object->getCurrentMetaTable()->getTableDBName();

//    std::string distinctvarname = typevar->getCurrentVariableName();
//    std::string timevarname = timevar->getCurrentVariableName();

//    PropertyList list;
//    list.addProperty ("element", typevar->getDataType());
//    list.addProperty ("count", PropertyDataType::UINT);
//    list.addProperty ("time_min", typevar->getDataType());
//    list.addProperty ("time_max", typevar->getDataType());

//    DBCommand *command = new DBCommand ();
////    command->setPropertyList(list);
////    command->setCommandString("SELECT "+distinctvarname+",COUNT(*), MIN("+timevarname+"), MAX("+timevarname+")"
////            " FROM " + table_name + " WHERE "+data_source_key+"="+intToString(sensor_number)+ " GROUP BY "+distinctvarname);

//    return command;
//}

//DBCommand *SQLGenerator::getSelectInfoCommand(const std::string &dbo_type, std::vector<unsigned int> ids,
//        DBOVariableSet read_list, bool use_filters,
//        std::string order_by_variable, bool ascending, unsigned int limit_min, unsigned int limit_max)
//{
//    assert (ATSDB::getInstance().existsDBObject (DBO_UNDEFINED));
//    assert (ATSDB::getInstance().existsDBOVariable (DBO_UNDEFINED, "id"));

//    DBOVariable *idvar = ATSDB::getInstance().getDBOVariable (DBO_UNDEFINED, "id");
//    assert (idvar->existsIn (type));

//    assert (ATSDB::getInstance().existsDBObject(type));
//    MetaDBTable *table = ATSDB::getInstance().getDBObject(type)->getCurrentMetaTable ();
//    std::vector <std::string> filtered_variable_names;

//    std::stringstream filter;

//    if (ids.size() != 0)
//    {
//        filter << "(";

//        std::vector<unsigned int>::iterator it;
//        for ( it = ids.begin(); it != ids.end(); it++)
//        {
//            if (it != ids.begin())
//                filter << " OR ";


//            filter <<  table->getTableDBName()<< "." << idvar->getFor(type)->id_+"='"+intToString(*it)+"'";
//        }

//        filter << ")";
//    }

//    if (use_filters)
//    {
//        std::string filter_condition = FilterManager::getInstance().getSQLCondition(type, filtered_variable_names);

//        if (filter_condition.size() != 0)
//        {
//            if (ids.size() != 0)
//                filter << " AND ";

//            filter << filter_condition;
//        }
//    }

//    std::string order;
//    if (order_by_variable.size() != 0)
//    {
//        order = order_by_variable;
//        if (ascending)
//            order += " ASC";
//        else
//            order += " DESC";
//    }

//    std::string limit;
//    if (limit_min != limit_max)
//    {
//        assert (limit_min < limit_max);
//        limit = " LIMIT " + intToString(limit_min)+", "+ intToString(limit_max);
//    }

//    DBCommand *command = getSelectCommand (read_list.getPropertyList(type), table, filtered_variable_names, filter.str(), order, limit, false, true);

//    logdbg  << "SQLGenerator: getSelectInfoCommand: type " << ATSDB::getInstance().getDBObject(type)->getName() << " sql '" << command->getCommandString() << "'";
//}

//std::string SQLGenerator::getContainsStatement (std::string table_name)
//{
//    if (!db_type_set_)
//    {
//        db_type_=db_interface_->getDBInfo()->getType();
//        db_type_set_=true;
//    }

////    if (db_type_ == DB_TYPE_SQLITE)
////        return "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='" + table_name + "';";
////    else  || db_type_ == DB_TYPE_MYSQLCon
//    if (db_type_ == DB_TYPE_MYSQLpp)
//    {
//        //return "SELECT COUNT(*)  FROM information_schema.tables WHERE table_schema = '"+db_interface_->getDatabaseName()+"' AND table_name ='" + table_name + "';";
//        return "SHOW TABLES LIKE '"+table_name+"';";
//    }
//    else
//        throw std::runtime_error ("SQLGenerator: getContainsStatement: unknown db type");

//}

//std::string SQLGenerator::getCountStatement (const std::string &dbo_type)
//{
//    assert (ATSDB::getInstance().existsDBObject(dbo_type));

//    std::string table_name = ATSDB::getInstance().getDBObject(dbo_type)->getCurrentMetaTable()->getTableDBName();

//    return "SELECT COUNT(*) FROM " + table_name;
//}

//DBCommand *SQLGenerator::getTableSelectMinMaxNormalStatement (std::string table_name)
//{
//    logdbg  << "SQLGenerator: getTableSelectMinMaxNormalStatement: start for table " << table_name;

//    assert (DBSchemaManager::getInstance().getCurrentSchema()->hasTable(table_name));
//    std::map <std::string, DBTableColumn *> &columns =  DBSchemaManager::getInstance().getCurrentSchema()->getTable(table_name)->getColumns ();
//    std::map <std::string, DBTableColumn *>::iterator it;

//    stringstream ss;

//    DBCommand *command = new DBCommand ();
//    assert (command);

//    PropertyList command_list;

//    ss << "SELECT ";

//    DBTableColumn *column;
//    bool first=true;

//    for (it = columns.begin(); it != columns.end(); it++)
//    {
//        column = it->second;
//        if (column->hasSpecialNull())
//            continue;

//        logdbg  << "SQLGenerator: getMainTableSelectMinMaxStatement: variable " << column->getName();

//        std::string current_name = column->getName ();

//        logdbg  << "SQLGenerator: getMainTableSelectMinMaxStatement: current name " << current_name;

//        if (!first)
//            ss << ",";

//        ss << "MIN("<< current_name <<"),MAX("<< current_name <<")";

//        command_list.addProperty(current_name+"MIN", PropertyDataType::STRING);
//        command_list.addProperty(current_name+"MAX", PropertyDataType::STRING);

//        first = false;
//    }

//    ss << " FROM " << DBSchemaManager::getInstance().getCurrentSchema()->getTable(table_name)->getDBName();

//    command->setCommandString (ss.str());
//    command->setPropertyList(command_list);

//    //delete list;

//    logdbg  << "SQLGenerator: getMainTableSelectMinMaxStatement: sql '" << ss.str() << "'";

//    return command;
//}

//DBCommand *SQLGenerator::getColumnSelectMinMaxStatement (DBTableColumn *column, std::string table_name)
//{
//    stringstream ss;

//    DBCommand *command = new DBCommand ();
//    assert (command);

//    PropertyList command_list;

//    ss << "SELECT ";

//    //DBTableColumn *column;
//    //bool first=true;
//    std::string column_name = column->getName();

//    ss << "MIN("<< column_name <<"),MAX("<< column_name <<")";

//    command_list.addProperty(column_name+"MIN", PropertyDataType::STRING);
//    command_list.addProperty(column_name+"MAX", PropertyDataType::STRING);

//    ss << " FROM " << DBSchemaManager::getInstance().getCurrentSchema()->getTable(table_name)->getDBName();

//    if (column->hasSpecialNull())
//        ss << " WHERE " << column_name << " != " << column->getSpecialNull();

//    command->setCommandString (ss.str());
//    command->setPropertyList(command_list);

//    //delete list;

//    loginf  << "SQLGenerator: getMainTableSelectMinMaxStatement: sql '" << ss.str() << "'";

//    return command;

//}

//std::string SQLGenerator::getInsertPropertyStatement (std::string id, std::string value)
//{
//    stringstream ss;
//    //ss << "INSERT INTO " << table_name_properties_ << " VALUES ('" << id <<"', '" << value <<"');";
//    // REPLACE into table (id, name, age) values(1, "A", 19)
//    ss << "REPLACE INTO " << table_name_properties_ << " VALUES ('" << id <<"', '" << value <<"');";
//    return ss.str();
//}
//std::string SQLGenerator::getSelectPropertyStatement (std::string id)
//{
//    stringstream ss;
//    ss << "SELECT value FROM " << table_name_properties_ << " WHERE id = '" << id << "'";
//    return ss.str();
//}

//std::string SQLGenerator::getInsertMinMaxStatement (std::string id, const std::string &dbo_type, std::string min, std::string max)
//{
//    stringstream ss;
//    //ss << "INSERT INTO " << table_name_minxmax_<< " VALUES ('" << id <<"', '" << type <<"', '" << min<< "', '"<< max <<"');";
//    ss << "REPLACE INTO " << table_name_minxmax_ << " VALUES ('" << id <<"', '" << dbo_type <<"', '" << min<< "', '"<< max <<"');";
//    return ss.str();
//}
//std::string SQLGenerator::getSelectMinMaxStatement (std::string id, const std::string &dbo_type)
//{
//    stringstream ss;
//    ss << "SELECT min,max FROM " << table_name_minxmax_<< " WHERE id = '" << id << "' AND dbo_type = '" << dbo_type << "'";
//    return ss.str();
//}

//std::string SQLGenerator::getSelectMinMaxStatement ()
//{
//    stringstream ss;
//    ss << "SELECT id,dbo_type,min,max FROM " << table_name_minxmax_;
//    return ss.str();
//}

//std::string SQLGenerator::getTableMinMaxCreateStatement ()
//{
//    return table_minmax_create_statement_;
//}
//std::string SQLGenerator::getTablePropertiesCreateStatement ()
//{
//    return table_properties_create_statement_;
//}

//std::string SQLGenerator::createDBInsertStringBind(Buffer *buffer, std::string tablename)
//{
//    assert (buffer);
//    assert (tablename.size() > 0);

//    const PropertyList &list = buffer->properties();

//    unsigned int size = list.size();
//    logdbg  << "SQLGenerator: createDBInsertStringBind: creating db string";
//    std::stringstream ss;//create a stringstream

//    ss << "INSERT INTO " << tablename;

//    if (!db_type_set_)
//    {
//        db_type_=db_interface_->getDBInfo()->getType();
//        db_type_set_=true;
//    }

//    if (db_type_ == DB_TYPE_MYSQLpp) // || db_type_ == DB_TYPE_MYSQLCon
//    {
//        //const std::vector <Property> &properties = list.getProperties();
//        ss << "(";
//        for (unsigned int cnt=0; cnt < size; cnt++)
//        {
//            ss << list.at(cnt).getId();

//            if (cnt != size-1)
//            {
//                ss << ", ";
//            }
//        }
//        ss << ")";
//    }

////    if (db_type_ == DB_TYPE_SQLITE)
////        ss << " VALUES (@VAR0, ";
////    else   || db_type_ == DB_TYPE_MYSQLCon
//    if (db_type_ == DB_TYPE_MYSQLpp)
//        ss << " VALUES (";
//    else
//        throw std::runtime_error ("SQLGenerator: createDBCreateString: unknown db type");


//    for (unsigned int cnt=0; cnt < size; cnt++)
//    {
////        if (db_type_ == DB_TYPE_SQLITE)
////            ss << "@VAR"+String::intToString(cnt+1);
////        else
//        if (db_type_ == DB_TYPE_MYSQLpp)
//            ss << "%"+String::intToString(cnt);
////        else if (db_type_ == DB_TYPE_MYSQLCon)
////            ss << "?";
//        else
//            throw std::runtime_error ("SQLGenerator: createDBInsertStringBind: unknown database type");

//        if (cnt != size-1)
//        {
//            ss << ", ";
//        }
//    }
//    ss << ");";


//    logdbg  << "SQLGenerator: createDBInsertStringBind: tablename " << tablename << " bind str '" << ss.str() << "'";

//    return ss.str();
//}

//std::string SQLGenerator::createDBUpdateStringBind(Buffer *buffer, std::string tablename)
//{
//    assert (buffer);
//    assert (tablename.size() > 0);

//    const PropertyList &list = buffer->properties();

//    // UPDATE table_name SET col1=val1,col2=value2 WHERE somecol=someval;

//    unsigned int size = list.size();
//    logdbg  << "SQLGenerator: createDBUpdateStringBind: creating db string";
//    std::stringstream ss;//create a stringstream

//    const std::string &dbo_type = buffer->getDBOType();

//    assert (ATSDB::getInstance().existsDBOVariable(DBO_UNDEFINED, "id"));
//    DBOVariable *idvar = ATSDB::getInstance().getDBOVariable(DBO_UNDEFINED, "id");
//    assert (idvar->existsIn(dbotype));
//    std::string dboidvar_name = idvar->getFor(dbotype)->getCurrentVariableName();

//    loginf << "SQLGenerator: createDBUpdateStringBind: idvar name " << dboidvar_name;

//    ss << "UPDATE " << tablename << " SET ";

//    if (!db_type_set_)
//    {
//        db_type_=db_interface_->getDBInfo()->getType();
//        db_type_set_=true;
//    }

//    std::vector <Property*> *properties =list->getProperties ();
//    if (dboidvar_name != properties->at(size-1)->id_)
//        throw std::runtime_error ("SQLGenerator: createDBUpdateStringBind: id var not at last position");

//    if (db_type_ == DB_TYPE_SQLITE || db_type_ == DB_TYPE_MYSQLpp || db_type_ == DB_TYPE_MYSQLCon)
//    {
//        for (unsigned int cnt=0; cnt < size; cnt++)
//        {
//            if (dboidvar_name == properties->at(cnt)->id_)
//            {
//                if (cnt == size-1)
//                    continue;
//                else
//                    throw std::runtime_error ("SQLGenerator: createDBUpdateStringBind: id var at other than last position "+
//                            intToString (cnt));
//            }
//            ss << properties->at(cnt)->id_ << "=";

//            if (db_type_ == DB_TYPE_SQLITE)
//                ss << "@VAR"+intToString(cnt+1);
//            else if (db_type_ == DB_TYPE_MYSQLpp)
//                ss << "%"+intToString(cnt+1);
//            else if (db_type_ == DB_TYPE_MYSQLCon)
//                ss << "?";

//            if (cnt != size-2)
//            {
//                ss << ", ";
//            }
//        }
//    }
//    else
//        throw std::runtime_error ("SQLGenerator: createDBUpdateStringBind: not yet implemented db type "+intToString (db_type_));

//    ss << " WHERE " << dboidvar_name << "=";

//    if (db_type_ == DB_TYPE_SQLITE)
//        ss << "@VAR" << intToString (size);
//    else if (db_type_ == DB_TYPE_MYSQLpp)
//        ss << "%" << intToString (size);
//    else if (db_type_ == DB_TYPE_MYSQLCon)
//        ss << "?";

//    ss << ";";

//    loginf << "SQLGenerator: createDBUpdateStringBind: var update string '" << ss.str() << "'";

//    return ss.str();
//}

//std::string SQLGenerator::createDBCreateString (Buffer *buffer, std::string tablename)
//{
//    assert (buffer);
//    assert (tablename.size() > 0);

//    const PropertyList &list = buffer->properties();

//    unsigned int size =	list.size();

//    std::stringstream ss;//create a stringstream
//    ss << "CREATE TABLE IF NOT EXISTS "<< tablename << " (" << "id INTEGER PRIMARY KEY ";

//    if (!db_type_set_)
//    {
//        db_type_=db_interface_->getDBInfo()->getType();
//        db_type_set_=true;
//    }

////    if (db_type_ == DB_TYPE_SQLITE)
////        ss << "AUTOINCREMENT, ";
////    else  || db_type_ == DB_TYPE_MYSQLCon
//    if (db_type_ == DB_TYPE_MYSQLpp)
//        ss << "AUTO_INCREMENT, ";
//    else
//        throw std::runtime_error ("SQLGenerator: createDBCreateString: unknown db type");

//    for (unsigned int cnt=0; cnt < size; cnt++)
//    {
//        const Property &prop = list.at(cnt);

//        ss << prop.getId() << " ";

//        PropertyDataType type = prop.getDataType();

//        switch (type)
//        {
//        case PropertyDataType::BOOL:
//            ss << "BOOLEAN";
//            break;
//        case PropertyDataType::UCHAR:
//        case PropertyDataType::CHAR:
//            ss << "TINYINT";
//            break;
//        case PropertyDataType::INT:
//        case PropertyDataType::UINT:
//            ss << "INT";
//            break;
//        case PropertyDataType::STRING:
//            ss << "VARCHAR(MAX)"; //TODO should work, verify (was String::intToString(prop.))
//            break;
//        case PropertyDataType::FLOAT:
//            ss << "FLOAT";
//            break;
//        case PropertyDataType::DOUBLE:
//            ss << "DOUBLE";
//            break;
//        default:
//            throw std::runtime_error("SQLGenerator: createDBCreateString: unspecified data type "+prop.asDataTypeString());
//        }

//        if (cnt != size-1)
//            ss << ", ";
//    }

//    ss << ");";

//    logdbg  << "SQLGenerator: createDBCreateString: tablename " << tablename << " create str '" << ss.str() << "'";

//    return ss.str();

//}


//std::string SQLGenerator::getMinMaxSelectStatement (std::string variable, std::string table, std::string condition)
//{
//    std::stringstream ss;

//    assert (variable.size() > 0);
//    assert (table.size() > 0);

//    ss << "SELECT MIN("<< variable << "),MAX(" << variable << ") FROM " << table;

//    if (condition.size() != 0)
//      ss << " WHERE " << condition;

//    ss << ";";

//    return ss.str();
//}

//std::string SQLGenerator::getMinMaxSelectStatements (std::vector <std::string> variables, std::string table)
//{
//    std::stringstream ss;

//    assert (table.size() > 0);
//    assert (variables.size() > 0);

//    ss << "SELECT";

//    for (unsigned int cnt=0; cnt < variables.size(); cnt++)
//    {
//        if (cnt != 0)
//            ss << ",";
//        ss << " MIN("<< variables.at(cnt) << "),MAX(" << variables.at(cnt) << ")";
//    }
//    ss  << "FROM " << table << " WHERE ";

//    //  for (unsigned int cnt=0; cnt < variables.size(); cnt++)
//    //  {
//    //    if (cnt != 0)
//    //      ss << " AND";
//    //    ss << " " << variables.at(cnt) << " IS NOT NULL";
//    //  }


//    ss << ";";

//    return ss.str();
//}


DBCommand *SQLGenerator::getSelectCommand (const PropertyList &variables, const MetaDBTable &table,
        const std::vector <std::string> &filtered_variable_names, const std::string &filter, const std::string &order, const std::string &limit,
        bool distinct, bool left_join)
{
    logdbg  << "SQLGenerator: getSelectCommand: meta table " << table.name() << " var list size " <<
            variables.size();
    assert (variables.size() != 0);

    DBCommand *command = new DBCommand ();
    assert (command);

    std::stringstream ss;

    ss << "SELECT ";

    if (distinct)
        ss << "DISTINCT ";

    std::vector <std::string> used_tables;

    for (unsigned int cnt = 0; cnt < variables.size(); cnt++)
        // look what tables are needed for loaded variables and add variables to sql query
    {
        if (cnt != 0)
            ss << ", ";

        std::string table_db_name = table.tableDBNameForVariable(variables.at(cnt).getId());

        if (find (used_tables.begin(), used_tables.end(), table_db_name) == used_tables.end())
            used_tables.push_back (table_db_name);

        ss << table_db_name << "." << variables.at(cnt).getId();
    }

    if (order.size() > 0)
    {
        std::vector<std::string> elems;
        elems = String::split(order, ' ', elems);
        assert (elems.size() != 0);

        if (table.hasColumn (elems.at(0)))
        {

            std::string table_db_name = table.tableDBNameForVariable(elems.at(0));
            if (find (used_tables.begin(), used_tables.end(), table_db_name) == used_tables.end())
                used_tables.push_back (table_db_name);
        }
    }

    ss << " FROM "; // << table->getAllTableNames();

    bool where_added = false;
    std::string subtableclause; // for !left_join

    // find all tables needed for variables to be filtered on
    for (auto it:  filtered_variable_names)
        // look what tables are needed for filtered variables
    {
        std::string table_db_name = table.tableDBNameForVariable(it);

        if (find (used_tables.begin(), used_tables.end(), table_db_name) == used_tables.end())
            used_tables.push_back (table_db_name);
    }

    if (!left_join)
    {
        //select cmp_aa.AZIMUTH_ERROR_DEG FROM sd_radar, cmp_aa WHERE  sd_radar.REC_NUM = cmp_aa.REC_NUM
        assert (used_tables.size() > 0);
        std::vector <std::string>::iterator it;
        for (it = used_tables.begin(); it != used_tables.end(); it++)
        {
            if (it != used_tables.begin())
                ss << ", ";
            ss << *it;
        }

        assert (false); //TODO
        //subtableclause = table->getSubTablesWhereClause(used_tables); TODO nedd that again plz

        if (subtableclause.size() != 0)
        {
            logdbg  << "SQLGenerator: getSelectCommand: subtableclause '" << subtableclause << "'";

            if (!where_added)
            {
                ss << " WHERE ";
                where_added=true;
            }

            ss << subtableclause;
        }
    }
    else
    {
        //    SELECT news.id, users.username, news.title, news.date, news.body, COUNT(comments.id)
        //    FROM news
        //    LEFT JOIN users
        //    ON news.user_id = users.id
        //    LEFT JOIN comments
        //    ON comments.news_id = news.id
        //    GROUP BY news.id
        std::string main_table_name = table.tableDBName();

        ss << main_table_name;

        assert (used_tables.size() > 0);
        std::vector <std::string>::iterator it;
        for (it = used_tables.begin(); it != used_tables.end(); it++)
        {
            if (it->compare(main_table_name) != 0) // not main table
            {
                ss << " LEFT JOIN " << *it;
                //ss << " ON " << table->getSubTableKeyClause (*it);
                assert (false); //TODO
            }
        }

    }

    // add filter statement
    if (filter.size() > 0)
    {
        if (!where_added)
        {
            ss << " WHERE ";
            where_added=true;
        }

        if (!left_join && subtableclause.size() != 0)
            ss << " AND ";

        ss << filter;
    }

    if (left_join)
    {
        ss << " GROUP BY " << table.getTable().dbName() << "." << table.getTable().dbName();
    }

    if (order.size() > 0)
        ss << " ORDER BY " << order;

    if (limit.size() > 0)
        ss << " " << limit;

    ss << ";";


    command->setCommandString(ss.str());
    command->setPropertyList(variables);

    logdbg  << "SQLGenerator: getSelectCommand: command sql '" << ss.str() << "'";

    return command;
}

//std::string SQLGenerator::getDeleteStatement (DBTableColumn *column, std::string value, std::string filter)
//{
//    assert (column);
//    // DELETE FROM table_name [WHERE Clause]
//    return "DELETE FROM "+column->getDBTableName()+" WHERE "+column->getName()+"="+value+" AND "+filter+";";
//}

//std::string SQLGenerator::getUpdateStatement (DBTableColumn *column, std::string value, std::string new_value, std::string filter)
//{
//    assert (column);
//    // update students set first_name=’Suba’ where rec_id=678;
//    return "UPDATE "+column->getDBTableName()+" SET "+column->getName()+"="+new_value+" WHERE "+column->getName()+"="+value+" AND "+filter+";";
//}

//std::string SQLGenerator::getDistinctSelectStatement (DBTableColumn *column, std::string filter_condition)
//{
//    assert (column);
//     std::stringstream ss;
//     ss << "SELECT DISTINCT("+column->getName() << ") FROM " << column->getDBTableName();

//     if (filter_condition.size() != 0)
//      ss << " WHERE " << filter_condition;

//     ss << ";";

//     return ss.str();
//}

//std::string SQLGenerator::getShowDatabasesStatement ()
//{
//    return "SHOW DATABASES LIKE 'job%';"; // wow so hard so many dabes
//}

