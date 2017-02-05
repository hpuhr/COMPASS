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

#include "boost/date_time/posix_time/posix_time.hpp"
//#include <QProgressDialog>
//#include <QCoreApplication>
//#include <QApplication>

#include "buffer.h"
//#include "BufferWriter.h"
#include "config.h"
#include "dbobject.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "mysqlserver.h"
#include "dbconnection.h"
#include "mysqlppconnection.h"
#include "dbinterfacewidget.h"
#include "dbinterface.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "dbresult.h"
//#include "MetaDBTable.h"
//#include "MySQLConConnection.h"
//#include "SQLGenerator.h"
//#include "SQLiteConnection.h"
//#include "StructureDescriptionManager.h"
//#include "DBSchemaManager.h"
//#include "DBSchema.h"
//#include "DBTable.h"
//#include "DBTableColumn.h"
#include "quantity.h"
#include "unit.h"
#include "unitmanager.h"
#include "dbtableinfo.h"

#include "data.h"
#include "string.h"

using namespace Utils::Data;
using namespace Utils;


/**
 * Creates SQLGenerator, several containers based in DBOs (prepared_, reading_done_, exists_, count_), creates
 * write_table_names_,
 */
DBInterface::DBInterface(std::string class_id, std::string instance_id, Configurable *parent)
    : Configurable (class_id, instance_id, parent), current_connection_(nullptr), widget_(nullptr)//, buffer_writer_(0)
{
    boost::mutex::scoped_lock l(mutex_);

    //registerParameter ("database_name", &database_name_, "");
    registerParameter ("read_chunk_size", &read_chunk_size_, 20000);

//    sql_generator_ = new SQLGenerator (this);

//    const std::map <std::string, DBObject*> &objects = DBObjectManager::getInstance().getDBObjects ();
//    std::map <std::string, DBObject*>::const_iterator it;

//    for (it = objects.begin(); it != objects.end(); it++)
//    {
//        if (it->second->isMeta() || !it->second->isLoadable())
//            continue;

//        std::string type = it->first;

//        prepared_[type] = false;
//        reading_done_[type] = true;
//        exists_[type]=false;
//        count_[type]=0;
//    }

    //TODO writing process should be different.
//    write_table_names_[DBO_PLOTS] = "Plot";
//    write_table_names_[DBO_SYSTEM_TRACKS] = "SystemTrack";
//    write_table_names_[DBO_MLAT] = "MLAT";
//    write_table_names_[DBO_ADS_B] = "ADSB";
//    write_table_names_[DBO_REFERENCE_TRAJECTORIES] = "ReferenceTrajectory";
//    write_table_names_[DBO_SENSOR_INFORMATION] = "PlotSensor";

    createSubConfigurables();
}

/**
 * If required, deletes connection, sql_generator_ and buffer_writer_.
 */
DBInterface::~DBInterface()
{
    logdbg  << "DBInterface: desctructor: start";

    boost::mutex::scoped_lock l(mutex_);

    for (auto it : connections_)
        delete it.second;

    connections_.clear();

    assert (!widget_);

//    delete sql_generator_;
//    sql_generator_=0;

//    if (buffer_writer_)
//    {
//        delete buffer_writer_;
//        buffer_writer_=0;
//    }

    logdbg  << "DBInterface: desctructor: end";
}

/**
 * Generates connection based on the DB_CONNECTION_TYPE of info, calls init on it. If a new database will be created, creates
 * the buffer_writer_, else calls updateExists and updateCount.
 */
void DBInterface::useConnection (const std::string &connection_type)
{
    logdbg << "DBInterface: useConnection: '" << connection_type << "'";
    if (current_connection_)
        assert (!current_connection_->ready());

    current_connection_ = connections_.at(connection_type);

    assert (current_connection_);
}

void DBInterface::closeConnection ()
{
    boost::mutex::scoped_lock l(mutex_);

    logdbg << "DBInterface: closeConnection";
    current_connection_->disconnect();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    current_connection_ = 0;

    table_info_.clear();
}

//void DBInterface::openDatabase (std::string database_name)
//{

//    assert (connection_);
//    connection_->openDatabase(database_name);
//    updateTableInfo ();

//    //    if (info->isNew())
//    //    {
//    //        buffer_writer_ = new BufferWriter (connection_, sql_generator_);
//    //    }
//    //    else
//    //    {
////            updateExists();
////            updateCount();
//    //    }

//}

void DBInterface::updateTableInfo ()
{
    table_info_.clear();

    assert (current_connection_);
    table_info_ = current_connection_->getTableInfo();

    loginf << "DBInterface::updateTableInfo: found " << table_info_.size() << " tables";

//    for (auto it : table_info_)
//    {
//        loginf << "DBInterface::updateTableInfo: table '" << it.first << "' with " << it.second.size() << " columns";
//    }
}

DBInterfaceWidget *DBInterface::widget()
{
    if (!widget_)
    {
        widget_ = new DBInterfaceWidget (*this);
    }

    assert (widget_);
    return widget_;
}

QWidget *DBInterface::connectionWidget()
{
    assert (current_connection_);
    return current_connection_->widget();
}

void DBInterface::databaseOpened ()
{
    updateTableInfo();
}

std::vector <std::string> DBInterface::getDatabases ()
{
    assert (current_connection_);
    return current_connection_->getDatabases();
}

bool DBInterface::ready ()
{
    if (!current_connection_)
        return false;

    return current_connection_->ready();
}

void DBInterface::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
  logdbg  << "DBInterface: generateSubConfigurable: generating variable " << instance_id;
  if (class_id == "MySQLppConnection")
  {
    MySQLppConnection *connection = new MySQLppConnection (instance_id, this);
    assert (connections_.count (connection->getInstanceId()) == 0);
    connections_.insert (std::pair <std::string, DBConnection*> (connection->getInstanceId(), dynamic_cast<DBConnection*>(connection)));
  }
  else
    throw std::runtime_error ("DBInterface: generateSubConfigurable: unknown class_id "+class_id );
}

