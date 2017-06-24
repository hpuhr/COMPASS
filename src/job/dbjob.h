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
 * DBJob.h
 *
 *  Created on: Feb 6, 2013
 *      Author: sk
 */

#ifndef DBJOB_H_
#define DBJOB_H_

#include "job.h"

class DBInterface;

/**
 * @brief Job specialization for database operations
 *
 * Was created to ensure that database jobs are performed in the correct order in one thread (performance gain
 * of multiple threads dubious).
 *
 * Requires a DBInterface instance, calls the done_function when completed or obsolete_function when aborted.
 */
class DBJob : public Job
{
public:
    /// @brief Constructor
    DBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function, boost::function<void (Job*)> obsolete_function,
            DBInterface *db_interface);
    /// @brief Destructor
    virtual ~DBJob();

protected:
    /// Database interface
    DBInterface *db_interface_;
};

#endif /* DBJOB_H_ */
