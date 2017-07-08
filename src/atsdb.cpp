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
 * ATSDB.cpp
 *
 *  Created on: Jul 12, 2011
 *      Author: sk
 */

#include "atsdb.h"
#include "config.h"
//#include "Buffer.h"
//#include "BufferReceiver.h"
//#include "DataSource.h"
//#include "DBOVariable.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbtableinfo.h"
#include "dbschemamanager.h"
#include "dbinterface.h"
#include "global.h"
#include "logger.h"
#include "filtermanager.h"
//#include "StructureReader.h"
//#include "Job.h"
//#include "PostProcessDBJob.h"
//#include "WriteBufferDBJob.h"
//#include "DBOReadDBJob.h"
#include "jobmanager.h"
//#include "DBOInfoDBJob.h"
//#include "FinalizeDBOReadJob.h"
//#include "DBOActiveDataSourcesDBJob.h"
//#include "DBOVariableMinMaxDBJob.h"
//#include "DBOCountDBJob.h"
//#include "DBOVariableDistinctStatisticsDBJob.h"
//#include "UpdateBufferDBJob.h"
#include "viewmanager.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <qobject.h>

using namespace std;

/**
 * Locks state_mutex_, sets init state, creates members, starts the thread using go.
 */
ATSDB::ATSDB()
 : Configurable ("ATSDB", "ATSDB0", 0, "conf/atsdb.xml"), initialized_(false), db_interface_(nullptr), dbo_manager_(nullptr), db_schema_manager_ (nullptr),
   filter_manager_(nullptr), view_manager_(nullptr)
//: export_active_(false), dbo_reads_active_(0)
{
    logdbg  << "ATSDB: constructor: start";

    JobManager::instance().start();

    createSubConfigurables ();

    assert (db_interface_);
    assert (db_schema_manager_);
    assert (dbo_manager_);
    assert (filter_manager_);
    assert (view_manager_);

    QObject::connect (db_schema_manager_, SIGNAL(schemaChangedSignal()), dbo_manager_, SLOT(updateSchemaInformationSlot()));
    QObject::connect(db_interface_, SIGNAL(databaseOpenedSignal()), dbo_manager_, SLOT(databaseOpenedSlot()));
    QObject::connect(db_interface_, SIGNAL(databaseOpenedSignal()), filter_manager_, SLOT(databaseOpenedSlot()));

    //reference_point_defined_=false;

    logdbg  << "ATSDB: constructor: end";
}

void ATSDB::initialize()
{
    assert (!initialized_);
    initialized_=true;

    dbo_manager_->updateSchemaInformationSlot();
}

/**
 * Deletes members.
 */
ATSDB::~ATSDB()
{
    logdbg  << "ATSDB: destructor: start";

    assert (!initialized_);

    //delete struct_reader_;

    assert (!dbo_manager_);
    assert (!db_schema_manager_);
    assert (!db_interface_);
    assert (!filter_manager_);
    assert (!view_manager_);

//    if (dbo_read_jobs_.size() > 0)
//        logerr << "ATSDB: destructor: unfinished dbo read jobs " << dbo_read_jobs_.size();

//    if (dbo_finalize_jobs_.size() > 0)
//        logerr << "ATSDB: destructor: unfinished dbo finalize jobs " << dbo_finalize_jobs_.size();

//    std::map <std::string, std::map <std::pair<unsigned char, unsigned char>, DataSource* > >::iterator it;
//    for (it = data_sources_instances_.begin(); it != data_sources_instances_.end(); it++)
//    {
//        std::map <std::pair<unsigned char, unsigned char>, DataSource* >::iterator it2;

//        for (it2 = it->second.begin(); it2 != it->second.end(); it2++)
//        {
//            delete it2->second;
//        }
//        it->second.clear();
//    }
//    data_sources_instances_.clear();

    logdbg  << "ATSDB: destructor: end";
}

void ATSDB::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    logdbg  << "ATSDB: generateSubConfigurable: class_id " << class_id << " instance_id " << instance_id;
    if (class_id == "DBInterface")
    {
        assert (db_interface_ == nullptr);
        db_interface_ = new DBInterface (class_id, instance_id, this);
        assert (db_interface_ != nullptr);
    }
    else if (class_id == "DBObjectManager")
    {
        assert (dbo_manager_ == nullptr);
        dbo_manager_ = new DBObjectManager (class_id, instance_id, this);
        assert (dbo_manager_ != nullptr);
    }
    else if (class_id == "DBSchemaManager")
    {
        assert (db_schema_manager_ == nullptr);
        db_schema_manager_ = new DBSchemaManager (class_id, instance_id, this);
        assert (db_schema_manager_ != nullptr);
    }
    else if (class_id == "FilterManager")
    {
        assert (filter_manager_ == nullptr);
        filter_manager_ = new FilterManager (class_id, instance_id, this);
        assert (filter_manager_ != nullptr);
    }
    else if (class_id == "ViewManager")
    {
        assert (view_manager_ == nullptr);
        view_manager_ = new ViewManager (class_id, instance_id, this);
        assert (view_manager_ != nullptr);
    }
    else
        throw std::runtime_error ("ATSDB: generateSubConfigurable: unknown class_id "+class_id );
}