void DBInterface::checkSubConfigurables ()
{
  if (connections_.count("MySQL++ Connection") == 0)
  {
      addNewSubConfiguration ("MySQLppConnection", "MySQL++ Connection");
      generateSubConfigurable ("MySQLppConnection", "MySQL++ Connection");
  }
}

/**
 * Calls queryContains for all DBOs in exists_.
 */
//void DBInterface::updateExists ()
//{
//    logdbg  << "DBInterface: updateExists: size " << exists_.size();

//    std::map <std::string, bool>::iterator it;

//    for (it = exists_.begin(); it != exists_.end(); it++)
//    {
//        it->second = queryContains (it->first);
//        logdbg  << "DBInterface: updateExists: type " << it->first << " exists " <<  it->second;
//    }

//}

///**
// * Calls queryCount for all DBOs in count_, if they are in exists_.
// */
//void DBInterface::updateCount ()
//{
//    logdbg  << "DBInterface: updateCount: size " << count_.size();

//    std::map <std::string, unsigned int>::iterator it;

//    for (it = count_.begin(); it != count_.end(); it++)
//    {
//        if (exists_[it->first])
//        {
//            it->second = queryCount (it->first);
//            logdbg  << "DBInterface: updateCount: type " << it->first << " exists, count " << it->second ;
//        }
//    }
//}

///**
// * Gets SQL command if the table exists and checks if the resulting count is larger as 0.
// */
//bool DBInterface::existsTable (std::string table_name)
//{
//    logdbg  << "DBInterface: existsTable: start sql " << sql_generator_->getContainsStatement(table_name);

//    boost::mutex::scoped_lock l(mutex_);

//    DBCommand command;
//    command.setCommandString(sql_generator_->getContainsStatement(table_name));

////    PropertyList list;
////    list.addProperty("exists", P_TYPE_INT);
////    command.setPropertyList(list);
////
////    DBResult *result = connection_->execute(&command);
////
////    assert (result->containsData());
////    int tmp = *((int*) result->getBuffer()->get(0,0));
////
////    delete result;
////
////    loginf  << "DBInterface: existsTable: '" <<  tmp << "'";
////    return tmp > 0;

//    PropertyList list;
//    list.addProperty("table", PropertyDataType::STRING);
//    command.setPropertyList(list);

//    DBResult *result = connection_->execute(&command);

//    assert (false);
//    // TODO FIXME

//    return false;
////    assert (result->containsData());
////    std::string tmp = *((std::string*) result->getBuffer()->get(0,0));

////    delete result;

////    logdbg  << "DBInterface: existsTable: '" <<  tmp << "'";
////    return tmp == table_name;

//}

///**
// * Checks if DBO has a main database table, and returns existsTable, else false.
// */
//bool DBInterface::queryContains (const std::string &type)
//{
//    logdbg  << "DBInterface: queryContains: start";

//    assert (DBObjectManager::getInstance().existsDBObject(type));

//    if (!DBObjectManager::getInstance().getDBObject(type)->hasCurrentMetaTable())
//    {
//        logdbg  << "DBInterface: queryContains: object type " << type << " has no current meta table";
//        return false;
//    }

//    std::string table_name = DBObjectManager::getInstance().getDBObject(type)->getCurrentMetaTable()->getTableDBName();

//    return existsTable (table_name);
//}

///**
// * Returns existsTable for table name.
// */
//bool DBInterface::existsMinMaxTable ()
//{
//    return existsTable (sql_generator_->getMinMaxTableName());
//}

///**
// * Returns existsTable for table name.
// */
//bool DBInterface::existsPropertiesTable ()
//{
//    return existsTable (sql_generator_->getPropertiesTableName());
//}

///**
// * Gets SQL command for element count and returns the result.
// */
//unsigned int DBInterface::queryCount (const std::string &type)
//{
//    logdbg  << "DBInterface: queryCount: start";

//    boost::mutex::scoped_lock l(mutex_);

//    std::string sql = sql_generator_->getCountStatement(type);

//    logdbg  << "DBInterface: queryCount: sql '" << sql << "'";

//    DBCommand command;
//    command.setCommandString(sql);

//    PropertyList list;
//    list.addProperty("count", PropertyDataType::INT);
//    command.setPropertyList(list);

//    DBResult *result = connection_->execute(&command);

//    // TODO FIXME
//    assert (false);
//    return 0;

////    assert (result->containsData());
////    int tmp = *((int*) result->getBuffer()->get(0,0));

////    delete result;

////    logdbg  << "DBInterface: queryCount: " << type << ": "<< tmp <<" end";
////    return (unsigned int) tmp;
//}

///**
// * Gets SQL command for data sources list and packs the resulting buffer into a set, which is returned.
// */
//std::set<int> DBInterface::queryActiveSensorNumbers(const std::string &type)
//{
//    logdbg  << "DBInterface: queryActiveSensorNumbers: start";

//    boost::mutex::scoped_lock l(mutex_);

//    assert (DBObjectManager::getInstance().existsDBObject(type));
//    assert (DBObjectManager::getInstance().getDBObject(type)->hasCurrentDataSource());

//    std::set<int> data;

//    DBCommand *command = sql_generator_->getDistinctDataSourcesSelectCommand(type);

//    DBResult *result = connection_->execute(command);

//    assert (result->containsData());

//    // TODO FIXME
//    assert (false);

////    Buffer *buffer = result->getBuffer();
////    for (unsigned int cnt=0; cnt < buffer->getSize(); cnt++)
////    {
////        int tmp = *((int*) result->getBuffer()->get(cnt,0));
////        data.insert (tmp);
////    }

////    delete result;

//    logdbg << "DBInterface: queryActiveSensorNumbers: done";
//    return data;
//}

