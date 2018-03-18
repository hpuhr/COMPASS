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

#include "dboactivedatasourcesdbjob.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "stringconv.h"

using namespace Utils;

DBOActiveDataSourcesDBJob::DBOActiveDataSourcesDBJob(DBInterface& db_interface, DBObject &object)
: Job("DBOActiveDataSourcesDBJob"), db_interface_(db_interface), object_(object)
{
    assert (object_.existsInDB());
}

DBOActiveDataSourcesDBJob::~DBOActiveDataSourcesDBJob()
{
}

void DBOActiveDataSourcesDBJob::run ()
{
    loginf  << "DBOActiveDataSourcesDBJob: run: object " << object_.name();

    boost::posix_time::ptime loading_start_time_;
    boost::posix_time::ptime loading_stop_time_;

    loading_start_time_ = boost::posix_time::microsec_clock::local_time();

    loginf  << "PostProcessDBJob: run: creating properties";
    if (!db_interface_.existsPropertiesTable())
        db_interface_.createPropertiesTable ();

    assert (db_interface_.existsPropertiesTable());

    assert (object_.hasCurrentDataSourceDefinition());

    loginf  << "DBOActiveDataSourcesDBJob: run: creating active sensors for dbo " << object_.name();

    std::set<int> active = db_interface_.queryActiveSensorNumbers(object_);
    std::stringstream ss;

    std::set<int>::iterator it2;

    int cnt=0;
    for (it2 = active.begin(); it2 != active.end(); it2++)
    {
        if (cnt == 0)
            ss << *it2;
        else
            ss << "," << *it2;
        ++cnt;
    }
    db_interface_.setProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX+object_.name(), ss.str());
    loginf  << "DBOActiveDataSourcesDBJob: run: dbo " << object_.name() << " active sensors '" << ss.str() << "'";
    assert (db_interface_.hasProperty(ACTIVE_DATA_SOURCES_PROPERTY_PREFIX+object_.name()));

    loading_stop_time_ = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time_ - loading_start_time_;
    load_time= diff.total_milliseconds()/1000.0;

    loginf  << "DBOActiveDataSourcesDBJob: run: done (" << load_time << " s).";
    done_=true;
}

