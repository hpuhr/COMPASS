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

#include "sqlgenerator.h"

#include <algorithm>
#include <iomanip>
#include <string>

#include "atsdb.h"
#include "buffer.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "dbconnection.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "dbschema.h"
#include "dbschemamanager.h"
#include "dbtable.h"
#include "dbtablecolumn.h"
#include "filtermanager.h"
#include "logger.h"
#include "metadbtable.h"
#include "propertylist.h"
#include "stringconv.h"

using namespace Utils;
using namespace std;

SQLGenerator::SQLGenerator(DBInterface& db_interface) : db_interface_(db_interface)
{
    // db_type_set_=false;

    std::stringstream ss;

    ss << "CREATE TABLE " << TABLE_NAME_MINMAX
       << " (variable_name VARCHAR(255), object_name VARCHAR(255), min VARCHAR(255), max "
          "VARCHAR(255),"
          "PRIMARY KEY (variable_name, object_name));";
    table_minmax_create_statement_ = ss.str();
    ss.str(std::string());

    ss << "CREATE TABLE " << TABLE_NAME_PROPERTIES
       << "(id VARCHAR(255), value VARCHAR(1701), PRIMARY KEY (id));";
    table_properties_create_statement_ = ss.str();
    ss.str(std::string());

    ss << "CREATE TABLE " << TABLE_NAME_SECTORS
       << "(id INT, name VARCHAR(255), layer_name VARCHAR(255), json VARCHAR(65535), PRIMARY KEY (id));";
    table_sectors_create_statement_ = ss.str();
    ss.str(std::string());

    ss << "CREATE TABLE " << TABLE_NAME_VIEWPOINTS
       << "(id INT, json VARCHAR(65535), PRIMARY KEY (id));";
    table_view_points_create_statement_ = ss.str();
    ss.str(std::string());
}

SQLGenerator::~SQLGenerator() {}

std::string SQLGenerator::getCreateTableStatement(const DBTable& table)
{
    std::stringstream ss;

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

    ss << "CREATE TABLE " << table.name() << "(";

    std::string data_type;
    std::string connection_type = db_interface_.connection().type();

    unsigned int cnt = 0;
    for (auto& col_it : table.columns())
    {
        ss << col_it.second->name();

        data_type = col_it.second->type();

        if (connection_type == MYSQL_IDENTIFIER)
        {
            if (data_type == "varchar")
                data_type = "text";  // hacky
            else if (data_type == "enum")
                data_type = "int";  // hacky
        }

        //        if (connection_type == SQLITE_IDENTIFIER) // && connection_type != MYSQL_IDENTIFIE
        //        {
        //            if (data_type == "BOOL")
        //                data_type = "BOOLEAN";
        //            else if (data_type == "STRING")
        //                data_type = "TEXT";
        //            else if (data_type == "DOUBLE")
        //                data_type = "REAL";
        //            else if (data_type == "INT")
        //                data_type = "INTEGER";
        //        }

        if (col_it.second->isKey())
        {
            if (connection_type == SQLITE_IDENTIFIER)
            {
                if (data_type == "int")  // mysql defaults autoincrement
                    ss << " INTEGER PRIMARY KEY AUTOINCREMENT";
                else
                    ss << " " << data_type << " PRIMARY KEY";
            }
            else
            {
                assert(connection_type == MYSQL_IDENTIFIER);
                if (data_type == "int")
                    ss << " INTEGER PRIMARY KEY AUTO_INCREMENT";
                else
                    ss << " " << data_type << " PRIMARY KEY";
            }

            ss << " NOT NULL";
        }
        else
            ss << " " << data_type;

        if (cnt != table.columns().size() - 1)
            ss << ",";

        cnt++;
    }

    ss << ");";

    loginf << "SQLGenerator: getCreateTableStatement: sql '" << ss.str() << "'";
    return ss.str();
}

// std::shared_ptr<DBCommand> SQLGenerator::getSelectCommand(const DBObject &object, const
// DBOVariableSet &read_list, const std::string &custom_filter_clause,
//                                                          std::vector<std::string>
//                                                          &filtered_variable_names, DBOVariable
//                                                          *order,  const std::string &limit_str)
//{
//    const MetaDBTable &table = object.currentMetaTable ();

////    const PropertyList &variables, const MetaDBTable &meta_table,
////                                               const std::vector <std::string>
///&filtered_variable_names, const std::string &filter, const std::string &order, / const
/// std::string &limit, bool distinct, bool left_join
//    return getSelectCommand (read_list.getPropertyList(), table, filtered_variable_names,
//    custom_filter_clause, "",
//            limit_str, false, true);
//    //(PropertyList variables, MetaDBTable *table,
//    //    std::vector <std::string> &filtered_variable_names, std::string filter, std::string
//    order, std::string limit,
//    //    bool distinct, bool left_join)

