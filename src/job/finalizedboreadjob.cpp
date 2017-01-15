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
 * FinalizeDBOReadJob.cpp
 *
 *  Created on: Feb 25, 2013
 *      Author: sk
 */

#include "FinalizeDBOReadJob.h"
#include "DBObjectManager.h"
#include "DBObject.h"
#include "DBOVariable.h"
#include "PropertyList.h"
#include "DBTableColumn.h"
#include "DBSchemaManager.h"
#include "DBOVariableSet.h"
#include "DBInterface.h"
#include "UnitManager.h"
#include "Unit.h"
#include "Buffer.h"
#include "DBSchema.h"
#include "MetaDBTable.h"
#include "WorkerThreadManager.h"
#include "Data.h"

using namespace Utils::Data;

FinalizeDBOReadJob::FinalizeDBOReadJob(JobOrderer *orderer, boost::function<void (Job*)> done_function,
        boost::function<void (Job*)> obsolete_function, Buffer *buffer, DBOVariableSet read_list)
: Job (orderer, done_function, obsolete_function), buffer_ (buffer), read_list_(read_list)
{
    assert (buffer_);
    type_ = buffer_->getDBOType();

    assert (type_ != DBO_UNDEFINED);

    WorkerThreadManager::getInstance().addJob(this);
}

FinalizeDBOReadJob::~FinalizeDBOReadJob()
{

}

void FinalizeDBOReadJob::execute ()
{
    finalizeDBData(type_, buffer_, read_list_);

    done_=true;
}