///**
// * Gets SQL command, executes it and returns resulting buffer.
// */
//Buffer *DBInterface::getDataSourceDescription (const std::string &type)
//{
//    logdbg  << "DBInterface: getDataSourceDescription: start";

//    boost::mutex::scoped_lock l(mutex_);

//    DBCommand *command = sql_generator_->getDataSourcesSelectCommand(type);

//    logdbg << "DBInterface: getDataSourceDescription: sql '" << command->getCommandString() << "'";

//    DBResult *result = connection_->execute(command);
//    assert (result->containsData());
//    Buffer *buffer = result->getBuffer();
//    delete result;
//    delete command;

//    return buffer;
//}

//void DBInterface::insertProperty (std::string id, std::string value)
//{
//    boost::mutex::scoped_lock l(mutex_);

//    std::string str = sql_generator_->getInsertPropertyStatement(id, value);
//    connection_->executeSQL (str);
//}

//std::string DBInterface::getProperty (std::string id)
//{
//    logdbg  << "DBInterface: getProperty: start";

//    boost::mutex::scoped_lock l(mutex_);

//    DBCommand command;
//    command.setCommandString(sql_generator_->getSelectPropertyStatement(id));

//    PropertyList list;
//    list.addProperty("property", PropertyDataType::STRING);
//    command.setPropertyList(list);

//    DBResult *result = connection_->execute(&command);

//    assert (result->containsData());

//    std::string text;
//    Buffer *buffer = result->getBuffer();

//    // TODO FIXXME
//    assert (false);

////    assert (buffer);
////    if (buffer->getFirstWrite())
////        throw std::runtime_error ("DBInterface: getProperty: id "+id+" does not exist");

////    text = *((std::string*) buffer->get(0,0));

////    delete result;

//    logdbg  << "DBInterface: getProperty: end id " << id << " value " << text;
//    return text;
//}

//bool DBInterface::hasProperty (std::string id)
//{
//    logdbg  << "DBInterface: hasProperty: start";

//    boost::mutex::scoped_lock l(mutex_);

//    DBCommand command;
//    command.setCommandString(sql_generator_->getSelectPropertyStatement(id));

//    PropertyList list;
//    list.addProperty("property", PropertyDataType::STRING);
//    command.setPropertyList(list);

//    DBResult *result = connection_->execute(&command);

//    assert (result->containsData());

//    std::string text;
//    Buffer *buffer = result->getBuffer();

//    assert (buffer);
//    bool found = !buffer->firstWrite();

//    delete buffer;

//    delete result;

//    logdbg  << "DBInterface: hasProperty: end id " << id << " found " << found;
//    return found;
//}

//void DBInterface::insertMinMax (std::string id, const std::string &type, std::string min, std::string max)
//{
//    boost::mutex::scoped_lock l(mutex_);

//    std::string str = sql_generator_->getInsertMinMaxStatement(id, type, min, max);
//    connection_->executeSQL (str);
//}

///**
// * If variable is a not meta variable, min/max values just for the variable. If it is, gets min/max values for all subvariables
// * and calculates the min/max for all subvariables. If the variable needs a unit transformation, it is performed (locally
// * in this thread).
// */
//Buffer *DBInterface::getMinMaxString (DBOVariable *var)
//{
//    boost::mutex::scoped_lock l(mutex_);
//    assert (var);

//    Buffer *string_buffer;
//    PropertyList list;
//    list.addProperty("min", PropertyDataType::STRING);
//    list.addProperty("max", PropertyDataType::STRING);

//    // get min max as strings

//    if (!var->isMetaVariable())
//    {
//        logdbg  << "DBInterface: getMinMax: is not meta";
//        DBCommand command;
//        command.setCommandString(sql_generator_->getSelectMinMaxStatement(var->getId(), var->getDBOType()));

//        command.setPropertyList(list);

//        DBResult *result = connection_->execute(&command);

//        assert (result->containsData());
//        string_buffer = result->getBuffer();
//        assert (string_buffer);
//        if (string_buffer->size() != 1)
//        {
//            logwrn  << "DBInterface: getMinMaxString: string buffer for variaible " << var->getId() << " empty";
//        }
//        delete result;
//    }
//    else
//    {
//        logdbg  << "DBInterface: getMinMax: is meta";
//        DBCommandList command_list;

//        const std::map <std::string, std::string> &subvars = var->getSubVariables ();
//        std::map <std::string, std::string>::const_iterator it;

//        for (it =subvars.begin(); it != subvars.end(); it++)
//        {
//            if (exists_[it->first])
//                command_list.addCommandString(sql_generator_->getSelectMinMaxStatement(it->second, it->first));
//        }

//        command_list.setPropertyList(list);

//        DBResult *result = connection_->execute(&command_list);

//        assert (result->containsData());
//        string_buffer = result->getBuffer();
//        assert (string_buffer);
//        delete result;
//    }

//    // got final (possibly multiple) min max in a buffer

//    assert (false);
//    // TODO FIXMME

////    for (unsigned int cnt=0; cnt < string_buffer->size(); cnt++)
////    {
////        logdbg << "DBInterface: getMinMax: var " << var->getName() << " cnt " << cnt
////                << " string min " << *((std::string*)string_buffer->get(cnt,0))
////                << " max " << *((std::string*)string_buffer->get(cnt,1));
////    }

////    Buffer *data_buffer = createFromMinMaxStringBuffer (string_buffer, var->getDBOType());

////    // only 1 minmax line
////    assert (data_buffer->size() == 1);

////    // check unit transformation

////    DBOVariable *tmpvar = var->getFirst();

////    std::string meta_tablename = tmpvar->getCurrentMetaTable ();
////    std::string table_varname = tmpvar->getCurrentVariableName ();

////    DBTableColumn *table_column = DBSchemaManager::getInstance().getCurrentSchema ()->getMetaTable(meta_tablename)->getTableColumn(table_varname);

