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

#include "DBOVariableDistinctStatisticsDBJob.h"
#include "DBInterface.h"
#include "Buffer.h"
#include "DBResult.h"

DBOVariableDistinctStatisticsDBJob::DBOVariableDistinctStatisticsDBJob(JobOrderer *orderer,
        boost::function<void (Job*)> done_function,
        boost::function<void (Job*)> obsolete_function, DBInterface *db_interface, DB_OBJECT_TYPE type,
        DBOVariable *variable, unsigned int sensor_number)
 : DBJob (orderer, done_function, obsolete_function, db_interface),type_ (type), variable_(variable),
   sensor_number_ (sensor_number), result_buffer_ (0)
{


}

DBOVariableDistinctStatisticsDBJob::~DBOVariableDistinctStatisticsDBJob()
{

}

void DBOVariableDistinctStatisticsDBJob::execute ()
{
    DBResult *result = db_interface_->getDistinctStatistics (type_, variable_, sensor_number_);

    result_buffer_ = result->getBuffer();

    assert (!result_buffer_->getFirstWrite());

    delete result;

    done_=true;
}