void ATSDB::checkSubConfigurables ()
{
    if (db_interface_ == nullptr)
    {
        addNewSubConfiguration ("DBInterface", "DBInterface0");
        generateSubConfigurable ("DBInterface", "DBInterface0");
        assert (db_interface_ != nullptr);
    }
    if (dbo_manager_ == nullptr)
    {
        addNewSubConfiguration ("DBObjectManager", "DBObjectManager0");
        generateSubConfigurable ("DBObjectManager", "DBObjectManager0");
        assert (dbo_manager_ != nullptr);
    }
    if (db_schema_manager_ == nullptr)
    {
        addNewSubConfiguration ("DBSchemaManager", "DBSchemaManager0");
        generateSubConfigurable ("DBSchemaManager", "DBSchemaManager0");
        assert (dbo_manager_ != nullptr);
    }
    if (filter_manager_ == nullptr)
    {
        addNewSubConfiguration ("FilterManager", "FilterManager0");
        generateSubConfigurable ("FilterManager", "FilterManager0");
        assert (filter_manager_ != nullptr);
    }
    if (view_manager_ == nullptr)
    {
        addNewSubConfiguration ("ViewManager", "ViewManager0");
        generateSubConfigurable ("ViewManager", "ViewManager0");
        assert (view_manager_ != nullptr);
    }
}

DBInterface &ATSDB::interface ()
{
    assert (db_interface_);
    assert (initialized_);
    return *db_interface_;
}

DBSchemaManager &ATSDB::schemaManager ()
{
    assert (db_schema_manager_);
    assert (initialized_);
    return *db_schema_manager_;
}

DBObjectManager &ATSDB::objectManager ()
{
    assert (dbo_manager_);
    assert (initialized_);
    return *dbo_manager_;
}

FilterManager &ATSDB::filterManager ()
{
    assert (filter_manager_);
    assert (initialized_);
    return *filter_manager_;
}

ViewManager &ATSDB::viewManager ()
{
    assert (view_manager_);
    assert (initialized_);
    return *view_manager_;
}

bool ATSDB::ready ()
{
    if (!db_interface_ || !initialized_)
        return false;

    return db_interface_->ready();
}

/**
 * Initializes the DB, post-processes if required, builsd minimum, maximum, data sources and active data sources information. Sets
 * state to DB_STATE_WRITE if a new database was created or DB_STATE_READ_IDLE if an existing one was opened. Sets db_opened_
 * on success.
 *
 * \param info DBConnectionInfo pointer defining what database system and parameters to use
 */
//void ATSDB::connect (DBConnectionInfo *info)
//{
//    logdbg  << "ATSDB: connect: start";

//    assert (info);

//    loginf  << "ATSDB: connect: initialising connection";
//    db_interface_->initConnection(info);

//    //buildDatabases ();
//}

//void ATSDB::open (std::string database_name)
//{
//    db_interface_->openDatabase(database_name);

//    loginf  << "ATSDB: open: data sources";
    //buildDataSources();

//    logdbg  << "ATSDB: open: checking if new";

//    if (!info->isNew())
//    {
//        if (db_interface_->existsPropertiesTable())
//        {
//            loadActiveDataSources ();
//            loginf  << "ATSDB: open: building active data sources";
//        }
//        if (db_interface_->existsMinMaxTable())
//        {
//            loginf  << "ATSDB: open: building minmax values";
//            loadMinMaxValues();
//        }
//    }

    // Now we have opened the database
    //loginf  <<  "ATSDB: init: database '" << filename << "' opened";
//    logdbg  << "ATSDB: open: state to 'DB_STATE_READ_IDLE'";

//    db_opened_=true;

//    //testUpdate();

//    logdbg  << "ATSDB: open: end";
//}

///**
// * Calls stop, locks state_mutex_. If data was written uning the StructureReader, this process is finished correctly.
// * State is set to DB_STATE_SHUTDOWN and ouput buffers are cleared.
// */
void ATSDB::shutdown ()
{
    loginf  << "ATSDB: database shutdown";

    assert (initialized_);

    assert (db_interface_);
    db_interface_->closeConnection();

    assert (dbo_manager_);
    delete dbo_manager_;
    dbo_manager_ = nullptr;

    assert (db_schema_manager_);
    delete db_schema_manager_;
    db_schema_manager_ = nullptr;

    assert (db_interface_);
    delete db_interface_;
    db_interface_ = nullptr;

    assert (filter_manager_);
    delete filter_manager_;
    filter_manager_ = nullptr;

        assert (view_manager_);
    view_manager_->close();
    delete view_manager_;
    view_manager_ = nullptr;

    initialized_=false;

    JobManager::instance().shutdown();

//    if (struct_reader_->hasUnwrittenData())
//    {
//        loginf << "ATSDB: shutdown: finalizing data insertion";
//        struct_reader_->finalize();

//        if (!WorkerThreadManager::getInstance().noJobs())
//            loginf << "ATSDB: shutdown: waiting on data insertion finish";

//        while (!WorkerThreadManager::getInstance().noJobs())
//        {
//            sleep(1);
//        }
//        WorkerThreadManager::getInstance().shutdown();
//    }
//    else
//    {
//        setJobsObsolete();

//        while (active_jobs_.size() != 0)
//        {
//            loginf << "ATSDB: shutdown: waiting for " << active_jobs_.size() << " job(s) to finish";
//            sleep(1);
//        }
//    }
    logdbg  << "ATSDB: shutdown: end";
}

