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

#include "job.h"
#include "global.h"
#include "dbovariableset.h"

class DBObject;
class Buffer;

/**
 * @brief Finalizes read DBObject data from DBOReadDBJob
 *
 * Uses finalizeDBData() from Util.
 */
class FinalizeDBOReadJob : public Job
{
public:
    FinalizeDBOReadJob (DBObject &dbobject, DBOVariableSet &read_list, std::shared_ptr<Buffer> buffer);
    virtual ~FinalizeDBOReadJob();

    virtual void run ();

    std::shared_ptr<Buffer> buffer () { assert (buffer_); return buffer_; }

protected:
    DBObject &dbobject_;
    DBOVariableSet read_list_;
    std::shared_ptr<Buffer> buffer_;
};

#endif /* FINALIZEDBOREADJOB_H_ */
