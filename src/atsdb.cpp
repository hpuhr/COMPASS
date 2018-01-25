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

#include "atsdb.h"
#include "config.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbtableinfo.h"
#include "dbschemamanager.h"
#include "dbinterface.h"
#include "global.h"
#include "logger.h"
#include "filtermanager.h"
//#include "StructureReader.h"
//#include "WriteBufferDBJob.h"
#include "jobmanager.h"
#include "taskmanager.h"
//#include "DBOInfoDBJob.h"
//#include "DBOVariableDistinctStatisticsDBJob.h"
#include "viewmanager.h"
#include "projectionmanager.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <qobject.h>

using namespace std;

/**
 * Sets init state, creates members, starts the thread using go.
 */
ATSDB::ATSDB()
 : Configurable ("ATSDB", "ATSDB0", 0, "atsdb.xml"), initialized_(false), db_interface_(nullptr), dbo_manager_(nullptr), db_schema_manager_ (nullptr),
   filter_manager_(nullptr), view_manager_(nullptr)
{
    logdbg  << "ATSDB: constructor: start";

    JobManager::instance().start();

    createSubConfigurables ();

    assert (db_interface_);
    assert (db_schema_manager_);
    assert (dbo_manager_);
    assert (filter_manager_);
    assert (task_manager_);
    assert (view_manager_);

    QObject::connect (db_schema_manager_, SIGNAL(schemaChangedSignal()), dbo_manager_, SLOT(updateSchemaInformationSlot()));
    QObject::connect(db_interface_, SIGNAL(databaseOpenedSignal()), dbo_manager_, SLOT(databaseOpenedSlot()));
    //QObject::connect(db_interface_, SIGNAL(databaseOpenedSignal()), filter_manager_, SLOT(databaseOpenedSlot()));

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
    assert (!task_manager_);
    assert (!view_manager_);

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
    else if (class_id == "TaskManager")
    {
        assert (task_manager_ == nullptr);
        task_manager_ = new TaskManager (class_id, instance_id, this);
        assert (task_manager_ != nullptr);
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
    if (task_manager_ == nullptr)
    {
        addNewSubConfiguration ("TaskManager", "TaskManager0");
        generateSubConfigurable ("TaskManager", "TaskManager0");
        assert (task_manager_ != nullptr);
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

TaskManager& ATSDB::taskManager ()
{
    assert (task_manager_);
    assert (initialized_);
    return *task_manager_;
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

///**
// * Calls stop. If data was written uning the StructureReader, this process is finished correctly.
// * State is set to DB_STATE_SHUTDOWN and ouput buffers are cleared.
// */
void ATSDB::shutdown ()
{
    loginf  << "ATSDB: database shutdown";

    if (!initialized_)
    {
        logerr  << "ATSDB: already shut down";
        return;
    }

    JobManager::instance().shutdown();
    ProjectionManager::instance().shutdown();

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

    assert (task_manager_);
    task_manager_->shutdown();
    delete task_manager_;
    task_manager_ = nullptr;

    assert (view_manager_);
    view_manager_->close();
    delete view_manager_;
    view_manager_ = nullptr;

    initialized_=false;

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

//void ATSDB::getDistinctStatistics (JobOrderer *orderer, boost::function<void (Job*)> done_function,
//        boost::function<void (Job*)> obsolete_function, const std::string &type,
//        DBOVariable *variable, unsigned int sensor_number)
//{
//    //TODO
//    assert (false);

////    DBOVariableDistinctStatisticsDBJob *distinct_job = new DBOVariableDistinctStatisticsDBJob (orderer, done_function,
////            obsolete_function, db_interface_, type, variable, sensor_number);
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
