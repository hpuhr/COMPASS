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
 * DBJob.cpp
 *
 *  Created on: Feb 6, 2013
 *      Author: sk
 */

#include "DBJob.h"
#include "WorkerThreadManager.h"

/**
 * Adds itself to the WorkerThreadManager (to be executed).
 *
 * \param db_interface used to perform database operations
 */
DBJob::DBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function, boost::function<void (Job*)> obsolete_function,
    DBInterface *db_interface)
 : Job (orderer, done_function, obsolete_function), db_interface_(db_interface)
{
  assert (db_interface_);
  WorkerThreadManager::getInstance().addDBJob(this);

}

DBJob::~DBJob()
{
}