////    if (tmpvar->hasUnit () || table_column->hasUnit())
////    {
////        if (tmpvar->hasUnit () != table_column->hasUnit())
////        {
////            logerr << "DBInterface: getMinMax: unit transformation inconsistent: var " << tmpvar->getName () << " has unit " << tmpvar->hasUnit ()
////                                                      << " table column " << table_column->getName() << " has unit " << table_column->hasUnit();
////            throw std::runtime_error ("DBInterface: getMinMax: unit transformation error 1");
////        }

////        if (tmpvar->getUnitDimension().compare(table_column->getUnitDimension()) != 0)
////        {
////            logerr << "DBInterface: getMinMax: unit transformation inconsistent: var " << tmpvar->getName () << " has dimension " << tmpvar->getUnitDimension ()
////                                                      << " table column " << table_column->getName() << " has dimension " << table_column->getUnitDimension();
////            throw std::runtime_error ("DBInterface: getMinMax: unit transformation error 2");
////        }

////        Unit *unit = UnitManager::getInstance().getUnit (tmpvar->getUnitDimension());
////        double factor = unit->getFactor (table_column->getUnitUnit(), tmpvar->getUnitUnit());
////        logdbg  << "DBInterface: getMinMax: adapting " << tmpvar->getName () << " unit transformation with factor " << factor;

////        multiplyData (data_buffer->get(0,0), (PROPERTY_DATA_TYPE) tmpvar->data_type_int_, factor);
////        multiplyData (data_buffer->get(0,1), (PROPERTY_DATA_TYPE) tmpvar->data_type_int_, factor);
////    }
////    // write values back to string buffer
////    *((std::string *)(string_buffer->get(0,0))) = tmpvar->getValueFrom (data_buffer->get(0,0));
////    *((std::string *)(string_buffer->get(0,1))) = tmpvar->getValueFrom (data_buffer->get(0,1));

////    delete data_buffer;

//    return string_buffer;
//}

//std::map <std::pair<std::string, std::string>, std::pair<std::string, std::string> > DBInterface::getMinMaxInfo ()
//{
//    boost::mutex::scoped_lock l(mutex_);

//    std::map <std::pair<std::string, std::string>, std::pair<std::string, std::string> > min_max_values;

//    PropertyList list;
//    list.addProperty("id", PropertyDataType::STRING);
//    list.addProperty("dbo_type", PropertyDataType::STRING);
//    list.addProperty("min", PropertyDataType::STRING);
//    list.addProperty("max", PropertyDataType::STRING);

//    // get min max as strings

//    logdbg  << "DBInterface: getMinMax: is not meta";
//    DBCommand command;
//    command.setCommandString(sql_generator_->getSelectMinMaxStatement());

//    command.setPropertyList(list);

//    DBResult *result = connection_->execute(&command);

//    assert (result->containsData());
//    Buffer *min_max_buffer = result->getBuffer();
//    assert (min_max_buffer);

//    if (min_max_buffer->firstWrite())
//    {
//        logerr << "DBInterface: getMinMaxInfo: no minmax values defined";
//        delete min_max_buffer;
//        delete result;
//        return min_max_values;
//    }

//    assert (false);
//    // TODO FIXMEE

////    for (unsigned int cnt=0; cnt < min_max_buffer->getSize(); cnt ++)
////    {
////        std::string id = *(std::string*)min_max_buffer->get(cnt, 0);
////        DB_OBJECT_TYPE dbo_type = (DB_OBJECT_TYPE) *(int*)min_max_buffer->get(cnt, 1);
////        std::string min = *(std::string*)min_max_buffer->get(cnt, 2);
////        std::string max = *(std::string*)min_max_buffer->get(cnt, 3);

////        if (min.size() == 0 && max.size() == 0)
////        {
////            logwrn << "DBInterace: getMinMaxInfo: var " << id << " dbo " << dbo_type << " min '" << min << "' max '"
////                    << max << "' unusable";
////            continue;
////        }
////        assert (min_max_values.find(std::pair <DB_OBJECT_TYPE, std::string> (dbo_type, id)) == min_max_values.end());
////        min_max_values[std::pair <DB_OBJECT_TYPE, std::string> (dbo_type, id)] = std::pair<std::string, std::string> (min, max);
////    }


//    delete min_max_buffer;
//    delete result;

//    return min_max_values;
//}

///**
// * Retrieves result from connection stepPreparedCommand, calls activateKeySearch on buffer and returns it.
// */
//Buffer *DBInterface::readDataChunk (const std::string &type, bool activate_key_search)
//{
//    boost::mutex::scoped_lock l(mutex_);

//    assert (DBObjectManager::getInstance().existsDBObject (type));

//    DBResult *result = connection_->stepPreparedCommand(read_chunk_size_);
//    if (!result)
//    {
//        logerr  << "DBInterface: readDataChunk: connection returned error";
//        reading_done_.at(type) = true;
//        throw std::runtime_error ("DBInterface: readDataChunk: connection returned error");
//    }
//    assert (result->containsData());
//    Buffer *buffer = result->getBuffer();
//    buffer->setDBOType(type);
//    delete result;

//    assert (buffer);
//    if (buffer->firstWrite())
//    {
//        reading_done_.at(type) = true;
//        return buffer; // HACK UGGGA WAS 0
//    }

//    bool last_one = connection_->getPreparedCommandDone();
//    reading_done_.at(type) =last_one;

//    assert (false);
//    // TODO FIXME

////    buffer->setLastOne (last_one);
////    //buffer->setDBOType(type);

