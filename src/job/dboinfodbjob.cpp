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
 * DBOInfoDBJob.cpp
 *
 *  Created on: Feb 6, 2013
 *      Author: sk
 */

#include "DBOInfoDBJob.h"
#include "DBJob.h"
#include "Buffer.h"
#include "DBInterface.h"
#include "JobOrderer.h"


DBOInfoDBJob::DBOInfoDBJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
    boost::function<void (Job*)> obsolete_function, DBInterface *interface, DB_OBJECT_TYPE type,
    std::vector<unsigned int> ids, DBOVariableSet read_list, bool use_filters, std::string order_by_variable,
    bool ascending, unsigned int limit_min, unsigned int limit_max, bool finalize)
 : DBJob (orderer, done_function, obsolete_function, interface), type_(type), ids_(ids), read_list_(read_list),
   use_filters_(use_filters), order_by_variable_(order_by_variable), ascending_(ascending), limit_min_(limit_min),
   limit_max_(limit_max), result_buffer_(0), finalize_(finalize)
{

}

DBOInfoDBJob::~DBOInfoDBJob()
{

}

void DBOInfoDBJob::execute ()
{
  assert (!result_buffer_);

  result_buffer_ = db_interface_->getInfo(type_, ids_, read_list_, use_filters_, order_by_variable_,
      ascending_, limit_min_, limit_max_, finalize_);

  assert (result_buffer_);
  done_=true;
}
