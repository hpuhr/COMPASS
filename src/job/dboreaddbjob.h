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
class DBOReadDBJob : public DBJob
{
    Q_OBJECT
signals:
    void intermediateSignal (std::shared_ptr <Job> job, std::shared_ptr<Buffer> buffer);

public:
    DBOReadDBJob(DBInterface &db_interface, const DBObject &dbobject, DBOVariableSet read_list, std::string custom_filter_clause,
                 DBOVariable *order, bool activate_key_search);
    virtual ~DBOReadDBJob();

    virtual void execute ();

    DBOVariableSet getReadList () { return read_list_; }

protected:
    const DBObject &dbobject_;
    DBOVariableSet read_list_;
    std::string custom_filter_clause_;
    DBOVariable *order_;
    bool activate_key_search_;
};

#endif /* DBOREADDBJOB_H_ */