////    if (activate_key_search)
////    {
////        assert (DBObjectManager::getInstance().existsDBOVariable (DBO_UNDEFINED, "id"));
////        assert (DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id")->existsIn (type));
////        std::string id_name = DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id")->getFor(type)->getName();
////        assert (buffer->getPropertyList()->hasProperty(id_name));
////        logdbg << "DBInterface: readDataChunk: key search id " << id_name << " index " << buffer->getPropertyList()->getPropertyIndex(id_name)
////                                                    << " buffer first " << buffer->getFirstWrite() << " size " << buffer->getSize();
////        buffer->activateKeySearch(buffer->getPropertyList()->getPropertyIndex(id_name));
////    }
//    return buffer;
//}


//bool DBInterface::isPrepared (const std::string &type)
//{
//    logdbg  << "DBInterface: isPrepared: type " << type;
//    assert (DBObjectManager::getInstance().existsDBObject (type));
//    return prepared_.at(type);
//}

//bool DBInterface::getReadingDone (const std::string &type)
//{
//    assert (DBObjectManager::getInstance().existsDBObject (type));
//    return reading_done_.at(type);
//}

//bool DBInterface::isReadingDone ()
//{
//    bool reading_done = false;
//    std::map <std::string, bool>::iterator it;
//    for (it = reading_done_.begin(); it != reading_done_.end(); it++)
//        reading_done |= it->second;

//    return reading_done;
//}


//bool DBInterface::exists (const std::string &type)
//{
//    logdbg  << "DBInterface: exists: type " << type;
//    assert (DBObjectManager::getInstance().existsDBObject (type));

////    if (type == DBO_UNDEFINED)
////        return true;

//    logdbg  << "DBInterface: exists: type " << type << " exists " << exists_.at(type);
//    return exists_.at(type);
//}

//unsigned int DBInterface::count (const std::string &type)
//{
//    assert (DBObjectManager::getInstance().existsDBObject (type));
//    return count_.at(type);
//}

//DBResult *DBInterface::count (const std::string &type, unsigned int sensor_number)
//{
//    boost::mutex::scoped_lock l(mutex_);

//    loginf << "DBInterface: count: for type " << type << " number " << sensor_number;

//    assert (DBObjectManager::getInstance().existsDBObject (type));
//    assert (DBObjectManager::getInstance().getDBObject (type)->hasCurrentDataSource());

//    DBCommand *command = sql_generator_->getCountStatement(type, sensor_number);

//    DBResult *result = connection_->execute(command);

//    delete command;

//    return result;
//}

//void DBInterface::finalizeReadStatement (const std::string &type)
//{
//    boost::mutex::scoped_lock l(mutex_);

//    assert (DBObjectManager::getInstance().existsDBObject (type));
//    logdbg  << "DBInterface: finishReadSystemTracks: start ";
//    prepared_.at(type)=false;
//    connection_->finalizeCommand();
//}

//void DBInterface::clearResult ()
//{
//    boost::mutex::scoped_lock l(mutex_);

//    std::map <std::string, bool>::iterator it;

//    for (it = reading_done_.begin(); it != reading_done_.end(); it++)
//    {
//        it->second=true;
//    }
//}

//bool DBInterface::hasActiveDataSourcesInfo (const std::string &type)
//{
//    if (!existsPropertiesTable())
//        return false;

//    return hasProperty("activeSensorNumbers"+DBObjectManager::getInstance().getDBObject(type)->getName());
//}

///**
// * Gets active sensor numbers as property, splits it and packs it into a set.
// */
//std::set<int> DBInterface::getActiveSensorNumbers (const std::string &type)
//{
//    logdbg  << "DBInterface: getActiveRadarNumbers: start";

//    assert (DBObjectManager::getInstance().existsDBObject(type));

//    std::string tmp = getProperty("activeSensorNumbers"+DBObjectManager::getInstance().getDBObject(type)->getName());

//    std::set<int> ret;

//    std::vector<std::string> tmp2 = String::split(tmp, ',');

//    logdbg  << "DBInterface: getActiveRadarNumbers: got "<< tmp2.size() << " parts from '" << tmp << "'" ;

//    for (unsigned int cnt=0; cnt < tmp2.size(); cnt++)
//    {
//        ret.insert (String::intFromString(tmp2.at(cnt)));
//        logdbg  << "DBInterface: getActiveRadarNumbers: got active radar "<< cnt << " '" << String::intFromString(tmp2.at(cnt)) << "'" ;
//    }


//    logdbg  << "DBInterface: getActiveRadarNumbers: end";
//    return ret;
//}

//void DBInterface::writeBuffer (Buffer *data)
//{
//    if (!buffer_writer_)
//        buffer_writer_ = new BufferWriter (connection_, sql_generator_);

//    boost::mutex::scoped_lock l(mutex_);

//    assert (buffer_writer_);
//    assert (data);

//    std::string type = data->dboType();

//    //TODO FIXME
//    assert (false);

//    //assert (write_table_names_.find(type) != write_table_names_.end());

//    //buffer_writer_->write (data, write_table_names_[type]);
//}

//void DBInterface::writeBuffer (Buffer *data, std::string table_name)
//{
//    if (!buffer_writer_)
//        buffer_writer_ = new BufferWriter (connection_, sql_generator_);

//    boost::mutex::scoped_lock l(mutex_);

//    assert (buffer_writer_);
//    assert (data);

//    buffer_writer_->write (data, table_name);
//}

//void DBInterface::updateBuffer (Buffer *data)
//{
//    boost::mutex::scoped_lock l(mutex_);

//    if (!buffer_writer_)
//        buffer_writer_ = new BufferWriter (connection_, sql_generator_);

//    assert (buffer_writer_);
//    assert (data);

//    std::string type = data->dboType();
//    DBTable *table = DBObjectManager::getInstance ().getDBObject(type)->getCurrentMetaTable()->getTable();

//    const PropertyList &properties = data->properties();

//    for (unsigned int cnt=0; cnt < properties.size(); cnt++)
//    {
//        if (!table->hasTableColumn(properties.at(cnt).getId()))
//            throw std::runtime_error ("DBInterface: updateBuffer: column '"+properties.at(cnt).getId()+"' does not exist in table "+table->getDBName());
//    }