//bool ATSDB::hasDataSources (const std::string &type)
//{
//    return data_sources_.find (type) != data_sources_.end();
//}

//const std::map<int, std::string> &ATSDB::getDataSources (const std::string &type)
//{
//    assert (hasDataSources(type));
//    return data_sources_[type];
//}

///**
// * If not found, returns string "Unknown".
// */
//std::string ATSDB::getNameOfSensor (const std::string &type, unsigned int num)
//{
//    if (data_sources_.find (type) != data_sources_.end())
//    {
//        std::map<int, std::string> &sources = data_sources_[type];

//        if (sources.find (num) != sources.end())
//        {
//            logdbg  << "ATSDB: getNameOfSensor: type " << type << " num " << num << " name " << sources [num];
//            return sources [num];
//        }
//    }

//    return std::string("Unknown");
//}

//bool ATSDB::hasActiveDataSourcesInfo (const std::string &type)
//{
//    //    loginf << "ATSDB: hasActiveDataSourcesInfo: " << type << " info " << active_data_sources_info_exists_ [type];

//    return db_interface_->hasActiveDataSourcesInfo(type);//active_data_sources_info_exists_ [type];
//}

//void ATSDB::buildActiveDataSourcesInfo (const std::string &type)
//{
//    assert (!hasActiveDataSourcesInfo(type));

//    assert (false);
//    //TODO
////    DBOActiveDataSourcesDBJob *ds_job = new DBOActiveDataSourcesDBJob (this,
////            boost::bind( &ATSDB::activeDataSourcesDone, this, _1),
////            boost::bind( &ATSDB::jobAborted, this, _1) , db_interface_, type);
//}

///**
// * Uses state_mutex_ lock, calls quitReading if previous reading process is active. Clears result and read DBO todos, adds loadable
// * DBOs to todos. Assumes state DB_STATE_READ_IDLE, sets state to DB_STATE_READ_WORKING.
// *
// * \exception std::runtime_error if called in wrong state
// */
//void ATSDB::startReading (BufferReceiver *receiver, DBOVariableSet read_list)
//{
//    loginf  << "ATSDB: startReading: start";

//    assert (receiver);

//    setJobsObsoleteForBufferReceiver (receiver);

//    while (1)
//    {
//        unsigned int num_jobs = getNumJobsForBufferReceiver(receiver);

//        if (num_jobs == 0)
//            break;

//        loginf << "ATSDB: startReading: waiting for " << num_jobs << " job(s) to finish";
//        //sleep(1); // sync issues when not
//        sleep(1);
//    }

//    db_interface_->clearResult();

//    const std::map <std::string, DBObject*> &objects = DBObjectManager::getInstance().getDBObjects ();
//    std::map <std::string, DBObject*>::const_iterator it;

//    for (it = objects.begin(); it != objects.end(); it++)
//    {
//        if (it->second->isLoadable() && db_interface_->exists(it->first) && FilterManager::getInstance().getLoad(it->first))
//        {
//            loginf << "ATSDB: startReading: creating read job for type " << it->second->getName();
//            // TODO
//            assert (false);
////            DBOReadDBJob *read_dbo = new DBOReadDBJob (this, boost::bind( &ATSDB::readDBOIntermediate, this, _1, _2),
////                    boost::bind( &ATSDB::readDBODone, this, _1),
////                    boost::bind( &ATSDB::jobAborted, this, _1) , db_interface_, it->first, read_list);
////            boost::mutex::scoped_lock l(read_jobs_mutex_);
////            dbo_read_jobs_ [read_dbo] = receiver;
//        }
//    }

//    logdbg  << "ATSDB: startReading: done";
//}

//void ATSDB::startReading (BufferReceiver *receiver, const std::string &type, DBOVariableSet read_list, std::string custom_filter_clause,
//        DBOVariable *order)
//{
//    logdbg  << "ATSDB: startReading: start";

//    setJobsObsoleteForBufferReceiver (receiver);

//    while (1)
//    {
//        unsigned int num_jobs = getNumJobsForBufferReceiver(receiver);

//        if (num_jobs == 0)
//            break;

//        loginf << "ATSDB: startReading: waiting for " << num_jobs << " job(s) to finish";
//        //sleep(1); // sync issues when not
//        sleep(1);
//    }

//    db_interface_->clearResult();

