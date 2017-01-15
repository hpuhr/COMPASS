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
 * DBOActiveDataSourcesDBJob.cpp
 *
 *  Created on: Mar 3, 2013
 *      Author: sk
 */

#include "DBOActiveDataSourcesDBJob.h"
#include "DBInterface.h"
#include "DBObjectManager.h"
#include "DBObject.h"
#include "SQLGenerator.h"
#include "String.h"

using namespace Utils::String;

DBOActiveDataSourcesDBJob::DBOActiveDataSourcesDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
        boost::function<void (Job*)> obsolete_function, DBInterface *db_interface, DB_OBJECT_TYPE type)
: DBJob(orderer, done_function, obsolete_function, db_interface), type_(type)
{
}

DBOActiveDataSourcesDBJob::~DBOActiveDataSourcesDBJob()
{
}

void DBOActiveDataSourcesDBJob::execute ()
{
    loginf  << "DBOActiveDataSourcesDBJob: execute: type " << type_;

    boost::posix_time::ptime loading_start_time_;
    boost::posix_time::ptime loading_stop_time_;

    loading_start_time_ = boost::posix_time::microsec_clock::local_time();

    db_interface_->updateExists();

    SQLGenerator *sql_generator = db_interface_->getSQLGenerator();

    loginf  << "PostProcessDBJob: execute: creating properties";
    if (!db_interface_->existsPropertiesTable())
        db_interface_->createPropertiesTable ();
//    else
//        db_interface_->clearTableContent (sql_generator->getPropertiesTableName());

    createActiveDataSources ();

    loading_stop_time_ = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time_ - loading_start_time_;
    load_time= diff.total_milliseconds()/1000.0;

    loginf  << "DBOActiveDataSourcesDBJob: execute: done (" << doubleToString (load_time) << " s).";
    done_=true;
}

void DBOActiveDataSourcesDBJob::createActiveDataSources ()
{
    assert (db_interface_);
    assert (db_interface_->existsPropertiesTable());
    assert (DBObjectManager::getInstance().existsDBObject(type_));

    DBObject *object = DBObjectManager::getInstance().getDBObject(type_);
    assert (object->hasCurrentDataSource());

    loginf  << "DBOActiveDataSourcesDBJob: createActiveDataSources: creating active sensors for dbo " << object->getName();

    std::set<int> active = db_interface_->queryActiveSensorNumbers(type_);
    std::stringstream ss;

    std::set<int>::iterator it2;

    int cnt=0;
    for (it2 = active.begin(); it2 != active.end(); it2++)
    {
        if (cnt == 0)
            ss << intToString(*it2);
        else
            ss << "," << intToString(*it2);
        ++cnt;
    }
    db_interface_->insertProperty("activeSensorNumbers"+object->getName(), ss.str());
    loginf  << "DBOActiveDataSourcesDBJob: createActiveDataSources: dbo " << object->getName() << " active sensors '" << ss.str() << "'";
    assert (db_interface_->hasProperty("activeSensorNumbers"+object->getName()));
}
