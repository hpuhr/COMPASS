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

#include "boost/date_time/posix_time/posix_time.hpp"

#include "job.h"
#include "dbovariableset.h"

class Buffer;
class DBObject;

/**
 * @brief DBO reading job
 *
 * Incrementally reads data record from DBO tables and writes the results into a DBDataSet.
 *
 */
class DBOReadDBJob : public Job
{
    Q_OBJECT
signals:
    void intermediateSignal (std::shared_ptr<Buffer> buffer);

public:
    DBOReadDBJob(DBInterface &db_interface, DBObject &dbobject, DBOVariableSet read_list, std::string custom_filter_clause,
                 const std::vector<std::string> &filtered_variables, DBOVariable *order, const std::string &limit_str, bool activate_key_search);
    virtual ~DBOReadDBJob();

    virtual void run ();

    DBOVariableSet getReadList () { return read_list_; }

protected:
    DBObject &dbobject_;
    DBOVariableSet read_list_;
    std::string custom_filter_clause_;
    std::vector<std::string> filtered_variables_;
    DBOVariable *order_;
    std::string limit_str_;
    bool activate_key_search_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
};

#endif /* DBOREADDBJOB_H_ */
