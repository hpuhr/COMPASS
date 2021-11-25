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

#include "dbinterface.h"
#include "compass.h"
#include "buffer.h"
#include "config.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "sqliteconnection.h"
#include "dbinterfaceinfowidget.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "dbresult.h"
#include "dbtableinfo.h"
#include "dimension.h"
#include "jobmanager.h"
#include "sqliteconnection.h"
#include "stringconv.h"
#include "unit.h"
#include "unitmanager.h"
#include "sector.h"
#include "sectorlayer.h"
#include "evaluationmanager.h"
#include "dbcontentdbdatasource.h"

#include <QApplication>
#include <QMessageBox>
#include <QMutexLocker>
#include <QThread>

#include "boost/date_time/posix_time/posix_time.hpp"

#include <fstream>

using namespace Utils;
using namespace std;
//using namespace nlohmann;

DBInterface::DBInterface(string class_id, string instance_id, COMPASS* compass)
    : Configurable(class_id, instance_id, compass), sql_generator_(*this)
{
    QMutexLocker locker(&connection_mutex_);

    registerParameter("read_chunk_size", &read_chunk_size_, 50000);

    createSubConfigurables();
}

DBInterface::~DBInterface()
{
    logdbg << "DBInterface: desctructor: start";

    QMutexLocker locker(&connection_mutex_);

    current_connection_ = nullptr;

    logdbg << "DBInterface: desctructor: end";
}

void DBInterface::databaseOpenend()
{
    loginf << "DBInterface: databaseOpenend";

    updateTableInfo();

    if (!existsPropertiesTable())
        createPropertiesTable();

    loadProperties();

    if (!existsSectorsTable())
        createSectorsTable();

    COMPASS::instance().evaluationManager().loadSectors(); // init done in mainwindow

    COMPASS::instance().objectManager().databaseOpenendSlot();
}

void DBInterface::databaseContentChanged()
{
    loginf << "DBInterface: databaseContentChanged";

    updateTableInfo();

    emit databaseContentChangedSignal();
}

void DBInterface::closeConnection()
{
    QMutexLocker locker(&connection_mutex_);

    if (properties_loaded_)  // false if database not opened
        saveProperties();

    logdbg << "DBInterface: closeConnection";
    assert (current_connection_);
    current_connection_->disconnect();


    if (info_widget_)
    {
        delete info_widget_;
        info_widget_ = nullptr;
    }

    table_info_.clear();
    logdbg << "DBInterface: closeConnection: done";
}

void DBInterface::updateTableInfo()
{
    QMutexLocker locker(&connection_mutex_);
    loginf << "DBInterface: updateTableInfo";
    table_info_.clear();

    assert(current_connection_);
    table_info_ = current_connection_->getTableInfo();

    loginf << "DBInterface: updateTableInfo: found " << table_info_.size() << " tables";
}

DBInterfaceInfoWidget* DBInterface::infoWidget()
{
    if (!info_widget_)
    {
        info_widget_ = new DBInterfaceInfoWidget(*this);
    }

    assert(info_widget_);
    return info_widget_;
}

QWidget* DBInterface::connectionWidget()
{
    assert(current_connection_);
    return current_connection_->widget();
}

vector<string> DBInterface::getDatabases()
{
    assert(current_connection_);
    return current_connection_->getDatabases();
}

bool DBInterface::ready()
{
    if (!current_connection_)
    {
        logdbg << "DBInterface: ready: no connection";
        return false;
    }

    logdbg << "DBInterface: ready: connection ready " << current_connection_->ready();
    return current_connection_->ready();
}

SQLiteConnection& DBInterface::connection()
{
    assert(current_connection_);
    return *current_connection_;
}

void DBInterface::generateSubConfigurable(const string& class_id,
                                          const string& instance_id)
{
    logdbg << "DBInterface: generateSubConfigurable: generating variable " << instance_id;

    if (class_id == "SQLiteConnection")
    {
        assert (!current_connection_);

        current_connection_.reset( new SQLiteConnection(class_id, instance_id, this));
        assert (current_connection_);

    }
    else
        throw runtime_error("DBInterface: generateSubConfigurable: unknown class_id " +
                            class_id);
}

void DBInterface::checkSubConfigurables()
{
    if (!current_connection_)
    {
        addNewSubConfiguration("SQLiteConnection", "SQLite Connection");
        generateSubConfigurable("SQLiteConnection", "SQLite Connection");
    }
}

bool DBInterface::existsTable(const string& table_name)
{
    QMutexLocker locker(&connection_mutex_);  // TODO
    return table_info_.count(table_name) == 1;
}

void DBInterface::createTable(const DBObject& object)
{
    loginf << "DBInterface: createTable: obj " << object.name();
    if (existsTable(object.dbTableName()))
    {
        logerr << "DBInterface: createTable: table " << object.dbTableName() << " already exists";
        return;
    }

    string statement = sql_generator_.getCreateTableStatement(object);

    QMutexLocker locker(&connection_mutex_);

    current_connection_->executeSQL(statement);

    locker.unlock();

    updateTableInfo();

    loginf << "DBInterface: createTable: checking " << object.dbTableName();
    assert(existsTable(object.dbTableName()));
}

//bool DBInterface::existsMinMaxTable() { return existsTable(TABLE_NAME_MINMAX); }

