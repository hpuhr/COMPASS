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
 * DBOVariableMinMaxDBJob.h
 *
 *  Created on: Mar 5, 2013
 *      Author: sk
 */

#ifndef DBOVARIABLEMINMAXDBJOB_H_
#define DBOVARIABLEMINMAXDBJOB_H_

#include "job.h"

class DBOVariable;
class DBInterface;

/**
 * \brief Creates the minimum/maximum information for a specific DBOVariable
 *
 * DBJob, Uses DBInterface.
 */
class DBOVariableMinMaxDBJob : public Job
{
public:
    DBOVariableMinMaxDBJob(DBInterface &db_interface, DBOVariable &variable);
    virtual ~DBOVariableMinMaxDBJob();

    virtual void run ();

    DBOVariable &dboVariable () { return variable_; }
    std::string min () { return min_; }
    std::string max () { return max_; }

protected:
    DBInterface &db_interface_;
    DBOVariable &variable_;
    std::string min_;
    std::string max_;
};

#endif /* DBOVARIABLEMINMAXDBJOB_H_ */