//    buffer_writer_->update (data, table->getDBName());
//}

//void DBInterface::prepareRead (const std::string &type, DBOVariableSet read_list, std::string custom_filter_clause,
//        DBOVariable *order)
//{
//    boost::mutex::scoped_lock l(mutex_);

//    assert (DBObjectManager::getInstance().existsDBObject (type));

//    DBCommand *read = sql_generator_->getSelectCommand (type, read_list, custom_filter_clause, order);
//    loginf  << "DBInterface: prepareRead: type " << type << " sql '" << read->getCommandString() << "'";
//    connection_->prepareCommand(read);

//    prepared_.at(type)=true;
//    reading_done_.at(type)=false;
//}

//void DBInterface::createPropertiesTable ()
//{
//    assert (!existsPropertiesTable());
//    boost::mutex::scoped_lock l(mutex_);
//    connection_->executeSQL(sql_generator_->getTablePropertiesCreateStatement());
//}
//void DBInterface::createMinMaxTable ()
//{
//    assert (!existsMinMaxTable());
//    boost::mutex::scoped_lock l(mutex_);
//    connection_->executeSQL(sql_generator_->getTableMinMaxCreateStatement());
//}

//bool DBInterface::isPostProcessed ()
//{
//    return existsMinMaxTable ();// && existsPropertiesTable();
//}

///**
// * Gets SQL command from SQLGenerator, executes it, transforms units on results if required, returns buffer.
// *
// * \param type DBO type
// * \param ids Vector with key ids of elements of interest
// * \param list Property list of all data columns of interest
// * \param use_filters Whether to use filter configuration or not
// * \param order_by_varaible Name of data column to be ordered by
// * \param ascending True is ascending, false descending, only used if order_by_varaible set
// * \param limit_min For SQL Limit statement
// * \param limit_max For SQL Limit statement
// */
//Buffer *DBInterface::getInfo (const std::string &type, std::vector<unsigned int> ids, DBOVariableSet read_list,
//        bool use_filters, std::string order_by_variable, bool ascending, unsigned int limit_min,
//        unsigned int limit_max, bool finalize)
//{
//    logdbg  << "DBInterface: getInfo: start";

//    boost::mutex::scoped_lock l(mutex_);

//    DBCommand *command = sql_generator_->getSelectInfoCommand(type, ids, read_list, use_filters, order_by_variable, ascending, limit_min, limit_max);

//    loginf  << "DBInterface: getInfo: sql '" << command->getCommandString() << "'";

//    DBResult *result = connection_->execute(command);
//    assert (result->containsData());
//    Buffer *buffer = result->getBuffer();
//    buffer->setDBOType (type);
//    delete result;
//    delete command;

//    // TODO FIXME
//    assert (false);

////    if (finalize)
////        finalizeDBData (type, buffer, read_list);

//    return buffer;
//}

//void DBInterface::setContextReferencePoint (bool defined, float latitude, float longitude)
//{
//    loginf  << "DBInterface: setContextReferencePoint: start";

//    if (!defined)
//        return;

//    createPropertiesTable();

//    logdbg  << "DBInterface: setContextReferencePoint: defined";
//    insertProperty("reference_point_defined", String::intToString(defined));
//    logdbg  << "DBInterface: setContextReferencePoint: lat";
//    insertProperty("reference_point_latitude", String::doubleToString(latitude));
//    logdbg  << "DBInterface: setContextReferencePoint: long";
//    insertProperty("reference_point_longitude", String::doubleToString(longitude));
//    logdbg  << "DBInterface: setContextReferencePoint: end";
//}
//bool DBInterface::getContextReferencePointDefined ()
//{
//    if (!hasProperty ("reference_point_defined"))
//        return false;
//    else
//        return (bool) String::intFromString(getProperty ("reference_point_defined"));
//}
//std::pair<float, float> DBInterface::getContextReferencePoint ()
//{
//    std::pair<float, float> point;
//    point.first = String::doubleFromString(getProperty ("reference_point_latitude"));
//    point.second = String::doubleFromString(getProperty ("reference_point_longitude"));
//    return point;
//}

//Buffer *DBInterface::getTableList()
//{
//    boost::mutex::scoped_lock l(mutex_);
//    return connection_->getTableList ();
//}

//Buffer *DBInterface::getColumnList(std::string table)
//{
//    boost::mutex::scoped_lock l(mutex_);
//    return connection_->getColumnList(table);
//}

//void DBInterface::printDBSchema ()
//{
//    loginf  << "DBInterface: printDBSchema";

//    Buffer *tables = getTableList ();

//    if (tables->firstWrite())
//    {
//        loginf  << "DBInterface: printDBSchema: no data returned";
//        delete tables;
//        return;
//    }

//    assert (false);
//    // TODO FIXME

////    tables->setIndex(0);
////    std::string table_name;

////    for (unsigned int cnt=0; cnt < tables->getSize(); cnt++)
////    {
////        if (cnt != 0)
////            tables->incrementIndex();

////        table_name = *(std::string *) tables->get(0);

////        loginf  << "DBInterface: printDBSchema: found table '" << table_name << "'";

////        Buffer *columns = getColumnList(table_name);

////        PropertyList *list = columns->getPropertyList();
////        unsigned int name_index = list->getPropertyIndex ("name");
////        unsigned int type_index = list->getPropertyIndex ("type");
////        unsigned int key_index = list->getPropertyIndex ("key");

////        std::string column_name;
////        std::string type;
////        bool key;

////        columns->setIndex(0);

////        for (unsigned int cnt2=0; cnt2 < columns->getSize(); cnt2++)
////        {
////            if (cnt2 != 0)
////                columns->incrementIndex();

