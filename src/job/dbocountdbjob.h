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
 * DBOCountDBJob.h
 *
 *  Created on: Jun 6, 2013
 *      Author: sk
 */

#ifndef DBOCOUNTDBJOB_H_
#define DBOCOUNTDBJOB_H_

#include "DBJob.h"
#include "Global.h"

class DBInterface;

class DBOCountDBJob : public DBJob
{
public:
    DBOCountDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
            boost::function<void (Job*)> obsolete_function, DBInterface *interface, DB_OBJECT_TYPE type,
            unsigned int sensor_number);
    virtual ~DBOCountDBJob();

    virtual void execute ();

    DB_OBJECT_TYPE getType () { return type_; }
    unsigned int getSensorNumber() { return sensor_number_; }
    unsigned int getCount () { return count_; }

protected:
  /// DBO type
  DB_OBJECT_TYPE type_;
  unsigned int sensor_number_;
  unsigned int count_;
};

#endif /* DBOCOUNTDBJOB_H_ */
