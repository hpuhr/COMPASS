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

#include <QMutexLocker>
#include <QMessageBox>
#include <QThread>
#include <QProgressDialog>

#include "atsdb.h"
#include "buffer.h"
#include "config.h"
#include "dbobject.h"
#include "dbodatasource.h"
#include "dbcommand.h"
#include "dbcommandlist.h"
#include "mysqlserver.h"
#include "dbconnection.h"
#include "mysqlppconnection.h"
#include "sqliteconnection.h"
#include "dbinterfacewidget.h"
#include "dbinterfaceinfowidget.h"
#include "dbinterface.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "dbresult.h"
#include "metadbtable.h"
#include "dbschemamanager.h"
#include "dbschema.h"
//#include "StructureDescriptionManager.h"
#include "jobmanager.h"
#include "dboactivedatasourcesdbjob.h"
#include "dbominmaxdbjob.h"
#include "dimension.h"
#include "unit.h"
#include "unitmanager.h"
#include "dbtableinfo.h"
#include "dbtable.h"
#include "stringconv.h"

using namespace Utils;


/**
 * Creates SQLGenerator, several containers based in DBOs (prepared_, reading_done_, exists_, count_), creates
 * write_table_names_,
 */
DBInterface::DBInterface(std::string class_id, std::string instance_id, ATSDB *atsdb)
    : Configurable (class_id, instance_id, atsdb), current_connection_(nullptr), sql_generator_(*this),
      widget_(nullptr), info_widget_(nullptr)
{
    QMutexLocker locker(&connection_mutex_);

    registerParameter ("read_chunk_size", &read_chunk_size_, 50000);
    registerParameter ("used_connection", &used_connection_, "");

    createSubConfigurables();
}

/**
 * If required, deletes connection, sql_generator_ and buffer_writer_.
 */
DBInterface::~DBInterface()
{
    logdbg  << "DBInterface: desctructor: start";

    QMutexLocker locker(&connection_mutex_);

    for (auto it : connections_)
        delete it.second;

    connections_.clear();

    assert (!widget_);

    logdbg  << "DBInterface: desctructor: end";
}

/**
 * Generates connection based on the DB_CONNECTION_TYPE of info, calls init on it. If a new database will be created,
 * creates the buffer_writer_, else calls updateExists and updateCount.
 */
void DBInterface::useConnection (const std::string &connection_type)
{
    logdbg << "DBInterface: useConnection: '" << connection_type << "'";
    if (current_connection_)
        assert (!current_connection_->ready());

    current_connection_ = connections_.at(connection_type);
    used_connection_ = connection_type;

    assert (current_connection_);
}

void DBInterface::databaseContentChanged ()
{
    updateTableInfo();

    if (!existsPropertiesTable())
        createPropertiesTable();

    loginf << "DBInterface: databaseOpened: post-processed " << isPostProcessed ();

    emit databaseContentChangedSignal();
}

void DBInterface::closeConnection ()
{
    QMutexLocker locker(&connection_mutex_);

    logdbg  << "DBInterface: closeConnection";
    for (auto it : connections_)
        it.second->disconnect ();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

    if (info_widget_)
    {
        delete info_widget_;
        info_widget_ = nullptr;
    }

    table_info_.clear();
    logdbg  << "DBInterface: closeConnection: done";
}