bool DBInterface::existsPropertiesTable() { return existsTable(TABLE_NAME_PROPERTIES); }

set<int> DBInterface::queryActiveSensorNumbers(DBObject& object)
{
    logdbg << "DBInterface: queryActiveSensorNumbers: start";

    assert (false); // TODO

//    assert(object.existsInDB());

//    QMutexLocker locker(&connection_mutex_);

//    string local_key_dbovar = object.currentDataSourceDefinition().localKey();
//    assert(object.hasVariable(local_key_dbovar));
//    const DBTableColumn& local_key_col = object.variable(local_key_dbovar).currentDBColumn();

//    set<int> data;

//    shared_ptr<DBCommand> command = sql_generator_.getDistinctDataSourcesSelectCommand(object);

//    shared_ptr<DBResult> result = current_connection_->execute(*command);

//    assert(result->containsData());

//    shared_ptr<Buffer> buffer = result->buffer();
//    for (unsigned int cnt = 0; cnt < buffer->size(); cnt++)
//    {
//        if (buffer->get<int>(local_key_col.name()).isNull(cnt))
//        {
//            logwrn << "DBInterface: queryActiveSensorNumbers: object " << object.name()
//                   << " has NULL ds_id's, which will be omitted";
//        }
//        else
//        {
//            int tmp = buffer->get<int>(local_key_col.name()).get(cnt);
//            data.insert(tmp);
//        }
//    }

//    logdbg << "DBInterface: queryActiveSensorNumbers: done";
//    return data;
}