//    assert (DBObjectManager::getInstance().existsDBObject(type));
//    DBObject *object = DBObjectManager::getInstance().getDBObject(type);


//    if (object->isLoadable() && db_interface_->exists(type))
//    {
//        loginf << "ATSDB: startReading: creating read job for type " << object->getName();
//        //TODO
//        assert (false);
////        DBOReadDBJob *read_dbo = new DBOReadDBJob (this, boost::bind( &ATSDB::readDBOIntermediate, this, _1, _2),
////                boost::bind( &ATSDB::readDBODone, this, _1),
////                boost::bind( &ATSDB::jobAborted, this, _1) , db_interface_, type, read_list,
////                custom_filter_clause, order);
////        boost::mutex::scoped_lock l(read_jobs_mutex_);
////        dbo_read_jobs_ [read_dbo] = receiver;
//    }

//    logdbg  << "ATSDB: startReading: done";
//}

///**
// * Should only be called if state_mutex_ lock is active. Clears output buffers and finishes all prepared statements.
// *
// * \exception std::runtime_error if called in wrong state
// */
//void ATSDB::quitReading ()
//{
//    logdbg  << "ATSDB: quitReading: start";

//    //clearOutputBuffers ();

//    const std::map <std::string, DBObject*> &dobs = DBObjectManager::getInstance().getDBObjects ();
//    std::map <std::string, DBObject*>::const_iterator it;

//    for (it = dobs.begin(); it != dobs.end(); it++)
//    {
//        if (!it->second->isMeta())
//        {
//            if (db_interface_->isPrepared(it->first))
//                db_interface_->finalizeReadStatement(it->first);
//        }
//    }

//    logdbg  << "ATSDB: quitReading: end";
//}

////void ATSDB::buildMinMaxInfo (DBOVariable *var)
////{
////    assert (var);
////    loginf << "ATSDB: buildMinMaxInfo: var " << var->getName () << " type " << var->getDBOType();

////    if (var->isMetaVariable())
////    {
////        logerr << "ATSDB: buildMinMaxInfo: var " << var->getName () << " is meta";
////        return;
////    }

////    if (var->hasMinMaxInfo())
////        logwrn << "ATSDB: buildMinMaxInfo: var " << var->getName() << " already has defined minmax info";

////    DBOVariableMinMaxDBJob *ds_job = new DBOVariableMinMaxDBJob (this,
////            boost::bind( &ATSDB::minMaxDone, this, _1),
////            boost::bind( &ATSDB::jobAborted, this, _1) , db_interface_, var);
////}

////std::string ATSDB::getMinAsString (DBOVariable *var)
////{
////    logdbg  << "ATSDB: getMinAsString: start var "  << var->id_ << " type " << var->dbo_type_int_;

////    assert (var->hasMinMaxInfo());
////    return var->getMinString();
////}
////std::string ATSDB::getMaxAsString (DBOVariable *var)
////{
////    logdbg  << "ATSDB: getMaxAsString: start";

////    assert (var->hasMinMaxInfo());
////    return var->getMaxString();
////}


////bool ATSDB::error()
////{
////    //return (state_ == DB_STATE_ERROR);
////    return false; //TODO
////}


////void ATSDB::insert (const std::string &type, void *data)
////{
////    logdbg  << "ATSDB: insert: got type " << type;

////    if (type == DBO_SENSOR_INFORMATION)
////    {
////        logerr << "ATSDB: insert: sensor info write unavailable";
////        return;
////    }

////    struct_reader_->add (type, data);
////}

//void ATSDB::insert (Buffer *buffer, std::string table_name)
//{
//    assert (buffer);
//    assert (table_name.size() != 0);

//    assert (db_interface_);
//    db_interface_->writeBuffer(buffer, table_name);
//}

///**
// * Within the given buffer, one column has to contain the id of the DBObject, the others are the values to be set. Such values
// * have to be within the main DBTable, others are not implemented at the moment.
// */
//void ATSDB::update (Buffer *data)
//{
//    logdbg << "ATSDB: update";

//    assert (data);

//    const std::string &dbotype = data->dboType();

//    //TODO
//    assert (false);

////    assert (DBObjectManager::getInstance().existsDBOVariable(DBO_UNDEFINED, "id"));
////    DBOVariable *idvar = DBObjectManager::getInstance().getDBOVariable(DBO_UNDEFINED, "id");
////    assert (idvar->existsIn(dbotype));
////    std::string dboidvar_name = idvar->getFor(dbotype)->getCurrentVariableName();

////    PropertyList *list = data->properties();
////    assert (list->hasProperty(dboidvar_name));

////    UpdateBufferDBJob *job = new UpdateBufferDBJob (this, boost::bind( &ATSDB::updateDBODone, this, _1 ),
////            boost::bind( &ATSDB::updateDBOAborted, this, _1 ), db_interface_, data);

//    logdbg << "ATSDB: update: done";
//}

