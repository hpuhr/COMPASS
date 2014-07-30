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
 * DBOVariableDistinctStatisticsDBJob.h
 *
 *  Created on: Jun 7, 2013
 *      Author: sk
 */

#ifndef DBOVARIABLEDISTINCTSTATISTICSDBJOB_H_
#define DBOVARIABLEDISTINCTSTATISTICSDBJOB_H_

#include <cassert>

#include "DBJob.h"
#include "Global.h"

class Buffer;
class DBOVariable;

class DBOVariableDistinctStatisticsDBJob : public DBJob
{
public:
    DBOVariableDistinctStatisticsDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
            boost::function<void (Job*)> obsolete_function, DBInterface *db_interface, DB_OBJECT_TYPE type,
            DBOVariable *variable, unsigned int sensor_number);
    virtual ~DBOVariableDistinctStatisticsDBJob();

    virtual void execute ();

    Buffer *getResultBuffer () { assert (result_buffer_); return result_buffer_; }

protected:
    DB_OBJECT_TYPE type_;
    DBOVariable *variable_;
    unsigned int sensor_number_;

    Buffer *result_buffer_;
};

#endif /* DBOVARIABLEDISTINCTSTATISTICSDBJOB_H_ */