//}

std::shared_ptr<DBCommand> SQLGenerator::getDataSourcesSelectCommand(DBObject& object)
{
    assert(object.hasCurrentDataSourceDefinition());

    const DBODataSourceDefinition& ds = object.currentDataSourceDefinition();
    const DBSchema& schema = ATSDB::instance().schemaManager().getCurrentSchema();

    if (!schema.hasMetaTable(ds.metaTableName()))
        throw std::invalid_argument(
                "SQLGenerator: getDataSourcesSelectCommand: schema does has no meta table " +
                ds.metaTableName());

    const MetaDBTable& meta = schema.metaTable(ds.metaTableName());
    logdbg << "SQLGenerator: getDataSourcesSelectCommand: object " << object.name()
           << " meta table " << meta.name() << " key col " << ds.foreignKey() << " name col "
           << ds.nameColumn();

    // select all part

    //    std::vector<const DBTableColumn*> columns;

    //    const std::map<std::string, const DBTableColumn&>& meta_columns = meta.columns();

    //    logdbg << "SQLGenerator: getDataSourcesSelectCommand: object " << object.name();
    //    for (const auto& col_it : meta_columns)
    //    {
    //        if (col_it.second.hasKnownPropertyType()) // slightly hacky, skip those with unknown type
    //        {
    //            logdbg << "SQLGenerator: getDataSourcesSelectCommand: adding column '" << col_it.second.name()
    //                   << "' type '" << col_it.second.type() << "'";

    //            columns.push_back(&col_it.second);
    //        }
    //        else
    //            logdbg << "SQLGenerator: getDataSourcesSelectCommand: skipping column '" << col_it.second.name()
    //                   << "' type '" << col_it.second.type() << "'";

    //    }


    if (!meta.hasColumn(ds.foreignKey()))
        throw std::runtime_error(
                "SQLGenerator: getDataSourcesSelectCommand: meta table has no column " +
                ds.foreignKey());

    const DBTableColumn& foreign_key_col = meta.column(ds.foreignKey());

    if (!meta.hasColumn(ds.nameColumn()))
        throw std::runtime_error(
                "SQLGenerator: getDataSourcesSelectCommand: meta table has no column " +
                ds.foreignKey());

    const DBTableColumn& name_col = meta.column(ds.nameColumn());

    std::vector<const DBTableColumn*> columns;
    columns.push_back(&foreign_key_col);
    columns.push_back(&name_col);

    if (ds.hasShortNameColumn())
    {
        assert (meta.hasColumn(ds.shortNameColumn()));
        columns.push_back(&meta.column(ds.shortNameColumn()));
    }

    if (ds.hasSacColumn())
    {
        assert (meta.hasColumn(ds.sacColumn()));
        columns.push_back(&meta.column(ds.sacColumn()));
    }

    if (ds.hasSicColumn())
    {
        assert (meta.hasColumn(ds.sicColumn()));
        columns.push_back(&meta.column(ds.sicColumn()));
    }

    if (ds.hasLatitudeColumn())
    {
        assert (meta.hasColumn(ds.latitudeColumn()));
        columns.push_back(&meta.column(ds.latitudeColumn()));
    }

    if (ds.hasLongitudeColumn())
    {
        assert (meta.hasColumn(ds.longitudeColumn()));
        columns.push_back(&meta.column(ds.longitudeColumn()));
    }

    if (ds.hasAltitudeColumn())
    {
        assert (meta.hasColumn(ds.altitudeColumn()));
        columns.push_back(&meta.column(ds.altitudeColumn()));
    }

    // psr
    if (ds.hasPrimaryAzimuthStdDevColumn())
    {
        assert (meta.hasColumn(ds.primaryAzimuthStdDevColumn()));
        columns.push_back(&meta.column(ds.primaryAzimuthStdDevColumn()));
    }

    if (ds.hasPrimaryRangeStdDevColumn())
    {
        assert (meta.hasColumn(ds.primaryRangeStdDevColumn()));
        columns.push_back(&meta.column(ds.primaryRangeStdDevColumn()));
    }

    if (ds.hasPrimaryIRMinColumn())
    {
        assert (meta.hasColumn(ds.primaryIRMinColumn()));
        columns.push_back(&meta.column(ds.primaryIRMinColumn()));
    }

    if (ds.hasPrimaryIRMaxColumn())
    {
        assert (meta.hasColumn(ds.primaryIRMaxColumn()));
        columns.push_back(&meta.column(ds.primaryIRMaxColumn()));
    }

    // ssr
    if (ds.hasSecondaryAzimuthStdDevColumn())
    {
        assert (meta.hasColumn(ds.secondaryAzimuthStdDevColumn()));
        columns.push_back(&meta.column(ds.secondaryAzimuthStdDevColumn()));
    }

    if (ds.hasSecondaryRangeStdDevColumn())
    {
        assert (meta.hasColumn(ds.secondaryRangeStdDevColumn()));
        columns.push_back(&meta.column(ds.secondaryRangeStdDevColumn()));
    }

    if (ds.hasSecondaryIRMinColumn())
    {
        assert (meta.hasColumn(ds.secondaryIRMinColumn()));
        columns.push_back(&meta.column(ds.secondaryIRMinColumn()));
    }

    if (ds.hasSecondaryIRMaxColumn())
    {
        assert (meta.hasColumn(ds.secondaryIRMaxColumn()));
        columns.push_back(&meta.column(ds.secondaryIRMaxColumn()));
    }

    // mode s
    if (ds.hasModeSAzimuthStdDevColumn())
    {
        assert (meta.hasColumn(ds.modeSAzimuthStdDevColumn()));
        columns.push_back(&meta.column(ds.modeSAzimuthStdDevColumn()));
    }

    if (ds.hasModeSRangeStdDevColumn())
    {
        assert (meta.hasColumn(ds.modeSRangeStdDevColumn()));
        columns.push_back(&meta.column(ds.modeSRangeStdDevColumn()));
    }

    if (ds.hasModeSIRMinColumn())
    {
        assert (meta.hasColumn(ds.modeSIRMinColumn()));
        columns.push_back(&meta.column(ds.modeSIRMinColumn()));
    }

    if (ds.hasModeSIRMaxColumn())
    {
        assert (meta.hasColumn(ds.modeSIRMaxColumn()));
        columns.push_back(&meta.column(ds.modeSIRMaxColumn()));
    }

    return getSelectCommand(meta, columns);
}