//void ATSDB::postProcess (JobOrderer *orderer, boost::function<void(Job*)> done, boost::function<void(Job*)> obsolete)
//{
//    logdbg  << "ATSDB: postProcess: start";

//    //don't you have a job to do ... a thought repeating in that barbaric brain of yours ... edgar friendly

//    //TODO
//    assert (false);

////    PostProcessDBJob *ppjob = new PostProcessDBJob (this, done, obsolete, db_interface_);
////    ppjob->done_signal_.connect( boost::bind( &ATSDB::postProcessingDone, this, _1 ) );
////    ppjob->obsolete_signal_.connect( boost::bind( &ATSDB::jobAborted, this, _1 ) );

//    logdbg  << "ATSDB: postProcess: end";
//}

//void ATSDB::getInfo (JobOrderer *orderer, boost::function<void (Job*)> done_function,
//        boost::function<void (Job*)> obsolete_function, const std::string &type, unsigned int id, DBOVariableSet read_list)
//{

//    //TODO
//    assert (false);

////    std::vector <unsigned int> ids;
////    ids.push_back(id);

////    DBOInfoDBJob *infojob = new DBOInfoDBJob (orderer, done_function, obsolete_function, db_interface_,
////            type, ids, read_list, false, "", true, 0, 0, true);
//}

//void ATSDB::getInfo (JobOrderer *orderer, boost::function<void (Job*)> done_function,
//        boost::function<void (Job*)> obsolete_function, const std::string &type,
//        std::vector<unsigned int> ids, DBOVariableSet read_list, bool use_filters, std::string order_by_variable,
//        bool ascending, unsigned int limit_min, unsigned int limit_max, bool finalize)
//{
//    //TODO
//    assert (false);

////    DBOInfoDBJob *infojob = new DBOInfoDBJob (orderer, done_function, obsolete_function, db_interface_,
////            type, ids, read_list, use_filters, order_by_variable, ascending, limit_min, limit_max, finalize);
//}

//void ATSDB::getCount (JobOrderer *orderer, boost::function<void (Job*)> done_function,
//        boost::function<void (Job*)> obsolete_function, const std::string &type, unsigned int sensor_number)
//{
//    //TODO
//    assert (false);

////    DBOCountDBJob *countjob = new DBOCountDBJob (orderer, done_function, obsolete_function, db_interface_,
////            type, sensor_number);
//}

//void ATSDB::getDistinctStatistics (JobOrderer *orderer, boost::function<void (Job*)> done_function,
//        boost::function<void (Job*)> obsolete_function, const std::string &type,
//        DBOVariable *variable, unsigned int sensor_number)
//{
//    //TODO
//    assert (false);

////    DBOVariableDistinctStatisticsDBJob *distinct_job = new DBOVariableDistinctStatisticsDBJob (orderer, done_function,
////            obsolete_function, db_interface_, type, variable, sensor_number);
//}

//void ATSDB::buildDatabases ()
//{
//    assert (db_interface_);
//    databases_ = db_interface_->getDatabases();
//}

//void ATSDB::loadMinMaxValues ()
//{
//    logdbg  << "ATSDB: buildMinMaxValues: start";

//    //TODO
//    assert (false);

////    std::map <std::pair<std::string, std::string>, std::pair<std::string, std::string> > min_max_values =
////            db_interface_->getMinMaxInfo ();

////    std::map <std::pair<std::string, std::string>, std::pair<std::string, std::string> >::iterator it;

////    for (it = min_max_values.begin(); it != min_max_values.end(); it++)
////    {
////        if (!DBObjectManager::getInstance().existsDBOVariable (it->first.first, it->first.second))
////            logerr << "ATSDB: loadMinMaxValues: variable does not exist, type " << it->first.first << " name '"
////            << it->first.second <<"'";
////        else
////            DBObjectManager::getInstance().getDBOVariable (it->first.first, it->first.second)->setMinMax(it->second.first, it->second.second);
////    }

//    logdbg  << "ATSDB: buildMinMaxValues: done";
//}

//void ATSDB::buildDataSources()
//{
//    logdbg << "ATSDB: buildDataSources: start";
//    data_sources_.clear();

//    const std::map <std::string, DBObject*> &objects = DBObjectManager::getInstance().getDBObjects();
//    std::map <std::string, DBObject*>::const_iterator ito;

//    for (ito = objects.begin(); ito != objects.end(); ito++)
//    {
//        logdbg  << "ATSDB: buildDataSources: building dbo " << ito->second->getName();
//        if (!db_interface_->exists(ito->first) || !ito->second->hasCurrentDataSource ())
//        {
//            logdbg << "ATSDB: buildDataSources: not processed exists " << db_interface_->exists(ito->first) <<
//                    " has data source " << ito->second->hasCurrentDataSource ();
//            continue;
//        }

//        logdbg  << "ATSDB: buildDataSources: building data sources for " << ito->second->getName();

//        Buffer *buffer = db_interface_->getDataSourceDescription (ito->first);

