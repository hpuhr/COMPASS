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
 * DBOActiveDataSourcesDBJob.h
 *
 *  Created on: Mar 3, 2013
 *      Author: sk
 */

#ifndef DBOACTIVEDATASOURCESDBJOB_H_
#define DBOACTIVEDATASOURCESDBJOB_H_

#include "DBJob.h"
#include "Global.h"

/**
 * @brief Generates the active data sources information for a DBObject
 *
 * For a given DBObject type, uses the DBInterface to generate the active data sources. The compacted information is
 * added as property string.
 */
class DBOActiveDataSourcesDBJob : public DBJob
{
public:
    DBOActiveDataSourcesDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
            boost::function<void (Job*)> obsolete_function, DBInterface *db_interface,
            DB_OBJECT_TYPE type);
    virtual ~DBOActiveDataSourcesDBJob();

    virtual void execute ();

    DB_OBJECT_TYPE getDBOType () { return type_; }

protected:
    DB_OBJECT_TYPE type_;
  /// @brief Creates properties table and values
  void createActiveDataSources ();
};

#endif /* DBOACTIVEDATASOURCESDBJOB_H_ */
