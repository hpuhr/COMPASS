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
 * DBOCountDBJob.cpp
 *
 *  Created on: Jun 6, 2013
 *      Author: sk
 */

#include "Buffer.h"
#include "DBOCountDBJob.h"
#include "DBInterface.h"
#include "DBResult.h"
#include "Logger.h"

DBOCountDBJob::DBOCountDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
        boost::function<void (Job*)> obsolete_function, DBInterface *interface, DB_OBJECT_TYPE type,
        unsigned int sensor_number)
: DBJob (orderer, done_function, obsolete_function, interface), type_(type), sensor_number_(sensor_number), count_(0)
{

}

DBOCountDBJob::~DBOCountDBJob()
{

}

void DBOCountDBJob::execute()
{
    DBResult *result = db_interface_->count(type_, sensor_number_);
    Buffer *buffer = result->getBuffer();

    assert (!buffer->getFirstWrite());
    assert (buffer->getSize() == 1);

    count_ = *(unsigned int*) buffer->get(0,0);

    loginf << "DBOCountDBJob: execute: got count " << count_ << " for type " << type_ << " number " << sensor_number_;

    delete result;
    delete buffer;

    done_=true;
}