//        std::map<int, std::string> &data_sources = data_sources_[ito->first];
//        std::map <std::pair<unsigned char, unsigned char>, DataSource* > &dbo_data_sources = data_sources_instances_ [ito->first];

//        if (!buffer->firstWrite())
//        {
//            //buffer->print(buffer->getSize ());

//            //TODO
//            assert (false);
////            buffer->setIndex(0);
////            unsigned int size = buffer->getSize ();
////
////            unsigned int cnt=0;
////
////
////
////            while (cnt < size)
////            {
////                if (cnt != 0)
////                    buffer->incrementIndex();
////
////                std::vector<void *>*addresses = buffer->getAdresses();
////                assert (addresses->size() == 8);
////
//////                list.addProperty (ds->getForeignKey(), P_TYPE_INT);
//////                list.addProperty (ds->getNameColumn(), P_TYPE_STRING); //DS_NAME SAC SIC
//////                list.addProperty ("DS_NAME", P_TYPE_STRING);
//////                list.addProperty ("SAC", P_TYPE_UCHAR);
//////                list.addProperty ("SIC", P_TYPE_UCHAR);
//////                list.addProperty ("POS_LAT_DEG", P_TYPE_DOUBLE);
//////                list.addProperty ("POS_LONG_DEG", P_TYPE_DOUBLE);
//////                list.addProperty ("GROUND_ALTITUDE_AMSL_M", P_TYPE_DOUBLE);
////
////                int sensor_number = *((int*)addresses->at(0));
////                std::string sensor_name = *((std::string*)addresses->at(1));
////                std::string sensor_name_long = *((std::string*)addresses->at(2));
////                unsigned char sac = *((unsigned char*)addresses->at(3));
////                unsigned char sic = *((unsigned char*)addresses->at(4));
////                double latitude = *((double*)addresses->at(5));
////                double longitude = *((double*)addresses->at(6));
////                double altitude = *((double*)addresses->at(7));
////
////                if (sensor_name.size() == 0)
////                    sensor_name = sensor_name_long;
////
////                logdbg  << "ATSDB: buildDataSources: at index " << cnt << " number " <<  sensor_number << " name " << sensor_name << " lat " << latitude+0.1 << " lon " << longitude;
////
////                assert (data_sources.find (sensor_number) == data_sources.end());
////                data_sources [sensor_number] = sensor_name;
////
////                //std::map <DB_OBJECT_TYPE, std::map <std::pair<unsigned char, unsigned char>, DataSource* > > data_sources_instances_;
////
////                std::pair <unsigned char, unsigned char > sac_sic_key (sac, sic);
////
////                assert (dbo_data_sources.find (sac_sic_key) == dbo_data_sources.end());
////
////                dbo_data_sources [sac_sic_key] = new DataSource ();
////                DataSource *data_source = dbo_data_sources [sac_sic_key];
////                data_source->setDBOType(ito->first);
////                data_source->setId(sensor_number);
////                data_source->setShortName(sensor_name);
////                data_source->setName(sensor_name_long);
////                data_source->setSac(sac);
////                data_source->setSic(sic);
////                data_source->setLatitude(latitude);
////                data_source->setLongitude(longitude);
////                data_source->setAltitude(altitude);
////
////                data_source->finalize();
////
////                cnt++;
////            }
////
////            loginf  << "ATSDB: buildDataSources: found " << data_sources.size() << " data sources for " << ito->second->getName();
//        }

//        delete buffer;
//    }
//    logdbg << "ATSDB: buildDataSources: end";
//}

//void ATSDB::loadActiveDataSources ()
//{
//    const std::map <std::string, DBObject*> &objects = DBObjectManager::getInstance().getDBObjects();
//    std::map <std::string, DBObject*>::const_iterator ito;

//    for (ito = objects.begin(); ito != objects.end(); ito++)
//    {
//        if (!db_interface_->exists(ito->first) || !ito->second->hasCurrentDataSource ()
//                || !db_interface_->hasActiveDataSourcesInfo(ito->first))
//        {
//            logdbg  << "ATSDB: loadActiveDataSources: cannot load data sources for type " << ito->first;
//            continue;
//        }

//        assert (false);
//        // TODO

//        //ito->second->setActiveDataSources(db_interface_->getActiveSensorNumbers (ito->first));
//        //        logdbg << "ATSDB: buildActiveDataSources: dbo " << ito->second->getName() << " has " << active_data_sources_ [ito->first] .size()
//        //                            << " active sensors";
//    }
//}

//void ATSDB::setExportActive (bool active)
//{
//    export_active_=active;
//}


//bool ATSDB::getExportActive ()
//{
//    return export_active_;
//}

//void ATSDB::postProcessingDone( Job *job )
//{
//    logdbg << "ATSDB: postProcessingDone";

//    //delete job; // is deleted in caller

//    loadMinMaxValues ();
//}

