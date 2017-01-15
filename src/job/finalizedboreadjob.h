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
 * FinalizeDBOReadJob.h
 *
 *  Created on: Feb 25, 2013
 *      Author: sk
 */

#ifndef FINALIZEDBOREADJOB_H_
#define FINALIZEDBOREADJOB_H_

#include "Job.h"
#include "Global.h"
#include "DBOVariableSet.h"

class Buffer;

/**
 * @brief Finalizes read DBObject data from DBOReadDBJob
 *
 * Uses finalizeDBData() from Util.
 */
class FinalizeDBOReadJob : public Job
{
public:
    FinalizeDBOReadJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
            boost::function<void (Job*)> obsolete_function, Buffer *buffer, DBOVariableSet read_list);
    virtual ~FinalizeDBOReadJob();

    virtual void execute ();

    Buffer *getBuffer () { assert (buffer_); return buffer_; }

protected:
    DB_OBJECT_TYPE type_;
    Buffer *buffer_;
    DBOVariableSet read_list_;
};

#endif /* FINALIZEDBOREADJOB_H_ */