std::shared_ptr<DBCommand> SQLGenerator::getDistinctDataSourcesSelectCommand(DBObject& object)
{
    // "SELECT DISTINCT sensor_number__value FROM " << table_names_.at(DBO_PLOTS) << " WHERE
    // mapped_position__present = '1' AND sensor_number__present = '1' ORDER BY
    // sensor_number__value;";
    // return distinct_radar_numbers_statement_;

    assert(object.hasCurrentDataSourceDefinition());

    std::string local_key_dbovar = object.currentDataSourceDefinition().localKey();
    assert(object.hasVariable(local_key_dbovar));
    const DBTableColumn& local_key_col = object.variable(local_key_dbovar).currentDBColumn();

    std::vector<const DBTableColumn*> columns;
    columns.push_back(&local_key_col);

    PropertyList list;
    list.addProperty(local_key_col.name(), PropertyDataType::INT);

    return getSelectCommand(object.currentMetaTable(), columns, true);
}

std::string SQLGenerator::getCreateAssociationTableStatement(const std::string& table_name)
{
    std::stringstream ss;

    ss << "CREATE TABLE " << table_name
       << " (assoc_id INTEGER PRIMARY KEY AUTOINCREMENT, rec_num INTEGER, utn INTEGER, src_rec_num "
          "INTEGER,"
          " ds_id INTEGER);";

    return ss.str();
}