////            column_name = *(std::string *) columns->get(name_index);
////            type = *(std::string *) columns->get(type_index);
////            key = *(bool *) columns->get(key_index);

////            loginf  << "DBInterface: printDBSchema: in table '" << table_name << "' is column '" << column_name << "' type '" << type << "' key " << key;
////        }

////        delete columns;
////    }

//    delete tables;
//}

//void DBInterface::clearTableContent (std::string table_name)
//{
//    boost::mutex::scoped_lock l(mutex_);
//    //DELETE FROM tablename;
//    connection_->executeSQL("DELETE FROM "+table_name+";");
//}

//DBResult *DBInterface::queryMinMaxNormalForTable (std::string table)
//{
//    boost::mutex::scoped_lock l(mutex_);
//    logdbg  << "DBInterface: queryMinMaxForTable: getting command";
//    DBCommand *command = sql_generator_->getTableSelectMinMaxNormalStatement (table);

//    //loginf  << "DBInterface: queryMinMaxForTable: executing command '" << command->getCommandString() << "'";
//    DBResult *result = connection_->execute(command);
//    return result;
//}

//DBResult *DBInterface::queryMinMaxForColumn (DBTableColumn *column, std::string table)
//{
//    assert (column);
//    assert (table.size() > 0);

//    boost::mutex::scoped_lock l(mutex_);
//    logdbg  << "DBInterface: queryMinMaxForColumn: getting command";
//    DBCommand *command = sql_generator_->getColumnSelectMinMaxStatement(column, table);

//    logdbg  << "DBInterface: queryMinMaxForTable: executing command";
//    DBResult *result = connection_->execute(command);
//    return result;
//}

//DBResult *DBInterface::getDistinctStatistics (const std::string &type, DBOVariable *variable, unsigned int sensor_number)
//{
//    boost::mutex::scoped_lock l(mutex_);

//    assert (DBObjectManager::getInstance().existsDBObject(type));
//    assert (variable->existsIn(type));

//    DBCommand *command = sql_generator_->getDistinctStatistics(type, variable, sensor_number);
//    DBResult *result = connection_->execute(command);
//    return result;
//}

//Buffer *DBInterface::createFromMinMaxStringBuffer (Buffer *string_buffer, PropertyDataType type)
//{
//    assert (string_buffer);
//    assert (string_buffer->size() >= 1);

//    PropertyList result_list;
//    result_list.addProperty ("min", type);
//    result_list.addProperty ("max", type);


//    //TODO FIXME
//    assert (false);
//    return 0;

////    Buffer *result_buffer = new Buffer (result_list, DBO_UNDEFINED);

////    bool first=true;

////    logdbg  << "DBInterface: createDataFromStringBuffer: result size " << string_buffer->getSize();

////    for (unsigned int cnt=0; cnt < string_buffer->getSize(); cnt ++)
////    {
////        logdbg  << "DBInterface: createDataFromStringBuffer: checking result " << cnt;
////        if (type == P_TYPE_BOOL)
////        {
////            bool min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            bool max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: bool min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: bool first " << min << " max " << max;
////                *((bool*) result_buffer->get(cnt,0)) = min;
////                *((bool*) result_buffer->get(cnt,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: bool other min " << min << " max " << max;
////                if (min < *((bool*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: bool new min " << min;
////                    *((bool*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((bool*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: bool new max " << max;
////                    *((bool*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_CHAR)
////        {
////            int min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            int max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: char min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: char first " << min << " max " << max;
////                *((char*) result_buffer->get(cnt,0)) = min;
////                *((char*) result_buffer->get(cnt,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: char other min " << min << " max " << max;
////                if (min < *((char*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: char new min " << min;
////                    *((char*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((char*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: char new max " << max;
////                    *((char*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_INT)
////        {
////            int min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            int max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: int min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: int first " << min << " max " << max;
////                *((int*) result_buffer->get(0,0)) = min;
////                *((int*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: int other min " << min << " max " << max;
////                if (min < *((int*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: int new min " << min;
////                    *((int*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((int*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: int new max " << max;
////                    *((int*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_UCHAR)
////        {
////            unsigned int min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            unsigned int max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: uchar min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: uchar first " << min << " max " << max;
////                *((unsigned char*) result_buffer->get(0,0)) = min;
////                *((unsigned char*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: uchar other min " << min << " max " << max;
////                if (min < *((unsigned char*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: uchar new min " << min;
////                    *((unsigned char*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((unsigned char*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: uchar new max " << max;
////                    *((unsigned char*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_UINT)
////        {
////            unsigned int min = intFromString (*((std::string*) string_buffer->get(cnt,0)));
////            unsigned int max = intFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: uint min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: uint first " << min << " max " << max;
////                *((unsigned int*) result_buffer->get(0,0)) = min;
////                *((unsigned int*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: uint other min " << min << " max " << max;
////                if (min < *((unsigned int*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: uint new min " << min;
////                    *((unsigned int*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((unsigned int*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: uint new max " << max;
////                    *((unsigned int*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_STRING)
////        {
////            std::string min = *((std::string*) string_buffer->get(cnt,0));
////            std::string max = *((std::string*) string_buffer->get(cnt,1));

////            logdbg  << "DBInterface: createDataFromStringBuffer: string min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: string first " << min << " max " << max;
////                *((std::string*) result_buffer->get(0,0)) = min;
////                *((std::string*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: string other min " << min << " max " << max;
////                if (min.compare(*((std::string*) result_buffer->get(0,0))) < 0)
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: string new min " << min;
////                    *((std::string*) result_buffer->get(0,0)) = min;
////                }
////                if (max.compare(*((std::string*) result_buffer->get(0,1))) > 0)
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: string new max " << max;
////                    *((std::string*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_FLOAT)
////        {
////            float min = doubleFromString (*((std::string*) string_buffer->get(cnt,0)));
////            float max = doubleFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: float min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: float first " << min << " max " << max;
////                *((float*) result_buffer->get(0,0)) = min;
////                *((float*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: float other min " << min << " max " << max;
////                if (min < *((float*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: float new min " << min;
////                    *((float*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((float*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: float new max " << max;
////                    *((float*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else if (type == P_TYPE_DOUBLE)
////        {
////            double min = doubleFromString (*((std::string*) string_buffer->get(cnt,0)));
////            double max = doubleFromString (*((std::string*) string_buffer->get(cnt,1)));