//void ATSDB::activeDataSourcesDone( Job *job )
//{
//    assert (false);
//    // TODO
////    logdbg << "ATSDB: activeDataSourcesDone";
////    DBOActiveDataSourcesDBJob *ads_job = (DBOActiveDataSourcesDBJob*) job;
////    DB_OBJECT_TYPE type = ads_job->getDBOType();
////    assert (DBObjectManager::getInstance().existsDBObject(type));
////    DBObject *object = DBObjectManager::getInstance().getDBObject(type);
////    assert (object->hasCurrentDataSource());
////    assert (db_interface_->hasActiveDataSourcesInfo(type));
////    object->setActiveDataSources(db_interface_->getActiveSensorNumbers(type));

//    delete job;
//}

//void ATSDB::minMaxDone( Job *job )
//{
//    assert (false);
//    // TODO
////    logdbg << "ATSDB: minMaxDone";
////    DBOVariableMinMaxDBJob *minmaxjob = (DBOVariableMinMaxDBJob*) job;
////    minmaxjob->getDBOVariable()->setMinMax(minmaxjob->getMin(), minmaxjob->getMax());
////    logdbg << "ATSDB: minMaxDone: var " << minmaxjob->getDBOVariable()->id_;
////    delete minmaxjob;
//}

//void ATSDB::readDBODone( Job *job )
//{
//    assert (false);
//    // TODO

//    logdbg << "ATSDB: readDBODone";

////    DBOReadDBJob *read_job = (DBOReadDBJob*) job;

////    boost::mutex::scoped_lock l(read_jobs_mutex_);
////    std::map <DBOReadDBJob*, BufferReceiver*>::iterator it;
////    it = dbo_read_jobs_.find(read_job);
////    assert (it != dbo_read_jobs_.end());

////    logdbg << "ATSDB: readDBODone: deleting job " << read_job;

////    BufferReceiver *receiver = it->second;
////    dbo_read_jobs_.erase(it);
////    l.unlock();

////    unsigned int num_jobs = getNumJobsForBufferReceiver(receiver);
////    logdbg << "ATSDB: readDBODone: num_jobs " << num_jobs;
////    if (num_jobs == 0)
////    {
////        receiver->loadingDone();
////    }

////    delete read_job;
//    logdbg << "ATSDB: readDBODone: done";
//}

//void ATSDB::readDBOIntermediate( Job *job, Buffer *buffer )
//{
//    assert (false);
//    // TODO

////    logdbg << "ATSDB: readDBOIntermidiate";

////    assert (buffer);
////    assert (buffer->dboType() != DBO_UNDEFINED);

////    DBOReadDBJob *read_job = (DBOReadDBJob*) job;
////    DBOVariableSet read_list = read_job->getReadList();

////    boost::mutex::scoped_lock l(read_jobs_mutex_);
////    std::map <DBOReadDBJob*, BufferReceiver*>::iterator it;
////    it = dbo_read_jobs_.find(read_job);
////    assert (it != dbo_read_jobs_.end());

////    BufferReceiver *receiver = it->second;
//////    loginf << "ATSDB: readDBOIntermediate: removing read job " << read_job;
//////    dbo_read_jobs_.erase(read_job);
////    l.unlock();

////    FinalizeDBOReadJob *finalize_read_dbo = new FinalizeDBOReadJob (this, boost::bind( &ATSDB::finalizeReadDBODone, this, _1),
////            boost::bind( &ATSDB::abortFinalizeReadDBODone, this, _1) , buffer, read_list);

////    boost::mutex::scoped_lock l2(finalize_jobs_mutex_);
////    dbo_finalize_jobs_ [finalize_read_dbo] = receiver;

//    logdbg << "ATSDB: readDBOIntermidiate: done";
//}

//void ATSDB::finalizeReadDBODone( Job *job )
//{
//    assert (false);
//    // TODO

////    logdbg << "ATSDB: finalizeReadDBODone";

////    FinalizeDBOReadJob *final_job = (FinalizeDBOReadJob*) job;
////    Buffer *buffer = final_job->getBuffer();

////    boost::mutex::scoped_lock l(finalize_jobs_mutex_);
////    std::map <FinalizeDBOReadJob*, BufferReceiver*>::iterator it;
////    it = dbo_finalize_jobs_.find(final_job);
////    assert (it != dbo_finalize_jobs_.end());

////    BufferReceiver *receiver = it->second;
////    dbo_finalize_jobs_.erase (it);
////    l.unlock();

////    assert (receiver);
////    receiver->receive(buffer);

////    delete final_job;
////    logdbg << "ATSDB: finalizeReadDBODone: done";
//}

//void ATSDB::updateDBODone( Job *job )
//{
//    assert (false);
//    // TODO

////    logdbg << "ATSDB: updateDBODone";
////    UpdateBufferDBJob *updatejob = (UpdateBufferDBJob*) job;


////    logdbg << "ATSDB: updateDBODone: done";
////    delete updatejob->getBuffer();
////    delete updatejob;
//}

//void ATSDB::updateDBOAborted( Job *job )
//{
//    assert (false);
//    // TODO

////    logdbg << "ATSDB: updateDBOAborted";
////    UpdateBufferDBJob *updatejob = (UpdateBufferDBJob*) job;

