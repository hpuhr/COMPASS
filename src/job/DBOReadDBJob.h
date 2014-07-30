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
 * DBOReadDBJob.h
 *
 *  Created on: Feb 6, 2013
 *      Author: sk
 */

#ifndef DBOREADDBJOB_H_
#define DBOREADDBJOB_H_

#include "DBJob.h"
#include "Global.h"
//#include "DBDataSet.h"
#include "DBOVariableSet.h"

class Buffer;

/**
 * @brief DBO reading job
 *
 * Incrementally reads data record from DBO tables and writes the results into a DBDataSet.
 *
 */
class DBOReadDBJob : public DBJob
{
public:
    /// Emitted signal if Job was performed
    boost::signal<void (Job*, Buffer*)> intermediate_signal_;

    DBOReadDBJob(JobOrderer *orderer, boost::function<void (Job*, Buffer*)> intermediate_function,
            boost::function<void (Job*)> done_function, boost::function<void (Job*)> obsolete_function,
            DBInterface *db_interface, DB_OBJECT_TYPE type, DBOVariableSet read_list,
            std::string custom_filter_clause="", DBOVariable *order=0);
    virtual ~DBOReadDBJob();

    virtual void execute ();

    DBOVariableSet getReadList () { return read_list_; }

protected:
    /// DBO type
    DB_OBJECT_TYPE type_;
    DBOVariableSet read_list_;
    std::string custom_filter_clause_;
    DBOVariable *order_;
};

#endif /* DBOREADDBJOB_H_ */