void DBInterface::updateTableInfo ()
{
    QMutexLocker locker(&connection_mutex_);
    loginf << "DBInterface: updateTableInfo";
    table_info_.clear();

    assert (current_connection_);
    table_info_ = current_connection_->getTableInfo();

    loginf << "DBInterface: updateTableInfo: found " << table_info_.size() << " tables";
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

DBInterfaceInfoWidget *DBInterface::infoWidget()
{
    if (!info_widget_)
    {
        info_widget_ = new DBInterfaceInfoWidget (*this);
    }

    assert (info_widget_);
    return info_widget_;
}

QWidget *DBInterface::connectionWidget()
{
    assert (current_connection_);
    return current_connection_->widget();
}

std::vector <std::string> DBInterface::getDatabases ()
{
    assert (current_connection_);
    return current_connection_->getDatabases();
}

bool DBInterface::ready ()
{
    if (!current_connection_)
    {
        logwrn << "DBInterface: ready: no connection";
        return false;
    }

    logdbg << "DBInterface: ready: connection ready " << current_connection_->ready();
    return current_connection_->ready();
}

DBConnection &DBInterface::connection ()
{
    assert (ready());
    return *current_connection_;
}

void DBInterface::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "DBInterface: generateSubConfigurable: generating variable " << instance_id;
    if (class_id == "MySQLppConnection")
    {
        MySQLppConnection *connection = new MySQLppConnection (class_id, instance_id, this);
        assert (connections_.count (connection->instanceId()) == 0);
        connections_.insert (std::pair <std::string, DBConnection*> (connection->instanceId(),
                                                                     dynamic_cast<DBConnection*>(connection)));
    }
    else if (class_id == "SQLiteConnection")
    {
        SQLiteConnection *connection = new SQLiteConnection (class_id, instance_id, this);
        assert (connections_.count (connection->instanceId()) == 0);
        connections_.insert (std::pair <std::string, DBConnection*> (connection->instanceId(),
                                                                     dynamic_cast<DBConnection*>(connection)));
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

    if (connections_.count("SQLite Connection") == 0)
    {
        addNewSubConfiguration ("SQLiteConnection", "SQLite Connection");
        generateSubConfigurable ("SQLiteConnection", "SQLite Connection");
    }
}

bool DBInterface::existsTable (const std::string& table_name)
{
    QMutexLocker locker(&connection_mutex_); // TODO
    return table_info_.count (table_name) == 1;
}

void DBInterface::createTable (DBTable& table)
{
    loginf << "DBInterface: createTable: " << table.name();
    if (existsTable(table.name()))
    {
        logerr << "DBInterface: createTable: table " << table.name() << " already exists";
        return;
    }

    std::string statement = sql_generator_.getCreateTableStatement(table);

    QMutexLocker locker(&connection_mutex_);

    current_connection_->executeSQL(statement);

    locker.unlock();

    updateTableInfo();
    table.updateOnDatabase();

    loginf << "DBInterface: createTable: checking " << table.name();
    assert (table.existsInDB());
    //emit databaseContentChangedSignal();
}

/**
 * Returns existsTable for table name.
 */
bool DBInterface::existsMinMaxTable ()
{
    return existsTable(TABLE_NAME_MINMAX);
}

/**
 * Returns existsTable for table name.
 */
bool DBInterface::existsPropertiesTable ()
{
    return existsTable(TABLE_NAME_PROPERTIES);
}

///**
// * Gets SQL command for data sources list and packs the resulting buffer into a set, which is returned.
// */
std::set<int> DBInterface::queryActiveSensorNumbers(DBObject &object)
{
    logdbg  << "DBInterface: queryActiveSensorNumbers: start";

    assert (object.existsInDB());
    assert (object.hasCurrentDataSourceDefinition());

    QMutexLocker locker(&connection_mutex_);

    std::string local_key_dbovar = object.currentDataSourceDefinition().localKey();
    assert (object.hasVariable(local_key_dbovar));
    const DBTableColumn& local_key_col = object.variable(local_key_dbovar).currentDBColumn();

    std::set<int> data;

    std::shared_ptr<DBCommand> command = sql_generator_.getDistinctDataSourcesSelectCommand(object);

    std::shared_ptr<DBResult> result = current_connection_->execute(*command);

    assert (result->containsData());

    std::shared_ptr<Buffer> buffer = result->buffer();
    for (unsigned int cnt=0; cnt < buffer->size(); cnt++)
    {
        if (buffer->get<int>(local_key_col.name()).isNone(cnt))
        {
            logwrn << "DBInterface: queryActiveSensorNumbers: object " << object.name()
                   << " has NULL ds_id's, which will be omitted";
        }
        else
        {
            int tmp = buffer->get<int>(local_key_col.name()).get(cnt);
            data.insert (tmp);
        }
    }

    logdbg << "DBInterface: queryActiveSensorNumbers: done";
    return data;
}

bool DBInterface::hasDataSourceTables (DBObject& object)
{
//    if (!object.existsInDB())
//        return false;

    if (!object.hasCurrentDataSourceDefinition())
        return false;

    const DBODataSourceDefinition &ds = object.currentDataSourceDefinition ();
    const DBSchema &schema = ATSDB::instance().schemaManager().getCurrentSchema();

    if (!schema.hasMetaTable(ds.metaTableName()))
        return false;

    const MetaDBTable& meta = schema.metaTable(ds.metaTableName());

    if (!meta.hasColumn(ds.foreignKey()))
        return false;

    if (!meta.hasColumn(ds.nameColumn()))
        return false;

    std::string main_table_name = meta.mainTableName();

    if (!existsTable(main_table_name))
        return false;

    return true;
}

void DBInterface::updateDataSource (DBODataSource& data_source)
{
    loginf << "DBInterface: updateDataSource: object " << data_source.object().name()
           << " source " << data_source.id();

    // TODO
}

///**
// * Gets SQL command, executes it and returns resulting buffer.
// */
std::map <int, DBODataSource> DBInterface::getDataSources (DBObject &object)
{
    logdbg  << "DBInterface: getDataSources: start";

    //assert (object.existsInDB());

    QMutexLocker locker(&connection_mutex_);

    std::shared_ptr<DBCommand> command = sql_generator_.getDataSourcesSelectCommand(object);

    logdbg << "DBInterface: getDataSources: sql '" << command->get() << "'";

    std::shared_ptr <DBResult> result = current_connection_->execute(*command);
    assert (result->containsData());
    std::shared_ptr <Buffer> buffer = result->buffer();

    const DBODataSourceDefinition &ds = object.currentDataSourceDefinition ();
    const DBSchema &schema = ATSDB::instance().schemaManager().getCurrentSchema();
    assert (schema.hasMetaTable(ds.metaTableName()));

    const MetaDBTable& meta =  schema.metaTable(ds.metaTableName());

    const DBTableColumn& foreign_key_col = meta.column(ds.foreignKey());
    const DBTableColumn& name_col = meta.column(ds.nameColumn());

    assert (buffer->properties().hasProperty(foreign_key_col.name()));
    assert (buffer->properties().get(foreign_key_col.name()).dataType() == PropertyDataType::INT);
    assert (buffer->properties().hasProperty(name_col.name()));
    assert (buffer->properties().get(name_col.name()).dataType() == PropertyDataType::STRING);

    bool has_short_name = ds.hasShortNameColumn();
    std::string short_name_col_name;
    if (has_short_name)
    {
        short_name_col_name = meta.column(ds.shortNameColumn()).name();
        assert (buffer->properties().hasProperty(short_name_col_name)
                && buffer->properties().get(short_name_col_name).dataType() == PropertyDataType::STRING);
    }

    bool has_sac = ds.hasSacColumn();
    std::string sac_col_name;
    if (has_sac)
    {
        sac_col_name = meta.column(ds.sacColumn()).name();
        assert (buffer->properties().hasProperty(sac_col_name)
                && buffer->properties().get(sac_col_name).dataType() == PropertyDataType::CHAR);
    }

    bool has_sic = ds.hasSicColumn();
    std::string sic_col_name;
    if (has_sic)
    {
        sic_col_name = meta.column(ds.sicColumn()).name();
        assert (buffer->properties().hasProperty(sic_col_name)
                && buffer->properties().get(sic_col_name).dataType() == PropertyDataType::CHAR);
    }

    bool has_latitude = ds.hasLatitudeColumn();
    std::string latitude_col_name;
    if (has_latitude)
    {
        latitude_col_name = meta.column(ds.latitudeColumn()).name();
        assert (buffer->properties().hasProperty(latitude_col_name)
                && buffer->properties().get(latitude_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_longitude = ds.hasLongitudeColumn();
    std::string longitude_col_name;
    if (has_longitude)
    {
        longitude_col_name = meta.column(ds.longitudeColumn()).name();
        assert (buffer->properties().hasProperty(longitude_col_name)
                && buffer->properties().get(longitude_col_name).dataType() == PropertyDataType::DOUBLE);
    }

    bool has_altitude = ds.hasAltitudeColumn();
    std::string altitude_col_name;
    if (has_altitude)
    {
        altitude_col_name = meta.column(ds.altitudeColumn()).name();
        assert (buffer->properties().hasProperty(altitude_col_name)
                && buffer->properties().get(altitude_col_name).dataType() == PropertyDataType::DOUBLE);
    }


    std::map <int, DBODataSource> sources;

    for (unsigned cnt = 0; cnt < buffer->size(); cnt++)
    {
        if (buffer->get<int>(foreign_key_col.name()).isNone(cnt))
        {
            loginf << "DBInterface: getDataSources: object " << object.name()
                   << " has NULL key, which will be omitted";
            continue;
        }

        if (buffer->get<std::string>(name_col.name()).isNone(cnt))
        {
            loginf << "DBInterface: getDataSources: object " << object.name()
                   << " has NULL name, which will be omitted";
            continue;
        }

        int key = buffer->get<int>(foreign_key_col.name()).get(cnt);
        std::string name = buffer->get<std::string>(name_col.name()).get(cnt);

        assert (sources.count(key) == 0);
        logdbg << "DBInterface: getDataSources: object " << object.name() << " key " << key << " name " << name;
        //sources.insert(std::make_pair(key, DBODataSource(key, name)));

        sources.emplace(std::piecewise_construct,
                     std::forward_as_tuple(key),  // args for key
                     std::forward_as_tuple(object, key, name));  // args for mapped value

        if (has_short_name && !buffer->get<std::string>(short_name_col_name).isNone(cnt))
            sources.at(key).shortName(buffer->get<std::string>(short_name_col_name).get(cnt));

        if (has_sac && !buffer->get<char>(sac_col_name).isNone(cnt))
            sources.at(key).sac(buffer->get<char>(sac_col_name).get(cnt));

        if (has_sic && !buffer->get<char>(sic_col_name).isNone(cnt))
            sources.at(key).sic(buffer->get<char>(sic_col_name).get(cnt));

        if (has_latitude && !buffer->get<double>(latitude_col_name).isNone(cnt))
            sources.at(key).latitude(buffer->get<double>(latitude_col_name).get(cnt));

        if (has_longitude && !buffer->get<double>(longitude_col_name).isNone(cnt))
            sources.at(key).longitude(buffer->get<double>(longitude_col_name).get(cnt));

        if (has_altitude && !buffer->get<double>(altitude_col_name).isNone(cnt))
            sources.at(key).altitude(buffer->get<double>(altitude_col_name).get(cnt));
    }

    return sources;
}

size_t DBInterface::count (const std::string &table)
{
    logdbg  << "DBInterface: count: table " << table;
    assert (existsTable(table));

    QMutexLocker locker(&connection_mutex_);
    assert (current_connection_);

    std::string sql = sql_generator_.getCountStatement(table);

    logdbg  << "DBInterface: count: sql '" << sql << "'";

    DBCommand command;
    command.set(sql);

    PropertyList list;
    list.addProperty("count", PropertyDataType::INT);
    command.list(list);

    std::shared_ptr <DBResult> result = current_connection_->execute(command);

    assert (result->containsData());
    int tmp = result->buffer()->get<int>("count").get(0);

    logdbg  << "DBInterface: count: " << table << ": "<< tmp <<" end";
    return static_cast<size_t> (tmp);
}

void DBInterface::setProperty (const std::string& id, const std::string& value)
{
    QMutexLocker locker(&connection_mutex_);
    assert (current_connection_);

    std::string str = sql_generator_.getInsertPropertyStatement(id, value);
    current_connection_->executeSQL (str);
}

std::string DBInterface::getProperty (const std::string& id)
{
    logdbg  << "DBInterface: getProperty: start";

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectPropertyStatement(id));

    PropertyList list;
    list.addProperty("property", PropertyDataType::STRING);
    command.list(list);

    std::shared_ptr <DBResult> result = current_connection_->execute(command);

    assert (result->containsData());

    std::string text;
    std::shared_ptr <Buffer> buffer = result->buffer();

    assert (buffer);

    if (buffer->firstWrite())
        throw std::invalid_argument ("DBInterface: getProperty: id "+id+" does not exist");

    assert (buffer->size() == 1);

    text = buffer->get<std::string>("property").get(0);

    logdbg  << "DBInterface: getProperty: end id " << id << " value " << text;
    return text;
}

bool DBInterface::hasProperty (const std::string& id)
{
    logdbg  << "DBInterface: hasProperty: start";

    QMutexLocker locker(&connection_mutex_);

    DBCommand command;
    command.set(sql_generator_.getSelectPropertyStatement(id));

    PropertyList list;
    list.addProperty("property", PropertyDataType::STRING);
    command.list(list);

    std::shared_ptr <DBResult> result = current_connection_->execute(command);

    assert (result->containsData());

    std::shared_ptr <Buffer> buffer = result->buffer();

    assert (buffer);

    return buffer->size() == 1;
}

void DBInterface::insertMinMax (const std::string& id, const std::string& object_name, const std::string& min,
                                const std::string& max)
{
    QMutexLocker locker(&connection_mutex_);

    std::string str = sql_generator_.getInsertMinMaxStatement(id, object_name, min, max);
    current_connection_->executeSQL (str);
}

/**
 * If variable is a not meta variable, min/max values just for the variable. If it is, gets min/max values for all
 *  subvariables and calculates the min/max for all subvariables. If the variable needs a unit transformation, it is
 *  performed (locally in this thread).
 */
std::pair<std::string, std::string> DBInterface::getMinMaxString (const DBOVariable& var)
{
    logdbg << "DBInterface: getMinMaxString: var " << var.name();

    if (!var.dbObject().existsInDB()) // object doesn't exist in this database
    {
        logerr << "DBInterface: getMinMaxString: parent object of var " << var.name() << " does not exist in db";
        return std::pair<std::string, std::string> (NULL_STRING, NULL_STRING);
    }

    if (!var.dbObject().count()) // object doesn't exist in this database
    {
        logerr << "DBInterface: getMinMaxString: parent object of var " << var.name() << " has no data in db";
        return std::pair<std::string, std::string> (NULL_STRING, NULL_STRING);
    }

    if (!var.existsInDB()) // variable doesn't exist in this database
    {
        logerr << "DBInterface: getMinMaxString: var " << var.name() << " does not exist in db";
        return std::pair<std::string, std::string> (NULL_STRING, NULL_STRING);
    }

    QMutexLocker locker(&connection_mutex_);

    PropertyList list;
    list.addProperty("min", PropertyDataType::STRING);
    list.addProperty("max", PropertyDataType::STRING);

    // get min max as strings

    DBCommand command;
    command.set(sql_generator_.getSelectMinMaxStatement(var.currentDBColumn().name(), var.dboName()));
    command.list(list);

    logdbg << "DBInterface: getMinMaxString: sql '" << command.get() << "'";

    std::shared_ptr<DBResult> result = current_connection_->execute(command);

    assert (result);
    assert (result->containsData());
    std::shared_ptr<Buffer> buffer = result->buffer();

    assert (buffer);
    if (buffer->size() != 1)
    {
        //        throw std::invalid_argument ("DBInterface: getMinMaxString: string buffer for variable "
        //                                     + var.name() + " empty");
        logerr << "DBInterface: getMinMaxString: variable " << var.name() << " has " << buffer->size()
               << " minmax values";
        return std::pair <std::string, std::string> (NULL_STRING, NULL_STRING);
    }

    if (buffer->get<std::string>("min").isNone(0) || buffer->get<std::string>("max").isNone(0))
    {
        logerr << "DBInterface: getMinMaxString: variable " << var.name() << " has NULL minimum/maximum";
        return std::pair <std::string, std::string> (NULL_STRING, NULL_STRING);
    }

    std::string min = buffer->get<std::string>("min").get(0);
    std::string max = buffer->get<std::string>("max").get(0);

    logdbg << "DBInterface: getMinMaxString: minstr '" << min << " maxstr " << max;

    const DBTableColumn &column = var.currentDBColumn ();
    if (column.unit() != var.unitConst()) // do unit conversion stuff
    {
        const Dimension &dimension = UnitManager::instance().dimension (var.dimensionConst());
        double factor = dimension.getFactor (column.unit(), var.unitConst());

        min = var.multiplyString(min, factor);
        max = var.multiplyString(max, factor);
    }

    logdbg << "DBInterface: getMinMaxString: var " << var.name() << " min " << min << " max " << max;
    return std::pair <std::string, std::string> (min, max);
}

bool DBInterface::isPostProcessed ()
{
    return hasProperty("postProcessed") && getProperty("postProcessed") == "Yes";

    //return existsMinMaxTable ();// && existsPropertiesTable();
}

void DBInterface::setPostProcessed (bool value)
{
    setProperty("postProcessed", value ? "Yes" : "Nope");
}

void DBInterface::postProcess ()
{
    loginf << "DBInterface: postProcess: creating jobs";

    bool any_data=false;

    for (auto obj_it : ATSDB::instance().objectManager())
        if (obj_it.second->hasData())
            any_data=true;

    if (!any_data)
    {
        logwrn << "DBInterface: postProcess: no data in objects";

        QMessageBox m_warning (QMessageBox::Warning, "No Data in Objects",
                               "None of the database objects contains any data. Post-processing was not performed.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    if (!existsMinMaxTable())
        createMinMaxTable();
    else
        clearTableContent (TABLE_NAME_MINMAX);

    for (auto obj_it : ATSDB::instance().objectManager())
    {
        if (!obj_it.second->hasData())
            continue;

        {
            DBOActiveDataSourcesDBJob* job = new DBOActiveDataSourcesDBJob (ATSDB::instance().interface(),
                                                                            *obj_it.second);

            std::shared_ptr<Job> shared_job = std::shared_ptr<Job> (job);
            connect (job, SIGNAL(doneSignal()), this, SLOT(postProcessingJobDoneSlot()), Qt::QueuedConnection);
            JobManager::instance().addDBJob(shared_job);
            postprocess_jobs_.push_back(shared_job);
        }
        {
            DBOMinMaxDBJob* job = new DBOMinMaxDBJob (ATSDB::instance().interface(), *obj_it.second);
            std::shared_ptr<Job> shared_job = std::shared_ptr<Job> (job);
            connect (job, SIGNAL(doneSignal()), this, SLOT(postProcessingJobDoneSlot()), Qt::QueuedConnection);
            JobManager::instance().addDBJob(shared_job);
            postprocess_jobs_.push_back(shared_job);
        }
    }

    assert (!postprocess_dialog_);
    postprocess_dialog_ = new QProgressDialog (tr("Post-Processing"), tr(""), 0,
                                               static_cast<int>(postprocess_jobs_.size()));
    postprocess_dialog_->setCancelButton(0);
    postprocess_dialog_->setWindowModality(Qt::ApplicationModal);
    postprocess_dialog_->show();

    postprocess_job_num_ = postprocess_jobs_.size();
}

void DBInterface::postProcessingJobDoneSlot()
{
    loginf << "DBInterface: postProcessingJobDoneSlot: " << postprocess_jobs_.size() << " active jobs" ;

    Job* job_sender = static_cast <Job*> (QObject::sender());
    assert (job_sender);
    assert (postprocess_jobs_.size() > 0);
    assert (postprocess_dialog_);

    bool found=false;
    for (auto job_it = postprocess_jobs_.begin(); job_it != postprocess_jobs_.end(); job_it++)
    {
        Job *current = job_it->get();
        if (current == job_sender)
        {
            postprocess_jobs_.erase(job_it);
            found = true;
            break;
        }
    }
    assert (found);

    if (postprocess_jobs_.size() == 0)
    {
        loginf << "DBInterface: postProcessingJobDoneSlot: done";
        setPostProcessed(true);

        delete postprocess_dialog_;
        postprocess_dialog_=nullptr;

        emit postProcessingDoneSignal();
    }
    else
        postprocess_dialog_->setValue(postprocess_job_num_-postprocess_jobs_.size());
}

bool DBInterface::hasActiveDataSources (DBObject &object)
{
    if (!object.existsInDB())
        return false;

    if (!existsPropertiesTable())
        return false;

    return hasProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX+object.name());
}

/**
 * Gets active sensor numbers as property, splits it and packs it into a set.
 */
std::set<int> DBInterface::getActiveDataSources (DBObject &object)
{
    logdbg  << "DBInterface: getActiveDataSources: start";

    assert (hasActiveDataSources (object));

    std::string tmp = getProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX+object.name());

    std::set<int> ret;

    std::vector<std::string> tmp2 = String::split(tmp, ',');

    loginf  << "DBInterface: getActiveDataSources: got "<< tmp2.size() << " parts from '" << tmp << "'" ;

    for (unsigned int cnt=0; cnt < tmp2.size(); cnt++)
    {
        ret.insert (std::stoi(tmp2.at(cnt)));
        loginf  << "DBInterface: getActiveDataSources: got active source " << cnt << " '"
                << std::stoi(tmp2.at(cnt)) << "'" ;
    }

    logdbg  << "DBInterface: getActiveDataSources: end";
    return ret;
}

//void DBInterface::writeBuffer (Buffer *data)
//{
//    if (!buffer_writer_)
//        buffer_writer_ = new BufferWriter (connection_, sql_generator_);

//    std::scoped_lock l(mutex_);

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

//    std::scoped_lock l(mutex_);

//    assert (buffer_writer_);
//    assert (data);

//    buffer_writer_->write (data, table_name);
//}

void DBInterface::insertBuffer (MetaDBTable& meta_table, std::shared_ptr<Buffer> buffer)
{
    loginf << "DBInterface: insertBuffer: meta " << meta_table.name() << " buffer size " << buffer->size();

    std::shared_ptr<Buffer> partial_buffer = getPartialBuffer(meta_table.mainTable(), buffer);
    assert (partial_buffer->size());
    partialInsertBuffer(meta_table.mainTable(), partial_buffer);

    for (auto& sub_it : meta_table.subTables())
    {
        partial_buffer = getPartialBuffer(sub_it.second, buffer);
        assert (partial_buffer->size());
        partialInsertBuffer(sub_it.second, partial_buffer);
    }

}

std::shared_ptr<Buffer> DBInterface::getPartialBuffer (DBTable& table, std::shared_ptr<Buffer> buffer)
{
    loginf << "DBInterface: getPartialBuffer: table " << table.name() << " buffer size " << buffer->size();
    std::shared_ptr<Buffer> tmp_buffer {new Buffer()};

    PropertyList org_properties = buffer->properties();
    for (unsigned int cnt=0; cnt < org_properties.size(); ++cnt)
    {
        Property org_prop = org_properties.at(cnt);

        if (table.hasColumn(org_prop.name()))
            // && table.column(org_prop.name()).propertyType() == org_prop.dataType()
        {
            loginf << "DBInterface: getPartialBuffer: table " << table.name() << " adding property " << org_prop.name();
            tmp_buffer->addProperty(org_prop);

            switch (org_prop.dataType())
            {
            case PropertyDataType::BOOL:
                tmp_buffer->get<bool>(org_prop.name()) = buffer->get<bool>(org_prop.name());
                break;
            case PropertyDataType::CHAR:
                tmp_buffer->get<char>(org_prop.name()) = buffer->get<char>(org_prop.name());
                break;
            case PropertyDataType::UCHAR:
                tmp_buffer->get<unsigned char>(org_prop.name()) = buffer->get<unsigned char>(org_prop.name());
                break;
            case PropertyDataType::INT:
                tmp_buffer->get<int>(org_prop.name()) = buffer->get<int>(org_prop.name());
                break;
            case PropertyDataType::UINT:
                tmp_buffer->get<unsigned int>(org_prop.name()) = buffer->get<unsigned int>(org_prop.name());
                break;
            case PropertyDataType::LONGINT:
                tmp_buffer->get<long int>(org_prop.name()) = buffer->get<long int>(org_prop.name());
                break;
            case PropertyDataType::ULONGINT:
                tmp_buffer->get<unsigned long int>(org_prop.name()) = buffer->get<unsigned long int>(org_prop.name());
                break;
            case PropertyDataType::FLOAT:
                tmp_buffer->get<float>(org_prop.name()) = buffer->get<float>(org_prop.name());
                break;
            case PropertyDataType::DOUBLE:
                tmp_buffer->get<double>(org_prop.name()) = buffer->get<double>(org_prop.name());
                break;
            case PropertyDataType::STRING:
                tmp_buffer->get<std::string>(org_prop.name()) = buffer->get<std::string>(org_prop.name());
                break;
            default:
                logerr  <<  "IDBInterface: getPartialBuffer: unknown property type "
                         << Property::asString(org_prop.dataType());
                throw std::runtime_error ("DBInterface: getPartialBuffer: unknown property type "
                                          + Property::asString(org_prop.dataType()));
            }
        }
        else
            loginf << "DBInterface: getPartialBuffer: table " << table.name()
                   << " skipping property " << org_prop.name();
    }

    loginf << "DBInterface: getPartialBuffer: end with partial buffer size " << tmp_buffer->size();
    return tmp_buffer;
}

void DBInterface::partialInsertBuffer (DBTable& table, std::shared_ptr<Buffer> buffer)
{
    loginf << "DBInterface: partialInsertBuffer: table " << table.name() << " buffer size " << buffer->size();

    assert (current_connection_);
    assert (buffer);

    const PropertyList &properties = buffer->properties();

    for (unsigned int cnt=0; cnt < properties.size(); ++cnt)
    {
        logdbg << "DBInterface: partialInsertBuffer: checking column '" << properties.at(cnt).name() << "'";

        if (!table.hasColumn(properties.at(cnt).name()))
            throw std::runtime_error ("DBInterface: partialInsertBuffer: column '"+properties.at(cnt).name()
                                      +"' does not exist in table "+table.name());
    }

    if (!table.existsInDB() && !existsTable(table.name())) // check for both since information might not be updated yet
        createTable(table);

    assert (table.existsInDB());

    std::string bind_statement = sql_generator_.insertDBUpdateStringBind(buffer, table.name());

    QMutexLocker locker(&connection_mutex_);

    logdbg  << "DBInterface: partialInsertBuffer: preparing bind statement";
    current_connection_->prepareBindStatement(bind_statement);
    current_connection_->beginBindTransaction();

    logdbg  << "DBInterface: partialInsertBuffer: starting inserts";
    size_t size = buffer->size();
    for (unsigned int cnt=0; cnt < size; ++cnt)
    {
        insertBindStatementUpdateForCurrentIndex(buffer, cnt);
    }

    logdbg  << "DBInterface: partialInsertBuffer: ending bind transactions";
    current_connection_->endBindTransaction();
    logdbg  << "DBInterface: partialInsertBuffer: finalizing bind statement";
    current_connection_->finalizeBindStatement();
}

bool DBInterface::checkUpdateBuffer (DBObject &object, DBOVariable &key_var, DBOVariableSet& list,
                                     std::shared_ptr<Buffer> buffer)
{
    if (!object.existsInDB())
        return false;

    if (!key_var.existsInDB())
        return false;

    const DBTable& table = object.currentMetaTable().mainTable();

    if (!table.existsInDB()) // might be redundant
        return false;

    const PropertyList &properties = buffer->properties();

    for (auto& var_it : list.getSet())
    {
        if (!properties.hasProperty(var_it->name()))
            return false;

        if (!var_it->hasCurrentDBColumn())
            return false;

        const DBTableColumn& col = var_it->currentDBColumn ();

        if (!col.existsInDB())
            return false;

    }

//    for (unsigned int cnt=0; cnt < properties.size(); cnt++)
//    {
//        if (!table.hasColumn(properties.at(cnt).name()))
//            return false;

//        if (!table.column(properties.at(cnt).name()).existsInDB())
//            return false;
//    }

    return true;
}

// TODO: only works on main table
void DBInterface::updateBuffer (DBObject& object, DBOVariable& key_var, std::shared_ptr<Buffer> buffer,
                                size_t from_index, size_t to_index)
{
    //assert (checkUpdateBuffer(object, key_var, buffer));
    assert (current_connection_);
    assert (buffer);

    const DBTable& table = object.currentMetaTable().mainTable();

    const PropertyList &properties = buffer->properties();

    for (unsigned int cnt=0; cnt < properties.size(); cnt++)
    {
        if (!table.hasColumn(properties.at(cnt).name()))
            throw std::runtime_error ("DBInterface: updateBuffer: column '"+properties.at(cnt).name()
                                      +"' does not exist in table "+table.name());
    }

    std::string bind_statement =  sql_generator_.createDBUpdateStringBind(buffer, object, key_var, table.name());

    QMutexLocker locker(&connection_mutex_);

    logdbg  << "DBInterface: updateBuffer: preparing bind statement";
    current_connection_->prepareBindStatement(bind_statement);
    current_connection_->beginBindTransaction();

    logdbg  << "DBInterface: updateBuffer: starting inserts";
    for (unsigned int cnt=from_index; cnt <= to_index; cnt++)
    {
        insertBindStatementUpdateForCurrentIndex(buffer, cnt);
    }

    logdbg  << "DBInterface: updateBuffer: ending bind transactions";
    current_connection_->endBindTransaction();
    logdbg  << "DBInterface: update: finalizing bind statement";
    current_connection_->finalizeBindStatement();
}

void DBInterface::prepareRead (const DBObject &dbobject, DBOVariableSet read_list, std::string custom_filter_clause,
                               std::vector <DBOVariable *> filtered_variables, bool use_order,
                               DBOVariable *order_variable, bool use_order_ascending, const std::string &limit)
{
    assert (current_connection_);

    assert (dbobject.existsInDB());

    for (auto& var_it : read_list.getSet())
        assert(var_it->existsInDB());

    for (auto& var_it : filtered_variables)
        assert(var_it->existsInDB());

    if (order_variable)
        assert (order_variable->existsInDB());

    connection_mutex_.lock();

    std::shared_ptr<DBCommand> read = sql_generator_.getSelectCommand (
                dbobject.currentMetaTable(), read_list, custom_filter_clause, filtered_variables, use_order,
                order_variable, use_order_ascending, limit, true);

    loginf  << "DBInterface: prepareRead: dbo " << dbobject.name() << " sql '" << read->get() << "'";
    current_connection_->prepareCommand(read);
}

/**
 * Retrieves result from connection stepPreparedCommand, calls activateKeySearch on buffer and returns it.
 */
std::shared_ptr <Buffer> DBInterface::readDataChunk (const DBObject &dbobject)
{
    // locked by prepareRead
    assert (current_connection_);

    std::shared_ptr <DBResult> result = current_connection_->stepPreparedCommand(read_chunk_size_);

    if (!result)
    {
        logerr  << "DBInterface: readDataChunk: connection returned error";
        throw std::runtime_error ("DBInterface: readDataChunk: connection returned error");
    }

    if (!result->containsData())
    {
        logerr  << "DBInterface: readDataChunk: buffer does not contain data";
        throw std::runtime_error ("DBInterface: readDataChunk: buffer does not contain data");
    }

    std::shared_ptr <Buffer> buffer = result->buffer();

    buffer->dboName(dbobject.name());

    assert (buffer);

    bool last_one = current_connection_->getPreparedCommandDone();
    buffer->lastOne (last_one);

    return buffer;
}


void DBInterface::finalizeReadStatement (const DBObject &dbobject)
{
    connection_mutex_.unlock();
    assert (current_connection_);

    logdbg  << "DBInterface: finishReadSystemTracks: start ";
    //prepared_.at(dbobject.name())=false;
    current_connection_->finalizeCommand();
}

void DBInterface::createPropertiesTable ()
{
    assert (!existsPropertiesTable());
    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getTablePropertiesCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo ();
}
void DBInterface::createMinMaxTable ()
{
    assert (!existsMinMaxTable());
    connection_mutex_.lock();
    current_connection_->executeSQL(sql_generator_.getTableMinMaxCreateStatement());
    connection_mutex_.unlock();

    updateTableInfo ();
}

void DBInterface::clearTableContent (const std::string& table_name)
{
    QMutexLocker locker(&connection_mutex_);
    //DELETE FROM tablename;
    current_connection_->executeSQL("DELETE FROM "+table_name+";");
}

std::shared_ptr<DBResult> DBInterface::queryMinMaxNormalForTable (const DBTable& table)
{
    QMutexLocker locker(&connection_mutex_);
    logdbg  << "DBInterface: queryMinMaxForTable: getting command";
    std::shared_ptr <DBCommand> command = sql_generator_.getTableSelectMinMaxNormalStatement (table);

    //loginf  << "DBInterface: queryMinMaxForTable: executing command '" << command->getCommandString() << "'";
    std::shared_ptr<DBResult> result = current_connection_->execute(*command);
    return result;
}

void DBInterface::insertBindStatementUpdateForCurrentIndex (std::shared_ptr<Buffer> buffer, unsigned int row)
{
    assert (buffer);
    logdbg  << "DBInterface: insertBindStatementUpdateForCurrentIndex: start";
    const PropertyList &list =buffer->properties();
    unsigned int size = list.size();
    logdbg  << "DBInterface: insertBindStatementUpdateForCurrentIndex: creating bind for " << size << " elements";

    std::string connection_type = current_connection_->type();

    assert (connection_type == MYSQL_IDENTIFIER || connection_type == SQLITE_IDENTIFIER);

    unsigned int index_cnt=0;

    logdbg << "DBInterface: insertBindStatementUpdateForCurrentIndex: starting for loop";
    for (unsigned int cnt=0; cnt < size; cnt++)
    {
        const Property &property = list.at(cnt);
        PropertyDataType data_type = property.dataType();

        logdbg  << "DBInterface: insertBindStatementUpdateForCurrentIndex: for at cnt " << cnt << " id "
                << property.name() << " index cnt " << index_cnt;

        if (connection_type == SQLITE_IDENTIFIER)
            index_cnt=cnt+1;
        else if (connection_type == MYSQL_IDENTIFIER)
            index_cnt=cnt+1;
        else
            throw std::runtime_error ("DBInterface: insertBindStatementForCurrentIndex: unknown db type");

        if (buffer->isNone(property, row))
        {
            current_connection_->bindVariableNull (index_cnt);
            continue;
        }

        switch (data_type)
        {
        case PropertyDataType::BOOL:
            current_connection_->bindVariable (index_cnt,
                                               static_cast<int> (buffer->get<bool>(property.name()).get(row)));
            break;
        case PropertyDataType::CHAR:
            current_connection_->bindVariable (index_cnt,
                                               static_cast<int> (buffer->get<char>(property.name()).get(row)));
            break;
        case PropertyDataType::UCHAR:
            current_connection_->bindVariable (index_cnt,
                                               static_cast<int> (buffer->get<unsigned char>(property.name()).get(row)));
            break;
        case PropertyDataType::INT:
            current_connection_->bindVariable (index_cnt,
                                               static_cast<int> (buffer->get<int>(property.name()).get(row)));
            break;
        case PropertyDataType::UINT:
            assert (false);
            break;
        case PropertyDataType::LONGINT:
            assert (false);
            break;
        case PropertyDataType::ULONGINT:
            assert (false);
            break;
        case PropertyDataType::FLOAT:
            current_connection_->bindVariable (index_cnt,
                                               static_cast<double> (buffer->get<float>(property.name()).get(row)));
            break;
        case PropertyDataType::DOUBLE:
            current_connection_->bindVariable (index_cnt, buffer->get<double>(property.name()).get(row));
            break;
        case PropertyDataType::STRING:
            if (connection_type == SQLITE_IDENTIFIER)
                current_connection_->bindVariable (index_cnt, buffer->get<std::string>(property.name()).get(row));
            else //MYSQL assumed
                current_connection_->bindVariable (index_cnt, "'"+buffer->get<std::string>(property.name()).get(row)+"'");
            break;
        default:
            logerr  <<  "Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type "
                     << Property::asString(data_type);
            throw std::runtime_error ("Buffer: insertBindStatementUpdateForCurrentIndex: unknown property type "
                                      + Property::asString(data_type));
        }
    }

    current_connection_->stepAndClearBindings();

    logdbg  << "DBInterface: insertBindStatementUpdateForCurrentIndex: done";
}

//DBResult *DBInterface::getDistinctStatistics (const std::string &type, DBOVariable *variable, unsigned int sensor_number)
//{
//    std::scoped_lock l(mutex_);

//    assert (DBObjectManager::getInstance().existsDBObject(type));
//    assert (variable->existsIn(type));

//    DBCommand *command = sql_generator_->getDistinctStatistics(type, variable, sensor_number);
//    DBResult *result = connection_->execute(command);
//    return result;
//}

//void DBInterface::deleteAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string filter)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    std::scoped_lock l(mutex_);
//    connection_->executeSQL(sql_generator_->getDeleteStatement(variable->getCurrentDBColumn(), value, filter));
//}

//void DBInterface::updateAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string new_value,
// std::string filter)
//{
//    assert (sql_generator_);

//    assert (!variable->isMetaVariable());
//    assert (variable->hasCurrentDBColumn());

//    connection_->executeSQL(sql_generator_->getUpdateStatement(variable->getCurrentDBColumn(), value, new_value,
//filter));
//}

//void DBInterface::getMinMaxOfVariable (DBOVariable *variable, std::string filter_condition, std::string &min,
//std::string &max)
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

//    loginf << "DBInterface: getMinMaxOfVariable: variable " << variable->getName() << " min " << min << " max "
//<< max;
//}

////void DBInterface::getDistinctValues (DBOVariable *variable, std::string filter_condition,
/// std::vector<std::string> &values)
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
////        loginf << "DBInterface: getDistinctValues: variable " << variable->getName() << " value: "
/// << *(std::string *) (buffer->get(0, cnt));
////    }
////
////
////}

//Buffer *DBInterface::getTrackMatches (bool has_mode_a, unsigned int mode_a, bool has_ta, unsigned int ta,
// bool has_ti, std::string ti,
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
//        ss << " AND TOD>"+String::doubleToStringNoScientific(tod_min)+" AND TOD<"
//+String::doubleToStringNoScientific(tod_max);

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

//void DBInterface::testReading ()
//{
//    loginf << "DBInterface: testReading";

//    boost::posix_time::ptime start_time;
//    boost::posix_time::ptime stop_time;

//    DBObject &object = ATSDB::instance().objectManager().object("MLAT");
//    DBOVariableSet read_list;

//    loginf << "DBInterface: testReading: adding all variables";
//    for (auto variable_it : object.variables())
//        read_list.add(variable_it.second);

//    loginf << "DBInterface: testReading: preparing reading";
//    prepareRead (object, read_list); //, std::string custom_filter_clause="", DBOVariable *order=0);

//    start_time = boost::posix_time::microsec_clock::local_time();

//    loginf << "DBInterface: testReading: starting reading";
//    std::vector<std::shared_ptr <Buffer>> buffer_vector;

//    unsigned int num_rows=0;

//    while (!getReadingDone(object))
//    {
//        std::shared_ptr <Buffer> buffer = readDataChunk (object, false);
//        buffer_vector.push_back(buffer);

//        stop_time = boost::posix_time::microsec_clock::local_time();
//        boost::posix_time::time_duration diff = stop_time - start_time;

//        num_rows += buffer->size();
//        if (diff.total_seconds() > 0)
//            loginf << "DBInterface: testReading: got buffer size " << buffer->size() << " all " << num_rows
//<< " elapsed " << diff << " #el/sec " << num_rows/diff.total_seconds();
//    }

//    boost::posix_time::time_duration diff = stop_time - start_time;
//    loginf << "DBInterface: testReading: reading done: all " << num_rows << " elapsed " << diff << " #el/sec "
//<< num_rows/diff.total_seconds();
//    finalizeReadStatement (object);


//    loginf << "DBInterface: testReading: clearing buffers";
//    buffer_vector.clear();

//    loginf << "DBInterface: testReading: done";
//}