std::shared_ptr<DBCommand> SQLGenerator::getSelectAssociationsCommand(const std::string& table_name)
{
    std::shared_ptr<DBCommand> command = std::make_shared<DBCommand>(DBCommand());

    std::stringstream ss;

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

// DBCommand *SQLGenerator::getCountStatement (const std::string &dbo_type, unsigned int
// sensor_number)
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
//    command->setCommandString("SELECT COUNT(*) FROM " + table_name + " WHERE
//    "+data_source_key+"="+String::intToString(sensor_number));

//    return command;
//}

// DBCommand *SQLGenerator::getDistinctStatistics (const std::string &dbo_type, DBOVariable
// *variable, unsigned int sensor_number)
//{
//    assert (ATSDB::getInstance().existsDBObject(dbo_type));
//    DBObject *object = ATSDB::getInstance().getDBObject(dbo_type);
//    assert (object);
//    assert (object->hasCurrentDataSource());

//    assert (variable->existsIn(dbo_type));

//    DBOVariable *typevar = variable->getFor(dbo_type);

//    assert (ATSDB::getInstance().getDBObject(DBO_UNDEFINED)->hasVariable("frame_time"));
//    DBOVariable *metatimevar =
//    ATSDB::getInstance().getDBObject(DBO_UNDEFINED)->getVariable("frame_time"); assert
//    (metatimevar->existsIn(type)); DBOVariable *timevar = metatimevar->getFor(type);

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
////    command->setCommandString("SELECT "+distinctvarname+",COUNT(*), MIN("+timevarname+"),
/// MAX("+timevarname+")" /            " FROM " + table_name + " WHERE
///"+data_source_key+"="+intToString(sensor_number)+ " GROUP BY "+distinctvarname);

//    return command;
//}

// DBCommand *SQLGenerator::getSelectInfoCommand(const std::string &dbo_type, std::vector<unsigned
// int> ids,
//        DBOVariableSet read_list, bool use_filters,
//        std::string order_by_variable, bool ascending, unsigned int limit_min, unsigned int
//        limit_max)
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

//            filter <<  table->getTableDBName()<< "." <<
//            idvar->getFor(type)->id_+"='"+intToString(*it)+"'";
//        }

//        filter << ")";
//    }

//    if (use_filters)
//    {
//        std::string filter_condition = FilterManager::getInstance().getSQLCondition(type,
//        filtered_variable_names);

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

//    DBCommand *command = getSelectCommand (read_list.getPropertyList(type), table,
//    filtered_variable_names, filter.str(), order, limit, false, true);

//    logdbg  << "SQLGenerator: getSelectInfoCommand: type " <<
//    ATSDB::getInstance().getDBObject(type)->getName() << " sql '" << command->getCommandString()
//    << "'";
//}

// std::string SQLGenerator::getContainsStatement (std::string table_name)
//{
//    if (!db_type_set_)
//    {
//        db_type_=db_interface_->getDBInfo()->getType();
//        db_type_set_=true;
//    }

////    if (db_type_ == DB_TYPE_SQLITE)
////        return "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='" + table_name +
///"';"; /    else  || db_type_ == DB_TYPE_MYSQLCon
//    if (db_type_ == DB_TYPE_MYSQLpp)
//    {
//        //return "SELECT COUNT(*)  FROM information_schema.tables WHERE table_schema =
//        '"+db_interface_->getDatabaseName()+"' AND table_name ='" + table_name + "';"; return
//        "SHOW TABLES LIKE '"+table_name+"';";
//    }
//    else
//        throw std::runtime_error ("SQLGenerator: getContainsStatement: unknown db type");

//}

std::string SQLGenerator::getCountStatement(const std::string& table)
{
    return "SELECT COUNT(*) FROM " + table + ";";
}

std::shared_ptr<DBCommand> SQLGenerator::getTableSelectMinMaxNormalStatement(const DBTable& table)
{
    logdbg << "SQLGenerator: getTableSelectMinMaxNormalStatement: start for table " << table.name();

    stringstream ss;

    std::shared_ptr<DBCommand> command(new DBCommand());
    assert(command);

    PropertyList command_list;

    ss << "SELECT ";

    bool first = true;

    for (auto col_it : table.columns())
    {
        //        if (column->hasSpecialNull())
        //            continue;

        logdbg << "SQLGenerator: getTableSelectMinMaxNormalStatement: current name "
               << col_it.first;

        if (!first)
            ss << ",";

        ss << "MIN(" << col_it.first << "),MAX(" << col_it.first << ")";

        command_list.addProperty(col_it.first + "MIN", PropertyDataType::STRING);
        command_list.addProperty(col_it.first + "MAX", PropertyDataType::STRING);

        first = false;
    }

    ss << " FROM " << table.name() << ";";

    command->set(ss.str());
    command->list(command_list);

    logdbg << "SQLGenerator: getTableSelectMinMaxNormalStatement: sql '" << ss.str() << "'";

    return command;
}

// DBCommand *SQLGenerator::getColumnSelectMinMaxStatement (DBTableColumn *column, std::string
// table_name)
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

//    ss << " FROM " <<
//    DBSchemaManager::getInstance().getCurrentSchema()->getTable(table_name)->getDBName();

//    if (column->hasSpecialNull())
//        ss << " WHERE " << column_name << " != " << column->getSpecialNull();

//    command->setCommandString (ss.str());
//    command->setPropertyList(command_list);

//    //delete list;

//    loginf  << "SQLGenerator: getMainTableSelectMinMaxStatement: sql '" << ss.str() << "'";

//    return command;

//}

std::string SQLGenerator::getInsertPropertyStatement(const std::string& id,
                                                     const std::string& value)
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
std::string SQLGenerator::getSelectPropertyStatement(const std::string& id)
{
    stringstream ss;
    ss << "SELECT value FROM " << TABLE_NAME_PROPERTIES << " WHERE id = '" << id << "';";
    return ss.str();
}

std::string SQLGenerator::getSelectAllPropertiesStatement()
{
    stringstream ss;
    ss << "SELECT id, value FROM " << TABLE_NAME_PROPERTIES << ";";
    return ss.str();
}

std::string SQLGenerator::getInsertViewPointStatement(const unsigned int id, const std::string& json)
{
    stringstream ss;

    // REPLACE into table (id, name, age) values(1, "A", 19)
    ss << "REPLACE INTO " << TABLE_NAME_VIEWPOINTS << " VALUES ('" << id << "', '" << json
       << "');";
    return ss.str();
}

std::string SQLGenerator::getSelectAllViewPointsStatement()
{
    stringstream ss;
    ss << "SELECT id, json FROM " << TABLE_NAME_VIEWPOINTS << ";";
    return ss.str();
}

std::string SQLGenerator::getReplaceSectorStatement(const unsigned int id, const std::string& name,
                                                    const std::string& layer_name, const std::string& json)
{
    stringstream ss;

    // REPLACE into table (id, name, age) values(1, "A", 19)
    ss << "REPLACE INTO " << TABLE_NAME_SECTORS << " VALUES ('" << id << "', '" << name << "', '"
       << layer_name << "', '" << json << "');";
    return ss.str();
}

std::string SQLGenerator::getSelectAllSectorsStatement()
{
    stringstream ss;
    ss << "SELECT id, name, layer_name, json FROM " << TABLE_NAME_SECTORS << ";";
    return ss.str();
}

std::string SQLGenerator::getInsertMinMaxStatement(const std::string& variable_name,
                                                   const std::string& object_name,
                                                   const std::string& min, const std::string& max)
{
    stringstream ss;
    ss << "REPLACE INTO " << TABLE_NAME_MINMAX << " VALUES ('" << variable_name << "', '"
       << object_name << "', '" << min << "', '" << max << "');";
    return ss.str();
}
std::string SQLGenerator::getSelectMinMaxStatement(const std::string& variable_name,
                                                   const std::string& object_name)
{
    stringstream ss;
    ss << "SELECT min,max FROM " << TABLE_NAME_MINMAX << " WHERE variable_name = '" << variable_name
       << "' AND object_name = '" << object_name << "';";
    return ss.str();
}

std::string SQLGenerator::getSelectMinMaxStatement()
{
    stringstream ss;
    ss << "SELECT variable_name,object_name,min,max FROM " << TABLE_NAME_MINMAX << ";";
    return ss.str();
}

std::string SQLGenerator::getTableMinMaxCreateStatement() { return table_minmax_create_statement_; }

std::string SQLGenerator::getTablePropertiesCreateStatement()
{
    return table_properties_create_statement_;
}

std::string SQLGenerator::getTableSectorsCreateStatement()
{
    return table_sectors_create_statement_;
}

std::string SQLGenerator::getTableViewPointsCreateStatement()
{
    return table_view_points_create_statement_;
}

// std::string SQLGenerator::createDBInsertStringBind(Buffer *buffer, std::string tablename)
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
//            throw std::runtime_error ("SQLGenerator: createDBInsertStringBind: unknown database
//            type");

//        if (cnt != size-1)
//        {
//            ss << ", ";
//        }
//    }
//    ss << ");";

//    logdbg  << "SQLGenerator: createDBInsertStringBind: tablename " << tablename << " bind str '"
//    << ss.str() << "'";

//    return ss.str();
//}

std::string SQLGenerator::insertDBUpdateStringBind(std::shared_ptr<Buffer> buffer,
                                                   std::string tablename)
{
    assert(buffer);
    // assert (object.existsInDB());
    // assert (key_var.existsInDB());
    assert(tablename.size() > 0);

    const std::vector<Property>& properties = buffer->properties().properties();

    // INSERT INTO table_name (column1, column2, column3, ...) VALUES (value1, value2, value3, ...);

    unsigned int size = properties.size();
    logdbg << "SQLGenerator: insertDBUpdateStringBind: creating db string";
    std::stringstream ss;  // create a stringstream

    ss << "INSERT INTO " << tablename << " (";

    std::string connection_type = db_interface_.connection().type();

    if (connection_type != SQLITE_IDENTIFIER && connection_type != MYSQL_IDENTIFIER)
        throw std::runtime_error(
                "SQLGenerator: insertDBUpdateStringBind: not yet implemented db type " +
                connection_type);

    std::stringstream values_ss;
    values_ss << "VALUES (";

    for (unsigned int cnt = 0; cnt < size; cnt++)
    {
        ss << properties.at(cnt).name();

        if (connection_type == SQLITE_IDENTIFIER)
            values_ss << "@VAR" + std::to_string(cnt + 1);
        else if (connection_type == MYSQL_IDENTIFIER)
            values_ss << "%" + std::to_string(cnt + 1);

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

std::string SQLGenerator::createDBUpdateStringBind(std::shared_ptr<Buffer> buffer,
                                                   const DBTableColumn& key_col,
                                                   std::string tablename)
{
    assert(buffer);
    assert(key_col.existsInDB());
    assert(tablename.size() > 0);

    const std::vector<Property>& properties = buffer->properties().properties();

    // UPDATE table_name SET col1=val1,col2=value2 WHERE somecol=someval;

    unsigned int size = properties.size();
    logdbg << "SQLGenerator: createDBUpdateStringBind: creating db string";
    std::stringstream ss;  // create a stringstream

    std::string key_col_name = key_col.name();

    logdbg << "SQLGenerator: createDBUpdateStringBind: idvar name " << key_col_name;

    ss << "UPDATE " << tablename << " SET ";

    //    if (!db_type_set_)
    //    {
    //        db_type_=db_interface_->getDBInfo()->getType();
    //        db_type_set_=true;
    //    }

    if (key_col_name != properties.at(size - 1).name())
        throw std::runtime_error(
                "SQLGenerator: createDBUpdateStringBind: id var not at last position");

    std::string connection_type = db_interface_.connection().type();

    if (connection_type != SQLITE_IDENTIFIER && connection_type != MYSQL_IDENTIFIER)
        throw std::runtime_error(
                "SQLGenerator: createDBUpdateStringBind: not yet implemented db type " +
                connection_type);

    for (unsigned int cnt = 0; cnt < size; cnt++)
    {
        if (key_col_name == properties.at(cnt).name())
        {
            if (cnt == size - 1)
                continue;
            else
                throw std::runtime_error(
                        "SQLGenerator: createDBUpdateStringBind: id var at other than last position " +
                        std::to_string(cnt));
        }
        ss << properties.at(cnt).name() << "=";

        if (connection_type == SQLITE_IDENTIFIER)
            ss << "@VAR" + std::to_string(cnt + 1);
        else if (connection_type == MYSQL_IDENTIFIER)
            ss << "%" + std::to_string(cnt + 1);

        if (cnt != size - 2)
        {
            ss << ", ";
        }
    }

    ss << " WHERE " << key_col_name << "=";

    if (connection_type == SQLITE_IDENTIFIER)
        ss << "@VAR" << std::to_string(size);
    else if (connection_type == MYSQL_IDENTIFIER)
        ss << "%" << std::to_string(size);

    ss << ";";

    logdbg << "SQLGenerator: createDBUpdateStringBind: var update string '" << ss.str() << "'";

    return ss.str();
}

// std::string SQLGenerator::createDBCreateString (Buffer *buffer, std::string tablename)
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
//            throw std::runtime_error("SQLGenerator: createDBCreateString: unspecified data type
//            "+prop.asDataTypeString());
//        }

//        if (cnt != size-1)
//            ss << ", ";
//    }

//    ss << ");";

//    logdbg  << "SQLGenerator: createDBCreateString: tablename " << tablename << " create str '" <<
//    ss.str() << "'";

//    return ss.str();

//}

// std::string SQLGenerator::getMinMaxSelectStatement (std::string variable, std::string table,
// std::string condition)
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

// std::string SQLGenerator::getMinMaxSelectStatements (std::vector <std::string> variables,
// std::string table)
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

std::shared_ptr<DBCommand> SQLGenerator::getSelectCommand(
        const MetaDBTable& meta_table, DBOVariableSet read_list, const std::string& filter,
        std::vector<DBOVariable*> filtered_variables, bool use_order, DBOVariable* order_variable,
        bool use_order_ascending, const std::string& limit, bool left_join)
{
    logdbg << "SQLGenerator: getSelectCommand: meta table " << meta_table.name()
           << " read list size " << read_list.getSize();
    assert(read_list.getSize() != 0);

    std::shared_ptr<DBCommand> command = std::make_shared<DBCommand>(DBCommand());

    std::stringstream ss;

    ss << "SELECT ";

    //    if (distinct)
    //        ss << "DISTINCT ";

    std::vector<std::string> used_tables;

    logdbg << "SQLGenerator: getSelectCommand: collecting required variables";

    PropertyList property_list;

    bool first = true;
    for (auto var_it : read_list.getSet())
        // look what tables are needed for loaded variables and add variables to sql query
    {
        DBOVariable* variable = var_it;

        if (!first)
            ss << ", ";

        const DBTableColumn& column = variable->currentDBColumn();
        std::string table_db_name = column.table().name();

        if (find(used_tables.begin(), used_tables.end(), table_db_name) == used_tables.end())
            used_tables.push_back(table_db_name);

        ss << table_db_name << "." << column.name();

        property_list.addProperty(column.name(), variable->dataType());

        first = false;
    }

    logdbg << "SQLGenerator: getSelectCommand: ordering table";
    if (use_order)
    {
        assert(order_variable);
        assert(meta_table.hasColumn(order_variable->currentDBColumn().identifier()));
        std::string table_db_name =
                meta_table.tableFor(order_variable->currentDBColumn().identifier()).name();
        if (find(used_tables.begin(), used_tables.end(), table_db_name) == used_tables.end())
            used_tables.push_back(table_db_name);
    }

    ss << " FROM ";  // << table->getAllTableNames();

    bool where_added = false;
    std::string subtableclause;  // for !left_join

    logdbg << "SQLGenerator: getSelectCommand: collecting sub table clauses";
    // find all tables needed for variables to be filtered on
    for (auto var_it : filtered_variables)
        // look what tables are needed for filtered variables
    {
        if (meta_table.hasColumn(var_it->currentDBColumn().identifier()))
        {
            std::string table_db_name =
                    meta_table.tableFor(var_it->currentDBColumn().identifier()).name();

            if (find(used_tables.begin(), used_tables.end(), table_db_name) == used_tables.end())
                used_tables.push_back(table_db_name);
        }
        else
            logwrn << "SQLGenerator: getSelectCommand: meta table does not contain variable '"
                   << var_it->name() << "'";
    }

    std::string main_table_name = meta_table.mainTableName();

    if (!left_join)
    {
        logdbg << "SQLGenerator: getSelectCommand: normal query";
        // select cmp_aa.AZIMUTH_ERROR_DEG FROM sd_radar, cmp_aa WHERE  sd_radar.REC_NUM =
        // cmp_aa.REC_NUM
        assert(used_tables.size() > 0);

        for (auto it = used_tables.begin(); it != used_tables.end(); it++)
        {
            if (it != used_tables.begin())
                ss << ", ";
            ss << *it;
        }

        subtableclause = subTablesWhereClause(meta_table, used_tables);

        if (subtableclause.size() != 0)
        {
            logdbg << "SQLGenerator: getSelectCommand: subtableclause '" << subtableclause << "'";

            if (!where_added)
            {
                ss << " WHERE ";
                where_added = true;
            }

            ss << subtableclause;
        }
    }
    else
    {
        logdbg << "SQLGenerator: getSelectCommand: left join query";
        //    SELECT news.id, users.username, news.title, news.date, news.body, COUNT(comments.id)
        //    FROM news
        //    LEFT JOIN users
        //    ON news.user_id = users.id
        //    LEFT JOIN comments
        //    ON comments.news_id = news.id
        //    GROUP BY news.id
        ss << main_table_name;

        assert(used_tables.size() > 0);
        std::vector<std::string>::iterator it;
        for (it = used_tables.begin(); it != used_tables.end(); it++)
        {
            if (it->compare(main_table_name) != 0)  // not main table
            {
                ss << " LEFT JOIN " << *it;
                ss << " ON " << subTableKeyClause(meta_table, *it);
            }
        }
    }

    logdbg << "SQLGenerator: getSelectCommand: filterting statement";
    // add filter statement
    if (filter.size() > 0)
    {
        if (!where_added)
        {
            ss << " WHERE ";
            where_added = true;
        }

        if (!left_join && subtableclause.size() != 0)
            ss << " AND ";

        ss << filter;
    }

    // TODO FIXME
    //    if (left_join)
    //    {
    //        logdbg << "SQLGenerator: getSelectCommand: grouping by";
    //        ss << " GROUP BY " << main_table_name << "." << meta_table.mainTable().getKeyId();
    //    }

    if (use_order)
    {
        assert(order_variable);
        assert(meta_table.hasColumn(order_variable->currentDBColumn().identifier()));
        std::string table_db_name =
                meta_table.tableFor(order_variable->currentDBColumn().identifier()).name();

        ss << " ORDER BY " << table_db_name << "." << order_variable->currentDBColumn().name();

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

std::shared_ptr<DBCommand> SQLGenerator::getSelectCommand(const MetaDBTable& meta_table,
                                                          std::vector<const DBTableColumn*> columns,
                                                          bool distinct)
{
    logdbg << "SQLGenerator: getSelectCommand: meta table " << meta_table.name()
           << " db columns size " << columns.size();
    assert(columns.size() != 0);

    std::shared_ptr<DBCommand> command = std::make_shared<DBCommand>(DBCommand());

    std::stringstream ss;

    ss << "SELECT ";

    if (distinct)
        ss << "DISTINCT ";

    std::vector<std::string> used_tables;

    logdbg << "SQLGenerator: getSelectCommand: collecting required variables";

    PropertyList property_list;

    bool first = true;
    for (auto col_it : columns)
        // look what tables are needed for loaded variables and add variables to sql query
    {
        if (property_list.hasProperty(col_it->name())) // for already added ones
            continue;

        if (!first)
            ss << ", ";

        std::string table_db_name = col_it->table().name();

        if (find(used_tables.begin(), used_tables.end(), table_db_name) == used_tables.end())
            used_tables.push_back(table_db_name);

        ss << table_db_name << "." << col_it->name();

        property_list.addProperty(col_it->name(), col_it->propertyType());

        first = false;
    }

    ss << " FROM ";

    std::string main_table_name = meta_table.mainTableName();

    logdbg << "SQLGenerator: getSelectCommand: left join query";
    //    SELECT news.id, users.username, news.title, news.date, news.body, COUNT(comments.id)
    //    FROM news
    //    LEFT JOIN users
    //    ON news.user_id = users.id
    //    LEFT JOIN comments
    //    ON comments.news_id = news.id
    //    GROUP BY news.id
    ss << main_table_name;

    assert(used_tables.size() > 0);
    std::vector<std::string>::iterator it;
    for (it = used_tables.begin(); it != used_tables.end(); it++)
    {
        if (it->compare(main_table_name) != 0)  // not main table
        {
            ss << " LEFT JOIN " << *it;
            ss << " ON " << subTableKeyClause(meta_table, *it);
        }
    }

    ss << ";";

    command->set(ss.str());
    command->list(property_list);

    logdbg << "SQLGenerator: getSelectCommand: command sql '" << ss.str() << "'";

    return command;
}

std::string SQLGenerator::subTablesWhereClause(const MetaDBTable& meta_table,
                                               const std::vector<std::string>& used_tables)
{
    std::stringstream ss;

    bool first = true;

    for (auto it : meta_table.subTableDefinitions())
    {
        if (find(used_tables.begin(), used_tables.end(), it.second->subTableName()) ==
                used_tables.end())
            continue;

        if (!first)
            ss << " AND ";

        ss << meta_table.mainTableName() << "." << it.second->mainTableKey() << "="
           << it.second->subTableName() << "." << it.second->subTableKey();
        first = false;
    }

    return ss.str();
}

std::string SQLGenerator::subTableKeyClause(const MetaDBTable& meta_table,
                                            const std::string& sub_table_name)
{
    if (meta_table.subTableDefinitions().count(sub_table_name) == 0)
        throw std::range_error("SQLGenerator: getSubTableKeyClause: sub_table_name " +
                               sub_table_name + " not found");

    auto subtable = meta_table.subTableDefinitions().at(sub_table_name);

    // found subtable
    std::stringstream ss;

    ss << meta_table.mainTableName() << "." << subtable->mainTableKey() << "="
       << subtable->subTableName() << "." << subtable->subTableKey();

    return ss.str();
}

std::string SQLGenerator::getDeleteStatement (const std::string& table, const std::string& filter)
{
    // DELETE FROM table_name [WHERE Clause]
    return "DELETE FROM "+table+" WHERE "+filter+";";
}

// std::string SQLGenerator::getUpdateStatement (DBTableColumn *column, std::string value,
// std::string new_value, std::string filter)
//{
//    assert (column);
//    // update students set first_name=’Suba’ where rec_id=678;
//    return "UPDATE "+column->getDBTableName()+" SET "+column->getName()+"="+new_value+" WHERE
//    "+column->getName()+"="+value+" AND "+filter+";";
//}

// std::string SQLGenerator::getDistinctSelectStatement (DBTableColumn *column, std::string
// filter_condition)
//{
//    assert (column);
//     std::stringstream ss;
//     ss << "SELECT DISTINCT("+column->getName() << ") FROM " << column->getDBTableName();

//     if (filter_condition.size() != 0)
//      ss << " WHERE " << filter_condition;

//     ss << ";";

//     return ss.str();
//}

// std::string SQLGenerator::getShowDatabasesStatement ()
//{
//    return "SHOW DATABASES LIKE 'job%';"; // wow so hard so many dabes
//}
