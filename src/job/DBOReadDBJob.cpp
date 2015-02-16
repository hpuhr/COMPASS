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
 * DBOReadDBJob.cpp
 *
 *  Created on: Feb 6, 2013
 *      Author: sk
 */

#include <boost/thread/thread.hpp>

#include "DBOReadDBJob.h"
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
#include "Logger.h"

DBOReadDBJob::DBOReadDBJob(JobOrderer *orderer, boost::function<void (Job*, Buffer*)> intermediate_function,
        boost::function<void (Job*)> done_function, boost::function<void (Job*)> obsolete_function, DBInterface *db_interface,
        DB_OBJECT_TYPE type, DBOVariableSet read_list, std::string custom_filter_clause, DBOVariable *order)
: DBJob (orderer, done_function, obsolete_function, db_interface), type_(type), read_list_(read_list),
  custom_filter_clause_ (custom_filter_clause), order_(order)
{
    assert (type_ != DBO_UNDEFINED);

    // AVIBIT HACK
//    if (order == 0)
//    {
//        order_ = DBObjectManager::getInstance().getDBOVariable (DBO_UNDEFINED, "id");
//        assert (order_->existsIn (type));
//    }

    intermediate_signal_.connect( intermediate_function );
}

DBOReadDBJob::~DBOReadDBJob()
{

}

void DBOReadDBJob::execute ()
{
    logdbg << "DBOReadDBJob: execute: start";

    assert (type_ > DBO_UNDEFINED && type_ < DBO_SENSOR_INFORMATION);

    if (custom_filter_clause_.size() == 0 && order_ )
    	db_interface_->prepareRead (type_, read_list_);
    else
    	db_interface_->prepareRead (type_, read_list_, custom_filter_clause_, order_);

    while (!db_interface_->getReadingDone(type_) && !obsolete_)
    {
        boost::this_thread::sleep( boost::posix_time::milliseconds(10) );

        // AVIBIT HACK
        //Buffer *buffer = db_interface_->readDataChunk(type_, order_->getName() == "id");
        Buffer *buffer = db_interface_->readDataChunk(type_, true);

        assert (buffer->getDBOType() != DBO_UNDEFINED && buffer->getDBOType() == type_);

        if (!buffer)
        {
            logwrn << "DBOReadDBJob: execute: got null buffer";
            db_interface_->finalizeReadStatement(type_);
            done_=true;
            return;
        }

        if (obsolete_)
        {
            delete buffer;
        }
        else
        {
            intermediate_signal_(this, buffer);
        }
    }

    db_interface_->finalizeReadStatement(type_);
    done_=true;

    logdbg << "DBOReadDBJob: execute: done";
    return;
}