////            logdbg  << "DBInterface: createDataFromStringBuffer: double min " << min << " max " << max;

////            if (first)
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer:double first min " << min << " max " << max;
////                *((double*) result_buffer->get(0,0)) = min;
////                *((double*) result_buffer->get(0,1)) = max;
////                first=false;
////            }
////            else
////            {
////                logdbg  << "DBInterface: createDataFromStringBuffer: double other min " << min << " max " << max;
////                if (min < *((double*) result_buffer->get(0,0)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: double new min " << min;
////                    *((double*) result_buffer->get(0,0)) = min;
////                }
////                if (max > *((double*) result_buffer->get(0,1)))
////                {
////                    logdbg  << "DBInterface: createDataFromStringBuffer: double new max " << max;
////                    *((double*) result_buffer->get(0,1)) = max;
////                }
////            }
////        }
////        else
////            throw std::runtime_error ("DBInterface: createDataFromStringBuffer: unknown property type");
////    }
////    //delete string_buffer;

////    return result_buffer;
//}

//void DBInterface::deleteAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string filter)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    boost::mutex::scoped_lock l(mutex_);
//    connection_->executeSQL(sql_generator_->getDeleteStatement(variable->getCurrentDBColumn(), value, filter));
//}

//void DBInterface::updateAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string new_value, std::string filter)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    connection_->executeSQL(sql_generator_->getUpdateStatement(variable->getCurrentDBColumn(), value, new_value, filter));
//}

//void DBInterface::getMinMaxOfVariable (DBOVariable *variable, std::string filter_condition, std::string &min, std::string &max)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    DBCommand command;
//    command.setCommandString(sql_generator_->getMinMaxSelectStatement(variable->getCurrentDBColumn()->getName(),
//            variable->getCurrentDBColumn()->getDBTableName(), filter_condition));

//    PropertyList list;
//    list.addProperty("min", PropertyDataType::STRING);
//    list.addProperty("max", PropertyDataType::STRING);
//    command.setPropertyList(list);

//    DBResult *result = connection_->execute (&command);
//    assert (result->containsData());
//    Buffer *buffer = result->getBuffer();
//    assert (!buffer->firstWrite());
//    assert (buffer->size() == 1);
//    min = buffer->getString("min").get(0);
//    max = buffer->getString("max").get(0);

//    delete result;
//    delete buffer;

//    loginf << "DBInterface: getMinMaxOfVariable: variable " << variable->getName() << " min " << min << " max " << max;
//}

////void DBInterface::getDistinctValues (DBOVariable *variable, std::string filter_condition, std::vector<std::string> &values)
////{
////    assert (sql_generator_);
////
////    assert (!variable->isMetaVariable());
////    assert (variable->hasCurrentDBColumn());
////
////    DBCommand command;
////    command.setCommandString(sql_generator_->getMinMaxSelectStatement(variable->getCurrentDBColumn()->getName(),
////            variable->getCurrentDBColumn()->getDBTableName(), filter_condition));
////
////    PropertyList list;
////    list.addProperty("value", P_TYPE_STRING);
////    command.setPropertyList(list);
////
////    DBResult *result = connection_->execute (&command);
////    assert (result->containsData());
////    Buffer *buffer = result->getBuffer();
////    assert (!buffer->getFirstWrite());
////
////    for (unsigned int cnt=0; cnt < buffer->getSize(); cnt++)
////    {
////        values.push_back(*(std::string *) (buffer->get(0, cnt)));
////        loginf << "DBInterface: getDistinctValues: variable " << variable->getName() << " value: " << *(std::string *) (buffer->get(0, cnt));
////    }
////
////
////}

//Buffer *DBInterface::getTrackMatches (bool has_mode_a, unsigned int mode_a, bool has_ta, unsigned int ta, bool has_ti, std::string ti,
//        bool has_tod, double tod_min, double tod_max)
//{
//    assert (sql_generator_);
//    assert (has_mode_a || has_ta || has_ti);


//    std::stringstream ss;

//    ss << "select track_num, min(tod), max(tod) from sd_track where mode3a_code";

//    if (has_mode_a)
//        ss << "=" << String::intToString(mode_a);
//    else
//        ss << " IS NULL";

//    ss << " AND target_addr";

//    if (has_ta)
//        ss << "=" << String::intToString(ta);
//    else
//        ss << " IS NULL";

//    ss << " AND callsign";

//    if (has_ti)
//        ss << "='" << ti <<"'";
//    else
//        ss << "='        '";

//    if (has_tod)
//        ss << " AND TOD>"+String::doubleToStringNoScientific(tod_min)+" AND TOD<"+String::doubleToStringNoScientific(tod_max);

//    ss << " group by track_num;";

//    loginf << "DBInterface: getTrackMatches: sql " << ss.str() << "'";

//    DBCommand command;
//    command.setCommandString(ss.str());

//    PropertyList list;
//    list.addProperty("track_num", PropertyDataType::INT);
//    list.addProperty("tod_min", PropertyDataType::DOUBLE);
//    list.addProperty("tod_max", PropertyDataType::DOUBLE);
//    command.setPropertyList(list);

//    DBResult *result = connection_->execute (&command);
//    assert (result->containsData());
//    Buffer *buffer = result->getBuffer();

//    delete result;

//    loginf << "DBInterface: getTrackMatches: done";

//    return buffer;
//}

