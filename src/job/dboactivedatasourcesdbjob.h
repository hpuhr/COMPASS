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

#ifndef DBOACTIVEDATASOURCESDBJOB_H_
#define DBOACTIVEDATASOURCESDBJOB_H_

#include "job.h"

class DBInterface;
class DBObject;

/**
 * @brief Generates the active data sources information for a DBObject
 *
 * For a given DBObject type, uses the DBInterface to generate the active data sources. The compacted information is
 * added as property string.
 */
class DBOActiveDataSourcesDBJob : public Job
{
public:
    DBOActiveDataSourcesDBJob(DBInterface& db_interface, DBObject &object);
    virtual ~DBOActiveDataSourcesDBJob();

    virtual void run ();

    DBObject& dbObject () { return object_; }

protected:
    DBInterface& db_interface_;
    DBObject &object_;
};

#endif /* DBOACTIVEDATASOURCESDBJOB_H_ */