std::map<unsigned int, std::tuple<std::set<unsigned int>, std::tuple<bool, unsigned int, unsigned int>,
std::tuple<bool, unsigned int, unsigned int>>> DBInterface::queryADSBInfo()
{
    DBObject& object = COMPASS::instance().objectManager().object("ADSB");

    assert(object.existsInDB());

    QMutexLocker locker(&connection_mutex_);

    std::map<unsigned int, std::tuple<std::set<unsigned int>, std::tuple<bool, unsigned int, unsigned int>,
    std::tuple<bool, unsigned int, unsigned int>>> data;

    shared_ptr<DBCommand> command = sql_generator_.getADSBInfoCommand(object);

    shared_ptr<DBResult> result = current_connection_->execute(*command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();
    assert (buffer->has<int>("TARGET_ADDR"));
    assert (buffer->has<int>("MOPS_VERSION"));
    assert (buffer->has<char>("MIN_NUCP_NIC"));
    assert (buffer->has<char>("MAX_NUCP_NIC"));
    assert (buffer->has<char>("MIN_NACP"));
    assert (buffer->has<char>("MAX_NACP"));

    NullableVector<int>& tas = buffer->get<int>("TARGET_ADDR");
    NullableVector<int>& mops = buffer->get<int>("MOPS_VERSION");
    NullableVector<char>& min_nus = buffer->get<char>("MIN_NUCP_NIC");
    NullableVector<char>& max_nus = buffer->get<char>("MAX_NUCP_NIC");
    NullableVector<char>& min_nas = buffer->get<char>("MIN_NACP");
    NullableVector<char>& max_nas = buffer->get<char>("MAX_NACP");

    for (unsigned int cnt = 0; cnt < buffer->size(); cnt++)
    {
        if (tas.isNull(cnt))
            continue;

        if (!mops.isNull(cnt))
            get<0>(data[tas.get(cnt)]).insert(mops.get(cnt));

        if (!min_nus.isNull(cnt) && !max_nus.isNull(cnt))
            get<1>(data[tas.get(cnt)]) =
                    std::tuple<bool, unsigned int, unsigned int>(true, min_nus.get(cnt), max_nus.get(cnt));

        if (!min_nas.isNull(cnt) && !max_nas.isNull(cnt))
            get<2>(data[tas.get(cnt)]) =
                    std::tuple<bool, unsigned int, unsigned int>(true, min_nas.get(cnt), max_nas.get(cnt));
    }

    return data;
}

bool DBInterface::hasDataSources()
{
    return existsTable(DBContent::DBDataSource::table_name_);
}

std::vector<std::unique_ptr<DBContent::DBDataSource>> DBInterface::getDataSources()
{
    logdbg << "DBInterface: getDataSources: start";

    QMutexLocker locker(&connection_mutex_);

    shared_ptr<DBCommand> command = sql_generator_.getDataSourcesSelectCommand();

    loginf << "DBInterface: getDataSources: sql '" << command->get() << "'";

    shared_ptr<DBResult> result = current_connection_->execute(*command);
    assert(result->containsData());
    shared_ptr<Buffer> buffer = result->buffer();

    logdbg << "DBInterface: getDataSources: json '" << buffer->asJSON().dump(4) << "'";

    using namespace DBContent;

    assert(buffer->properties().hasProperty(DBDataSource::id_column_));
    assert(buffer->properties().hasProperty(DBDataSource::db_content_type_column_));
    assert(buffer->properties().hasProperty(DBDataSource::sac_column_));
    assert(buffer->properties().hasProperty(DBDataSource::sic_column_));
    assert(buffer->properties().hasProperty(DBDataSource::name_column_));
    assert(buffer->properties().hasProperty(DBDataSource::short_name_));
    assert(buffer->properties().hasProperty(DBDataSource::info_column_));
    assert(buffer->properties().hasProperty(DBDataSource::counts_column_));

    std::vector<std::unique_ptr<DBContent::DBDataSource>> sources;

    for (unsigned cnt = 0; cnt < buffer->size(); cnt++)
    {
        if (buffer->get<unsigned int>(DBDataSource::id_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL id, will be omitted";
            continue;
        }

        if (buffer->get<string>(DBDataSource::db_content_type_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL content type, will be omitted";
            continue;
        }

        if (buffer->get<unsigned int>(DBDataSource::sac_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL sac, will be omitted";
            continue;
        }

        if (buffer->get<unsigned int>(DBDataSource::sic_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL sic, will be omitted";
            continue;
        }

        if (buffer->get<string>(DBDataSource::name_column_.name()).isNull(cnt))
        {
            logerr << "DBInterface: getDataSources: data source cnt " << cnt
                   << " has NULL name, will be omitted";
            continue;
        }

        std::unique_ptr<DBContent::DBDataSource> src;

        src->id(buffer->get<unsigned int>(DBDataSource::id_column_.name()).get(cnt));
        src->dbContentType(buffer->get<string>(DBDataSource::db_content_type_column_.name()).get(cnt));
        src->sac(buffer->get<unsigned int>(DBDataSource::sac_column_.name()).get(cnt));
        src->sic(buffer->get<unsigned int>(DBDataSource::sic_column_.name()).get(cnt));
        src->name(buffer->get<string>(DBDataSource::name_column_.name()).get(cnt));

        if (!buffer->get<string>(DBDataSource::short_name_.name()).isNull(cnt))
            src->shortName(buffer->get<string>(DBDataSource::short_name_.name()).get(cnt));

        if (!buffer->get<string>(DBDataSource::info_column_.name()).isNull(cnt))
            src->info(buffer->get<string>(DBDataSource::info_column_.name()).get(cnt));

        if (!buffer->get<string>(DBDataSource::counts_column_.name()).isNull(cnt))
            src->counts(buffer->get<string>(DBDataSource::counts_column_.name()).get(cnt));

        sources.emplace_back(move(src));
    }

    return sources;
}

void DBInterface::saveDataSources(const std::vector<std::unique_ptr<DBContent::DBDataSource>>& data_sources)
{
    loginf << "DBInterface: saveDataSources: num " << data_sources.size();

    using namespace DBContent;

    clearTableContent(DBDataSource::table_name_);

    PropertyList list;
    list.addProperty(DBDataSource::id_column_);
    list.addProperty(DBDataSource::db_content_type_column_);
    list.addProperty(DBDataSource::sac_column_);
    list.addProperty(DBDataSource::sic_column_);
    list.addProperty(DBDataSource::name_column_);
    list.addProperty(DBDataSource::short_name_);
    list.addProperty(DBDataSource::info_column_);
    list.addProperty(DBDataSource::counts_column_);

    shared_ptr<Buffer> buffer = make_shared<Buffer>(list);

    unsigned int cnt = 0;
    for (auto& ds_it : data_sources)
    {
        buffer->get<unsigned int>(DBDataSource::id_column_.name()).set(cnt, ds_it->id());
        buffer->get<string>(DBDataSource::db_content_type_column_.name()).set(cnt, ds_it->dbContentType());
        buffer->get<unsigned int>(DBDataSource::sac_column_.name()).set(cnt, ds_it->sac());
        buffer->get<unsigned int>(DBDataSource::sic_column_.name()).set(cnt, ds_it->sic());
        buffer->get<string>(DBDataSource::name_column_.name()).set(cnt, ds_it->name());

        if (ds_it->hasShortName())
            buffer->get<string>(DBDataSource::short_name_.name()).set(cnt, ds_it->shortName());

        buffer->get<string>(DBDataSource::info_column_.name()).set(cnt, ds_it->infoStr());
        buffer->get<string>(DBDataSource::counts_column_.name()).set(cnt, ds_it->countsStr());

        ++cnt;
    }

    insertBuffer(DBDataSource::table_name_, buffer);

    loginf << "DBInterface: saveDataSources: done";
}



size_t DBInterface::count(const string& table)
{
    logdbg << "DBInterface: count: table " << table;
    assert(existsTable(table));

    QMutexLocker locker(&connection_mutex_);
    assert(current_connection_);

    string sql = sql_generator_.getCountStatement(table);

    logdbg << "DBInterface: count: sql '" << sql << "'";

    DBCommand command;
    command.set(sql);

    PropertyList list;
    list.addProperty("count", PropertyDataType::INT);
    command.list(list);

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result->containsData());
    int tmp = result->buffer()->get<int>("count").get(0);

    logdbg << "DBInterface: count: " << table << ": " << tmp << " end";
    return static_cast<size_t>(tmp);
}

void DBInterface::setProperty(const string& id, const string& value)
{
    properties_[id] = value;
}

string DBInterface::getProperty(const string& id)
{
    assert(hasProperty(id));
    return properties_.at(id);
}

bool DBInterface::hasProperty(const string& id) { return properties_.count(id); }

void DBInterface::loadProperties()
{
    loginf << "DBInterface: loadProperties";

    assert (!properties_loaded_);

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllPropertiesStatement());

    PropertyList list;
    list.addProperty("id", PropertyDataType::STRING);
    list.addProperty("value", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    assert(buffer->has<string>("id"));
    assert(buffer->has<string>("value"));

    NullableVector<string> id_vec = buffer->get<string>("id");
    NullableVector<string> value_vec = buffer->get<string>("value");

    for (size_t cnt = 0; cnt < buffer->size(); ++cnt)
    {
        assert(!id_vec.isNull(cnt));

        if (properties_.count(id_vec.get(cnt)))
            logerr << "DBInterface: loadProperties: property '" << id_vec.get(cnt) << "' already exists";

        assert(!properties_.count(id_vec.get(cnt)));
        if (!value_vec.isNull(cnt))
            properties_[id_vec.get(cnt)] = value_vec.get(cnt);
    }

    for (auto& prop_it : properties_)
        loginf << "DBInterface: loadProperties: id '" << prop_it.first << "' value '"
               << prop_it.second << "'";

    properties_loaded_ = true;
}

void DBInterface::saveProperties()
{
    loginf << "DBInterface: saveProperties";

    if (!current_connection_)
    {
        logwrn << "DBInterface: saveProperties: failed since no database connection exists";
        return;
    }

    // QMutexLocker locker(&connection_mutex_); // done in closeConnection
    assert(current_connection_);
    assert (properties_loaded_);

    string str;

    for (auto& prop_it : properties_)
    {
        string str = sql_generator_.getInsertPropertyStatement(prop_it.first, prop_it.second);
        current_connection_->executeSQL(str);
    }

    loginf << "DBInterface: saveProperties: done";
}

std::vector<std::shared_ptr<SectorLayer>> DBInterface::loadSectors()
{
    loginf << "DBInterface: loadSectors";

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllSectorsStatement());

    PropertyList list;
    list.addProperty("id", PropertyDataType::INT);
    list.addProperty("name", PropertyDataType::STRING);
    list.addProperty("layer_name", PropertyDataType::STRING);
    list.addProperty("json", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    assert(buffer->has<int>("id"));
    assert(buffer->has<string>("name"));
    assert(buffer->has<string>("layer_name"));
    assert(buffer->has<string>("json"));

    NullableVector<int> id_vec = buffer->get<int>("id");
    NullableVector<string> name_vec = buffer->get<string>("name");
    NullableVector<string> layer_name_vec = buffer->get<string>("layer_name");
    NullableVector<string> json_vec = buffer->get<string>("json");

    int id;
    string name;
    string layer_name;
    string json_str;

    std::vector<std::shared_ptr<SectorLayer>> sector_layers;

    for (size_t cnt = 0; cnt < buffer->size(); ++cnt)
    {
        assert(!id_vec.isNull(cnt));
        assert(!name_vec.isNull(cnt));
        assert(!layer_name_vec.isNull(cnt));
        assert(!json_vec.isNull(cnt));

        id = id_vec.get(cnt);
        name = name_vec.get(cnt);
        layer_name = layer_name_vec.get(cnt);
        json_str = json_vec.get(cnt);

        auto lay_it = std::find_if(sector_layers.begin(), sector_layers.end(),
                                   [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});

        if (lay_it == sector_layers.end())
        {
            sector_layers.push_back(make_shared<SectorLayer>(layer_name));

            lay_it = std::find_if(
                        sector_layers.begin(), sector_layers.end(),
                        [&layer_name](const shared_ptr<SectorLayer>& x) { return x->name() == layer_name;});
        }

        shared_ptr<Sector> new_sector = make_shared<Sector>(id, name, layer_name, json_str);
        string layer_name = new_sector->layerName();

        (*lay_it)->addSector(new_sector);
        assert ((*lay_it)->hasSector(name));

        loginf << "DBInterface: loadSectors: loaded sector '" << name << "' in layer '"
               << layer_name << "' num points " << (*lay_it)->sector(name)->size();
    }

    return sector_layers;
}

//void DBInterface::insertMinMax(const string& id, const string& object_name,
//                               const string& min, const string& max)
//{
//    QMutexLocker locker(&connection_mutex_);

//    string str = sql_generator_.getInsertMinMaxStatement(id, object_name, min, max);
//    current_connection_->executeSQL(str);
//}

bool DBInterface::areColumnsNull (const std::string& table_name, const std::vector<std::string> columns)
{
    QMutexLocker locker(&connection_mutex_);

    string str = sql_generator_.getSelectNullCount(table_name, columns);

    loginf << "DBInterface: areColumnsNull: sql '" << str << "'";

    DBCommand command;
    command.set(str);

    PropertyList list;
    list.addProperty("count", PropertyDataType::INT);
    command.list(list);

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    assert(buffer->has<int>("count"));

    NullableVector<int> count_vec = buffer->get<int>("count");
    assert (count_vec.size() == 1);

    loginf << "DBInterface: areColumnsNull: null count " << count_vec.get(0);

    return count_vec.get(0) != 0;
}

//pair<string, string> DBInterface::getMinMaxString(const DBOVariable& var)
//{
//    logdbg << "DBInterface: getMinMaxString: var " << var.name();

//    if (!var.dbObject().existsInDB())  // object doesn't exist in this database
//    {
//        logerr << "DBInterface: getMinMaxString: parent object of var " << var.name()
//               << " does not exist in db";
//        return pair<string, string>(NULL_STRING, NULL_STRING);
//    }

//    if (!var.dbObject().count())  // object doesn't exist in this database
//    {
//        logerr << "DBInterface: getMinMaxString: parent object of var " << var.name()
//               << " has no data in db";
//        return pair<string, string>(NULL_STRING, NULL_STRING);
//    }

//    QMutexLocker locker(&connection_mutex_);

//    PropertyList list;
//    list.addProperty("min", PropertyDataType::STRING);
//    list.addProperty("max", PropertyDataType::STRING);

//    // get min max as strings

//    DBCommand command;
//    command.set(
//                sql_generator_.getSelectMinMaxStatement(var.dbColumnName(), var.dboName()));
//    command.list(list);

//    logdbg << "DBInterface: getMinMaxString: sql '" << command.get() << "'";

//    shared_ptr<DBResult> result = current_connection_->execute(command);

//    assert(result);
//    assert(result->containsData());
//    shared_ptr<Buffer> buffer = result->buffer();

//    assert(buffer);
//    if (buffer->size() != 1)
//    {
//        logerr << "DBInterface: getMinMaxString: variable " << var.name() << " has "
//               << buffer->size() << " minmax values";
//        return pair<string, string>(NULL_STRING, NULL_STRING);
//    }

//    if (buffer->get<string>("min").isNull(0) || buffer->get<string>("max").isNull(0))
//    {
//        logerr << "DBInterface: getMinMaxString: variable " << var.name()
//               << " has NULL minimum/maximum";
//        return pair<string, string>(NULL_STRING, NULL_STRING);
//    }

//    string min = buffer->get<string>("min").get(0);
//    string max = buffer->get<string>("max").get(0);

//    logdbg << "DBInterface: getMinMaxString: var " << var.name() << " min " << min << " max "
//           << max;
//    return pair<string, string>(min, max);
//}

bool DBInterface::existsViewPointsTable()
{
    return existsTable(TABLE_NAME_VIEWPOINTS);
}

void DBInterface::createViewPointsTable()
{
    assert(!existsViewPointsTable());

    setProperty("view_points_version", "0.1");

    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getTableViewPointsCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

void DBInterface::setViewPoint(const unsigned int id, const string& value)
{
    if (!current_connection_)
    {
        logwrn << "DBInterface: setViewPoint: failed since no database connection exists";
        return;
    }

    // QMutexLocker locker(&connection_mutex_); // done in closeConnection
    assert(current_connection_);

    if (!existsViewPointsTable())
        createViewPointsTable();

    string str = sql_generator_.getInsertViewPointStatement(id, value);

    logdbg << "DBInterface: setViewPoint: cmd '" << str << "'";
    current_connection_->executeSQL(str);
}

map<unsigned int, string> DBInterface::viewPoints()
{
    loginf << "DBInterface: viewPoints";

    assert (existsViewPointsTable());

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectAllViewPointsStatement());

    PropertyList list;
    list.addProperty("id", PropertyDataType::UINT);
    list.addProperty("json", PropertyDataType::STRING);
    command.list(list);

    shared_ptr<DBResult> result = current_connection_->execute(command);

    assert(result->containsData());

    shared_ptr<Buffer> buffer = result->buffer();

    assert(buffer);
    assert(buffer->has<unsigned int>("id"));
    assert(buffer->has<string>("json"));

    NullableVector<unsigned int> id_vec = buffer->get<unsigned int>("id");
    NullableVector<string> json_vec = buffer->get<string>("json");

    map<unsigned int, string> view_points;

    for (size_t cnt = 0; cnt < buffer->size(); ++cnt)
    {
        assert(!id_vec.isNull(cnt));
        assert(!view_points.count(id_vec.get(cnt)));
        if (!json_vec.isNull(cnt))
            view_points[id_vec.get(cnt)] = json_vec.get(cnt);
    }

    loginf << "DBInterface: loadViewPoints: loaded " << view_points.size() << " view points";

    return view_points;
}

void DBInterface::deleteViewPoint(const unsigned int id)
{
    QMutexLocker locker(&connection_mutex_);
    current_connection_->executeSQL(sql_generator_.getDeleteStatement(TABLE_NAME_VIEWPOINTS,
                                                                      "id="+to_string(id)));
}

void DBInterface::deleteAllViewPoints()
{
    clearTableContent(TABLE_NAME_VIEWPOINTS);
}


bool DBInterface::existsSectorsTable()
{
    return existsTable(TABLE_NAME_SECTORS);
}

void DBInterface::createSectorsTable()
{
    loginf << "DBInterface: createSectorsTable";

    assert(!existsSectorsTable());

    connection_mutex_.lock();

    loginf << "DBInterface: createSectorsTable: sql '" << sql_generator_.getTableSectorsCreateStatement() << "'";
    current_connection_->executeSQL(sql_generator_.getTableSectorsCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}


void DBInterface::clearSectorsTable()
{
    clearTableContent(TABLE_NAME_SECTORS);
}


void DBInterface::saveSector(shared_ptr<Sector> sector)
{
    loginf << "DBInterface: saveSector: sector " << sector->name() << " layer " << sector->layerName()
           << " id " << sector->id();

    if (!current_connection_)
    {
        logwrn << "DBInterface: saveSector: failed since no database connection exists";
        return;
    }

    assert(current_connection_);

    if (!existsSectorsTable())
        createSectorsTable();

    // insert and replace
    string str = sql_generator_.getReplaceSectorStatement(sector->id(), sector->name(), sector->layerName(),
                                                          sector->jsonDataStr());

    logdbg << "DBInterface: saveSector: cmd '" << str << "'";
    {
        QMutexLocker locker(&connection_mutex_);
        current_connection_->executeSQL(str);
    }


}

void DBInterface::deleteSector(shared_ptr<Sector> sector)
{
    unsigned int sector_id = sector->id();

    QMutexLocker locker(&connection_mutex_);
    string cmd = sql_generator_.getDeleteStatement(TABLE_NAME_SECTORS,"id="+to_string(sector_id));

    //loginf << "UGA '" << cmd << "'";
    current_connection_->executeSQL(cmd);
}

void DBInterface::deleteAllSectors()
{
    clearTableContent(TABLE_NAME_SECTORS);
}


//bool DBInterface::hasActiveDataSources(DBObject& object)
//{
////    if (!object.existsInDB())
////        return false;

////    if (!existsPropertiesTable())
////        return false;

////    return hasProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX + object.name());
//    assert (false); // TODO
//}

//set<int> DBInterface::getActiveDataSources(DBObject& object)
//{
//    logdbg << "DBInterface: getActiveDataSources: start";

//    assert (false); // TODO

////    assert(hasActiveDataSources(object));

////    string tmp = getProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX + object.name());

////    set<int> ret;

////    vector<string> tmp2 = String::split(tmp, ',');

////    loginf << "DBInterface: getActiveDataSources: got " << tmp2.size() << " parts from '" << tmp
////           << "'";

////    for (unsigned int cnt = 0; cnt < tmp2.size(); cnt++)
////    {
////        ret.insert(stoi(tmp2.at(cnt)));
////        loginf << "DBInterface: getActiveDataSources: got active source " << cnt << " '"
////               << stoi(tmp2.at(cnt)) << "'";
////    }

////    logdbg << "DBInterface: getActiveDataSources: end";
////    return ret;
//}

void DBInterface::insertBuffer(const DBObject& dbobject, std::shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: insertBuffer: dbo " << dbobject.name() << " buffer size "
           << buffer->size();

    if (!existsTable(dbobject.dbTableName()))
        createTable(dbobject);

    insertBuffer(dbobject.dbTableName(), buffer);
}

void DBInterface::insertBuffer(const string& table_name, shared_ptr<Buffer> buffer)
{
    logdbg << "DBInterface: insertBuffer: table name " << table_name << " buffer size "
           << buffer->size();

    assert(current_connection_);
    assert(buffer);

    if (!existsTable(table_name))
        throw runtime_error("DBInterface: insertBuffer: table with name '" + table_name +
                            "' does not exist");

    const PropertyList& properties = buffer->properties();

    assert(table_info_.count(table_name));

    DBTableInfo& table_info = table_info_.at(table_name);

    for (unsigned int cnt = 0; cnt < properties.size(); ++cnt)
    {
        logdbg << "DBInterface: insertBuffer: checking column '" << properties.at(cnt).name()
               << "'";

        if (!table_info.hasColumn(properties.at(cnt).name()))
            throw runtime_error("DBInterface: insertBuffer: column '" +
                                properties.at(cnt).name() + "' does not exist in table " +
                                table_name);
    }

    string bind_statement = sql_generator_.insertDBUpdateStringBind(buffer, table_name);

    QMutexLocker locker(&connection_mutex_);

    logdbg << "DBInterface: insertBuffer: preparing bind statement";
    current_connection_->prepareBindStatement(bind_statement);
    current_connection_->beginBindTransaction();

    logdbg << "DBInterface: insertBuffer: starting inserts";
    size_t size = buffer->size();

    for (unsigned int cnt = 0; cnt < size; ++cnt)
    {
        insertBindStatementUpdateForCurrentIndex(buffer, cnt);
    }

    logdbg << "DBInterface: insertBuffer: ending bind transactions";
    current_connection_->endBindTransaction();
    logdbg << "DBInterface: insertBuffer: finalizing bind statement";
    current_connection_->finalizeBindStatement();
}

//bool DBInterface::checkUpdateBuffer(DBObject& object, DBOVariable& key_var, DBOVariableSet& list,
//                                    shared_ptr<Buffer> buffer)
//{
//    if (!object.existsInDB())
//        return false;

//    if (!key_var.existsInDB())
//        return false;

//    const DBTable& table = object.currentMetaTable().mainTable();

//    if (!table.existsInDB())  // might be redundant
//        return false;

//    const PropertyList& properties = buffer->properties();

//    for (auto& var_it : list.getSet())
//    {
//        if (!properties.hasProperty(var_it->name()))
//            return false;

//        if (!var_it->hasCurrentDBColumn())
//            return false;

//        const DBTableColumn& col = var_it->currentDBColumn();

//        if (!col.existsInDB())
//            return false;
//    }

//    return true;
//}

//void DBInterface::updateBuffer(MetaDBTable& meta_table, const DBTableColumn& key_col,
//                               shared_ptr<Buffer> buffer, int from_index, int to_index)
//{
//    logdbg << "DBInterface: updateBuffer: meta " << meta_table.name() << " buffer size "
//           << buffer->size() << " key " << key_col.identifier();

//    shared_ptr<Buffer> partial_buffer = getPartialBuffer(meta_table.mainTable(), buffer);
//    assert(partial_buffer->size());
//    updateBuffer(meta_table.mainTable(), key_col, partial_buffer, from_index, to_index);

//    for (auto& sub_it : meta_table.subTables())
//    {
//        if (sub_it.second.hasColumn(key_col.name()))
//        {
//            const DBTableColumn& sub_key_col = sub_it.second.column(key_col.name());
//            logdbg << "DBInterface: updateBuffer: got sub table " << sub_it.second.name()
//                   << " key col " << sub_key_col.identifier();

//            partial_buffer = getPartialBuffer(sub_it.second, buffer);
//            if (partial_buffer->size())
//            {
//                logdbg << "DBInterface: updateBuffer: doing update for sub table "
//                       << sub_it.second.name();
//                updateBuffer(sub_it.second, sub_key_col, partial_buffer, from_index, to_index);
//            }
//            else
//                logdbg << "DBInterface: updateBuffer: empty buffer for sub table "
//                       << sub_it.second.name();
//        }
//        else
//            logdbg << "DBInterface: updateBuffer: key not found in sub table "
//                   << sub_it.second.name();
//    }
//}

void DBInterface::updateBuffer(const std::string& table_name, const std::string& key_col,
                               shared_ptr<Buffer> buffer, int from_index, int to_index)
{
    logdbg << "DBInterface: updateBuffer: table " << table_name << " buffer size "
           << buffer->size() << " key " << key_col;

    // assert (checkUpdateBuffer(object, key_var, buffer));
    assert(current_connection_);
    assert(buffer);

    // TODO check
//    const PropertyList& properties = buffer->properties();

//    for (unsigned int cnt = 0; cnt < properties.size(); cnt++)
//    {
//        if (!table.hasColumn(properties.at(cnt).name()))
//            throw runtime_error("DBInterface: updateBuffer: column '" +
//                                properties.at(cnt).name() + "' does not exist in table " +
//                                table.name());
//    }

    string bind_statement =
            sql_generator_.createDBUpdateStringBind(buffer, key_col, table_name);

    QMutexLocker locker(&connection_mutex_);

    logdbg << "DBInterface: updateBuffer: preparing bind statement '" << bind_statement << "'";
    current_connection_->prepareBindStatement(bind_statement);
    current_connection_->beginBindTransaction();

    if (from_index < 0)
        from_index = 0;
    if (to_index < 0)
        to_index = buffer->size() - 1;

    logdbg << "DBInterface: updateBuffer: starting inserts";
    for (int cnt = from_index; cnt <= to_index; cnt++)
    {
        logdbg << "DBInterface: updateBuffer: insert cnt " << cnt;
        insertBindStatementUpdateForCurrentIndex(buffer, cnt);
    }

    logdbg << "DBInterface: updateBuffer: ending bind transactions";
    current_connection_->endBindTransaction();
    logdbg << "DBInterface: update: finalizing bind statement";
    current_connection_->finalizeBindStatement();
}

void DBInterface::prepareRead(const DBObject& dbobject, DBOVariableSet read_list,
                              string custom_filter_clause,
                              vector<DBOVariable*> filtered_variables, bool use_order,
                              DBOVariable* order_variable, bool use_order_ascending,
                              const string& limit)
{
    assert(current_connection_);

    assert(dbobject.existsInDB());

    connection_mutex_.lock();

    shared_ptr<DBCommand> read = sql_generator_.getSelectCommand(
                dbobject, read_list, custom_filter_clause, use_order,
                order_variable, use_order_ascending, limit);

    logdbg << "DBInterface: prepareRead: dbo " << dbobject.name() << " sql '" << read->get() << "'";
    current_connection_->prepareCommand(read);
}

/**
 * Retrieves result from connection stepPreparedCommand, calls activateKeySearch on buffer and
 * returns it.
 */
shared_ptr<Buffer> DBInterface::readDataChunk(const DBObject& dbobject)
{
    // locked by prepareRead
    assert(current_connection_);

    shared_ptr<DBResult> result = current_connection_->stepPreparedCommand(read_chunk_size_);

    if (!result)
    {
        logerr << "DBInterface: readDataChunk: connection returned error";
        throw runtime_error("DBInterface: readDataChunk: connection returned error");
    }

    if (!result->containsData())
    {
        logerr << "DBInterface: readDataChunk: buffer does not contain data";
        throw runtime_error("DBInterface: readDataChunk: buffer does not contain data");
    }

    shared_ptr<Buffer> buffer = result->buffer();

    buffer->dboName(dbobject.name());

    assert(buffer);

    bool last_one = current_connection_->getPreparedCommandDone();
    buffer->lastOne(last_one);

    return buffer;
}

void DBInterface::finalizeReadStatement(const DBObject& dbobject)
{
    connection_mutex_.unlock();
    assert(current_connection_);

    logdbg << "DBInterface: finishReadSystemTracks: start ";
    current_connection_->finalizeCommand();
}

void DBInterface::createPropertiesTable()
{
    assert(!existsPropertiesTable());
    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getTablePropertiesCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo();
}

//void DBInterface::createMinMaxTable()
//{
//    assert(!existsMinMaxTable());
//    connection_mutex_.lock();
//    current_connection_->executeSQL(sql_generator_.getTableMinMaxCreateStatement());
//    connection_mutex_.unlock();

//    updateTableInfo();
//}

void DBInterface::clearTableContent(const string& table_name)
{
    QMutexLocker locker(&connection_mutex_);
    // DELETE FROM tablename;
    current_connection_->executeSQL("DELETE FROM " + table_name + ";");
}

shared_ptr<DBResult> DBInterface::queryMinMaxNormalForTable(const std::string& table_name)
{
    assert (false); // TODO

    //    QMutexLocker locker(&connection_mutex_);
//    logdbg << "DBInterface: queryMinMaxForTable: getting command";
//    shared_ptr<DBCommand> command = sql_generator_.getTableSelectMinMaxNormalStatement(table);

//    // loginf  << "DBInterface: queryMinMaxForTable: executing command '" <<
//    // command->getCommandString() << "'";
//    shared_ptr<DBResult> result = current_connection_->execute(*command);
//    return result;
}

void DBInterface::insertBindStatementUpdateForCurrentIndex(shared_ptr<Buffer> buffer,
                                                           unsigned int row)
{
    assert(buffer);
    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: start";
    const PropertyList& list = buffer->properties();
    unsigned int size = list.size();
    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: creating bind for " << size
           << " elements";

    string connection_type = current_connection_->type();

    assert(connection_type == SQLITE_IDENTIFIER);

    unsigned int index_cnt = 0;

    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: starting for loop";
    for (unsigned int cnt = 0; cnt < size; cnt++)
    {
        const Property& property = list.at(cnt);
        PropertyDataType data_type = property.dataType();

        logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: at cnt " << cnt << " id "
               << property.name() << " index cnt " << index_cnt;

        index_cnt = cnt + 1;

        if (buffer->isNone(property, row))
        {
            current_connection_->bindVariableNull(index_cnt);
            logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: at " << cnt
                   << " is null";
            continue;
        }

        switch (data_type)
        {
        case PropertyDataType::BOOL:
            current_connection_->bindVariable(
                        index_cnt, static_cast<int>(buffer->get<bool>(property.name()).get(row)));
            break;
        case PropertyDataType::CHAR:
            current_connection_->bindVariable(
                        index_cnt, static_cast<int>(buffer->get<char>(property.name()).get(row)));
            break;
        case PropertyDataType::UCHAR:
            current_connection_->bindVariable(
                        index_cnt,
                        static_cast<int>(buffer->get<unsigned char>(property.name()).get(row)));
            break;
        case PropertyDataType::INT:
            current_connection_->bindVariable(
                        index_cnt, static_cast<int>(buffer->get<int>(property.name()).get(row)));
            break;
        case PropertyDataType::UINT:
            current_connection_->bindVariable(
                        index_cnt, static_cast<int>(buffer->get<unsigned int>(property.name()).get(row)));
            break;
        case PropertyDataType::LONGINT:
            assert(false);
            break;
        case PropertyDataType::ULONGINT:
            assert(false);
            break;
        case PropertyDataType::FLOAT:
            current_connection_->bindVariable(
                        index_cnt, static_cast<double>(buffer->get<float>(property.name()).get(row)));
            break;
        case PropertyDataType::DOUBLE:
            current_connection_->bindVariable(index_cnt,
                                              buffer->get<double>(property.name()).get(row));
            break;
        case PropertyDataType::STRING:

            current_connection_->bindVariable(
                        index_cnt, buffer->get<string>(property.name()).get(row));
            break;
        default:
            logerr << "Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type "
                       << Property::asString(data_type);
            throw runtime_error(
                        "Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type " +
                        Property::asString(data_type));
        }
    }

    current_connection_->stepAndClearBindings();

    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: done";
}

void DBInterface::createAssociationsTable(const string& table_name)
{
    assert(!existsTable(table_name));
    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getCreateAssociationTableStatement(table_name));
    connection_mutex_.unlock();

    updateTableInfo();
}

DBOAssociationCollection DBInterface::getAssociations(const string& table_name)
{
    assert(existsTable(table_name));

    DBOAssociationCollection associations;
    shared_ptr<DBCommand> command = sql_generator_.getSelectAssociationsCommand(table_name);

    connection_mutex_.lock();

    shared_ptr<DBResult> result = current_connection_->execute(*command.get());

    connection_mutex_.unlock();

    if (result->containsData())
    {
        shared_ptr<Buffer> buffer = result->buffer();

        if (buffer->size())
        {
            size_t num_associations = buffer->size();
            assert(buffer->has<int>("rec_num"));
            assert(buffer->has<int>("utn"));
            assert(buffer->has<int>("src_rec_num"));

            NullableVector<int>& rec_nums = buffer->get<int>("rec_num");
            NullableVector<int>& utns = buffer->get<int>("utn");
            NullableVector<int>& src_rec_nums = buffer->get<int>("src_rec_num");

            bool has_src;

            for (size_t cnt = 0; cnt < num_associations; ++cnt)
            {
                assert(!rec_nums.isNull(cnt));
                assert(!utns.isNull(cnt));

                has_src = !src_rec_nums.isNull(cnt);

                if (has_src)
                    associations.add(rec_nums.get(cnt),
                                     DBOAssociationEntry(utns.get(cnt), true, src_rec_nums.get(cnt)));
                else
                    associations.add(rec_nums.get(cnt),
                                     DBOAssociationEntry(utns.get(cnt), false, 0));
            }
        }
    }

    return associations;
}