////    logdbg << "ATSDB: updateDBOAborted: done";
////    delete updatejob->getBuffer();
////    delete updatejob;
//}

//void ATSDB::abortFinalizeReadDBODone( Job *job )
//{
//    assert (false);
//    // TODO

////    logdbg << "ATSDB: abortFinalizeReadDBODone";

////    FinalizeDBOReadJob *final_job = (FinalizeDBOReadJob*) job;
////    delete final_job->getBuffer();
////    delete job;

////    logdbg << "ATSDB: abortFinalizeReadDBODone: done";
//}

//void ATSDB::jobAborted( Job *job )
//{
//    assert (false);
//    // TODO

//    logdbg << "ATSDB: jobAborted";

////    DBOReadDBJob *read_job = (DBOReadDBJob*) job;

////    boost::mutex::scoped_lock l(read_jobs_mutex_);
////    std::map <DBOReadDBJob*, BufferReceiver*>::iterator it;
////    it = dbo_read_jobs_.find(read_job);
////    assert (it != dbo_read_jobs_.end());

////    logdbg << "ATSDB: jobAborted: deleting job " << read_job;

////    dbo_read_jobs_.erase(read_job);

////    delete read_job;

////    logdbg << "ATSDB: jobAborted: done";
//}

//void ATSDB::setJobsObsoleteForBufferReceiver (BufferReceiver *receiver)
//{
//    logdbg << "ATSDB: setJobsObsoleteForBufferReceiver";

//    assert (false);
//    // TODO

////    boost::mutex::scoped_lock l(read_jobs_mutex_);
////    std::map <DBOReadDBJob*, BufferReceiver*>::iterator it;
////    for (it = dbo_read_jobs_.begin(); it != dbo_read_jobs_.end(); it++)
////    {
////        if (it->second == receiver)
////            it->first->setObsolete();
////    }
////    l.unlock();

////    boost::mutex::scoped_lock l2(finalize_jobs_mutex_);
////    std::map <FinalizeDBOReadJob*, BufferReceiver*>::iterator it2;
////    for (it2 = dbo_finalize_jobs_.begin(); it2 != dbo_finalize_jobs_.end(); it2++)
////    {
////        if (it2->second == receiver)
////            it2->first->setObsolete();
////    }
////    logdbg << "ATSDB: setJobsObsoleteForBufferReceiver: done";
//}

//unsigned int ATSDB::getNumJobsForBufferReceiver (BufferReceiver *receiver)
//{
//    logdbg << "ATSDB: getNumJobsForBufferReceiver";
//    unsigned int cnt=0;

//    assert (false);
//    // TODO

////    boost::mutex::scoped_lock l(read_jobs_mutex_);

////    std::map <DBOReadDBJob*, BufferReceiver*>::iterator it;
////    for (it = dbo_read_jobs_.begin(); it != dbo_read_jobs_.end(); it++)
////    {
////        if (it->second == receiver)
////            cnt++;
////    }
////    l.unlock();

////    boost::mutex::scoped_lock l2(finalize_jobs_mutex_);

////    std::map <FinalizeDBOReadJob*, BufferReceiver*>::iterator it2;
////    for (it2 = dbo_finalize_jobs_.begin(); it2 != dbo_finalize_jobs_.end(); it2++)
////    {
////        if (it2->second == receiver)
////            cnt++;
////    }

////    logdbg << "ATSDB: getNumJobsForBufferReceiver: done";
//    return cnt;
//}

//void ATSDB::deleteAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string filter)
//{
//    assert (db_interface_);
//    assert (variable);
//    assert (value.size() != 0);
//    assert (filter.size() != 0);

//    db_interface_->deleteAllRowsWithVariableValue(variable, value, filter);
//}

//void ATSDB::updateAllRowsWithVariableValue (DBOVariable *variable, std::string value, std::string new_value, std::string filter)
//{
//    assert (db_interface_);
//    assert (variable);
//    assert (value.size() != 0);
//    assert (new_value.size() != 0);
//    assert (filter.size() != 0);


//    db_interface_->updateAllRowsWithVariableValue(variable, value, new_value, filter);
//}

//void ATSDB::getMinMaxOfVariable (DBOVariable *variable, std::string filter_condition, std::string &min, std::string &max)
//{
//    assert (db_interface_);
//    db_interface_->getMinMaxOfVariable(variable, filter_condition, min, max);
//}

////void ATSDB::getDistinctValues (DBOVariable *variable, std::string filter_condition, std::vector<std::string> &values)
////{
////
////}

//Buffer *ATSDB::getTrackMatches (bool has_mode_a, unsigned int mode_a, bool has_ta, unsigned int ta, bool has_ti, std::string ti,
//        bool has_tod, double tod_min, double tod_max)
//{
//    assert (db_interface_);
//    return db_interface_->getTrackMatches(has_mode_a, mode_a, has_ta, ta, has_ti, ti, has_tod, tod_min, tod_max);
//}
